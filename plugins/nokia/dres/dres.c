#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>

#include <glib.h>
#include <gmodule.h>
#include <dbus/dbus.h>

#include <dres/dres.h>
#include <prolog/prolog.h>
#include <signaling.h>
#include <prolog/ohm-fact.h>           /* XXX */

#include <ohm-plugin.h>


/* imported functions */
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

OHM_IMPORTABLE(Transaction *, signal,
               (char *signal, int factc, char **factv, guint timeout));

OHM_IMPORTABLE(int, console_open , (char *address,
                                    void (*cb)(int, char *, void *),
                                    void *cb_data, int multiple));
OHM_IMPORTABLE(int, console_close , (int id));
OHM_IMPORTABLE(int, console_write , (int id, char *buf, size_t size));
OHM_IMPORTABLE(int, console_printf, (int id, char *fmt, ...));

static int  prolog_handler (dres_t *dres,
                            char *name, dres_action_t *action, void **ret);
static int  signal_handler (dres_t *dres,
                            char *name, dres_action_t *action, void **ret);
static int  retval_to_facts(char *name, char ***objects,
                            OhmFact **facts, int max);
static void console_handler(int id, char *buf, void *data);



static dres_t *dres;
static int     console;
 

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
        PROLOG_SYSDIR"extensions/relation",
        NULL
    };

    char *rules[] = {
        PROLOG_RULEDIR"hwconfig",
        PROLOG_RULEDIR"devconfig",
        PROLOG_RULEDIR"interface",
        PROLOG_RULEDIR"profile",
        PROLOG_RULEDIR"audio",
#if 0
        PROLOG_RULEDIR"test",
#endif
        NULL
    };

    if ((dres = dres_init(NULL)) == NULL)
        FAIL("failed to initialize DRES library");
    
    if (dres_register_handler(dres, "prolog", prolog_handler) != 0)
        FAIL("failed to register DRES prolog handler");

    if (dres_register_handler(dres, "signal_changed", signal_handler) != 0)
        FAIL("failed to register DRES signal_changed handler");

    if (dres_parse_file(dres, DRES_RULE_PATH))
        FAIL("failed to parse DRES rule file \"%s\"", DRES_RULE_PATH);

    if (prolog_setup(extensions, rules) != 0)
        FAIL("failed to load extensions and rules to prolog interpreter");

    console = console_open("127.0.0.1:2000", console_handler, NULL, FALSE);
    if (console < 0)
        g_warning("DRES plugin, %s: failed to open console", __FUNCTION__);
    
    return;

 fail:
    if (dres) {
        dres_exit(dres);
        dres = NULL;
    }
    exit(1);
}


/**
 * plugin_exit:
 **/
