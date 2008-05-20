#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glib.h>
#include <gmodule.h>

#include <ohm-plugin.h>

#define INVALID_ID   -1
#define BUFFER_CHUNK 128


#define ALLOC(type) ({                            \
            type   *__ptr;                        \
            size_t  __size = sizeof(type);        \
                                                  \
            if ((__ptr = malloc(__size)) != NULL) \
                memset(__ptr, 0, __size);         \
            __ptr; })

#define ALLOC_OBJ(ptr) ((ptr) = ALLOC(typeof(*ptr)))

#define ALLOC_ARR(type, n) ({                     \
            type   *__ptr;                        \
            size_t   __size = (n) * sizeof(type); \
                                                  \
            if ((__ptr = malloc(__size)) != NULL) \
                memset(__ptr, 0, __size);         \
            __ptr; })

#define REALLOC_ARR(ptr, o, n) ({                                       \
            typeof(ptr) __ptr;                                          \
            size_t      __size = sizeof(*ptr) * (n);                    \
                                                                        \
            if ((ptr) == NULL) {                                        \
                (__ptr) = ALLOC_ARR(typeof(*ptr), n);                   \
                ptr = __ptr;                                            \
            }                                                           \
            else if ((__ptr = realloc(ptr, __size)) != NULL) {          \
                if ((n) > (o))                                          \
                    memset(__ptr + (o), 0, ((n)-(o)) * sizeof(*ptr));   \
                ptr = __ptr;                                            \
            }                                                           \
            __ptr; })
                
#define FREE(obj) do { if (obj) free(obj); } while (0)

#define STRDUP(s) ({                                    \
            char *__s = s;                              \
            __s = ((s) ? strdup(s) : strdup(""));       \
            __s; })

#define DEBUG(fmt, args...) do {                                        \
        if (depth > 0)                                                  \
            printf("%*.*s ", depth*2, depth*2, "                  ");   \
        printf("[%s] "fmt"\n", __FUNCTION__, ## args);                  \
    } while (0)



enum {
    CONSOLE_NONE     = 0x0,
    CONSOLE_MULTIPLE = 0x1,
};

typedef struct console_s console_t;


struct console_s {
    console_t *parent;                   /* where we got accept(2)ed */
    int        nchild;                   /* number of children */
    int        flags;                    /* misc. flags */

    char      *endpoint;                 /* address:port to listen(2) on */
    int        sock;                     /* socket */
    char      *buf;                      /* input buffer */
    size_t     size;                     /* buffer size */
    size_t     used;                     /* buffer used */
    
    void   (*callback)(int, char *, void *); /* input callback */
    void    *data;                           /* opaque callback data */

    GIOChannel *gio;                     /* associated channel */
    guint       gid;                     /* glib source id */
};


static console_t **consoles;
static int         nconsole;


static gboolean console_accept (GIOChannel *, GIOCondition, gpointer);
static gboolean console_handler(GIOChannel *, GIOCondition, gpointer);


/*****************************************************************************
 *                       *** initialization & cleanup ***                    *
 *****************************************************************************/

/**
 * plugin_init:
 **/
static void
plugin_init(OhmPlugin *plugin)
{
    return;
}


/**
 * plugin_exit:
 **/
static void
plugin_exit(OhmPlugin *plugin)
{
    return;
}


/********************
 * new_console
 ********************/
static console_t *
new_console(void)
{
    int i;

    for (i = 0; i < nconsole; i++)
        if (consoles[i] == NULL)
            return ALLOC_OBJ(consoles[i]);
        else if(consoles[i]->sock < 0)
            return consoles[i];
    
    if (REALLOC_ARR(consoles, nconsole, nconsole + 1) == NULL)
        return NULL;
    nconsole++;

    return ALLOC_OBJ(consoles[i]);
}


/********************
 * del_console
 ********************/
static void
del_console(console_t *c)
{
    int i;
    
    FREE(c->endpoint);
    close(c->sock);
    
    c->endpoint = NULL;
    c->sock     = -1;
    c->used     = 0;
    memset(c->buf, 0, c->size);

    if (c->nchild > 0) {
        for (i = 0; i < nconsole; i++) {
            if (consoles[i] && consoles[i]->parent == c) {
                consoles[i]->parent = NULL;
                c->nchild--;
            }
        }
    }
}


/********************
 * lookup_console
 ********************/
static console_t *
lookup_console(int id)
{
    int i;

    for (i = 0; i < nconsole; i++)
        if (consoles[i] != NULL && consoles[i]->gid == (guint)id)
            return consoles[i];
    
    return NULL;
}


/*****************************************************************************
 *                           *** exported methods ***                        *
 *****************************************************************************/

/********************
 * console_open
 ********************/
