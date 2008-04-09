#include <stdio.h>
#include <string.h>

#include <gmodule.h>
#include <glib.h>
#include <dbus/dbus.h>

#include <ohm-plugin.h>

/* set/relation names, policy/prolog atoms */
#define ACCESSORIES "accessories"

#define EARPIECE   "earpiece"
#define IHF        "ihf"
#define MICROPHONE "microphone"
#define HEADSET    "headset"
#define HEADPHONE  "headphone"
#define BLUETOOTH  "bluetooth"
#define HEADMIKE   "headmike"
#define NODEV      "none"

enum {
    ID_HEADSET    = 0x00,
    ID_HEADPHONE  = 0x80,
    ID_LOOPSET    = 0x01,
    ID_CARFREE    = 0x02,
    ID_OFFICEFREE = 0x03,
    ID_NODEV      = 0xff
};

static char ***(*policy_actions)(DBusMessage *, int);

static int     (*set_insert)(char *, char *);
static int     (*set_delete)(char *, char *);
static void    (*set_reset) (char *);

static void    update_device(void);


static int   acc_type;
static char *acc_now, *acc_old;

char *hardwired[] = { EARPIECE, IHF, MICROPHONE, NULL };

enum {
    CONF_HEADSET_TYPE,
};

static void
plugin_init(OhmPlugin *plugin)
{
    char **dev;

    for (dev = hardwired; *dev; dev++)
        if (set_insert(ACCESSORIES, *dev))
            g_error("Failed to initialize set of accessories.");
        else
            printf("accessory %s added to ACCESSORIES...\n", *dev);

    ohm_plugin_conf_get_key(plugin, "headset.headset_type", &acc_type);

    acc_now = NODEV;
    update_device();
}


static void
plugin_exit(OhmPlugin *plugin)
{
    return;
}


static void
update_device(void)
{
    int type = acc_type &  0xff;
    int out  = acc_type >> 8;

    acc_old = acc_now;

    if (type == ID_HEADSET) {
        if (out == ID_NODEV)
            acc_now = HEADMIKE;
        else
            acc_now = HEADSET;
    }
    else {
        switch (type) {
        case ID_HEADSET:    acc_now = HEADSET;   break;
        case ID_LOOPSET:    acc_now = HEADSET;   break;
        case ID_CARFREE:    acc_now = HEADSET;   break;
        case ID_OFFICEFREE: acc_now = HEADSET;   break;
        case ID_HEADPHONE:  acc_now = HEADPHONE; break;
        case ID_NODEV:      acc_now = NODEV;     break;
        default:            acc_now = NODEV;     break;
        }
    }

    if (strcmp(acc_now, acc_old)) {
        printf("accessories: old: %s, new: %s\n", acc_old, acc_now);
        set_delete(ACCESSORIES, acc_old);
        set_insert(ACCESSORIES, acc_now);
        policy_actions(NULL, FALSE);
    }
}


static void
plugin_notify (OhmPlugin *plugin, gint id, gint value)
{
    if (id == CONF_HEADSET_TYPE)
        acc_type = value;
    
    update_device();
}



OHM_PLUGIN_DESCRIPTION("accessories",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       plugin_notify);

OHM_PLUGIN_REQUIRES_METHODS(
    { 0, "policy.actions"  , NULL, (void *)&policy_actions },
    { 0, "policy.set_insert", NULL, (void *)&set_insert    },
    { 0, "policy.set_delete", NULL, (void *)&set_delete    },
    { 0, "policy.set_reset" , NULL, (void *)&set_reset     });

OHM_PLUGIN_REQUIRES("headset");

OHM_PLUGIN_INTERESTED({ "headset.headset_type", CONF_HEADSET_TYPE });



/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
