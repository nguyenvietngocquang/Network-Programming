#include <gtk/gtk.h>
#include "close.h"

gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    return FALSE;
}

void destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
    exit(1);
}