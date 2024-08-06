#include "db.h"
#include "utils.h"

void
init_database (SteppyApp *steppy)
{
    gchar *db_path = g_build_filename (g_get_user_data_dir (), "steppy.db", NULL);
    sqlite3_open (db_path, &steppy->db);
    g_free (db_path);

    const char *sql = "CREATE TABLE IF NOT EXISTS steps "
                      "(date TEXT PRIMARY KEY, count INTEGER)";
    sqlite3_exec (steppy->db, sql, NULL, NULL, NULL);
}

void
update_database (SteppyApp *steppy)
{
    gchar *date = timestamp_to_local_date (steppy, get_current_timestamp (steppy));
    char *sql = sqlite3_mprintf ("INSERT OR REPLACE INTO steps (date, count) "
                                 "VALUES ('%q', %d)", date, steppy->step_count);
    sqlite3_exec (steppy->db, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    g_free (date);
}

guint
get_steps_for_date(SteppyApp *steppy, const gchar *date)
{
    guint count = 0;
    char *sql = sqlite3_mprintf("SELECT count FROM steps WHERE date = '%q'", date);
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(steppy->db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return count;
}
