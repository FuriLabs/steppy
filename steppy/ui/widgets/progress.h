#ifndef __PROGRESS_H__
#define __PROGRESS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define STP_CIRCULAR_PROGRESS_TYPE (stp_circular_progress_get_type ())
G_DECLARE_FINAL_TYPE(StpCircularProgress, stp_circular_progress, STP, CIRCULAR_PROGRESS, GtkWidget)

StpCircularProgress *stp_circular_progress_new(void);
void stp_circular_progress_set_progress(StpCircularProgress *self, double progress);
void stp_circular_progress_set_line_width(StpCircularProgress *self, int line_width);
void stp_circular_progress_set_child(StpCircularProgress *self, GtkWidget *child);

G_END_DECLS

#endif /* __PROGRESS_H__ */
