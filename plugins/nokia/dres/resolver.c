#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <glib.h>
#include <gmodule.h>
#include <dbus/dbus.h>

#include <dres/dres.h>
#include <prolog/prolog.h>
#include <prolog/ohm-fact.h>           /* XXX */

#include <ohm-plugin.h>
#include <ohm-plugin-debug.h>

#include "console.h"
#include "factstore.h"



typedef void (*completion_cb_t)(int transid, int success);




/* rule engine methods */
OHM_IMPORTABLE(int , prolog_setup , (char **extensions, char **files));
OHM_IMPORTABLE(void, prolog_free  , (void *retval));
OHM_IMPORTABLE(void, prolog_dump  , (void *retval));
OHM_IMPORTABLE(void, prolog_shell , (void));

OHM_IMPORTABLE(prolog_predicate_t *, prolog_lookup ,
               (char *name, int arity));
OHM_IMPORTABLE(int                 , prolog_invoke ,
               (prolog_predicate_t *pred, void *retval, ...));
OHM_IMPORTABLE(int                 , prolog_vinvoke,
               (prolog_predicate_t *pred, void *retval, va_list ap));
OHM_IMPORTABLE(int                 , prolog_ainvoke,
               (prolog_predicate_t *pred, void *retval,void **args, int narg));

#if 1
OHM_IMPORTABLE(int, signal_changed,(char *signal, int transid,
                                    int factc, char **factv,
                                    completion_cb_t callback,
                                    unsigned long timeout));
#else
int signal_changed(char *, int, int, char**, completion_cb_t, unsigned long);
#endif



OHM_IMPORTABLE(void, completion_cb, (int transid, int success));

static int  prolog_handler (dres_t *dres,
                            char *name, dres_action_t *action, void **ret);
static int  signal_handler (dres_t *dres,
                            char *name, dres_action_t *action, void **ret);
static void dump_signal_changed_args(char *signame, int transid, int factc,
                                     char**factv, completion_cb_t callback,
                                     unsigned long timeout);
static int  retval_to_facts(char ***objects, OhmFact **facts, int max);



static dres_t *dres;

static int DBG_RESOLVE, DBG_PROLOG, DBG_SIGNAL, DBG_FACTS;

OHM_DEBUG_PLUGIN(resolver,
    OHM_DEBUG_FLAG("resolver", "dependency resolving", &DBG_RESOLVE),
    OHM_DEBUG_FLAG("prolog"  , "prolog handler"      , &DBG_PROLOG),
    OHM_DEBUG_FLAG("signal"  , "decision emission"   , &DBG_SIGNAL),
    OHM_DEBUG_FLAG("facts"   , "fact handling"       , &DBG_FACTS));


/*****************************************************************************
 *                       *** initialization & cleanup ***                    *
 *****************************************************************************/

/**
 * plugin_init:
 **/
static void
plugin_init(OhmPlugin *plugin)
{
#define DRES_RULE_PATH "/tmp/policy/policy.dres"
#define PROLOG_SYSDIR  "/usr/share/prolog/"
#define PROLOG_RULEDIR "/tmp/policy/prolog/"

#define FAIL(fmt, args...) do {                                   \
        g_warning("DRES plugin, %s: "fmt, __FUNCTION__, ## args); \
        goto fail;                                                \
    } while (0)

    char *extensions[] = {
        PROLOG_SYSDIR"extensions/fact",
        NULL
    };

    char *rules[] = {
        PROLOG_RULEDIR"hwconfig",
        PROLOG_RULEDIR"devconfig",
        PROLOG_RULEDIR"interface",
        PROLOG_RULEDIR"profile",
        PROLOG_RULEDIR"audio",
        NULL
    };

    if (!OHM_DEBUG_INIT(resolver))
        FAIL("failed to initialize debugging");

    if ((dres = dres_init(NULL)) == NULL)
        FAIL("failed to initialize DRES library");
    
    if (dres_register_handler(dres, "prolog", prolog_handler) != 0)
        FAIL("failed to register RESOLVE prolog handler");

    if (dres_register_handler(dres, "signal_changed", signal_handler) != 0)
        FAIL("failed to register RESOLVE signal_changed handler");

    if (dres_parse_file(dres, DRES_RULE_PATH))
        FAIL("failed to parse RESOLVE rule file \"%s\"", DRES_RULE_PATH);

    dres_dump_targets(dres);

    if (prolog_setup(extensions, rules) != 0)
        FAIL("failed to load extensions and rules to prolog interpreter");

    if (console_init("127.0.0.1:3000"))
        g_warning("resolver plugin: failed to open console");

    if (factstore_init())
        FAIL("factstore initialization failed");
    
    OHM_DEBUG(DBG_RESOLVE, "resolver initialized");

    return;

 fail:
    if (dres) {
        dres_exit(dres);
        dres = NULL;
    }
    exit(1);

#undef FAIL
}


