#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>               /* SYS_gettid */

#include <glib.h>
#include <gmodule.h>
#include <dbus/dbus.h>

#include <policy/policy.h>
#include <policy/set.h>
#include <policy/relation.h>

#include <ohm/ohm-plugin.h>

#define DBUS_INTERFACE_POLICY "com.nokia.policy"
#define DBUS_PATH_POLICY      "/com/nokia/policy"

#define POLICY_ACTIONS  "actions"

#define PEP_PULSE "/com/nokia/policy/enforce/pulseaudio"
#define PEP_ARTD  "/com/nokia/policy/enforce/artd"
#define ID_NONE  0x0
#define ID_PULSE 0x1
#define ID_ARTD  0x2
#define ID_ALL   0x3

#define ACCESSORIES     "accessories"
#define ACTIVE_CHANNELS "active_channels"
#define AUDIO_ROUTE     "audio_route"
#define PRIVACY         "privacy"
#define CURRENT_PROFILE "current_profile"


#define TRANSACTION_TTL 5                  /* in seconds */

typedef struct {
    char           ***actions;
    DBusMessage      *message;
    uint32_t          txid;
    uint32_t          unacked;
    struct timeval    stamp;
} transaction_t;


static DBusHandlerResult plugin_ack(DBusConnection *c,
                                    DBusMessage *msg, void *data);
#if 0
static void plugin_prolog_prompt(void);
#endif

static int transaction_start(DBusMessage *msg, char ***actions, uint32_t txid);
static int transaction_end(transaction_t *transaction, int status,
                           int unhash);

static transaction_t *transaction_lookup(uint32_t txid);
static gboolean transaction_gc_cb(gpointer key, gpointer value, gpointer limit);
static gboolean transaction_gc(gpointer);
static int transaction_peers(transaction_t *transaction);

static GHashTable  *transactions;
static GList       *subscribed;


/**
 * plugin_init:
 **/
static void
plugin_init(OhmPlugin *plugin)
{
#ifndef DEFAULT_PROLOG_DIR
#  define DEFAULT_PROLOG_DIR \
    "/u/src/work/policy/checkout/policy/testing/proto/prolog"
#endif
    
    
    /*
     * Notes: policy-related paths are hardcoded for the time being
     */
    
    policy_config_t  cfg;
    char            *topdir;
    char             ruleset[PATH_MAX], setext[PATH_MAX], relext[PATH_MAX];
    char            *extensions[] = { setext, relext, NULL };

    DBusConnection *conn = ohm_plugin_dbus_get_connection();
    DBusError       error;
    char           *interface;
    int             flags;
    int             status;
    
    
    dbus_error_init(&error);

    interface = DBUS_INTERFACE_POLICY;
    flags     = DBUS_NAME_FLAG_REPLACE_EXISTING;
    status    = dbus_bus_request_name(conn, interface, flags, &error);
    
    if (status != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        g_error("Failed to register %s (%s)", interface, error.message);
    }

    if ((topdir = getenv("PROLOG_DIR")) == NULL)
        topdir = DEFAULT_PROLOG_DIR;

#define FULLPATH(var, relpath) do {                      \
        size_t __size = sizeof(var) - 1;                 \
        snprintf(var, __size, "%s/%s", topdir, relpath); \
        var[__size] = '\0';                              \
    } while (0)
    
    FULLPATH(ruleset, "rulesets/policy");
    FULLPATH(setext , "extensions/set");
    FULLPATH(relext , "extensions/relation");
    
    memset(&cfg, 0, sizeof(cfg));
    cfg.argv0      = "ohm-policy";
    cfg.ruleset    = ruleset;
    cfg.extensions = extensions;

    cfg.local_stack    = 16;
    cfg.global_stack   = 16;
    cfg.tail_stack     = 16;
    cfg.argument_stack = 16;

    if (policy_init(&cfg) != 0) {
        g_error("Failed to initialize policy library.");
    }
    else {
        set_create(ACCESSORIES    , NULL);
        set_create(ACTIVE_CHANNELS, NULL);
        set_create(CURRENT_PROFILE, NULL);
        set_create(PRIVACY        , NULL);

        relation_create(AUDIO_ROUTE, 2, NULL);

        transactions = g_hash_table_new(g_int_hash, g_int_equal);
        if (transactions == NULL) {
            g_error("Failed to create transaction hash table.");
        }
            
        g_timeout_add(TRANSACTION_TTL * 1000, transaction_gc, NULL);
    }

    dbus_error_free(&error);
}


