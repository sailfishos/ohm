
static pbreq_listhead_t  rq_head;

static void pbreq_init(OhmPlugin *plugin)
{
    rq_head.next = (pbreq_t *)&rq_head;
    rq_head.prev = (pbreq_t *)&rq_head;
}

static pbreq_t *pbreq_create(client_t *pb, DBusMessage *msg, const char *state)
{
    pbreq_t *req, *next, *prev;

    if (msg == NULL)
        req = NULL;
    else {
        if ((req = malloc(sizeof(*req))) != NULL) {
            dbus_message_ref(msg);

            memset(req, 0, sizeof(*req));            
            req->pb    = pb;
            req->sts   = pbreq_queued;
            req->msg   = msg;
            req->state = strdup(state);

            next = (pbreq_t *)&rq_head;
            prev = rq_head.prev;
            
            prev->next = req;
            req->next  = next;

            next->prev = req;
            req->prev  = prev;
        }
    }

    return req;
}

static void pbreq_destroy(pbreq_t *req)
{
    pbreq_t *prev, *next;

    if (req != NULL) {
        prev = req->prev;
        next = req->next;

        prev->next = req->next;
        next->prev = req->prev;

        if (req->msg != NULL)
            dbus_message_unref(req->msg);

        if (req->state)
            free(req->state);

        free(req);
    }
}

static pbreq_t *pbreq_get_first(void)
{
    pbreq_t *req = rq_head.next;

    if (req == (pbreq_t *)&rq_head)
        req = NULL;

    return req;
}

static int pbreq_process(void)
{
    pbreq_t  *req;
    client_t *pb;
    int       sts;
    char      state[64];
    char     *p, *q, *e, c;

    if ((req = pbreq_get_first()) == NULL)
        sts = PBREQ_PENDING;
    else {
        pb = req->pb;

        if (pb->group == NULL)
            sts = PBREQ_PENDING;
        else {
            p = req->state;
            e = (q = state) + sizeof(state) - 1;
            while ((c = *p++) && q < e)
                *q++ = tolower(c);
            *q = '\0';

            switch (req->sts) {

            case pbreq_queued:
                if (dresif_state_request(pb, state, TRUE)) {
                    sts = PBREQ_PENDING;
                    req->sts = pbreq_pending;
                }
                else {
                    sts = PBREQ_ERROR;
                    req->sts = pbreq_handled;
                }
                break;

            case pbreq_pending:
                sts = PBREQ_PENDING;
                break;
                
            case pbreq_handled:
                sts = PBREQ_FINISHED;
                break;
                
            default:
                sts = PBREQ_ERROR;
                break;
            }
        }
    }

    return sts;
}

static void pbreq_purge(client_t *pb)
{
    pbreq_t *req, *nxreq;

    for (req = rq_head.next;   req != (pbreq_t *)&rq_head;   req = nxreq) {
        nxreq = req->next;

        if (pb == req->pb)
            pbreq_destroy(req);
    }
}

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
