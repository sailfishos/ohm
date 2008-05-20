/**
 * @file signaling.c
 * @brief OHM signaling plugin 
 * @author ismo.h.puustinen@nokia.com
 *
 * Copyright (C) 2008, Nokia. All rights reserved.
 */

#include "signaling.h"
#include <ohm-plugin.h>

/* public API (inside OHM) */

OHM_EXPORTABLE(EnforcementPoint *, register_internal_enforcement_point, (gchar *uri))
{
    EnforcementPoint *ep = register_enforcement_point(uri, TRUE);

    /* ref so that the ep won't be deleted before caller has unreffed it */
    g_object_ref(ep);
    return ep;
}
    
OHM_EXPORTABLE(gboolean, unregister_internal_enforcement_point, (EnforcementPoint *ep))
{
    gchar *uri;
    gboolean ret;
    g_object_get(ep, "id", &uri, NULL);
    ret = unregister_enforcement_point(uri);
    g_free(uri);
    return ret;
}

OHM_EXPORTABLE(Transaction *, queue_policy_decision, (char ***actions, guint timeout))
{
    return queue_decision(actions, TRUE, timeout);
}

OHM_EXPORTABLE(void, queue_key_change, (char ***actions))
{
    queue_decision(actions, FALSE, 0);
    return;
}


/* init and exit */

    static void
plugin_init(OhmPlugin * plugin)
{
    DBusConnection *c = ohm_plugin_dbus_get_connection();
    /* should we ref the connection? */
    init_signaling(c);
    return;
}

    static void
plugin_exit(OhmPlugin * plugin)
{
    deinit_signaling();
    return;
}

OHM_PLUGIN_DESCRIPTION("signaling",
        "0.0.1",
        "ismo.h.puustinen@nokia.com",
        OHM_LICENSE_NON_FREE, plugin_init, plugin_exit,
        NULL);

OHM_PLUGIN_PROVIDES_METHODS(signaling, 4,
        OHM_EXPORT(register_internal_enforcement_point, "register_enforcement_point"),
        OHM_EXPORT(unregister_internal_enforcement_point, "unregister_enforcement_point"),
        OHM_EXPORT(queue_policy_decision, "queue_policy_decision"),
        OHM_EXPORT(queue_key_change, "queue_key_change"));

OHM_PLUGIN_DBUS_SIGNALS(
        {NULL, DBUS_INTERFACE_POLICY, SIGNAL_POLICY_ACK,
            NULL, dbus_ack, NULL},
        {NULL, DBUS_INTERFACE_FDO, SIGNAL_NAME_OWNER_CHANGED,
            NULL, update_external_enforcement_points, NULL}
        );

OHM_PLUGIN_DBUS_METHODS(
        {DBUS_PATH_POLICY, METHOD_POLICY_REGISTER,
            register_external_enforcement_point, NULL},
        {DBUS_PATH_POLICY, METHOD_POLICY_UNREGISTER,
            unregister_external_enforcement_point, NULL}
        );

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
