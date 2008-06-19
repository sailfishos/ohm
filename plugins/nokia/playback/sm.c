typedef int       (* sm_trfunc_t)(sm_evdata_t *, void *);
typedef sm_stid_t (* sm_cndfunc_t)(void *);

typedef struct {
    sm_evid_t      id;
    char          *name;
} sm_evdef_t;

typedef struct {
    sm_evid_t     evid;         /* event id */
    sm_trfunc_t   func;         /* transition function */
    char         *fname;        /* name of the transition function */
    sm_stid_t     stid;         /* new state where we go if func() succeeds */
    sm_cndfunc_t  cond;         /* function for conditional transition */
    char         *cname;        /* name of the condition function */
} sm_transit_t;

typedef struct {
    sm_stid_t     id;
    char         *name;
    sm_transit_t  trtbl[evid_max];
} sm_stdef_t;

typedef struct {
    sm_stid_t     stid;            /* initial state */
    sm_evdef_t   *evdef;           /* event definitions */
    sm_stdef_t    stdef[stid_max]; /* state definitions */
} sm_def_t;

typedef struct {
    sm_t         *sm;
    sm_evdata_t  *evdata;
    sm_evfree_t   evfree;
} sm_schedule_t;


sm_evdef_t  evdef[evid_max] = {
    { evid_invalid          ,  "invalid event"     },
    { evid_hello_signal     ,  "hello signal"      },
    { evid_state_signal     ,  "state signal"      },
    { evid_property_received,  "property received" }, 
    { evid_setup_complete   ,  "setup complete"    },
    { evid_playback_request ,  "playback request"  },
    { evid_playback_complete,  "playback complete" },
    { evid_playback_failed  ,  "playback failed"   },
    { evid_client_gone      ,  "client gone"       },
};

static int read_property(sm_evdata_t *, void *);
static int save_property(sm_evdata_t *, void *);
static int process_pbreq(sm_evdata_t *, void *);
static int reply_pbreq(sm_evdata_t *, void *);
static int reply_pbreq_deq(sm_evdata_t *, void *);
static int abort_pbreq_deq(sm_evdata_t *, void *);
static int check_queue(sm_evdata_t *, void *);
static int update_state(sm_evdata_t *, void *);
static int update_state_deq(sm_evdata_t *, void *);


#define DO(f)  f, # f "()"
#define GOTO     NULL, NULL
#define STATE(s) stid_##s, NULL, NULL
#define COND(f)  -1, f, # f "()"

