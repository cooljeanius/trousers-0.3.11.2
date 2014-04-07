/* Header file for callbacks.c */

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include <gtk/gtk.h>

#include "interface.h"

extern void on_inputdialog1_destroy(GtkObject *object,
									struct userdata *user_data);
extern void on_dialog1_close(GtkDialog *dialog, struct userdata *user_data);
extern void on_cancelbutton1_clicked(GtkButton *button,
									 struct userdata *user_data);
extern void on_okbutton1_clicked(GtkButton *button, struct userdata *user_data);
extern gboolean enter_event(GtkWidget *widget, struct userdata *user_data);
extern void on_entryPassword_activate(GtkEntry *entry,
									  struct userdata *user_data);
extern void on_entryConfirm_activate(GtkEntry *entry,
									 struct userdata *user_data);
extern void on_cancelbutton2_clicked(GtkButton *button,
									 struct userdata *user_data);
extern void on_okbutton2_clicked(GtkButton *button, struct userdata *user_data);

#endif /* !__CALLBACKS_H__ */

/* EOF */