/**
 * plugin_exit:
 **/
static void
plugin_exit(OhmPlugin *plugin)
{
    factstore_exit();

    if (dres) {
        dres_exit(dres);
        dres = NULL;
    }

    console_exit();
}



/********************
 * dres_parse_error
 ********************/
void
dres_parse_error(dres_t *dres, int lineno, const char *msg, const char *token)
{
    g_warning("error: %s, on line %d near input %s\n", msg, lineno, token);
    exit(1);
}



/*****************************************************************************
 *                           *** exported methods ***                        *
 *****************************************************************************/

/********************
 * dres/resolve
 ********************/
OHM_EXPORTABLE(int, update_goal, (char *goal, char **locals))
{
    return dres_update_goal(dres, goal, locals);
}


/*****************************************************************************
 *                          *** DRES action handlers ***                     *
 *****************************************************************************/


/********************
 * action_argument
 ********************/
static char *
action_argument(dres_t *dres, int argument)
{
    dres_variable_t *var;
    char             name[64], *value;

    
    dres_name(dres, argument, name, sizeof(name));
        
    switch (name[0]) {
    case '$':
        if ((var = dres_lookup_variable(dres, argument)) == NULL ||
            !dres_var_get_field(var->var, DRES_VAR_FIELD, NULL,
                                VAR_STRING, &value))
            return NULL;
        return value;
        
    case '&':
        if ((value = dres_scope_getvar(dres->scope, name + 1)) == NULL)
            return NULL;
        return value;
        
    case '<':
        return NULL;
        
    default:
        return STRDUP(name);
    }
}

/********************
 * action_arguments
 ********************/
static int
action_arguments(dres_t *dres, dres_action_t *action, char **args, int narg)
{
    int i;
    
    for (i = 1; i < action->nargument; i++)
        if ((args[i-1] = action_argument(dres, action->arguments[i])) == NULL)
            goto fail;

    return i;
    
 fail:
    narg = i;
    for (i = 0; narg - 1; i++)
        if (args[i] != NULL)
            FREE(args[i]);
    
    return -1;
}


/********************
 * prolog_handler
 ********************/
static int
prolog_handler(dres_t *dres, char *actname, dres_action_t *action, void **ret)
{
#define FAIL(ec) do { err = ec; goto fail; } while (0)
#define MAX_FACTS 63
    prolog_predicate_t *predicate;
    char                name[64];
    char             ***retval;
    OhmFact           **facts = NULL;
    int                 nfact, err;
    char               *args[32];
    int                 i, narg;

    OHM_DEBUG(DBG_RESOLVE, "prolog handler entered...");
    
    if (action->nargument < 1)
        return EINVAL;

    narg = 0;

    name[0] = '\0';
    dres_name(dres, action->arguments[0], name, sizeof(name));
    
    if ((predicate = prolog_lookup(name, action->nargument)) == NULL)
        FAIL(ENOENT);

    if ((narg = action_arguments(dres, action, args, narg)) < 0)
        FAIL(EINVAL);

    if (!prolog_ainvoke(predicate, &retval, (void **)args, narg))
        FAIL(EINVAL);

    OHM_DEBUG(DBG_RESOLVE, "rule engine gave the following results:");
    prolog_dump(retval);

    if ((facts = ALLOC_ARR(OhmFact *, MAX_FACTS + 1)) == NULL)
        FAIL(ENOMEM);

    if ((nfact = retval_to_facts(retval, facts, MAX_FACTS)) < 0)
        FAIL(EINVAL);
    
    facts[nfact] = NULL;
    
    if (ret == NULL)
        FAIL(0);                     /* kludge: free facts and return 0 */
    
    *ret = facts;
    return 0;
    
 fail:
    if (facts) {
        for (i = 0; i < nfact; i++)
            g_object_unref(facts[i]);
        FREE(facts);
    }
    for (i = 0; i < narg; i++)
        FREE(args[i]);
    
    return err;

#undef MAX_FACTS
}


/********************
 * signal_handler
 ********************/
