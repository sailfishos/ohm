
static client_listhead_t  cl_head;

static OhmFact  *find_factstore_entry(client_t *);


static void client_init(OhmPlugin *plugin)
{
    cl_head.next = (client_t *)&cl_head;
    cl_head.prev = (client_t *)&cl_head;
}

static client_t *client_create(const char *dbusid, const char *object)
{
    client_t *cl = client_find(dbusid, object);
    client_t *next;
    client_t *prev;

    if (cl == NULL) {
        if ((cl = malloc(sizeof(*cl))) != NULL) {
            memset(cl, 0, sizeof(*cl));

            cl->dbusid = strdup(dbusid);
            cl->object = strdup(object);

            next = (client_t *)&cl_head;
            prev = cl_head.prev;

            prev->next = cl;
            cl->next   = next;

            next->prev = cl;
            cl->prev   = prev;

            if (client_add_factsore_entry(dbusid, object))
                DEBUG("playback %s%s created", dbusid, object);
            else {
                client_destroy(cl);
                cl = NULL;
            }
        }
    }

    return cl;
}

static void client_destroy(client_t *cl)
{
    client_t *prev, *next;

    if (cl != NULL) {
        DEBUG("playback %s%s going to be destroyed", cl->dbusid, cl->object);

        client_delete_factsore_entry(cl);
        pbreq_purge(cl);

        if (cl->dbusid)
            free(cl->dbusid);

        if (cl->object)
            free(cl->object);

        if (cl->group)
            free(cl->group);

        next = cl->next;
        prev = cl->prev;

        prev->next = cl->next;
        next->prev = cl->prev;

        free(cl);
    }
}

static client_t *client_find(const char *dbusid, const char *object)
{
    client_t *cl;

    for (cl = cl_head.next;   cl != (client_t *)&cl_head;   cl = cl->next){
        if (!strcmp(dbusid, cl->dbusid) &&
            !strcmp(object, cl->object)   )
            return cl;
    }
    
    return NULL;
}

static void client_purge(const char *dbusid)
{
    client_t *cl, *nxcl;

    for (cl = cl_head.next;  cl != (client_t *)&cl_head;  cl = nxcl) {
        nxcl = cl->next;

        if (!strcmp(dbusid, cl->dbusid))
            client_destroy(cl);
    }
}

static int client_add_factsore_entry(const char *client, const char *object)
{
    OhmFactStore   *fs;
    OhmFact        *fact;
    GValue         *gval;

    fs = ohm_fact_store_get_fact_store();
    fact = ohm_fact_new(FACTSTORE_PLAYBACK);

    gval = ohm_str_value(client);
    ohm_fact_set(fact, "dbusid", gval);
    
    gval = ohm_str_value(object);
    ohm_fact_set(fact, "object", gval);

    gval = ohm_str_value("othermedia");
    ohm_fact_set(fact, "group", gval);

    gval = ohm_str_value("none");
    ohm_fact_set(fact, "state", gval);

    if (ohm_fact_store_insert(fs, fact))
        DEBUG("factstore entry %s created", FACTSTORE_PLAYBACK);
    else {
        DEBUG("Can't add %s to factsore", FACTSTORE_PLAYBACK);
        return FALSE;
    }
    
    return TRUE;
}

static void client_delete_factsore_entry(client_t *cl)
{
    OhmFactStore *fs;
    OhmFact      *fact;

    fs = ohm_fact_store_get_fact_store();

    if ((fact = find_factstore_entry(cl)) != NULL) {
        ohm_fact_store_remove(fs, fact);

        g_object_unref(fact);

        DEBUG("Factstore entry %s deleted", FACTSTORE_PLAYBACK);
    }
}

static void client_update_factsore_entry(client_t *cl,char *member,char *value)
{
    OhmFact *fact;
    GValue  *gval;

    if ((fact = find_factstore_entry(cl)) && member && value) {

        gval = ohm_str_value(value);
        ohm_fact_set(fact, member, gval);

        DEBUG("Factstore entry update %s[dbusid:%s,object:%s].%s = %s",
              FACTSTORE_PLAYBACK, cl->dbusid, cl->object, member, value);
    }
}

static OhmFact *find_factstore_entry(client_t *cl)
{
    OhmFactStore *fs;
    OhmFact      *fact;
    GSList       *list;
    GValue       *gval;

    if (cl != NULL) {
        fs = ohm_fact_store_get_fact_store();

        for (list  = ohm_fact_store_get_facts_by_name(fs, FACTSTORE_PLAYBACK);
             list != NULL;
             list  = g_slist_next(list))
        {
            fact = (OhmFact *)list->data;

            if ((gval = ohm_fact_get(fact, "dbusid")) == NULL ||
                strcmp(cl->dbusid, g_value_get_string(gval))     )
                continue;
                
            if ((gval = ohm_fact_get(fact, "object")) == NULL ||
                strcmp(cl->object, g_value_get_string(gval))     )
                continue;
            
            return fact;
        }
    }

    return NULL;
}

static void client_get_property(client_t *cl, char *prname,
                                get_property_cb_t usercb)
{
    dbusif_get_property(cl->dbusid, cl->object, prname, usercb);
}

#if 0
static void client_set_property(ohm_playback_t *cl, char *prname,
                                char *prvalue, set_property_cb_t usercb)
{
    static char        *ifname = DBUS_PLAYBACK_INTERFACE;

    DBusMessage        *msg;
    DBusPendingCall    *pend;
    int                 success;

    msg = dbus_message_new_method_call(DBUS_PLAYBACK_SERVICE,
                                       cl->object,
                                       DBUS_INTERFACE_PROPERTIES,
                                       "Set");
    if (msg == NULL) {
        DEBUG("Failed to create D-Dbus message to set properties");
        return;
    }

    success = dbus_message_append_args(msg,
                                       DBUS_TYPE_STRING, &ifname,
                                       DBUS_TYPE_STRING, &prname,
                                       DBUS_TYPE_STRING, &prvalue,
                                       DBUS_TYPE_INVALID);
    if (!success) {
        DEBUG("Can't setup D-Bus message to set properties");
        goto failed;
    }
    
    success = dbus_connection_send_with_reply(sess_conn, msg, &pend, 1000);
    if (!success) {
        DEBUG("Failed to set properties");
        goto failed;
    }

    success = dbus_pending_call_set_notify(pend, get_property_cb, ud,
                                           free_set_property_cb_data);
    if (!success) {
        DEBUG("Can't set notification for pending call");
    }

    return;

 failed:
    dbus_message_unref(msg);
    return;
}
#endif

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
