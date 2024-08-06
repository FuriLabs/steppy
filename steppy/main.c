#include "steppy.h"
#include "sensors.h"
#include "db.h"
#include "utils.h"

void
update_step_count(SteppyApp *steppy)
{
    gint32 session_id = steppy->session_id;

    if (session_id == -1) {
        session_id = request_sensor(steppy);
        if (session_id == -1) {
            return;
        }

        steppy->session_id = session_id;
    }

    guint32 steps = get_sensor_reading(steppy);

    gchar *today = timestamp_to_local_date(steppy, get_current_timestamp(steppy));
    guint current_steps = get_steps_for_date(steppy, today);

    steppy->step_count = current_steps + steps;

    update_database(steppy);

    g_free(today);
}

static void
update_ui (SteppyApp *steppy)
{
    gchar *step_text = g_strdup_printf ("%d", steppy->step_count);
    adw_action_row_set_subtitle (steppy->step_row, step_text);
    g_free (step_text);

    gint target = g_settings_get_int (steppy->settings, "step-target");
    gchar *target_text = g_strdup_printf ("%d / %d", steppy->step_count, target);
    adw_action_row_set_subtitle (steppy->step_row, target_text);
    g_free (target_text);
}

static gboolean
update_callback (gpointer user_data)
{
    SteppyApp *steppy = (SteppyApp *)user_data;
    update_step_count (steppy);
    update_ui (steppy);
    return G_SOURCE_CONTINUE;
}

static void
show_preferences (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    SteppyApp *steppy = (SteppyApp *)user_data;
    GtkWidget *dialog = adw_preferences_window_new ();
    AdwPreferencesPage *page = adw_preferences_page_new ();
    AdwPreferencesGroup *group = adw_preferences_group_new ();
    adw_preferences_window_add (ADW_PREFERENCES_WINDOW (dialog), page);
    adw_preferences_page_add (page, group);

    GtkWidget *spin = gtk_spin_button_new_with_range (1000, 100000, 1000);
    g_settings_bind (steppy->settings, "step-target", spin, "value", G_SETTINGS_BIND_DEFAULT);

    AdwActionRow *row = adw_action_row_new ();
    adw_action_row_set_subtitle (row, "Daily Step Target");
    adw_action_row_add_suffix (row, spin);
    adw_preferences_group_add (group, GTK_WIDGET (row));

    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (steppy->window));
    gtk_window_present (GTK_WINDOW (dialog));
}

static void
window_close_handler(GtkWindow *window, gpointer user_data)
{
    gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
}

static void
activate_cb (GtkApplication *app, gpointer user_data)
{
    SteppyApp *steppy = (SteppyApp *)user_data;

    if (steppy->window == NULL) {
        steppy->window = ADW_APPLICATION_WINDOW (adw_application_window_new (GTK_APPLICATION (app)));
        gtk_window_set_title (GTK_WINDOW (steppy->window), "Steppy");
        gtk_window_set_default_size (GTK_WINDOW (steppy->window), 300, 200);

        AdwHeaderBar *header = ADW_HEADER_BAR (adw_header_bar_new ());
        GtkButton *pref_button = GTK_BUTTON (gtk_button_new_from_icon_name ("preferences-system-symbolic"));
        adw_header_bar_pack_end (header, GTK_WIDGET (pref_button));

        GtkBox *content = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_box_append (content, GTK_WIDGET (header));

        AdwClamp *clamp = ADW_CLAMP (adw_clamp_new ());
        adw_clamp_set_maximum_size (clamp, 400);
        adw_clamp_set_tightening_threshold (clamp, 300);

        AdwPreferencesGroup *group = ADW_PREFERENCES_GROUP (adw_preferences_group_new ());
        steppy->step_row = ADW_ACTION_ROW (adw_action_row_new ());
        adw_action_row_set_subtitle (steppy->step_row, "Steps");
        adw_preferences_group_add (group, GTK_WIDGET (steppy->step_row));

        adw_clamp_set_child (clamp, GTK_WIDGET (group));
        gtk_box_append (content, GTK_WIDGET (clamp));

        adw_application_window_set_content (steppy->window, GTK_WIDGET (content));

        g_signal_connect_swapped (pref_button, "clicked", G_CALLBACK (show_preferences), steppy);
        g_signal_connect(steppy->window, "close-request", G_CALLBACK(window_close_handler), steppy);
    }

    update_ui (steppy);
    gtk_window_present (GTK_WINDOW (steppy->window));
}

static void
startup_cb (GtkApplication *app, gpointer user_data)
{
    SteppyApp *steppy = (SteppyApp *)user_data;

    steppy->timezone = g_time_zone_new_local ();
    steppy->settings = g_settings_new (APP_ID);
    
    GError *error = NULL;
    steppy->dbus_connection = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, &error);
    if (error) {
        g_warning ("Failed to connect to system bus: %s", error->message);
        g_error_free (error);
        return;
    }

    init_database (steppy);
    get_sensor_reading (steppy);
    update_step_count (steppy);

    steppy->update_source_id = g_timeout_add_seconds (UPDATE_INTERVAL, update_callback, steppy);

    GSimpleAction *pref_action = g_simple_action_new ("preferences", NULL);
    g_signal_connect (pref_action, "activate", G_CALLBACK (show_preferences), steppy);
    g_action_map_add_action (G_ACTION_MAP (app), G_ACTION (pref_action));

    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.preferences", (const char *[]){"<primary>p", NULL});
}

static void
shutdown_cb (GtkApplication *app, gpointer user_data)
{
    SteppyApp *steppy = (SteppyApp *)user_data;

    if (steppy->update_source_id > 0) {
        g_source_remove (steppy->update_source_id);
    }

    if (steppy->db) {
        sqlite3_close (steppy->db);
    }

    if (steppy->dbus_connection) {
        g_object_unref (steppy->dbus_connection);
    }

    if (steppy->timezone) {
        g_time_zone_unref (steppy->timezone);
    }
}

int
main (int argc, char *argv[])
{
    SteppyApp *steppy = g_new0 (SteppyApp, 1);
    int status;

    steppy->session_id = -1;
    steppy->app = adw_application_new (APP_ID, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (steppy->app, "activate", G_CALLBACK (activate_cb), steppy);
    g_signal_connect (steppy->app, "startup", G_CALLBACK (startup_cb), steppy);
    g_signal_connect (steppy->app, "shutdown", G_CALLBACK (shutdown_cb), steppy);

    status = g_application_run (G_APPLICATION (steppy->app), argc, argv);

    g_object_unref (steppy->app);
    g_free (steppy);

    return status;
}
