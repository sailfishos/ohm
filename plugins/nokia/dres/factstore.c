
/* factstore signal names */
#define FACT_INSERTED "inserted"
#define FACT_REMOVED  "removed"
#define FACT_UPDATED  "updated"


static void schedule_resolve(gpointer object, gpointer user_data);
static void schedule_updated(gpointer fact, gpointer name, gpointer value,
                             gpointer user_data);


static guint         update;                  /* id of next update, if any */
static OhmFactStore *store;                   /* fact store in use */




/*****************************************************************************
 *                       *** initialization & cleanup ***                    *
 *****************************************************************************/


/********************
 * factstore_init
 ********************/
static int
factstore_init(void)
{
    gpointer fs;
    
    if ((store = ohm_fact_store_get_fact_store()) == NULL)
        return EINVAL;
    
    fs = G_OBJECT(store);
    
    g_signal_connect(fs, FACT_INSERTED, G_CALLBACK(schedule_resolve), NULL);
    g_signal_connect(fs, FACT_REMOVED , G_CALLBACK(schedule_resolve), NULL);
    g_signal_connect(fs, FACT_UPDATED , G_CALLBACK(schedule_updated), NULL);
    
    return 0;
}


/********************
 * factstore_exit
 ********************/
static void
factstore_exit(void)
{
    gpointer fs;

    if (store == NULL)
        return;

    fs = G_OBJECT(store);
    g_signal_handlers_disconnect_by_func(fs, schedule_resolve, NULL);
    g_signal_handlers_disconnect_by_func(fs, schedule_updated, NULL);

    store = NULL;
}


/********************
 * update_all
 ********************/
static gboolean
update_all(gpointer data)
{
    printf("running DRES update all...\n");

    dres_update_goal(dres, "all", NULL);
    update = 0;

    return FALSE;
}


/********************
 * schedule_resolve
 ********************/
static void
schedule_resolve(gpointer object, gpointer user_data)
{
    printf("scheduling DRES update all...\n");

    if (!update)
        update = g_idle_add(update_all, NULL);
}


/********************
 * schedule_updated
 ********************/
static void
schedule_updated(gpointer fact, gpointer name, gpointer value,
                 gpointer user_data)
{
    schedule_resolve(fact, user_data);
}


/*****************************************************************************
 *                        *** rule engine / fact glue ***                    *
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
retval_to_facts(char ***objects, OhmFact **facts, int max)
{
    char **object;
    int    i;
    
    for (i = 0; (object = objects[i]) != NULL && i < max; i++) {
        if ((facts[i] = object_to_fact("foo", object)) == NULL)
            return -EINVAL;
    }
    
    return i;
}



/*****************************************************************************
 *                     *** variables / factstore glue ***                    *
 *****************************************************************************/

/*
 * this is copy-paste code from variables.c in the DRES library
 */

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
    char    selector[128], fullname[128];
    char   *str, *name, *member, *selfld, *selval, *value, *p, *q;
    int     len;
    int      n = 128;
    OhmFact *facts[n];
    
    selector[0] = '\0';
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
       
            if (name[0] == '.') {
                sprintf(fullname, "%s%s", dres_get_prefix(dres), name+1);
                name = fullname;
            }

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








/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

   