sm_def_t  sm_def = {
    stid_invalid,               /* state */
    evdef,                      /* evdef */
    {                           /* stdef */

        {stid_invalid, "initial", {
         /* event                 transition               new state        */ 
         /*-----------------------------------------------------------------*/
         {evid_invalid          , GOTO                   , STATE(invalid)    },
         {evid_hello_signal     , DO(read_property)      , STATE(setup)      },
         {evid_state_signal     , GOTO                   , STATE(invalid)    },
         {evid_property_received, GOTO                   , STATE(invalid)    },
         {evid_setup_complete   , GOTO                   , STATE(invalid)    },
         {evid_playback_request , GOTO                   , STATE(invalid)    },
         {evid_playback_complete, GOTO                   , STATE(invalid)    },
         {evid_playback_failed  , GOTO                   , STATE(invalid)    },
         {evid_client_gone      , GOTO                   , STATE(invalid)    }}
        },

        {stid_setup, "setup", {
         /* event                 transition               new state        */ 
         /*-----------------------------------------------------------------*/
         {evid_invalid          , GOTO                   , STATE(setup)      },
         {evid_hello_signal     , GOTO                   , STATE(setup)      },
         {evid_state_signal     , GOTO                   , STATE(setup)      },
         {evid_property_received, DO(save_property)      , STATE(setup)      },
         {evid_setup_complete   , DO(check_queue)        , STATE(idle)       },
         {evid_playback_request , GOTO                   , STATE(setup)      },
         {evid_playback_complete, GOTO                   , STATE(setup)      },
         {evid_playback_failed  , GOTO                   , STATE(setup)      },
         {evid_client_gone      , GOTO                   , STATE(invalid)    }}
        },

        {stid_idle, "idle", {
         /* event                 transition               new state        */ 
         /*-----------------------------------------------------------------*/
         {evid_invalid          , GOTO                   , STATE(idle)       },
         {evid_hello_signal     , GOTO                   , STATE(idle)       },
         {evid_state_signal     , GOTO                   , STATE(idle)       },
         {evid_property_received, GOTO                   , STATE(idle)       },
         {evid_setup_complete   , GOTO                   , STATE(idle)       },
         {evid_playback_request , DO(process_pbreq)      , STATE(pbreq)      },
         {evid_playback_complete, GOTO                   , STATE(idle)       },
         {evid_playback_failed  , GOTO                   , STATE(idle)       },
         {evid_client_gone      , GOTO                   , STATE(invalid)    }}
        },

        {stid_pbreq, "playback request", {
         /* event                 transition           new state        */ 
         /*-----------------------------------------------------------------*/
         {evid_invalid          , GOTO                   , STATE(pbreq)      },
         {evid_hello_signal     , GOTO                   , STATE(pbreq)      },
         {evid_state_signal     , DO(update_state)       , STATE(acked_pbreq)},
         {evid_property_received, GOTO                   , STATE(pbreq)      },
         {evid_setup_complete   , GOTO                   , STATE(pbreq)      },
         {evid_playback_request , GOTO                   , STATE(pbreq)      },
         {evid_playback_complete, DO(reply_pbreq)        , STATE(waitack)    },
         {evid_playback_failed  , DO(abort_pbreq_deq)    , STATE(idle)       },
         {evid_client_gone      , GOTO                   , STATE(invalid)    }}
        },

        {stid_acked_pbreq, "acknowledged playback request", {
         /* event                 transition           new state            */ 
         /*-----------------------------------------------------------------*/
         {evid_invalid          , GOTO               , STATE(acked_pbreq)},
         {evid_hello_signal     , GOTO               , STATE(acked_pbreq)    },
         {evid_state_signal     , GOTO               , STATE(acked_pbreq)    },
         {evid_property_received, GOTO               , STATE(acked_pbreq)    },
         {evid_setup_complete   , GOTO               , STATE(acked_pbreq)    },
         {evid_playback_request , GOTO               , STATE(acked_pbreq)    },
         {evid_playback_complete, DO(reply_pbreq_deq), STATE(idle)           },
         {evid_playback_failed  , DO(abort_pbreq_deq), STATE(idle)           },
         {evid_client_gone      , GOTO               , STATE(invalid)        }}
        },

        {stid_waitack, "wait for acknowledgement", {
         /* event                 transition               new state        */ 
         /*-----------------------------------------------------------------*/
         {evid_invalid          , GOTO                   , STATE(waitack)    },
         {evid_hello_signal     , GOTO                   , STATE(acked_pbreq)},
         {evid_state_signal     , DO(update_state_deq)   , STATE(idle)       },
         {evid_property_received, GOTO                   , STATE(acked_pbreq)},
         {evid_setup_complete   , GOTO                   , STATE(acked_pbreq)},
         {evid_playback_request , GOTO                   , STATE(acked_pbreq)},
         {evid_playback_complete, GOTO                   , STATE(acked_pbreq)},
         {evid_playback_failed  , GOTO                   , STATE(acked_pbreq)},
         {evid_client_gone      , GOTO                   , STATE(invalid)    }}
        },

    }                           /* stdef */
};

#undef COND
#undef STATE
#undef GOTO
#undef DO

/********************** hack ***************************/
void *foo = (void *)dbusif_set_property;
/****************** end of hack ************************/

