#include "utils.h"

gint64
get_current_timestamp (SteppyApp *steppy)
{
    GDateTime *now = g_date_time_new_now (steppy->timezone);
    gint64 timestamp = g_date_time_to_unix (now);
    g_date_time_unref (now);
    return timestamp;
}

gchar *
timestamp_to_local_date (SteppyApp *steppy, gint64 timestamp)
{
    GDateTime *dt = g_date_time_new_from_unix_local (timestamp);
    gchar *date_str = g_date_time_format (dt, "%Y-%m-%d");
    g_date_time_unref (dt);
    return date_str;
}