static int
signal_handler(dres_t *dres, char *name, dres_action_t *action, void **ret)
{
#define MAX_FACTS 128
#define MAX_LENGTH 64
    static int        transid = 1;

    dres_variable_t  *var;
    char             *signature;
    unsigned long     timeout;
    int               factc;
    char              signal_name[MAX_LENGTH];
    char             *cb_name;
    char              prefix[MAX_LENGTH];
    char              arg[MAX_LENGTH];
    char             *factv[MAX_FACTS + 1];
    char              buf[MAX_FACTS * MAX_LENGTH];
    char              namebuf[MAX_LENGTH];
    char             *p;
    int               i, j;
    int               offs;
    int               success;
    
    if (ret != NULL)
        *ret = NULL;

    factc = action->nargument - 2;

    if (factc < 1 || factc > MAX_FACTS)
        return EINVAL;
    
    signal_name[0] = '\0';
    dres_name(dres, action->arguments[0], signal_name, MAX_LENGTH);

    namebuf[0] = '\0';
    dres_name(dres, action->arguments[1], namebuf, MAX_LENGTH);

    prefix[MAX_LENGTH-1] = '\0';
    strncpy(prefix, dres_get_prefix(dres), MAX_LENGTH-1);

    if ((j = strlen(prefix)) > 0 && j < MAX_LENGTH-2 && prefix[j-1] != '.')
        strcpy(prefix+j, ".");

    switch (namebuf[0]) {

    case '&':
        cb_name = dres_scope_getvar(dres->scope, namebuf+1);
        break;

    case '$':
        cb_name = "";
        if ((var = dres_lookup_variable(dres, action->arguments[1]))) {
            dres_var_get_field(var->var, "value",NULL, VAR_STRING, &cb_name);
        }
        break;

    default:
        cb_name = namebuf;
        break;
    }

    if (cb_name == NULL)
        cb_name = "";
    OHM_DEBUG(DBG_SIGNAL, "%s(): cb_name='%s'\n", __FUNCTION__, cb_name);

    
    timeout = 5 * 1000;

    for (p = buf, i = 0;   i < factc;   i++) {

        dres_name(dres, action->arguments[i+2], arg, MAX_LENGTH);
            
        switch (arg[0]) {

        case '$':
            offs = 1;
            goto copy_string_arg;

        case '&':
            factv[i] = "";

            if ((var = dres_lookup_variable(dres, action->arguments[i+2]))) {
                dres_var_get_field(var->var, "value", NULL,
                                   VAR_STRING, &factv[i]);
            }
            break;

        default:
            offs = 0;
            /* intentional fall trough */

        copy_string_arg:
            factv[i] = p;
            p += snprintf(p, MAX_LENGTH, "%s%s",
                          strchr(arg+offs, '.') ? "" : prefix, arg+offs) + 1;
            break;
        }
    }
    factv[factc] = NULL;

    if (cb_name[0] == '\0') {
        dump_signal_changed_args(signal_name, 0, factc,factv, NULL, timeout);
        success = signal_changed(signal_name, 0, factc,factv, NULL, timeout);
    }
    else {
        signature = (char *)completion_cb_SIGNATURE;

        if (ohm_module_find_method(cb_name,&signature,(void *)&completion_cb)){
            dump_signal_changed_args(signal_name, transid, factc,factv,
                                     completion_cb, timeout);
            success = signal_changed(signal_name, transid++, factc,factv,
                                     completion_cb, timeout);
        }
        else {
            OHM_DEBUG(DBG_SIGNAL, "could not resolve signal.\n");
            success = FALSE;
        }
    }

    return success ? 0 : EINVAL;

#undef MAX_LENGTH
#undef MAX_FACTS
}

static void dump_signal_changed_args(char *signame, int transid, int factc,
                                     char**factv, completion_cb_t callback,
                                     unsigned long timeout)
{
    int i;

    OHM_DEBUG(DBG_SIGNAL, "calling signal_changed(%s, %d,  %d, %p, %p, %lu)",
          signame, transid, factc, factv, callback, timeout);

    for (i = 0;  i < factc;  i++) {
        OHM_DEBUG(DBG_SIGNAL, "   fact[%d]: '%s'", i, factv[i]);
    }
}




/*****************************************************************************
 *                        *** misc. helper routines ***                      *
 *****************************************************************************/


#include "console.c"
#include "factstore.c"
 


OHM_PLUGIN_DESCRIPTION("dres",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_REQUIRES("prolog", "console");

OHM_PLUGIN_PROVIDES_METHODS(dres, 1,
    OHM_EXPORT(update_goal, "resolve")
);

OHM_PLUGIN_REQUIRES_METHODS(dres, 15,
    OHM_IMPORT("prolog.setup"            , prolog_setup),
    OHM_IMPORT("prolog.lookup"           , prolog_lookup),
    OHM_IMPORT("prolog.call"             , prolog_invoke),
    OHM_IMPORT("prolog.vcall"            , prolog_vinvoke),
    OHM_IMPORT("prolog.acall"            , prolog_ainvoke),
    OHM_IMPORT("prolog.free_retval"      , prolog_free),
    OHM_IMPORT("prolog.dump_retval"      , prolog_dump),
    OHM_IMPORT("prolog.shell"            , prolog_shell),
    OHM_IMPORT("console.open"            , console_open),
    OHM_IMPORT("console.close"           , console_close),
    OHM_IMPORT("console.write"           , console_write),
    OHM_IMPORT("console.printf"          , console_printf),
    OHM_IMPORT("console.grab"            , console_grab),
    OHM_IMPORT("console.ungrab"          , console_ungrab),
    OHM_IMPORT("signaling.signal_changed", signal_changed)
);

                            

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
