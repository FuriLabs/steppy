#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "steppy.h"

gint32 request_sensor(SteppyApp *steppy);
guint32 get_sensor_reading(SteppyApp *steppy);

#endif // __SENSORS_H__