OHM_EXPORTABLE(int, console_open, (char *address,
                                   void (*cb)(int, char *, void *),
                                   void *cb_data, int multiple))
{
    console_t          *c = NULL;
    struct sockaddr_in  sin;
    char                addr[64], *portp, *end;
    int                 len, reuse;
    GIOCondition        events;

    if ((portp = strchr(address, ':')) == NULL)
        return -1;
    
    len = (int)portp - (int)address;
    strncpy(addr, address, len);
    addr[len] = '\0';
    portp++;

    sin.sin_family = AF_INET;
    if (!inet_aton(addr, &sin.sin_addr))
        return -1;
    
    sin.sin_port = strtoul(portp, &end, 10);
    sin.sin_port = htons(sin.sin_port);
    if (*end != '\0')
        return -1;
    
    if ((c = new_console()) == NULL)
        return -1;

    c->sock     = socket(sin.sin_family, SOCK_STREAM, 0);
    c->endpoint = STRDUP(address);
    c->callback = cb;
    c->data     = cb_data;
    c->flags    = multiple ? CONSOLE_MULTIPLE : CONSOLE_NONE;
    

    reuse = 1;
    setsockopt(c->sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    if (c->endpoint == NULL || c->sock < 0)
        goto fail;
    
    if (bind(c->sock, (struct sockaddr *)&sin, sizeof(sin)) != 0 ||
        listen(c->sock, 2) != 0)
        goto fail;
    
    if ((c->gio = g_io_channel_unix_new(c->sock)) == NULL)
        goto fail;
    
    events = G_IO_IN | G_IO_HUP;
    return (c->gid = g_io_add_watch(c->gio, events, console_accept, c));
    
 fail:
    if (c != NULL) {
        FREE(c->endpoint);
        if (c->sock >= 0)
            close(c->sock);
        c->endpoint = NULL;
        c->sock     = -1;
    }
    return -1;
}


/********************
 * console_close
 ********************/
OHM_EXPORTABLE(int, console_close, (int id))
{
    console_t *c = lookup_console(id);

    if (c == NULL)
        return EINVAL;
    
    g_source_remove(c->gid);
    g_io_channel_unref(c->gio);
    
    del_console(c);
    
    return 0;
}


/********************
 * console_write
 ********************/
OHM_EXPORTABLE(int, console_write, (int id, char *buf, size_t size))
{
    console_t *c = lookup_console(id);

    if (c == NULL)
        return EINVAL;

    if (size == 0)
        size = strlen(buf);

    return write(c->sock, buf, size);
}


/*****************************************************************************
 *                       *** misc. helper functions ***                      *
 *****************************************************************************/

/********************
 * console_accept
 ********************/
static gboolean
console_accept(GIOChannel *source, GIOCondition condition, gpointer data)
{
    console_t          *lc = (console_t *)data;
    console_t          *c;
    struct sockaddr_in  addr;
    socklen_t           addrlen = sizeof(addr);
    int                 sock;


    if (condition != G_IO_IN)
        return TRUE;
    
    if ((sock = accept(lc->sock, (struct sockaddr *)&addr, &addrlen)) < 0)
        return TRUE;
    
    if (lc->nchild > 1 && !(lc->flags & CONSOLE_MULTIPLE)) {
        #define BUSY_MESSAGE "Console is currently busy.\n"
        write(sock, BUSY_MESSAGE, sizeof(BUSY_MESSAGE) - 1);
        close(sock);
        return TRUE;
    }
    
    if ((c = new_console()) == NULL)
        goto fail;

    c->endpoint = STRDUP(lc->endpoint);
    c->size     = BUFFER_CHUNK;
    c->used     = 0;
    c->buf      = ALLOC_ARR(char, c->size);
    c->sock     = sock;
    c->callback = lc->callback;
    c->data     = lc->data;

    if (c->endpoint == NULL || c->buf == NULL)
        goto fail;

    if ((c->gio = g_io_channel_unix_new(c->sock)) == NULL)
        goto fail;
    
    c->parent = lc;
    lc->nchild++;
    
    c->gid = g_io_add_watch(c->gio, G_IO_IN | G_IO_HUP, console_handler, c);
    return TRUE;

 fail:
    if (sock >= 0)
        close(sock);
    if (c)
        FREE(c->endpoint);
    c->endpoint = NULL;
    c->sock    = -1;

    return TRUE;
}


/********************
 * console_read
 ********************/
static int
console_read(console_t *c)
{
    int n, left = c->size - c->used - 1;
    
    if (left < BUFFER_CHUNK) {
        if (REALLOC_ARR(c->buf, c->size, c->size + BUFFER_CHUNK) == NULL)
            return -ENOMEM;
        c->size += BUFFER_CHUNK;
    }

    if ((n = read(c->sock, c->buf + c->used, left)) < 0)
        return -errno;
    
    c->used += n;

    return c->used;
}


/********************
 * console_handler
 ********************/
static gboolean
console_handler(GIOChannel *source, GIOCondition condition, gpointer data)
{
    console_t *c = (console_t *)data;
    int        i, left = c->size - c->used - 1;
    
    if (condition & G_IO_IN) {
        if (console_read(c) > 0) {
            for (i = 0; i < c->used; i++)
                if (c->buf[i] == '\r') {
                    c->buf[i] = '\0';
                    if (i < c->used && c->buf[i+1] == '\n') {
                        i++;
                        c->buf[++i] = '\0';
                    }
#if 0
                    printf("##### console input \"%s\" #####\n", c->buf);
#endif
                    c->callback(c->gid, c->buf, c->data);
                    if ((left = c->used - i - 1) > 0) {
                        memmove(c->buf, c->buf + i + 1, left);
                        c->used         -= i + 1;
                        c->buf[c->used]  = '\0';
                        i                = 0;
                    }
                    else
                        c->used = 0;
                }
        }
    }
    
    if (condition & G_IO_HUP) {
        printf("##### closing console %d #####\n", c->gid);
        c->callback(c->gid, "", c->data);
        console_close(c->gid);
        return FALSE;
    }

    return TRUE;
}


OHM_PLUGIN_DESCRIPTION("console",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_PROVIDES_METHODS(console, 3,
    OHM_EXPORT(console_open , "open" ),
    OHM_EXPORT(console_close, "close"),
    OHM_EXPORT(console_write, "write")
);


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

