#include <stdio.h>

#include <gmodule.h>
#include <glib.h>

#include <dbus/dbus.h>

#if 0
#include <policy/set.h>
#include <policy/relation.h>
#endif

#include <ohm/ohm-plugin.h>

#define DBUS_PATH_POLICY     "/com/nokia/policy"
#define METHOD_CALL_PROGRESS "call_progress"

/* set/relation names, policy/prolog atoms */
#define ACTIVE_CHANNELS    "active_channels"
#define PRIVACY            "privacy"
#define PRIVATE            "private"
#define PUBLIC             "public"

#define ENTITY_CSCALL      "cscall"
#define REQUEST_RINGING    "ringing"
#define REQUEST_CONNECT    "connected"
#define REQUEST_DISCONNECT "disconnected"
#define REQUEST_PUBLIC     "public"
#define REQUEST_PRIVATE    "private"




enum {
    RINGING = 0,
    CONNECT,
    DISCONNECT,
    PRIVACY_PUBLIC,
    PRIVACY_PRIVATE,
};

enum {
    CHANNEL_RINGTONE = 0,
    CHANNEL_CSCALL,
    CHANNEL_OTHER
};

static char *channel_names[] = {
    [CHANNEL_RINGTONE] = "ringtone",
    [CHANNEL_CSCALL]   = "cscall",
    [CHANNEL_OTHER]    = "othermedia"
};

static int active_channels;


static void activate  (int channel);
static void deactivate(int channel);
static int  is_active (int channel);


static DBusHandlerResult call_progress(DBusConnection *c,
                                       DBusMessage *msg, void *data);


OHM_IMPORTABLE(int     , policy_request  , (char *, char *));
OHM_IMPORTABLE(int     , policy_actions  , (DBusMessage *, int));
OHM_IMPORTABLE(int     , set_insert      , (char *, char *));
OHM_IMPORTABLE(int     , set_delete      , (char *, char *));
OHM_IMPORTABLE(void    , set_reset       , (char *));
OHM_IMPORTABLE(int     , relation_insert , (char *, char **));
OHM_IMPORTABLE(int     , relation_delete , (char *, char **));
OHM_IMPORTABLE(void    , relation_reset  , (char *));

static int ringing   (void);
static int connect   (void);
static int disconnect(void);

static int privacy_public (void);
static int privacy_private(void);
static int privacy_default(void);



static void
plugin_init(OhmPlugin *plugin)
{
    return;
}


static void
plugin_exit(OhmPlugin *plugin)
{
    return;
}



/********************
 * call_progress
 ********************/
static DBusHandlerResult
call_progress(DBusConnection *c, DBusMessage *msg, void *data)
{
    DBusError  error;
    int        permit, state;

    dbus_error_init(&error);
    
    if (dbus_message_get_args(msg, &error,
                              DBUS_TYPE_UINT32, &state, DBUS_TYPE_INVALID)) {

        printf("call progress state: %u\n", state);
        
        switch (state) {
        case RINGING:         permit = ringing();         break;
        case CONNECT:         permit = connect();         break;
        case DISCONNECT:      permit = disconnect();      break;
        case PRIVACY_PUBLIC:  permit = privacy_public();  break;
        case PRIVACY_PRIVATE: permit = privacy_private(); break;
        default:              permit = FALSE;             break;
        }
    }
    else {
        g_warning("%s: Failed to parse DBUS message (%s)", __FUNCTION__,
                  error.message);
        permit = FALSE;
    }
    
    dbus_error_free(&error);

    printf("got call progress permit: %s\n", permit ? "TRUE" : "FALSE");

    if (!permit) {
        /* answer back with FALSE */;
    }
    
    
    policy_actions(msg, state == DISCONNECT);
    
    return DBUS_HANDLER_RESULT_HANDLED;
}


static void
activate(int channel)
{
    active_channels |= (1 << channel);
    set_insert(ACTIVE_CHANNELS, channel_names[channel]);
}

static void
deactivate(int channel)
{
    active_channels &= ~(1 << channel);
    set_delete(ACTIVE_CHANNELS, channel_names[channel]);
}


static int
is_active(int channel)
{
    return active_channels & (1 << channel);
}


static int
ringing(void)
{
    if (!policy_request(ENTITY_CSCALL, REQUEST_RINGING))
        return FALSE;

    activate(CHANNEL_RINGTONE);

    return TRUE;
}


static int
connect(void)
{
    if (!policy_request(ENTITY_CSCALL, REQUEST_CONNECT))
        return FALSE;
    
    deactivate(CHANNEL_RINGTONE);
    activate(CHANNEL_CSCALL);

    return TRUE;
}


static int
disconnect(void)
{
    if (!policy_request(ENTITY_CSCALL, REQUEST_DISCONNECT))
        return FALSE;
    
    privacy_default();
    deactivate(CHANNEL_RINGTONE);
    deactivate(CHANNEL_CSCALL);
    
    return TRUE;
}


static int
privacy_public(void)
{
    if (!is_active(CHANNEL_CSCALL))
        return FALSE;
    
    set_reset(PRIVACY);
    set_insert(PRIVACY, PUBLIC);

    return TRUE;
}


static int
privacy_private(void)
{
    if (!is_active(CHANNEL_CSCALL))
        return FALSE;
    
    set_reset(PRIVACY);
    set_insert(PRIVACY, PRIVATE);
    
    return TRUE;
}


static int
privacy_default(void)
{
    set_reset(PRIVACY);

    return TRUE;
}


OHM_PLUGIN_DESCRIPTION("cscall",
                       "0.0.0",
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_REQUIRES_METHODS(cscall, 8,
    OHM_IMPORT("policy.request"        , policy_request),
    OHM_IMPORT("policy.actions"        , policy_actions),
    OHM_IMPORT("policy.set_insert"     , set_insert),
    OHM_IMPORT("policy.set_delete"     , set_delete),
    OHM_IMPORT("policy.set_reset"      , set_reset),
    OHM_IMPORT("policy.relation_insert", relation_insert),
    OHM_IMPORT("policy.relation_delete", relation_delete),
    OHM_IMPORT("policy.relation_reset" , relation_reset));

OHM_PLUGIN_DBUS_METHODS(
    { DBUS_PATH_POLICY, METHOD_CALL_PROGRESS, call_progress, NULL });
    



/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