static void  verify_state_machine(void);
static int   fire_scheduled_event(void *);
static void  free_evdata(sm_evdata_t *);
static void  fire_hello_signal_event(char *, char *);
static void  fire_state_signal_event(char *, char *, char *, char *);
static void  read_property_cb(char *, char *, char *, char *);
static char *strncpylower(char *, const char *, int);
static char *class_to_group(char *);
static void  schedule_next_request(client_t *);


static void sm_init(OhmPlugin *plugin)
{
    verify_state_machine();

    dbusif_add_hello_notification(fire_hello_signal_event);
    dbusif_add_property_notification("State", fire_state_signal_event);
}

static sm_t *sm_create(char *name, void *user_data)
{
    sm_t *sm;

    if (!name) {
        DEBUG("name is <null>");
        return NULL;
    }

    if ((sm = malloc(sizeof(*sm))) == NULL) {
        DEBUG("[%s] Failed to allocate memory for state machine", name);
        return NULL;
    }

    memset(sm, 0, sizeof(*sm));
    sm->name = strdup(name);
    sm->stid = sm_def.stid;
    sm->data = user_data;

    DEBUG("[%s] state machine created", sm->name);

    return sm;
}

static int sm_destroy(sm_t *sm)
{
    if (!sm)
        return FALSE;

    if (sm->busy) {
        DEBUG("[%s] state machine is busy", sm->name);
        return FALSE;
    }

    DEBUG("[%s] state machine is going to be destroyed", sm->name);

    if (sm->sched != 0)
        g_source_remove(sm->sched);

    free(sm->name);
    free(sm);

    return TRUE;
}

static int sm_process_event(sm_t *sm, sm_evdata_t *evdata)
{
    sm_evid_t     evid;
    sm_evdef_t   *event;
    sm_stid_t     stid, next_stid;
    sm_stdef_t   *state, *next_state;
    sm_transit_t *transit;

    if (!sm || !evdata)
        return FALSE;

    if (sm->busy) {
        DEBUG("[%s] attempt for nested event processing", sm->name);
        return FALSE;
    }

    evid = evdata->evid;
    stid = sm->stid;

    if (evid <= evid_invalid || evid >= evid_max) {
        DEBUG("[%s] Event ID %d is out of range (%d - %d)",
              sm->name, evid, evid_invalid+1, evid_max-1);
        return FALSE;
    }

    if (stid < 0 || stid >= stid_max) {
        DEBUG("[%s] current state %d is out of range (0 - %d)",
              sm->name, stid, stid_max-1);
        return FALSE;
    }

    event   = sm_def.evdef + evid;
    state   = sm_def.stdef + stid;
    transit = state->trtbl + evid;

    DEBUG("[%s] recieved '%s' event in '%s' state",
          sm->name, event->name, state->name);

    sm->busy = TRUE;

    if (transit->func == NULL)
        next_stid = transit->stid;
    else {
        DEBUG("[%s] executing transition function '%s'",
              sm->name, transit->fname);

        if (transit->func(evdata, sm->data)) {
            DEBUG("[%s] transition function '%s' succeeded",
                  sm->name, transit->fname);

            next_stid = transit->stid;
        }
        else {
            DEBUG("[%s] transition function '%s' failed",
                  sm->name, transit->fname);

            next_stid = stid;
        }

    }

    if (next_stid < 0) {
        DEBUG("[%s] conditional transition. '%s' is called to find "
              "the next state", sm->name, transit->cname);

        next_stid = transit->cond(sm->data);

        DEBUG("[%s] '%s' returned %d", sm->name, transit->cname, next_stid);

        if (next_stid < 0)
            next_stid = stid;
    }

    next_state = sm_def.stdef + next_stid;

    DEBUG("[%s] %s '%s' state", sm->name,
          (next_stid == stid) ? "stays in" : "goes to", next_state->name);

    sm->stid = next_stid;
    sm->busy = FALSE;

    return TRUE;
}

