OHM_IMPORTABLE(int, resolve, (char *goal, char **locals));


static void dresif_init(OhmPlugin *plugin)
{
}


static int dresif_state_request(client_t *pb, char *state, int cb)
{
    char *vars[32];
    int   i;
    int   err;

    client_update_factsore_entry(pb, "state", state);

    vars[i=0] = "playback_state";
    vars[++i] = state;
    vars[++i] = "playback_group";
    vars[++i] = pb->group;
    vars[++i] = "playback_media";
    vars[++i] = "unknown";

    if (cb) {
        vars[++i] = "completion_callback";
        vars[++i] = "playback.completion_cb";
    }

    vars[++i] = NULL;

    if ((err = resolve("audio_playback_request", vars)) != 0)
        DEBUG("resolve() failed: (%d) %s", err, strerror(err));

    return err ? FALSE : TRUE;
}

OHM_EXPORTABLE(void, completion_cb, (int transid, int success))
{
    static char *stop = "Stop";

    pbreq_t     *req;
    client_t    *pb;

    DEBUG("*** playback.%s(%d, %s)\n", __FUNCTION__,
           transid, success ? "OK":"FAILED");

    if ((req = pbreq_get_first()) == NULL) {
        DEBUG("libplayback completion_cb: Can't find the request");
        return;
    }

    if (req->sts != pbreq_pending) {
        DEBUG("libplayback completion_cb: bad state %d", req->sts);
        return;
    }

    pb = req->pb;
    
    if (pb->state != NULL)
        free(pb->state);
    pb->state = strdup(success ? req->state : stop);

    dbusif_reply_to_req_state(req->msg, pb->state);

    pbreq_destroy(req);
}


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
