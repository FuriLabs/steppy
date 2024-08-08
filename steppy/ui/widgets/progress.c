#include "progress.h"
#include <gsk/gsk.h>
#include <math.h>

struct _StpCircularProgress
{
    GtkWidget parent_instance;
    double progress;
    int line_width;
    GtkWidget *child;
};

G_DEFINE_TYPE(StpCircularProgress, stp_circular_progress, GTK_TYPE_WIDGET)

static void
stp_circular_progress_dispose(GObject *object)
{
    StpCircularProgress *self = STP_CIRCULAR_PROGRESS(object);

    g_clear_pointer(&self->child, gtk_widget_unparent);

    G_OBJECT_CLASS(stp_circular_progress_parent_class)->dispose(object);
}

static void
stp_circular_progress_init(StpCircularProgress *self)
{
    self->progress = 0.0;
    self->line_width = 20;

    gtk_widget_set_overflow(GTK_WIDGET(self), GTK_OVERFLOW_HIDDEN);
}

static void
stp_circular_progress_measure(GtkWidget      *widget,
                          GtkOrientation  orientation,
                          int             for_size,
                          int            *minimum,
                          int            *natural,
                          int            *minimum_baseline,
                          int            *natural_baseline)
{
    StpCircularProgress *self = STP_CIRCULAR_PROGRESS(widget);
    
    *minimum = *natural = 200;

    if (self->child)
    {
        int child_min, child_nat;
        gtk_widget_measure(self->child, orientation, for_size,
                           &child_min, &child_nat, NULL, NULL);
        *minimum = MAX(*minimum, child_min);
        *natural = MAX(*natural, child_nat);
    }

    if (minimum_baseline)
        *minimum_baseline = -1;
    if (natural_baseline)
        *natural_baseline = -1;
}

static void
stp_circular_progress_snapshot(GtkWidget   *widget,
                           GtkSnapshot *snapshot)
{
    // TODO: this uses cairo, which is S-to-the-LOW. Use GSK instead whenever Debian ships a version of GTK from this decade.
    StpCircularProgress *self = STP_CIRCULAR_PROGRESS(widget);
    int width = gtk_widget_get_width(widget);
    int height = gtk_widget_get_height(widget);
    double center_x = width / 2.0;
    double center_y = height / 2.0;
    double radius = fmin(width, height) / 2.0 - self->line_width / 2.0;

    cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &GRAPHENE_RECT_INIT(0, 0, width, height));

    // Background circle
    cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 0.5);
    cairo_set_line_width(cr, self->line_width);
    cairo_arc(cr, center_x, center_y, radius, 0, 2 * G_PI);
    cairo_stroke(cr);

    // Progress arc
    cairo_set_source_rgba(cr, 1.0, 0.6, 1.0, 1.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_arc(cr, center_x, center_y, radius, -G_PI / 2, -G_PI / 2 + 2 * G_PI * self->progress);
    cairo_stroke(cr);

    cairo_destroy(cr);

    if (self->child)
        gtk_widget_snapshot_child(widget, self->child, snapshot);
}


static void
stp_circular_progress_class_init(StpCircularProgressClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = stp_circular_progress_dispose;

    widget_class->measure = stp_circular_progress_measure;
    widget_class->snapshot = stp_circular_progress_snapshot;

    gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
    gtk_widget_class_set_css_name(widget_class, "circular-progress");
}


StpCircularProgress *
stp_circular_progress_new(void)
{
    return g_object_new(STP_CIRCULAR_PROGRESS_TYPE, NULL);
}

void
stp_circular_progress_set_progress(StpCircularProgress *self, double progress)
{
    self->progress = CLAMP(progress, 0.0, 1.0);
    gtk_widget_queue_draw(GTK_WIDGET(self));
}

void
stp_circular_progress_set_line_width(StpCircularProgress *self, int line_width)
{
    self->line_width = line_width;
    gtk_widget_queue_draw(GTK_WIDGET(self));
}

void
stp_circular_progress_set_child(StpCircularProgress *self, GtkWidget *child)
{
    if (self->child == child)
        return;

    if (self->child)
        gtk_widget_unparent(self->child);

    self->child = child;

    if (child)
        gtk_widget_set_parent(child, GTK_WIDGET(self));

    gtk_widget_queue_resize(GTK_WIDGET(self));
}
