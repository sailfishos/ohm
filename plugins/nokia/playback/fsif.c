#include "prolog/ohm-fact.h"

static OhmFactStore *fs;

static OhmFact *find_entry(char *, fsif_field_t *);
static void     get_field(OhmFact *, fsif_fldtype_t, char *, void *);
static void     set_field(OhmFact *, fsif_fldtype_t, char *, void *);

static void fsif_init(OhmPlugin *plugin)
{
    fs = ohm_fact_store_get_fact_store();
}

static int fsif_add_factstore_entry(char *name, fsif_field_t *fldlist)
{
    OhmFact      *fact;
    fsif_field_t *fld;

    if (!name || !fldlist) {
        DEBUG("%s(): invalid arument", __FUNCTION__);
        return FALSE;
    }

    if ((fact = ohm_fact_new(name)) == NULL) {
        DEBUG("Can't create new fact");
        return FALSE;
    }

    for (fld = fldlist;   fld->type != fldtype_invalid;   fld++) {
        set_field(fact, fld->type, fld->name, (void *)&fld->value);
    }

    if (ohm_fact_store_insert(fs, fact))
        DEBUG("factstore entry %s created", name);
    else {
        DEBUG("Can't add %s to factsore", name);
        return FALSE;
    }

    return TRUE;
}

static int fsif_delete_factstore_entry(char *name, fsif_field_t *selist)
{
    OhmFact *fact;
    int      success;

    if ((fact = find_entry(name, selist)) == NULL) {
        DEBUG("Failed to delete '%s' entry: no entry found", name);
        success = FALSE;
    }
    else {
        ohm_fact_store_remove(fs, fact);

        g_object_unref(fact);

        DEBUG("Factstore entry %s deleted", name);

        success = TRUE;
    }

    return success;
}

static int fsif_update_factstore_entry(char *name, fsif_field_t *selist,
                                       fsif_field_t *fldlist)
{
    OhmFact      *fact;
    fsif_field_t *fld;

    if ((fact = find_entry(name, selist)) == NULL) {
        DEBUG("Failed to delete '%s' entry: no entry found", name);
        return FALSE;
    }

    for (fld = fldlist;   fld->type != fldtype_invalid;   fld++) {
        set_field(fact, fld->type, fld->name, (void *)&fld->value);

        DEBUG("Factstore entry update %s.%s", name, fld->name);
    }

    return TRUE;
}

static OhmFact *find_entry(char *name, fsif_field_t *selist)
{
    OhmFact      *fact;
    GSList       *list;
    GValue       *gval;
    fsif_field_t *se;
    char         *strval;
    long          intval;
    double        fltval;

    for (list  = ohm_fact_store_get_facts_by_name(fs, name);
         list != NULL;
         list  = g_slist_next(list))
    {
        fact = (OhmFact *)list->data;

        if (selist != NULL) {
            for (se = selist;   se->type != fldtype_invalid;   se++) {
                if ((gval = ohm_fact_get(fact, se->name)) == NULL)
                    continue;
                else {
                    switch (se->type) {
                        
                    case fldtype_string:
                        if (G_VALUE_TYPE(gval) != G_TYPE_STRING)
                            continue;
                        get_field(fact, fldtype_string, se->name, &strval);
                        if (strcmp(strval, se->value.string))
                            continue;
                        break;
                        
                    case fldtype_integer:
                        if (G_VALUE_TYPE(gval) != G_TYPE_LONG)
                            continue;
                        get_field(fact, fldtype_integer, se->name, &intval);
                        if (intval != se->value.integer)
                            continue;
                        break;
                        
                    case fldtype_floating:
                        if (G_VALUE_TYPE(gval) != G_TYPE_DOUBLE)
                            continue;
                        get_field(fact, fldtype_integer, se->name, &fltval);
                        if (fltval != se->value.floating)
                            continue;
                        break;

                    default:
                        continue;
                    }
                }
            } /* for se */
        }

        return fact;
    }


    return NULL;
}

static void get_field(OhmFact *fact, fsif_fldtype_t type,char *name,void *vptr)
{
    GValue  *gv;


    if (!fact || !name || !(gv = ohm_fact_get(fact, name))) {
        DEBUG("Cant find field %s", name ? name : "<null>");

        switch (type) {
        case fldtype_string:       *(char  **)vptr = NULL;         break;
        case fldtype_integer:      *(long   *)vptr = 0;            break;
        case fldtype_floating:     *(double *)vptr = 0.0;          break;
        default:                                                   break;
        }

        return;
    }

    switch (type) {
    case fldtype_string:   *(const char **)vptr = g_value_get_string(gv);break;
    case fldtype_integer:  *(long        *)vptr = g_value_get_long(gv);  break;
    case fldtype_floating: *(double      *)vptr = g_value_get_double(gv);break;
    default:                                                             break;
    }
} 

static void set_field(OhmFact *fact, fsif_fldtype_t type,char *name,void *vptr)
{
    fsif_value_t *v = (fsif_value_t *)vptr;
    GValue       *gv;

    switch (type) {
    case fldtype_string:    gv = ohm_value_from_string(v->string);    break;
    case fldtype_integer:   gv = ohm_value_from_int(v->integer);      break;
    case fldtype_floating:  gv = ohm_value_from_double(v->floating);  break;
    default:                DEBUG("Invalid type for %s", name);       return;
    }

    ohm_fact_set(fact, name, gv);
}


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
