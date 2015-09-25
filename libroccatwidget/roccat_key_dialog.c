/*
 * This file is part of roccat-tools.
 *
 * roccat-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * roccat-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with roccat-tools. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gaminggear/gdk_key_translations.h>
#include "roccat_key_dialog.h"
#include "i18n-lib.h"

#define ROCCAT_KEY_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ROCCAT_KEY_DIALOG_TYPE, RoccatKeyDialogClass))
#define IS_ROCCAT_KEY_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ROCCAT_KEY_DIALOG_TYPE))
#define ROCCAT_KEY_DIALOG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), ROCCAT_KEY_DIALOG_TYPE, RoccatKeyDialogPrivate))

typedef struct _RoccatKeyDialogClass RoccatKeyDialogClass;

struct _RoccatKeyDialogClass {
	GtkDialogClass parent_class;
};

struct _RoccatKeyDialogPrivate {
	GtkWidget *ok_button;
	GtkLabel *key_label;
	guint xkeycode;
};

G_DEFINE_TYPE(RoccatKeyDialog, roccat_key_dialog, GTK_TYPE_DIALOG);

guint16 roccat_key_dialog_get_xkeycode(RoccatKeyDialog *key_dialog) {
	return key_dialog->priv->xkeycode;
}

void roccat_key_dialog_set_xkeycode(RoccatKeyDialog *key_dialog, guint16 keycode) {
	RoccatKeyDialogPrivate *priv = key_dialog->priv;
	gchar *labeltext;

	labeltext = gaminggear_xkeycode_to_keyname(keycode);
	gtk_label_set_text(priv->key_label, labeltext);
	g_free(labeltext);

	priv->xkeycode = keycode;
	gtk_widget_set_sensitive(priv->ok_button, TRUE);
}

GtkWidget *roccat_key_dialog_new(GtkWindow *parent) {
	RoccatKeyDialog *key_dialog;
	RoccatKeyDialogPrivate *priv;

	key_dialog = g_object_new(ROCCAT_KEY_DIALOG_TYPE, NULL);
	priv = key_dialog->priv;

	priv->ok_button = gtk_dialog_add_button(GTK_DIALOG(key_dialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);
	gtk_widget_set_sensitive(priv->ok_button, FALSE);

	gtk_dialog_add_button(GTK_DIALOG(key_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT);

	gtk_window_set_title(GTK_WINDOW(key_dialog), _("Enter key"));
	gtk_window_set_transient_for(GTK_WINDOW(key_dialog), parent);
	gtk_window_set_modal(GTK_WINDOW(key_dialog), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(key_dialog), TRUE);

	return GTK_WIDGET(key_dialog);
}

static gboolean key_press_event_cb(GtkWidget *dialog, GdkEventKey *event, gpointer user_data) {
	roccat_key_dialog_set_xkeycode(ROCCAT_KEY_DIALOG(user_data), event->hardware_keycode);

	/* don't further process the event (disables return key to exit dialog) */
	return TRUE;
}

static void roccat_key_dialog_init(RoccatKeyDialog *key_dialog) {
	RoccatKeyDialogPrivate *priv = ROCCAT_KEY_DIALOG_GET_PRIVATE(key_dialog);
	GtkVBox *content_area;
	GtkLabel *text_label;

	key_dialog->priv = priv;

	content_area = GTK_VBOX(gtk_dialog_get_content_area(GTK_DIALOG(key_dialog)));

	text_label = GTK_LABEL(gtk_label_new(_("Please hit a key")));
	priv->key_label = GTK_LABEL(gtk_label_new(NULL));

	gtk_box_pack_start(GTK_BOX(content_area), GTK_WIDGET(text_label), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(content_area), GTK_WIDGET(priv->key_label), FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(key_dialog), "key-press-event", G_CALLBACK(key_press_event_cb), key_dialog);

	gtk_widget_show_all(GTK_WIDGET(content_area));
}

static void roccat_key_dialog_class_init(RoccatKeyDialogClass *klass) {
	g_type_class_add_private(klass, sizeof(RoccatKeyDialogPrivate));
}
