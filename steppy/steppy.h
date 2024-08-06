#ifndef __STEPPY_H__
#define __STEPPY_H__

#include <adwaita.h>
#include <sqlite3.h>
#include <gio/gio.h>

#define APP_ID "io.furios.Steppy"
#define UPDATE_INTERVAL 1

typedef struct {
    AdwApplication *app;
    GDBusConnection *dbus_connection;
    sqlite3 *db;
    guint step_count;
    GTimeZone *timezone;
    AdwApplicationWindow *window;
    AdwActionRow *step_row;
    GtkLabel *step_label;
    GSettings *settings;
    guint update_source_id;
    gint32 session_id;
    guint last_reading;
} SteppyApp;

#endif // __STEPPY_H__