static void
plugin_exit(OhmPlugin *plugin)
{
    if (dres) {
        dres_exit(dres);
        dres = NULL;
    }

    if (console > 0)
        console_close(console);
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
 * dres
 ********************/
OHM_EXPORTABLE(int, update_goal, (char *goal, ...))
{
    /* XXX local variable assignments... */
    return dres_update_goal(dres, goal);
}


/*****************************************************************************
 *                          *** DRES action handlers ***                     *
 *****************************************************************************/

/********************
 * prolog_handler
 ********************/
static int
prolog_handler(dres_t *dres, char *name, dres_action_t *action, void **ret)
{
#define MAX_FACTS 128
    struct {
        dres_array_t    head;
        OhmFact        *facts[MAX_FACTS];
    } arrbuf = { head: { len: 0 } };
    dres_array_t       *facts = &arrbuf.head;
    dres_variable_t    *var;
    char                buf[64], factname[128];

    char                pred_name[64];
    prolog_predicate_t *predicate;
    char   ***retval;
    int       i;
    
    if (action->nargument < 1)
        return EINVAL;

#if 0
    {
        char *name      = "printf";
        char *signature = (char *)console_printf_SIGNATURE;
        void *ptr;

        if (ohm_module_find_method(name, &signature, &ptr))
            printf("***** %s %s resolved to %p\n", name, signature, ptr);
        else
            printf("***** %s could NOT be resolved\n", name);
    }
#endif
    
    pred_name[0] = '\0';
    dres_name(dres, action->arguments[0], pred_name, sizeof(pred_name));
    
    if ((predicate = prolog_lookup(pred_name, action->nargument)) == NULL)
        return ENOENT;
    
    if (!prolog_invoke(predicate, &retval))
        return EINVAL;
    
    DEBUG("%s%s%s/%d gave the following result:",
          predicate->module ?: "", predicate->module ? ":" : "",
          predicate->name, predicate->arity);
    prolog_dump(retval);
    
    if (DRES_ID_TYPE(action->lvalue.variable) == DRES_TYPE_FACTVAR) {
        facts->len = retval_to_facts("foo", retval,
                                     (OhmFact **)facts->fact, MAX_FACTS);
        if (facts->len < 0)
            goto fail;
        
        dres_name(dres, action->lvalue.variable, buf, sizeof(buf));
        snprintf(factname, sizeof(factname), "%s%s",
                 dres_get_prefix(dres), buf + 1);
        
        if (action->lvalue.field != NULL) {
            /* uh-oh... */
            DEBUG("uh-oh... should set lvalue.field");
            goto fail;
        }
        
        if ((var = dres_lookup_variable(dres, action->lvalue.variable)) == NULL)
            goto fail;
        
        if (!dres_var_set(var->var, action->lvalue.selector,
                          VAR_FACT_ARRAY, facts))
            goto fail;
        prolog_free(retval);
        
        for (i = 0; i < facts->len; i++)
            g_object_unref(facts->fact[i]);
        printf("***** inserted new fact %s\n", factname);
    }

    return 0;

        
 fail:
    for (i = 0; i < facts->len; i++)
        g_object_unref(facts->fact[i]);
    
    return EINVAL;

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
    Transaction   *trans;
    int            factc;
    char           signal_name[MAX_LENGTH];
    char           callback_name[MAX_LENGTH];
    char          *factv[MAX_FACTS + 1];
    char        ***retval;
    char           buf[MAX_FACTS * MAX_LENGTH];
    char          *p;
    int            i;
    
    factc = action->nargument - 2;

    if (factc < 1 || factc > MAX_FACTS)
        return EINVAL;
    
    signal_name[0] = callback_name[0] = '\0';
    dres_name(dres, action->arguments[0], signal_name, MAX_LENGTH);
    dres_name(dres, action->arguments[1], callback_name, MAX_LENGTH);
    
    for (p = buf, i = 0;   i < factc;   i++, p += strlen(p)) {
        dres_name(dres, action->arguments[i+2], p, MAX_LENGTH);
        factv[i] = p;
    }
    factv[factc] = NULL;


    if ((transact = signal(signal_name, factc,factv, 5 * 1000)) == NULL) {
        /* call the completion_callback with an error */
    }
    else {
    }

    return 0;

#undef MAX_LENGTH
#undef MAX_FACTS
}


/*****************************************************************************
 *                        *** misc. helper routines ***                      *
 *****************************************************************************/


/********************
 * object_to_fact
 ********************/
static OhmFact *
object_to_fact(char *name, char **object)
{
    OhmFact *fact;
    GValue   value;
    char    *field;
    int      i;

    if (object == NULL || strcmp(object[0], "name") || object[1] == NULL)
        return NULL;
    
    if ((fact = ohm_fact_new(name)) == NULL)
        return NULL;
    
    for (i = 2; object[i] != NULL; i += 2) {
        field = object[i];
        value = ohm_value_from_string(object[i+1]);
        ohm_fact_set(fact, field, &value);
    }
    
    return fact;
}


/********************
 * retval_to_facts
 ********************/
static int
retval_to_facts(char *name, char ***objects, OhmFact **facts, int max)
{
    char **object;
    int    i;
    
    for (i = 0; (object = objects[i]) != NULL && i < max; i++) {
        if ((facts[i] = object_to_fact(name, object)) == NULL)
            return -EINVAL;
    }
    
    return i;
}


/********************
 * dump_fact_store
 ********************/
static void
dump_fact_store(int cid)
{
    char *dump = ohm_fact_store_to_string(ohm_fact_store_get_fact_store());
    console_write(cid, dump, 0);
}



typedef struct {
    char              *name;
    char              *value;
} dres_fldsel_t;

typedef struct {
    int                count;
    dres_fldsel_t     *field;
} dres_selector_t;


static dres_selector_t *parse_selector(char *descr)
{
    dres_selector_t *selector;
    dres_fldsel_t   *field;
    char            *p, *q, c;
    char            *str;
    char            *name;
    char            *value;
    char             buf[1024];
    int              i;

    
    if (descr == NULL) {
        errno = 0;
        return NULL;
    }

    for (p = descr, q = buf;  (c = *p) != '\0';   p++) {
        if (c > 0x20 && c < 0x7f)
            *q++ = c;
    }
    *q = '\0';

    if ((selector = malloc(sizeof(*selector))) == NULL)
        return NULL;
    memset(selector, 0, sizeof(*selector));

    for (i = 0, str = buf;   (name = strtok(str, ",")) != NULL;   str = NULL) {
        if ((p = strchr(name, ':')) == NULL)
            DEBUG("Invalid selctor: '%s'", descr);
        else {
            *p++ = '\0';
            value = p;

            selector->count++;
            selector->field = realloc(selector->field,
                                      sizeof(dres_fldsel_t) * selector->count);

            if (selector->field == NULL)
                return NULL; /* maybe better not to attempt to free anything */
            
            field = selector->field + selector->count - 1;

            field->name  = strdup(name);
            field->value = strdup(value);
        }
    }
   
    return selector;
}

static void free_selector(dres_selector_t *selector)
{
    int i;

    if (selector != NULL) {
        for (i = 0; i < selector->count; i++) {
            free(selector->field[i].name);
            free(selector->field[i].value);
        }

        free(selector);
    }
}

static int is_matching(OhmFact *fact, dres_selector_t *selector)
{
    dres_fldsel_t *fldsel;
    GValue        *gval;
    long int       ival;
    char          *e;
    int            i;
    int            match;
  
    if (fact == NULL || selector == NULL)
        match = FALSE;
    else {
        match = TRUE;

        for (i = 0;    match && i < selector->count;    i++) {
            fldsel = selector->field + i;

            if ((gval = ohm_fact_get(fact, fldsel->name)) == NULL)
                match = FALSE;
            else {
                switch (G_VALUE_TYPE(gval)) {
                    
                case G_TYPE_STRING:
                    match = !strcmp(g_value_get_string(gval), fldsel->value);
                    break;
                    
                case G_TYPE_INT:
                    ival  = strtol(fldsel->value, &e, 10);
                    match = (*e == '\0' && g_value_get_int(gval) == ival);
                    break;

                default:
                    match = FALSE;
                    break;
                }
            }
        } /* for */
    }

    return match;
}

static int find_facts(char *name, char *select, OhmFact **facts, int max)
{
    dres_selector_t *selector = parse_selector(select);
    
    GSList            *list;
    int                llen;
    OhmFact           *fact;
    int                flen;
    int                i;

    list   = ohm_fact_store_get_facts_by_name(ohm_fact_store_get_fact_store(),
                                              name);
    llen   = list ? g_slist_length(list) : 0;

    for (i = flen = 0;    list != NULL;   i++, list = g_slist_next(list)) {
        fact = (OhmFact *)list->data;

        if (!selector || is_matching(fact, selector))
            facts[flen++] = fact;

        if (flen >= max) {
            free_selector(selector);
            errno = ENOMEM;
            return -1;
        }
        
    }

    free_selector(selector);
    return flen;
}


/********************
 * set_fact
 ********************/
static void
set_fact(int cid, char *buf)
{
    GValue  gval;
    char    selector[128];
    char   *str, *name, *member, *selfld, *selval, *value, *p, *q;
    int     len;
    int      n = 128;
    OhmFact *facts[n];
    
    selector[0] = '\0';
    if (strchr(buf, '=') != NULL) {
        /*
         * here we parse command lines like:
         *    com.nokia.policy.audio_route[device:ihf].status = 0, ...
         * where
         *    'com.nokia.policy.audio_route' is a fact that has two fields:
         *    'device' and 'status'
         */
        
        for (str = buf; (name = strtok(str, ",")) != NULL; str = NULL) {
            if ((p = strchr(name, '=')) != NULL) {
                *p++ = 0;
                value = p;
                
                if ((p = strrchr(name, '.')) != NULL) {
                    *p++ = 0;
                    member = p;
                    
                    if (p[-2] == ']' && (q = strchr(name, '[')) != NULL) {
                        
                        len = p - 2 - q - 1;
                        strncpy(selector, q + 1, len);
                        selector[len] = '\0';
                        
                        *q = p[-2] = 0;
                        selfld = q + 1;
                        if ((p = strchr(selfld, ':')) == NULL) {
                            console_printf(cid, "Invalid input: %s\n", selfld);
                            continue;
                            }
                        else {
                            *p++ = 0;
                            selval = p;
                        }
                    }
                    
                    gval = ohm_value_from_string(value);
                    
                    if ((n = find_facts(name, selector, facts, n)) < 0)
                        console_printf(cid, "no fact matches %s[%s]\n",
                                       name, selector ?: "");
                    else {
                        int i;
                        for (i = 0; i < n; i++) {
                            ohm_fact_set(facts[i], member, &gval);
                            console_printf(cid, "%s:%s = %s\n", name,
                                           member, value);
                        }
                    }
                }
            }
        }
    }
}


/********************
 * dres_goal
 ********************/
static void
dres_goal(int cid, char *goal)
{
    dres_update_goal(dres, goal);
}


/********************
 * console_handler
 ********************/
static void
console_handler(int id, char *input, void *data)
{
    if (!strcmp(input, "dump"))
        dump_fact_store(id);
    else if (!strncmp(input, "set ", 4))
        set_fact(id, input + 4);
    else if (!strncmp(input, "dres ", 5))
        dres_goal(id, input + 5);
    else if (!strcmp(input, "prolog"))
        prolog_shell();
    else
        console_printf(id, "unknown command \"%s\"\n", input);

    console_printf(id, "ohm-dres> ");
}



OHM_PLUGIN_DESCRIPTION("dres",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_REQUIRES("prolog", "console");

OHM_PLUGIN_PROVIDES_METHODS(dres, 1,
    OHM_EXPORT(update_goal, "dres")
);

OHM_PLUGIN_REQUIRES_METHODS(dres, 12,
    OHM_IMPORT("prolog.setup"         , prolog_setup),
    OHM_IMPORT("prolog.lookup"        , prolog_lookup),
    OHM_IMPORT("prolog.call"          , prolog_invoke),
    OHM_IMPORT("prolog.vcall"         , prolog_vinvoke),
    OHM_IMPORT("prolog.acall"         , prolog_ainvoke),
    OHM_IMPORT("prolog.free_retval"   , prolog_free),
    OHM_IMPORT("prolog.dump_retval"   , prolog_dump),
    OHM_IMPORT("prolog.shell"         , prolog_shell),
    OHM_IMPORT("console.open"         , console_open),
    OHM_IMPORT("console.close"        , console_close),
    OHM_IMPORT("console.write"        , console_write),
    OHM_IMPORT("console.printf"       , console_printf)
);
    
#if 1
Transaction *signal(char *signame, int factc, char**factv, guint timeout)
{
    static Transaction t;

    int i;

    DEBUG("signal(%s, %d, %p, %lu)\n", signame, factc, factv, timeout);
    for (i = 0;  i < factc;  i++) {
        DEBUG("   fact[%03d]: '%s'\n", i, factv[i]);
    }

    return &t;
}
#endif
                            

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

