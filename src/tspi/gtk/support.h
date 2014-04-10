/* Header file for support.c */

#ifndef __SUPPORT_H__
#define __SUPPORT_H__ 1

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <gtk/gtk.h>
#include "gettext.h"

extern GtkWidget* lookup_widget(GtkWidget *widget, const gchar *widget_name);
extern void __tspi_add_pixmap_directory(const gchar *directory);
extern GtkWidget* create_pixmap(GtkWidget *widget, const gchar *filename);
extern GdkPixbuf* create_pixbuf(const gchar *filename);
extern void glade_set_atk_action_description(AtkAction *action,
											 const gchar *action_name,
											 const gchar *description);

#endif /* __SUPPORT_H__ */

/* EOF */
