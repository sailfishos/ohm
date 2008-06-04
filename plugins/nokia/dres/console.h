#ifndef __OHM_RESOLVER_CONSOLE_H__
#define __OHM_RESOLVER_CONSOLE_H__

#define CONSOLE_PROMPT "ohm-dres> "


typedef struct {
    char  *name;                      /* command name */
    char  *args;                      /* description of arguments */
    char  *description;               /* description of command */
    void (*handler)(int, char *);     /* command handler */
} command_t;


static int  console_init(char *address);
static void console_exit(void);


/* console interface */
OHM_IMPORTABLE(int, console_open  , (char  *address,
                                     void (*opened)(int, struct sockaddr *,int),
                                     void (*closed)(int),
                                     void (*input)(int, char *, void *),
                                     void  *data, int multiple));
OHM_IMPORTABLE(int, console_close , (int id));
OHM_IMPORTABLE(int, console_write , (int id, char *buf, size_t size));
OHM_IMPORTABLE(int, console_printf, (int id, char *fmt, ...));
OHM_IMPORTABLE(int, console_grab  , (int id, int fd));
OHM_IMPORTABLE(int, console_ungrab, (int id, int fd));

/* console event handlers */
static void console_opened (int id, struct sockaddr *peer, int peerlen);
static void console_closed (int id);
static void console_input  (int id, char *buf, void *data);



#endif /* __OHM_RESOLVER_CONSOLE_H__ */




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
