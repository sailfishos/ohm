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

#include <prolog/prolog.h>

#include <ohm/ohm-plugin.h>


static prolog_predicate_t *predicates;
static prolog_predicate_t *find_predicate(char *name, int arity);

 

/*****************************************************************************
 *                       *** initialization & cleanup ***                    *
 *****************************************************************************/

/**
 * plugin_init:
 **/
static void
plugin_init(OhmPlugin *plugin)
{
#define STACK_LIMIT 16,16,16,16                      /* 16k stacks */

    if (prolog_init("prolog-plugin", STACK_LIMIT) != 0) {
        g_warning("%s: Failed to initialize prolog library.", __FUNCTION__);
        exit(1);
    }
}


/**
 * plugin_exit:
 **/
static void
plugin_exit(OhmPlugin *plugin)
{
    prolog_free_predicates(predicates);
    prolog_exit();
}


/*****************************************************************************
 *                           *** exported methods ***                        *
 *****************************************************************************/

/********************
 * setup
 ********************/
OHM_EXPORTABLE(int, setup, (char **extensions, char **files))
{
    int   i;
    char *p;

    for (p = extensions[i=0]; p != NULL; p = extensions[++i])
        if (prolog_load_extension(p)) {
            g_warning("%s: failed to load extension \"%s\"", __FUNCTION__, p);
            return EINVAL;
        }

    for (p = files[i=0]; p != NULL; p = files[++i])
        if (prolog_load_file(p)) {
            g_warning("%s: failed to load file \"%s\"", __FUNCTION__, p);
            return EINVAL;
        }

    predicates = prolog_predicates(NULL);

    return 0;
}


/********************
 * lookup_predicate
 ********************/
OHM_EXPORTABLE(prolog_predicate_t *, lookup_predicate, (char *name, int arity))
{
    if (predicates == NULL)
        predicates = prolog_predicates(NULL);
    
    if (predicates == NULL)
        return NULL;

    return find_predicate(name, arity);
}


/********************
 * call_predicate
 ********************/
OHM_EXPORTABLE(int, call_predicate, (prolog_predicate_t *pred,
                                     void *retval, ...))
{
    va_list ap;
    int     status;

    va_start(ap, retval);
    status = prolog_vcall(pred, retval, ap);
    va_end(ap);
    
    return status;
}


/********************
 * vcall_predicate
 ********************/
OHM_EXPORTABLE(int, vcall_predicate, (prolog_predicate_t *pred,
                                      void *retval, va_list ap))
{
    va_list aq;
    int     status;

    va_copy(aq, ap);
    status = prolog_vcall(pred, retval, aq);
    va_end(aq);

    return status;
}


/********************
 * acall_predicate
 ********************/
OHM_EXPORTABLE(int, acall_predicate, (prolog_predicate_t *pred,
                                      void *retval, void **args, int narg))
{
    return prolog_acall(pred, retval, args, narg);
}


/********************
 * free_return_value
 ********************/
OHM_EXPORTABLE(void, free_return_value, (void *retval))
{
    char ***objects = (char ***)retval;
    prolog_free_objects(objects);
}


/********************
 * dump_return_value
 ********************/
OHM_EXPORTABLE(void, dump_return_value, (void *retval))
{
    char ***objects = (char ***)retval;
    prolog_dump_objects(objects);
}


/********************
 * prolog_shell
 ********************/
OHM_EXPORTABLE(void, prolog_shell, (void))
{
    prolog_prompt();
}


/*****************************************************************************
 *                        *** misc. helper routines ***                      *
 *****************************************************************************/

/********************
 * find_predicate
 ********************/
static prolog_predicate_t *
find_predicate(char *name, int arity)
{
    prolog_predicate_t *p;

    if (predicates == NULL)
        return NULL;

    for (p = predicates; p->name != NULL; p++)
        if ((arity < 0 || p->arity == arity) && !strcmp(p->name, name))
            return p;

    return NULL;
}






OHM_PLUGIN_DESCRIPTION("prolog",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_PROVIDES_METHODS(prolog, 8,
    OHM_EXPORT(setup            , "setup"),
    OHM_EXPORT(lookup_predicate , "lookup"),
    OHM_EXPORT(call_predicate   , "call"),
    OHM_EXPORT(vcall_predicate  , "vcall"),
    OHM_EXPORT(acall_predicate  , "acall"),
    OHM_EXPORT(free_return_value, "free_retval"),
    OHM_EXPORT(dump_return_value, "dump_retval"),
    OHM_EXPORT(prolog_shell     , "shell")
);

                            

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

