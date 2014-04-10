/* header for interface.c */

#ifndef __INTERFACE_H__
#define __INTERFACE_H__ 1

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include "gettext.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

struct userdata {
	char *string;
	unsigned string_len;
	GtkWidget *window;
	GtkWidget *entry;
	GtkWidget *entryPass;
	GtkWidget *entryConf;
};

extern GtkWidget* create_password_dialog(struct userdata *ud, char *message);
extern GtkWidget* create_new_password_dialog(struct userdata *ud,
											 char *message);

#endif /* __INTERFACE_H__ */

/* EOF */
