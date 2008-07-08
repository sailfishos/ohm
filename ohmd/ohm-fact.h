#ifndef __OHM_FACT_H__
#define __OHM_FACT_H__

#include <sys/time.h>

#include <ohm-factstore.h>

#define ohm_fact_store_get_fact_store ohm_get_fact_store


/*************************
 * ohm_fact_store_view_set_interested (excellent naming convention BTW...)
 *************************/
static inline void
ohm_fact_store_view_set_interested(OhmFactStoreView *view, GSList *patterns)
{
    OhmPattern *pattern;
    GSList     *l;
    
    for (l = patterns; l != NULL; l = g_slist_next(l)) {
        pattern = (OhmPattern *)l->data;
        ohm_fact_store_view_add(view, OHM_STRUCTURE(pattern));
    }
}


/*************************
 * ohm_view_add_pattern
 *************************/
static inline gboolean
ohm_view_add_pattern(OhmFactStoreView *view, char *key)
{
    OhmPattern *pattern = ohm_pattern_new(key);
    
    if (pattern == NULL)
        return FALSE;
    
    ohm_fact_store_view_add(view, OHM_STRUCTURE(pattern));
    
    return TRUE;
}


/*************************
 * ohm_view_get_changes
 *************************/
static inline GSList *
ohm_view_get_changes(OhmFactStoreView *view)
{
    OhmFactStoreSimpleView *sv = OHM_FACT_STORE_SIMPLE_VIEW(view);

    if (sv != NULL)
        return ohm_fact_store_change_set_get_matches(sv->change_set);
    else
        return NULL;
}


/*************************
 * ohm_view_reset_changes
 *************************/
static inline void
ohm_view_reset_changes(OhmFactStoreView *view)
{
    OhmFactStoreSimpleView *sv = OHM_FACT_STORE_SIMPLE_VIEW(view);

    if (sv)
        ohm_fact_store_change_set_reset(sv->change_set);
}


/*************************
 * ohm_value_from_unsigned
 *************************/
static inline GValue *
ohm_value_from_unsigned(unsigned long val)
{
    GValue* value;

    value = g_new0(GValue, 1);

    g_value_init(value, G_TYPE_ULONG);
    g_value_set_ulong(value, val);

    return value;
}


/*************************
 * ohm_value_from_double
 *************************/
static inline GValue *
ohm_value_from_double(double val)
{
    GValue* value;

    value = g_new0(GValue, 1);

    g_value_init(value, G_TYPE_DOUBLE);
    g_value_set_double(value, val);

    return value;
}


/*************************
 * ohm_value_from_time
 *************************/
static inline GValue *
ohm_value_from_time(guint64 val)
{
    GValue* value;
    value = g_new0(GValue, 1);

    g_value_init(value, G_TYPE_UINT64);
    g_value_set_uint64(value, val);

    return value;
}


/*************************
 * ohm_copy_value
 *************************/
static inline GValue *
ohm_copy_value(GValue *src)
{
    GType   type = G_VALUE_TYPE(src);
    GValue *dst  = g_new0(GValue, 1);

    if (dst == NULL)
        return NULL;
    
    g_value_init(dst, type);
    g_value_copy(src, dst);
    
    return dst;
}


/*************************
 * ohm_fact_del
 *************************/
static inline void
ohm_fact_del(OhmFact *fact, const char *field)
{
    ohm_fact_set(fact, field, NULL);
}


#endif



/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/