static void sm_schedule_event(sm_t *sm, sm_evdata_t *evdata,sm_evfree_t evfree)
{
    sm_schedule_t *schedule;

    if (!sm || !evdata) {
        DEBUG("[%s] failed to schedule event: <null> argument", sm->name);
        return;
    }

    if (evdata->evid < 0 || evdata->evid >= evid_max) {
        DEBUG("[%s] failed to schedule event: invalid event ID %d",
              sm->name, evdata->evid);
        return;
    }

    if (sm->sched != 0) {
        DEBUG("[%s] failed to schedule event: multiple requests", sm->name);
        return;
    }

    if ((schedule = malloc(sizeof(*schedule))) == NULL) {
        DEBUG("[%s] failed to schedule event: malloc failed", sm->name);
        return;
    }

    schedule->sm     = sm;
    schedule->evdata = evdata;
    schedule->evfree = evfree;

    sm->sched = g_idle_add(fire_scheduled_event, schedule);

    if (sm->sched == 0) {
        DEBUG("[%s] failed to schedule event: g_idle_add() failed", sm->name);

        free(schedule);

        if (evfree != NULL)
            evfree(evdata);
    }
    else {
        DEBUG("[%s] schedule event '%s'", sm->name, evdef[evdata->evid].name);
    }
}

static void verify_state_machine()
{
    sm_stdef_t   *stdef;
    sm_transit_t *tr;
    int           i, j;
    int           verified;

    verified = TRUE;

    for (i = 0;  i < evid_max;  i++) {
        if (evdef[i].id != i) {
            DEBUG("event definition entry %d is in wron position", i);
            verified = FALSE;
        }
    }
    
    if (sm_def.stid < 0 || sm_def.stid >= stid_max) {
        DEBUG("Initial state %d is out of range (0 - %d)",
              sm_def.stid, stid_max-1);
        verified = FALSE;
    }

    for (i = 0;   i < stid_max;   i++) {
        stdef = sm_def.stdef + i;

        if (stdef->id != i) {
            verified = FALSE;
            DEBUG("stid mismatch at sm.stdef[%d]", i);
            continue;
        }

        for (j = 0;   j < evid_max;   j++) {
            tr = stdef->trtbl + j;

            if (tr->stid < 0) {
                if (tr->cond == NULL) {
                    verified = FALSE;
                    DEBUG("Missing cond() at state '%s' for event '%s'",
                          stdef->name, evdef[j].name);
                }
            }
            else {
                if (tr->stid >= stid_max) {
                    verified = FALSE;
                    DEBUG("invalid transition at state '%s' for event '%s': "
                          "state %d is out of range (0-%d)", stdef->name,
                          sm_def.evdef[j].name, tr->stid, stid_max-1);
                }
                if (tr->cond) {
                    verified = FALSE;
                    DEBUG("can't set 'stid' to a valid state & specify 'cond' "
                          "at the same time in state '%s' for event '%s'",
                          stdef->name, evdef[j].name);
                }
            }
        }
    }

    if (!verified)
        exit(EINVAL);
}

static int fire_scheduled_event(void *data)
{
    sm_schedule_t *schedule = (sm_schedule_t *)data;
    sm_t          *sm       = schedule->sm;
    sm_evdata_t   *evdata   = schedule->evdata;

    sm->sched = 0;

    DEBUG("[%s] fire scheduled event", sm->name);

    sm_process_event(sm, evdata);

    if (schedule->evfree != NULL)
        schedule->evfree(evdata);

    free(schedule);

    return FALSE;               /* run only once */
}


static void free_evdata(sm_evdata_t *evdata)
{
#define FREE(d)                 \
    do {                        \
        if ((d) != NULL)        \
            free((void *)(d));  \
    } while(0)

    if (evdata != NULL) {
        switch (evdata->evid) {

        case evid_state_signal:
        case evid_property_received:
            FREE(evdata->property.name);
            FREE(evdata->property.value);
            break;

        default:
            break;
        }

        free(evdata);
    }

#undef FREE
}

