#ifndef __STEPPY_H__
#define __STEPPY_H__

#include <adwaita.h>
#include <sqlite3.h>
#include <gio/gio.h>

#include "ui/widgets/progress.h"

#define APP_ID "io.furios.Steppy"
#define UPDATE_INTERVAL_FOREGROUND 1
#define UPDATE_INTERVAL_BACKGROUND 300

typedef struct {
    AdwApplication *app;
    GDBusConnection *dbus_connection;
    sqlite3 *db;
    guint step_count;
    GTimeZone *timezone;
    AdwApplicationWindow *window;
    GtkLabel *step_label;
    GtkLabel *step_target_label;
    GSettings *settings;
    guint update_source_id;
    gint32 session_id;
    guint last_reading;
    StpCircularProgress *progress_widget;
} SteppyApp;

#endif // __STEPPY_H__
