#include <stdio.h>
#include <string.h>

#include <gmodule.h>
#include <glib.h>
#include <dbus/dbus.h>

#include <ohm/ohm-plugin.h>

/* set/relation names, policy/prolog atoms */
#define ACTIVE_CHANNELS    "active_channels"
#define AUDIO_ROUTE        "audio_route"
#define ACTION_AUDIO_ROUTE "audio_route"

#define CHANNEL_OTHER "othermedia"

OHM_IMPORTABLE(int , policy_subscribe, (void (*)(char ***)));
OHM_IMPORTABLE(int , set_insert      , (char *, char *));
OHM_IMPORTABLE(int , set_delete      , (char *, char *));
OHM_IMPORTABLE(void, set_reset       , (char *));
OHM_IMPORTABLE(int , relation_insert , (char *, char **));
OHM_IMPORTABLE(int , relation_delete , (char *, char **));
OHM_IMPORTABLE(void, relation_reset  , (char *));

static void parse_actions(char ***actions);


static void
plugin_init(OhmPlugin *plugin)
{
    if (!policy_subscribe(parse_actions))
        g_error("Failed to subscribe for policy actions.");
    if (set_insert(ACTIVE_CHANNELS, CHANNEL_OTHER))
        g_error("Failed to initialize active channels.");
}


static void
plugin_exit(OhmPlugin *plugin)
{
    return;
}


static void
parse_actions(char ***actions)
{
    int    i;
    char  *what, *where;
    char  *del[2]  = { NULL, NULL }, *add[3] = { NULL, NULL, NULL };
    
    for (i = 0; actions[i]; i++) {
        if (strcmp(actions[i][0], ACTION_AUDIO_ROUTE))
            continue;
        
        what   = actions[i][1];
        where  = actions[i][2];

        del[0] = what;
        add[0] = what, add[1] = where;
        relation_delete(AUDIO_ROUTE, del);
        relation_insert(AUDIO_ROUTE, add);

        printf("audioroute: %s routed to %s\n", what, where);
    }
}


OHM_PLUGIN_DESCRIPTION("audioroute",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_REQUIRES_METHODS(audioroute, 7,
    OHM_IMPORT("policy.subscribe"      , policy_subscribe),
    OHM_IMPORT("policy.set_insert"     , set_insert),
    OHM_IMPORT("policy.set_delete"     , set_delete),
    OHM_IMPORT("policy.set_reset"      , set_reset),
    OHM_IMPORT("policy.relation_insert", relation_insert),
    OHM_IMPORT("policy.relation_delete", relation_delete),
    OHM_IMPORT("policy.relation_reset" , relation_reset));



/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
