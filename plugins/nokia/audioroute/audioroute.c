#include <stdio.h>
#include <string.h>

#include <gmodule.h>
#include <glib.h>
#include <dbus/dbus.h>

#include <ohm-plugin.h>

/* set/relation names, policy/prolog atoms */
#define ACTIVE_CHANNELS    "active_channels"
#define ACTION_AUDIO_ROUTE "audio_route"

#define CHANNEL_OTHER "othermedia"

static int     (*policy_subscribe)(void (*)(char ***));

static int     (*set_insert)(char *, char *);
static int     (*set_delete)(char *, char *);
static void    (*set_reset) (char *);

static int     (*relation_insert)(char *, char **);
static int     (*relation_delete)(char *, char **);
static void    (*relation_reset) (char *);

static void    parse_actions(char ***actions);


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
        relation_delete(ACTIVE_CHANNELS, del);
        relation_insert(ACTIVE_CHANNELS, add);

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

OHM_PLUGIN_REQUIRES_METHODS(
    { 0, "policy.subscribe", NULL, (void *)&policy_subscribe },
    { 0, "policy.set_insert", NULL, (void *)&set_insert  },
    { 0, "policy.set_delete", NULL, (void *)&set_delete  },
    { 0, "policy.set_reset" , NULL, (void *)&set_reset   },
    { 0, "policy.relation_insert", NULL, (void *)&relation_insert },
    { 0, "policy.relation_delete", NULL, (void *)&relation_delete },
    { 0, "policy.relation_reset" , NULL, (void *)&relation_reset  });


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
