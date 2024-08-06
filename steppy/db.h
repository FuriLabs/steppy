#ifndef __DB_H__
#define __DB_H__

#include "steppy.h"

void init_database (SteppyApp *steppy);
void update_database (SteppyApp *steppy);
guint get_steps_for_date(SteppyApp *steppy, const gchar *date);

#endif // __DB_H__
