#ifndef __UTILS_H__
#define __UTILS_H__

#include "steppy.h"

gint64 get_current_timestamp (SteppyApp *steppy);
gchar *timestamp_to_local_date (SteppyApp *steppy, gint64 timestamp);

#endif // __UTILS_H__
