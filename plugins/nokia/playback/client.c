#define SELIST_DIM 3

static client_listhead_t  cl_head;

static int init_selist(client_t *, fsif_field_t *, int);

static void client_init(OhmPlugin *plugin)
{
    cl_head.next = (client_t *)&cl_head;
    cl_head.prev = (client_t *)&cl_head;
}

static client_t *client_create(char *dbusid, char *object,
                               char *pid, char *stream)
{
    client_t *cl = client_find_by_dbus(dbusid, object);
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

                cl->dbusid = dbusid ? strdup(dbusid) : NULL;
                cl->object = object ? strdup(object) : NULL;
                cl->pid    = pid    ? strdup(pid)    : NULL;
                cl->stream = stream ? strdup(stream) : NULL;
                cl->sm     = sm;  

                next = (client_t *)&cl_head;
                prev = cl_head.prev;
                
                prev->next = cl;
                cl->next   = next;
                
                next->prev = cl;
                cl->prev   = prev;
                
                if (client_add_factstore_entry(dbusid, object, pid, stream))
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
#define FREE(d)                 \
    do {                        \
        if ((d) != NULL)        \
            free((void *)(d));  \
    } while(0)

    static sm_evdata_t  evdata = { .evid = evid_client_gone };

    client_t *prev, *next;

    if (cl != NULL) {
        DEBUG("playback %s%s going to be destroyed", cl->dbusid, cl->object);

        sm_process_event(cl->sm, &evdata);
        sm_destroy(cl->sm);

        client_delete_factsore_entry(cl);

        pbreq_purge(cl);

        FREE(cl->dbusid);
        FREE(cl->object);
        FREE(cl->pid);
        FREE(cl->stream);
        FREE(cl->group);
        FREE(cl->reqstate);
        FREE(cl->state);
        FREE(cl->setstate);

        next = cl->next;
        prev = cl->prev;

        prev->next = cl->next;
        next->prev = cl->prev;

        free(cl);
    }

#undef FREE
}

static client_t *client_find_by_dbus(char *dbusid, char *object)
{
    client_t *cl;

    if (dbusid && object) {
        for (cl = cl_head.next;   cl != (client_t *)&cl_head;   cl = cl->next){
            if (cl->dbusid && !strcmp(dbusid, cl->dbusid) &&
                cl->object && !strcmp(object, cl->object)   )
                return cl;
        }
    }
    
    return NULL;
}

#if 0
static client_t *client_find_by_stream(char *pid, char *stream)
{
    client_t *cl;

    if (pid) {
        for (cl = cl_head.next;   cl != (client_t *)&cl_head;   cl = cl->next){
            if (cl->pid && !strcmp(pid, cl->pid)) {
                if (!stream)
                    return cl;

                if (cl->stream && !strcmp(stream, cl->stream))
                    return cl;
            }
        }
    }
    
    return NULL;
}
#endif

static void client_purge(char *dbusid)
{
    client_t *cl, *nxcl;

    for (cl = cl_head.next;  cl != (client_t *)&cl_head;  cl = nxcl) {
        nxcl = cl->next;

        if (cl->dbusid && !strcmp(dbusid, cl->dbusid))
            client_destroy(cl);
    }
}

static int client_add_factstore_entry(char *dbusid, char *object,
                                      char *pid, char *stream)
{
#define STRING(s) ((s) ? (s) : "")

# if 0
    time_t current_time = time(NULL);
#endif

    fsif_field_t  fldlist[] = {
        { fldtype_string , "dbusid"   , .value.string  = STRING(dbusid)},
        { fldtype_string , "object"   , .value.string  = STRING(object)},
        { fldtype_string , "pid"      , .value.string  = STRING(pid)   },
        { fldtype_string , "stream"   , .value.string  = STRING(stream)},
        { fldtype_string , "group"    , .value.string  = "othermedia"  },
        { fldtype_string , "state"    , .value.string  = "none"        },
        { fldtype_string , "reqstate" , .value.string  = "none"        },
        { fldtype_string , "setstate" , .value.string  = "none"        },
#if 0
        { fldtype_time   , "tstate"   , .value.integer = current_time },
        { fldtype_time   , "treqstate", .value.integer = current_time },
        { fldtype_time   , "tsetstate", .value.integer = current_time },
#endif
        { fldtype_invalid, NULL       , .value.string  = NULL         }
    };

    return fsif_add_factstore_entry(FACTSTORE_PLAYBACK, fldlist);

#undef STRING
}

static void client_delete_factsore_entry(client_t *cl)
{
    fsif_field_t  selist[SELIST_DIM];
 
    if (!init_selist(cl, selist, SELIST_DIM))
        return;

    fsif_delete_factstore_entry(FACTSTORE_PLAYBACK, selist);
}

static void client_update_factstore_entry(client_t *cl,char *field,char *value)
{
    fsif_field_t  selist[SELIST_DIM];
    fsif_field_t  fldlist[] = {
        { fldtype_string , field, .value.string = value },
#if 0
        { fldtype_invalid, NULL , .value.string = NULL  },
#endif
        { fldtype_invalid, NULL , .value.string = NULL  }
    };

    if (!init_selist(cl, selist, SELIST_DIM))
        return;

#if 0
    char               tsname[64];
    struct timeval     tv;
    unsigned long long cur_time;

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
#endif

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

static void client_save_state(client_t *cl, client_stype_t type, char *value)
{
    char **store;

    if (cl != NULL && value != NULL) {

        switch (type) {
        case client_reqstate:   store = &cl->reqstate;     break;
        case client_state:      store = &cl->state;        break;
        case client_setstate:   store = &cl->setstate;     break;
        default:                                           return;
        }

        if (*store != NULL)
            free((void *)*store);

        *store = strdup(value);
    }
}

static int init_selist(client_t *cl, fsif_field_t *selist, int dim)
{
    if (selist != NULL && dim > 0) {
        memset(selist, 0, sizeof(*selist) * dim);

        if (cl->dbusid && cl->object && dim >= 3) {
            selist[0].type = fldtype_string;
            selist[0].name = "dbusid";
            selist[0].value.string = cl->dbusid;
            
            selist[1].type = fldtype_string;
            selist[1].name = "object";
            selist[1].value.string = cl->object;

            return TRUE;
        }

        if (cl->pid && dim >= (cl->stream ? 3 : 2)) {
            selist[0].type = fldtype_string;
            selist[0].name = "pid";
            selist[0].value.string = cl->pid;

            if (cl->stream) {
                selist[1].type = fldtype_string;
                selist[1].name = "stream";
                selist[1].value.string = cl->stream;
            }
            
            return TRUE;
        }
    }

    return FALSE;
}

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
