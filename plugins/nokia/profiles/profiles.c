#include <stdio.h>
#include <string.h>

#include <gmodule.h>
#include <glib.h>
#include <dbus/dbus.h>

#include <ohm-plugin.h>

/* set/relation names, policy/prolog atoms */
#define CURRENT_PROFILE "current_profile"
#define PROFILE_SILENT  "silent"
#define PROFILE_MEETING "meeting"
#define PROFILE_GENERAL "general"
#define PROFILE_OUTDOOR "outdoor"

#define DBUS_INTERFACE_POLICY "com.nokia.policy"

static char ***(*policy_actions)(DBusMessage *, int);

static int     (*set_insert)(char *, char *);
static int     (*set_delete)(char *, char *);
static void    (*set_reset) (char *);

static int     (*relation_insert)(char *, char **);
static int     (*relation_delete)(char *, char **);
static void    (*relation_reset) (char *);


static void
plugin_init(OhmPlugin *plugin)
{
    if (set_insert(CURRENT_PROFILE, PROFILE_GENERAL))
        g_error("Failed to initialize current profile.");
}


static void
plugin_exit(OhmPlugin *plugin)
{
    return;
}


static DBusHandlerResult
profile_changed(DBusConnection *c, DBusMessage *msg, void *data)
{
    const char *interface = dbus_message_get_interface(msg);
    const char *member    = dbus_message_get_member(msg);

    char       *name;
    DBusError   error;
    
    if (member == NULL || strcmp(member, "profile_changed"))
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (interface == NULL || strcmp(interface, DBUS_INTERFACE_POLICY))
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    
    dbus_error_init(&error);
    if (!dbus_message_get_args(msg, &error,
                               DBUS_TYPE_STRING, &name,
                               DBUS_TYPE_INVALID)) {
        g_warning("Failed to parse profile change signal (%s)", error.message);
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    dbus_error_free(&error);
    
    
    set_reset(CURRENT_PROFILE);
    set_insert(CURRENT_PROFILE, name);
    
    printf("current profile changed to %s\n", name);

    policy_actions(NULL, FALSE);

    return DBUS_HANDLER_RESULT_HANDLED;
}



OHM_PLUGIN_DESCRIPTION("profiles",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_REQUIRES_METHODS(
    { 0, "policy.actions"  , NULL, (void *)&policy_actions },
    { 0, "policy.set_insert", NULL, (void *)&set_insert  },
    { 0, "policy.set_delete", NULL, (void *)&set_delete  },
    { 0, "policy.set_reset" , NULL, (void *)&set_reset   },
    { 0, "policy.relation_insert", NULL, (void *)&relation_insert },
    { 0, "policy.relation_delete", NULL, (void *)&relation_delete },
    { 0, "policy.relation_reset" , NULL, (void *)&relation_reset  });



OHM_PLUGIN_DBUS_SIGNALS(
    { NULL, DBUS_INTERFACE_POLICY, "profile_changed", NULL,
            profile_changed, NULL });


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
