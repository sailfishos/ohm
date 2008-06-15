#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <ohm-plugin.h>

#include "playback.h"
#include "client.h"
#include "pbreq.h"
#include "sm.h"
#include "dbusif.h"
#include "dresif.h"
#include "fsif.h"


static void plugin_init(OhmPlugin *);
static void plugin_destroy(OhmPlugin *);


static void plugin_init(OhmPlugin *plugin)
{
    client_init(plugin);
    pbreq_init(plugin);
    sm_init(plugin);
    dbusif_init(plugin);
    dresif_init(plugin);
    fsif_init(plugin);
}

static void plugin_destroy(OhmPlugin *plugin)
{

}


#include "client.c"
#include "pbreq.c"
#include "sm.c"
#include "dbusif.c"
#include "dresif.c"
#include "fsif.c"


OHM_PLUGIN_REQUIRES_METHODS(playback, 1, 
   OHM_IMPORT("dres.resolve", resolve)
);

OHM_PLUGIN_PROVIDES_METHODS(playback, 1,
   OHM_EXPORT(completion_cb, "completion_cb")
);

OHM_PLUGIN_DESCRIPTION(
    "OHM media playback",             /* description */
    "0.0.1",                          /* version */
    "janos.f.kovacs@nokia.com",       /* author */
    OHM_LICENSE_NON_FREE,             /* license */
    plugin_init,                      /* initalize */
    plugin_destroy,                   /* destroy */
    NULL                              /* notify */
);

OHM_PLUGIN_PROVIDES(
    "maemo.playback"
);


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