/**
 * plugin_exit:
 **/
static void
plugin_exit(OhmPlugin *plugin)
{
    DBusConnection *c = ohm_plugin_dbus_get_connection();
    DBusError       error;
    

    if (c) {
        dbus_error_init(&error);
        dbus_bus_release_name(c, DBUS_INTERFACE_POLICY, &error);
        dbus_error_free(&error);

        policy_exit();
    }
    
    if (transactions)
        g_hash_table_destroy(transactions);
}



/**
 * plugin_request:
 **/
OHM_EXPORTABLE(int, plugin_request, (char *entity, char *request))
{
    return policy_request(entity, request);
}


static void
action_notify(gpointer cbptr, gpointer data)
{
    void  (*notify)(char ***) = cbptr;
    char ***actions           = data;

    notify(actions);
}


/**
 * plugin_actions:
 **/
OHM_EXPORTABLE(int, plugin_actions, (DBusMessage *msg, int full))
{
    static uint32_t next = 1;
    dbus_uint32_t   txid = next++;
    
    char       ***actions   = policy_update(full);
    char         *path      = DBUS_PATH_POLICY"/decision";
    char         *interface = DBUS_INTERFACE_POLICY;

    DBusConnection  *c      = ohm_plugin_dbus_get_connection();
    DBusMessage     *sig;
    DBusMessageIter  msgit, arrit, actit;
    int              a, p;

    printf("emitting policy actions:\n");
    policy_dump_actions(actions);

#if 0
    plugin_prolog_prompt();
#endif
    
    g_list_foreach(subscribed, action_notify, actions);

    if ((sig = dbus_message_new_signal(path, interface, "actions")) == NULL)
        goto fail;
        
    dbus_message_iter_init_append(sig, &msgit);
    
    if (!dbus_message_iter_append_basic(&msgit, DBUS_TYPE_UINT32, &txid))
        goto fail;

    if (!dbus_message_iter_open_container(&msgit, DBUS_TYPE_ARRAY,
                                          "as", &arrit))
        goto fail;
    if (actions) {
        for (a = 0; actions[a]; a++) {
            if (!dbus_message_iter_open_container(&arrit, DBUS_TYPE_ARRAY,
                                                  "s", &actit))
                goto fail;
            for (p = 0; actions[a][p]; p++)
                if (!dbus_message_iter_append_basic(&actit, DBUS_TYPE_STRING,
                                                    actions[a] + p))
                    goto fail;
            dbus_message_iter_close_container(&arrit, &actit);
        }
    }
    
    dbus_message_iter_close_container(&msgit, &arrit);
    if (!dbus_connection_send(c, sig, NULL))
        goto fail;
    
    transaction_start(msg?dbus_message_ref(msg):NULL, actions, (uint32_t)txid);
    
    return TRUE;
    
 fail:
    policy_free_actions(actions);
    dbus_message_unref(sig);
    return FALSE;
}


/**
 * plugin_ack:
 **/
