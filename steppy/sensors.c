#include "sensors.h"

gint32
request_sensor(SteppyApp *steppy)
{
    GVariant *result;
    GError *error = NULL;
    gboolean standby_success = FALSE;
    gint32 session_id = -1;

    // Ensure the plugin is loaded
    result = g_dbus_connection_call_sync(steppy->dbus_connection,
                                         "com.nokia.SensorService",
                                         "/SensorManager",
                                         "local.SensorManager",
                                         "loadPlugin",
                                         g_variant_new("(s)", "stepcountersensor"),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Failed to load plugin: %s", error->message);
        g_error_free(error);
        return -1;
    }

    g_variant_unref(result);

    result = g_dbus_connection_call_sync(steppy->dbus_connection,
                                         "com.nokia.SensorService",
                                         "/SensorManager",
                                         "local.SensorManager",
                                         "requestSensor",
                                         g_variant_new("(sx)", "stepcountersensor", (gint64)getpid()),
                                         G_VARIANT_TYPE("(i)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Failed to request sensor: %s", error->message);
        g_error_free(error);
        return -1;
    }

    g_variant_get(result, "(i)", &session_id);
    g_variant_unref(result);

    result = g_dbus_connection_call_sync(steppy->dbus_connection,
                                         "com.nokia.SensorService",
                                         "/SensorManager/stepcountersensor",
                                         "local.StepCounterSensor",
                                         "setStandbyOverride",
                                         g_variant_new("(ib)", session_id, TRUE),
                                         G_VARIANT_TYPE("(b)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Failed to start sensor: %s", error->message);
        g_error_free(error);
    }

    g_variant_get(result, "(b)", &standby_success);
    g_variant_unref(result);

    if (!standby_success) {
        g_warning("Failed to set standby override");
    }

    result = g_dbus_connection_call_sync(steppy->dbus_connection,
                                         "com.nokia.SensorService",
                                         "/SensorManager/stepcountersensor",
                                         "local.StepCounterSensor",
                                         "start",
                                         g_variant_new("(i)", session_id),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Failed to start sensor: %s", error->message);
        g_error_free(error);
    }

    g_variant_unref(result);

    return session_id;
}

void
release_sensor(SteppyApp *steppy, gint32 session_id)
{
    GVariant *result;
    GError *error = NULL;

    result = g_dbus_connection_call_sync(steppy->dbus_connection,
                                         "com.nokia.SensorService",
                                         "/SensorManager/stepcountersensor",
                                         "local.StepCounterSensor",
                                         "stop",
                                         g_variant_new("(i)", session_id),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Failed to stop sensor: %s", error->message);
        g_error_free(error);
    }

    if (result) {
        g_variant_unref(result);
    }

    result = g_dbus_connection_call_sync(steppy->dbus_connection,
                                         "com.nokia.SensorService",
                                         "/SensorManager",
                                         "local.SensorManager",
                                         "releaseSensor",
                                         g_variant_new("(six)", "stepcountersensor", session_id, (gint64)getpid()),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Failed to release sensor: %s", error->message);
        g_error_free(error);
    }

    if (result) {
        g_variant_unref(result);
    }
}

guint32
get_sensor_reading(SteppyApp *steppy)
{
    GVariant *result;
    GError *error = NULL;
    guint32 steps = 0;
    guint32 retval;

    result = g_dbus_connection_call_sync(steppy->dbus_connection,
                                         "com.nokia.SensorService",
                                         "/SensorManager/stepcountersensor",
                                         "org.freedesktop.DBus.Properties",
                                         "Get",
                                         g_variant_new("(ss)", "local.StepCounterSensor", "steps"),
                                         G_VARIANT_TYPE("(v)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Failed to get sensor reading: %s", error->message);
        g_error_free(error);
        return 0;
    }

    GVariant *value;
    g_variant_get(result, "(v)", &value);
    
    guint64 timestamp;
    g_variant_get(value, "(tu)", &timestamp, &steps);

    g_variant_unref(value);
    g_variant_unref(result);

    retval = steps - steppy->last_reading;
    steppy->last_reading = steps;

    return retval;
}