static void fire_hello_signal_event(char *dbusid, char *object)
{
    sm_evdata_t  evdata;
    client_t    *cl;

    cl = client_create(dbusid, object);

    evdata.evid = evid_hello_signal;
    sm_process_event(cl->sm, &evdata);        
}

static void fire_state_signal_event(char *dbusid, char *object,
                                    char *prname, char *value)
{
    sm_evdata_t  evdata;
    client_t    *cl;

    if ((cl = client_find(dbusid, object)) == NULL)
        DEBUG("Can't find client for %s%s", dbusid, object);
    else {
        evdata.property.evid  = evid_state_signal;
        evdata.property.name  = prname; /* supposed to be 'State' */
        evdata.property.value = value;

        sm_process_event(cl->sm, &evdata);
    }
}

static int read_property(sm_evdata_t *evdata, void *usrdata)
{
    client_t  *cl = (client_t *)usrdata;

    client_get_property(cl, "PID"  , read_property_cb);
    client_get_property(cl, "Class", read_property_cb);
    client_get_property(cl, "State", read_property_cb);

    return TRUE;
}

static int save_property(sm_evdata_t *evdata, void *usrdata)
{
    static sm_evdata_t setup_complete = { .evid = evid_setup_complete };

    sm_evdata_property_t *property = &evdata->property;
    client_t  *cl = (client_t *)usrdata;
    pid_t      pid;
    char      *end;
    char      *group;
    char       state[64];

    if (!strcmp(property->name, "PID")) {
        pid = strtoul(property->value, &end, 10);

        if (*end != '\0')
            DEBUG("invalid pid value '%s'", property->value);
        else {
            cl->pid = pid;
            client_update_factstore_entry(cl, "pid", property->value);

            DEBUG("playback pid is set to %ul", cl->pid);
        }
    }
    else if (!strcmp(property->name, "Class")) {
        group = class_to_group(property->value);

        cl->group = strdup(group);
        client_update_factstore_entry(cl, "group", cl->group);
        
        DEBUG("playback group is set to %s", cl->group);
    }
    else if (!strcmp(property->name, "State")) {
        strncpylower(state, property->value, sizeof(state));

        cl->state = strdup(state);
        client_update_factstore_entry(cl, "state", cl->state);
        
        DEBUG("playback state is set to %s", cl->state);
    }
    else {
        DEBUG("Do not know anything about property '%s'", property->name);
    }

    if (cl->group != NULL && cl->state != NULL) {
        sm_schedule_event(cl->sm, &setup_complete, NULL);
    }

    return TRUE;
}


static int process_pbreq(sm_evdata_t *evdata, void *usrdata)
{
    static sm_evdata_t  static_evdata = { .evid = evid_playback_failed };

    client_t    *cl = (client_t *)usrdata;
    sm_t        *sm = cl->sm;
    pbreq_t     *req;
    sm_evdata_t *evrply;
    sm_evfree_t  evfree;
    char         state[64];

    if ((req = pbreq_get_first(cl)) == NULL)
        goto request_failure;

    switch (req->type) {

    case pbreq_state:
        strncpylower(state, req->state.name, sizeof(state));

        if (!dresif_state_request(cl, state, req->trid))
            goto request_failure;

        client_update_factstore_entry(cl, "reqstate", state);

        break;

    default:
        DEBUG("[%s] invalid playback request type %d", sm->name, req->type);
        goto request_failure;
    }

    return TRUE;

 request_failure:
    if (req == NULL) {
        evrply = &static_evdata;
        evfree = NULL;
    }
    else {
        evrply = malloc(sizeof(*evrply));
        evfree = free_evdata;

        if (evrply == NULL) {
            DEBUG("[%s] failed to schedule '%s' event: malloc() failed",
                  sm->name, evdef[evid_playback_failed].name);
            return FALSE;
        }

        memset(evrply, 0, sizeof(*evrply));
        evrply->pbreply.evid = evid_playback_failed;
        evrply->pbreply.req  = req;
    }

    sm_schedule_event(sm, evrply, evfree);

    return FALSE;               /* remain in the same state */
}