static DBusHandlerResult
plugin_ack(DBusConnection *c, DBusMessage *msg, void *data)
{
    const char *interface = dbus_message_get_interface(msg);
    const char *member    = dbus_message_get_member(msg);
    const char *path      = dbus_message_get_path(msg);
    uint32_t    who       = ID_NONE;

    DBusError      error;
    dbus_uint32_t  txid, status;
    transaction_t *transaction;

#if 0
    printf("got signal %s.%s, path %s\n", interface ?: "NULL", member,
           path ?: "NULL");
#endif

    if (member == NULL || strcmp(member, "status"))
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (interface == NULL || strcmp(interface, DBUS_INTERFACE_POLICY))
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    
    if (path == NULL)
        return DBUS_HANDLER_RESULT_HANDLED;
    
    dbus_error_init(&error);
    if (!dbus_message_get_args(msg, &error,
                               DBUS_TYPE_UINT32, &txid,
                               DBUS_TYPE_UINT32, &status,
                               DBUS_TYPE_INVALID)) {
        g_warning("Failed to parse policy status signal (%s)", error.message);
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    dbus_error_free(&error);
    
    if (!strcmp(path, PEP_PULSE))
        who = ID_PULSE;
    else if (!strcmp(path, PEP_ARTD))
        who = ID_ARTD;
    else {
        printf("transaction ACK/NAK from unknown peer %s, ignored...\n", path);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    if ((transaction = transaction_lookup(txid)) == NULL) {
        printf("(%u) transaction 0x%x unknown, ignored...\n",
               (unsigned int)syscall(SYS_gettid), txid);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    printf("transaction 0x%x %sed by peer 0x%x\n", txid,
           status ? "ACK" : "NAK", who);
    
    if (status == FALSE)
        transaction_end(transaction, FALSE, TRUE);
    
    transaction->unacked &= ~who;

    if (!transaction->unacked)
        transaction_end(transaction, TRUE, TRUE);
    else
        printf("transaction 0x%x ACK mask: 0x%x\n", txid, transaction->unacked);
    
    return DBUS_HANDLER_RESULT_HANDLED;
}


/**
 * plugin_subscribe:
 **/
OHM_EXPORTABLE(int, plugin_subscribe, (void (*callback)(char ***)))
{
    subscribed = g_list_append(subscribed, (gpointer)callback);
    return TRUE;
}


/**
 * plugin_prolog_prompt:
 **/
OHM_EXPORTABLE(void, plugin_prolog_prompt, (void))
{
    prolog_prompt();
}

/********************
 * transaction_start
 ********************/
static int
transaction_start(DBusMessage *message, char ***actions, uint32_t txid)
{
    transaction_t *transaction;

    if ((transaction = g_new0(transaction_t, 1)) == NULL)
        return FALSE;

    transaction->actions = actions;
    transaction->message = message;
    transaction->txid    = txid;

    /* short-circuit if there is nothing to be done/collect */
    if (transaction->actions == NULL || transaction->actions[0] == NULL)
        return transaction_end(transaction, TRUE, FALSE);
    
    if (transaction_peers(transaction)) {
        gettimeofday(&transaction->stamp, NULL);
        g_hash_table_insert(transactions, &transaction->txid, transaction);
    }
    else {
        g_free(transaction);
        return FALSE;
    }
    
    return TRUE;
}


/********************
 * transaction_end
 ********************/
static int
transaction_end(transaction_t *transaction, int status, int unhash)
{
    DBusConnection *c       = ohm_plugin_dbus_get_connection();
    DBusMessage    *request = transaction->message;
    DBusMessage    *reply   = NULL;
    dbus_uint32_t   serial;
    int             success = FALSE;
    
    if (request != NULL) {
        if ((reply = dbus_message_new_method_return(request)) == NULL ||
            !dbus_message_append_args(reply,
                                      DBUS_TYPE_BOOLEAN, &status,
                                      DBUS_TYPE_INVALID)) {
            dbus_message_unref(reply);
            goto out;
        }
        
        serial  = dbus_message_get_serial(request);
        success = dbus_connection_send(c, reply, &serial);
    }
    else
        success = TRUE;
    
 out:
    if (request)
        dbus_message_unref(request);
    if (unhash)
        g_hash_table_remove(transactions, &transaction->txid);

    printf("transaction 0x%x removed\n", transaction->txid);

    g_free(transaction);

    return success;
}


/********************
 * transaction_lookup
 ********************/
static transaction_t *
transaction_lookup(uint32_t txid)
{
    return (transaction_t *)g_hash_table_lookup(transactions, &txid);
}


/********************
 * transaction_gc_cb
 ********************/
static gboolean
transaction_gc_cb(gpointer key, gpointer value, gpointer limitp)
{
    transaction_t  *transaction = (transaction_t *)value;
    struct timeval *limit       = (struct timeval *)limitp;
    struct timeval *stamp       = &transaction->stamp;

    if (stamp->tv_sec < limit->tv_sec ||
        (stamp->tv_sec == limit->tv_sec && stamp->tv_usec < limit->tv_usec)) {
        printf("transaction 0x%x expired\n", transaction->txid);
        transaction_end(transaction, FALSE, FALSE);
        return TRUE;
    }
    
    return FALSE;
}


/********************
 * transaction_gc
 ********************/
static gboolean
transaction_gc(gpointer data)
{
    struct timeval limit;

    gettimeofday(&limit, NULL);
    if (limit.tv_sec >= TRANSACTION_TTL)
        limit.tv_sec -= TRANSACTION_TTL;
    else
        limit.tv_sec  = 0;

    g_hash_table_foreach_remove(transactions, transaction_gc_cb, &limit);

    return TRUE;
}


/********************
 * transaction_peers
 ********************/
static int
transaction_peers(transaction_t *transaction)
{
    char ***actions = transaction->actions;
    char   *what;
    int     a, who, seen;
    

    /*
     * Notes:
     *
     *   Eventually we'll have enforcement points register to us, the decision
     *   point. Among other things they'll us during registration what policy
     *   commands they handle. We'll collect this and create a table of
     *   commands vs. enforcement points and use this information to determine
     *   our peers for any transaction.
     *
     *   For now this is hardcoded.
     */
    
    /* should not be called without actions */
    if (transaction->actions == NULL || transaction->actions[0] == NULL)
        return FALSE;
    
    seen = ID_NONE;
    for (a = 0; actions[a]; a++) {
        what = actions[a][0];
        who  = ID_NONE;
        switch (what[0]) {
        case 'a':
            if (!strcmp(what+1, "udio_route"))
                who = ID_PULSE|ID_ARTD;
            /*  who = ID_ARTD;  test kludge */
            break;
        case 'v':
            if (!strcmp(what+1, "olume_limit"))
                who = ID_PULSE;
            /*  who = ID_ARTD; * test kludge */
            break;
        }
        
        if (who == ID_NONE)
            return FALSE;
        
        who &= ~seen;
        if (who) {
            printf("transaction peer mask for command %s: 0x%x...\n", what,who);
            transaction->unacked |= who;
        }
        
        seen |= who;
        
        if (transaction->unacked == ID_ALL)
            break;
    }
    
    printf("transacaction peer mask for 0x%x: 0x%x\n", transaction->txid,
           transaction->unacked);
    
    return TRUE;
}



/**
 * plugin_set_insert:
 **/
OHM_EXPORTABLE(int, plugin_set_insert, (char *name, char *item))
{
    set_t *set;
    
    if ((set = set_lookup(name)) == NULL)
        return ENOENT;
    else
        return set_insert(set, item);
}


/**
 * plugin_set_delete:
 **/
OHM_EXPORTABLE(int, plugin_set_delete, (char *name, char *item))
{
    set_t *set;
    
    if ((set = set_lookup(name)) == NULL)
        return ENOENT;
    else
        return set_delete(set, item);
}

/**
 * plugin_set_reset:
 **/
OHM_EXPORTABLE(void, plugin_set_reset, (char *name))
{
    set_t *set;
    
    if ((set = set_lookup(name)) == NULL)
        return;
    
    set_reset(set);
}



/**
 * plugin_relation_insert:
 **/
OHM_EXPORTABLE(int, plugin_relation_insert, (char *name, char **items))
{
    relation_t *r;
    
    if ((r = relation_lookup(name)) == NULL)
        return ENOENT;
    else
        return relation_insert(r, items);
}


/**
 * plugin_relation_delete:
 **/
OHM_EXPORTABLE(int, plugin_relation_delete, (char *name, char **items))
{
    relation_t *r;
    
    if ((r = relation_lookup(name)) == NULL)
        return ENOENT;
    else
        return relation_delete(r, items);
}

/**
 * plugin_relation_reset:
 **/
OHM_EXPORTABLE(void, plugin_relation_reset, (char *name))
{
    relation_t *r;
    
    if ((r = relation_lookup(name)) == NULL)
        return;
    
    relation_reset(r);
}



OHM_PLUGIN_DESCRIPTION("policy",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_PROVIDES_METHODS(policy, 10,
    OHM_EXPORT(plugin_request,   "request"),
    OHM_EXPORT(plugin_actions,   "actions"),
    OHM_EXPORT(plugin_subscribe, "subscribe"),

    OHM_EXPORT(plugin_set_insert, "set_insert"),
    OHM_EXPORT(plugin_set_delete, "set_delete"),
    OHM_EXPORT(plugin_set_reset , "set_reset"),

    OHM_EXPORT(plugin_relation_insert, "relation_insert"),
    OHM_EXPORT(plugin_relation_delete, "relation_delete"),
    OHM_EXPORT(plugin_relation_reset , "relation_reset"),

    OHM_EXPORT(plugin_prolog_prompt, "prolog_prompt"));


OHM_PLUGIN_DBUS_SIGNALS(
    { NULL, DBUS_INTERFACE_POLICY, "status", NULL, plugin_ack, NULL });
                            

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

