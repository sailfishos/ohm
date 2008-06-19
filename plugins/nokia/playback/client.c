
static client_listhead_t  cl_head;


static void client_init(OhmPlugin *plugin)
{
    cl_head.next = (client_t *)&cl_head;
    cl_head.prev = (client_t *)&cl_head;
}

static client_t *client_create(char *dbusid, char *object)
{
    client_t *cl = client_find(dbusid, object);
    client_t *next;
    client_t *prev;
    sm_t     *sm;
    char      smname[256];
    char     *p, *q;

    if (cl == NULL) {
        p = (q = strrchr(object, '/')) ? q+1 : object;
        snprintf(smname, sizeof(smname), "%s:%s", dbusid, p);

        if ((cl = malloc(sizeof(*cl))) != NULL) {
            if ((sm = sm_create(smname, (void *)cl)) == NULL) {
                free(cl);
                cl = NULL;
            }                
            else {
                memset(cl, 0, sizeof(*cl));

                cl->dbusid = strdup(dbusid);
                cl->object = strdup(object);
                cl->sm     = sm;  

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
    }

    return cl;
}

static void client_destroy(client_t *cl)
{
    static sm_evdata_t  evdata = { .evid = evid_client_gone };

    client_t *prev, *next;

    if (cl != NULL) {
        DEBUG("playback %s%s going to be destroyed", cl->dbusid, cl->object);

        sm_process_event(cl->sm, &evdata);
        sm_destroy(cl->sm);

        client_delete_factsore_entry(cl);

        pbreq_purge(cl);

        if (cl->dbusid)
            free(cl->dbusid);

        if (cl->object)
            free(cl->object);

        if (cl->group)
            free(cl->group);

        if (cl->state)
            free(cl->state);

        next = cl->next;
        prev = cl->prev;

        prev->next = cl->next;
        next->prev = cl->prev;

        free(cl);
    }
}

static client_t *client_find(char *dbusid, char *object)
{
    client_t *cl;

    if (dbusid && object) {
        for (cl = cl_head.next;   cl != (client_t *)&cl_head;   cl = cl->next){
            if (!strcmp(dbusid, cl->dbusid) &&
                !strcmp(object, cl->object)   )
                return cl;
        }
    }
    
    return NULL;
}

static void client_purge(char *dbusid)
{
    client_t *cl, *nxcl;

    for (cl = cl_head.next;  cl != (client_t *)&cl_head;  cl = nxcl) {
        nxcl = cl->next;

        if (!strcmp(dbusid, cl->dbusid))
            client_destroy(cl);
    }
}

static int client_add_factsore_entry(char *dbusid, char *object)
{
    time_t current_time = time(NULL);

    fsif_field_t  fldlist[] = {
        { fldtype_string , "dbusid"   , .value.string  = dbusid       },
        { fldtype_string , "object"   , .value.string  = object       },
        { fldtype_unsignd, "pid"      , .value.unsignd = 0            },
        { fldtype_string , "group"    , .value.string  = "othermedia" },
        { fldtype_string , "state"    , .value.string  = "none"       },
        { fldtype_string , "reqstate" , .value.string  = "none"       },
        { fldtype_string , "setstate" , .value.string  = "none"       },
        { fldtype_time   , "tstate"   , .value.integer = current_time },
        { fldtype_time   , "treqstate", .value.integer = current_time },
        { fldtype_time   , "tsetstate", .value.integer = current_time },
        { fldtype_invalid, NULL       , .value.string  = NULL         }
    };

    return fsif_add_factstore_entry(FACTSTORE_PLAYBACK, fldlist);
}

static void client_delete_factsore_entry(client_t *cl)
{
    fsif_field_t  selist[] = {
        { fldtype_string , "dbusid", .value.string = cl->dbusid },
        { fldtype_string , "object", .value.string = cl->object },
        { fldtype_invalid, NULL    , .value.string = NULL       }
    };
 
    fsif_delete_factstore_entry(FACTSTORE_PLAYBACK, selist);
}

static void client_update_factstore_entry(client_t *cl,char *field,char *value)
{
    fsif_field_t  selist[] = {
        { fldtype_string , "dbusid", .value.string = cl->dbusid },
        { fldtype_string , "object", .value.string = cl->object },
        { fldtype_invalid, NULL    , .value.string = NULL       }
    };

    fsif_field_t  fldlist[] = {
        { fldtype_string , field, .value.string = value },
        { fldtype_invalid, NULL , .value.string = NULL  },
        { fldtype_invalid, NULL , .value.string = NULL  }
    };

    char              *end;
    char               tsname[64];
    struct timeval     tv;
    unsigned long long cur_time;

    if (!strcmp(field, "pid")) {
        fldlist[0].type = fldtype_unsignd;
        fldlist[0].value.unsignd = strtoul(value, &end, 10);

        if (*end != '\0') {
            DEBUG("Invalid PID value '%s'", value);
            return;
        }
    }


    if (!strcmp(field, "state")    ||
        !strcmp(field, "reqstate") ||
        !strcmp(field, "setstate")   )
    {
        snprintf(tsname, sizeof(tsname), "t%s", field);

        gettimeofday(&tv, NULL);
        cur_time  = (unsigned long long)tv.tv_sec * 1000ULL;
        cur_time += (unsigned long long)(tv.tv_usec / 1000);

        fldlist[1].type = fldtype_time;
        fldlist[1].name = tsname;
        fldlist[1].value.time = cur_time;
    }

    fsif_update_factstore_entry(FACTSTORE_PLAYBACK, selist, fldlist);
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