static int reply_pbreq(sm_evdata_t *evdata, void *usrdata)
{
    client_t *cl  = (client_t *)usrdata;
    sm_t     *sm  = cl->sm;
    pbreq_t  *req = evdata->pbreply.req;

    switch (req->type) {

    case pbreq_state:
        dbusif_reply_to_req_state(req->msg, req->state.name);
        break;

    default:
        DEBUG("[%s] invalid request type %d", sm->name, req->type);
        break;
    }

    pbreq_destroy(req);

    return TRUE;
}


static int reply_pbreq_deq(sm_evdata_t *evdata, void *usrdata)
{
    client_t *cl  = (client_t *)usrdata;

    reply_pbreq(evdata, usrdata);

    schedule_next_request(cl);

    return TRUE;
}


static int abort_pbreq_deq(sm_evdata_t *evdata, void *usrdata)
{
    client_t *cl  = (client_t *)usrdata;
    sm_t     *sm  = cl->sm;
    pbreq_t  *req = evdata->pbreply.req;

    if (req != NULL) {
        switch (req->type) {

        case pbreq_state:
            dbusif_reply_to_req_state(req->msg, "Stop");
            break;

        default:
            DEBUG("[%s] invalid request type %d", sm->name, req->type);
            break;
        }

        pbreq_destroy(req);
    }

    schedule_next_request(cl);

    return TRUE;
}

static int check_queue(sm_evdata_t *evdata, void *usrdata)
{
    client_t *cl  = (client_t *)usrdata;

    schedule_next_request(cl);

    return TRUE;
}

static int update_state(sm_evdata_t *evdata, void *usrdata)
{
    sm_evdata_property_t *property = &evdata->property;
    client_t             *cl       = (client_t *)usrdata;
    char                  state[64];

    strncpylower(state, property->value, sizeof(state));
    client_update_factstore_entry(cl, "state", state);

    DEBUG("playback state is set to %s", state);

    return TRUE;
}

static int update_state_deq(sm_evdata_t *evdata, void *usrdata)
{
    client_t *cl = (client_t *)usrdata;

    update_state(evdata, usrdata);

    schedule_next_request(cl);

    return TRUE;
}

static void read_property_cb(char *dbusid, char *object,
                             char *prname, char *prvalue)
{
    client_t             *cl;
    sm_evdata_t           evdata;
    sm_evdata_property_t *property = &evdata.property;

    if ((cl = client_find(dbusid, object)) == NULL) {
        DEBUG("Can't find client %s%s any more", dbusid, object);
        return;
    }

    property->evid  = evid_property_received;
    property->name  = prname;
    property->value = prvalue; 

    sm_process_event(cl->sm, &evdata);
}

static char *strncpylower(char *to, const char *from, int tolen)
{
    const char *p;
    char *q, *e, c;

    p = from;
    e = (q = to) + tolen - 1;

    while ((c = *p++) && q < e)
        *q++ = tolower(c);
    *q = '\0';
    
    return to;
}

static char *class_to_group(char *klass)
{
    static struct {char *klass; char *group;}  map[] = {
        {"None"      , "othermedia"},
        {"Test"      , "othermedia"},
        {"Event"     , "ringtone"  },
        {"VoIP"      , "ipcall"    },
        {"Media"     , "player"    },
        {"Background", "othermedia"},
        {NULL        , "othermedia"}
    };

    int i;

    for (i = 0;   map[i].klass != NULL;   i++) {
        if (!strcmp(klass, map[i].klass))
            break;
    }

    return map[i].group;
}


static void schedule_next_request(client_t *cl)
{
    static sm_evdata_t  evdata = { .evid = evid_playback_request };

    if (pbreq_get_first(cl))
        sm_schedule_event(cl->sm, &evdata, NULL);
}

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

