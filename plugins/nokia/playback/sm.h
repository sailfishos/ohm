#ifndef __OHM_PLAYBACK_SM_H__
#define __OHM_PLAYBACK_SM_H__

typedef enum {
    evid_same_state = -1,
    evid_invalid = 0,
    evid_hello_signal,
    evid_state_signal,
    evid_property_received,
    evid_setup_complete,
    evid_playback_request,
    evid_playback_complete,
    evid_playback_failed,
    evid_client_gone,
    evid_max
} sm_evid_t;

typedef enum {
    stid_invalid = 0,
    stid_setup,
    stid_idle,
    stid_pbreq,
    stid_acked_pbreq,
    stid_waitack,
    stid_max
} sm_stid_t;

typedef struct {
    char         *name;       /* name of the state machine instance */
    sm_stid_t     stid;       /* ID of the current state */
    int           busy;       /* to prevent nested event processing */
    unsigned int  sched;      /* event source for scheduled event if any */
    void         *data;       /* passed to trfunc() as second arg */
} sm_t;

/*
 * event data
 */
typedef struct {
    sm_evid_t  evid;
    char      *name;
    char      *value;
} sm_evdata_property_t;

typedef struct {
    sm_evid_t       evid;
    struct pbreq_s *req;
} sm_evdata_pbreply_t;

typedef union {
    sm_evid_t             evid;
    sm_evdata_property_t  property;
    sm_evdata_pbreply_t   pbreply;
} sm_evdata_t;

typedef void (*sm_evfree_t)(sm_evdata_t *);

/*
 * state machine public interfaces
 */

static void  sm_init(OhmPlugin *);
static sm_t *sm_create(char *, void *);
static int   sm_destroy(sm_t *);
static int   sm_process_event(sm_t *, sm_evdata_t *);
static void  sm_schedule_event(sm_t *, sm_evdata_t *, sm_evfree_t);


#endif /* __OHM_PLAYBACK_SM_H__ */

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

