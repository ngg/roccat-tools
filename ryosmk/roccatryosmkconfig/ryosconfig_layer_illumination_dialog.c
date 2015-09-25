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

#include <gaminggear/key_translations.h>
#include "ryosconfig_layer_illumination_dialog.h"
#include "roccat_warning_dialog.h"
#include "roccat_helper.h"
#include "ryos_custom_lights.h"
#include "ryos_light_control.h"
#include "gtk_roccat_helper.h"
#include "i18n.h"

#define RYOSCONFIG_LAYER_ILLUMINATION_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOSCONFIG_LAYER_ILLUMINATION_DIALOG_TYPE, RyosconfigLayerIlluminationDialogClass))
#define IS_RYOSCONFIG_LAYER_ILLUMINATION_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOSCONFIG_LAYER_ILLUMINATION_DIALOG_TYPE))
#define RYOSCONFIG_LAYER_ILLUMINATION_DIALOG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOSCONFIG_LAYER_ILLUMINATION_DIALOG_TYPE, RyosconfigLayerIlluminationDialogPrivate))

typedef struct _RyosconfigLayerIlluminationDialogClass RyosconfigLayerIlluminationDialogClass;

struct _RyosconfigLayerIlluminationDialogClass {
	GtkDialogClass parent_class;
};

struct _RyosconfigLayerIlluminationDialogPrivate {
	RoccatDevice *ryos;
	RyosLightLayer *light_layer;
	guint source;
	gboolean periodic_blink_state;
	GtkToggleButton *blink_button;
};

G_DEFINE_TYPE(RyosconfigLayerIlluminationDialog, ryosconfig_layer_illumination_dialog, GTK_TYPE_DIALOG);

static gchar const * const array_key = "array";

static guint8 preset_main_keys[75] = { 0x0c, 0x0d, 0x19, 0x1a, 0x1b, 0x27, 0x1c, 0x28, 0x1d, 0x29,
		0x12, 0x1e, 0x13, 0x20, 0x2c, 0x24, 0x25, 0x31, 0x26, 0x32,
		0x33, 0x34, 0x40, 0x35, 0x2a, 0x36, 0x1f, 0x2b, 0x38, 0x3c,
		0x3d, 0x49, 0x3e, 0x4a, 0x3f, 0x4b, 0x4c, 0x41, 0x42, 0x4e,
		0x37, 0x43, 0x44, 0x54, 0x55, 0x61, 0x56, 0x62, 0x57, 0x63,
		0x58, 0x4d, 0x59, 0x5a, 0x4f, 0x5b, 0x50, 0x6c, 0x6d, 0x6e,
		0x64, 0x70, 0x65, 0x71, 0x72, 0x73, 0x67, 0x5c, 0x2d, 0x39,
		0x45, 0x51, 0x5d, 0x69, RYOS_LIGHT_LAYER_INDEX_INVALID };
static guint8 preset_all_keys[120] = { 0x00, 0x18, 0x30, 0x48, 0x60, 0x01, 0x02, 0x0e, 0x03, 0x0f,
		0x04, 0x10, 0x05, 0x11, 0x06, 0x07, 0x08, 0x14, 0x09, 0x15,
		0x21, 0x78, 0x79, 0x7a, 0x0c, 0x0d, 0x19, 0x1a, 0x1b, 0x27,
		0x1c, 0x28, 0x1d, 0x29, 0x12, 0x1e, 0x13, 0x20, 0x2c, 0x24,
		0x25, 0x31, 0x26, 0x32, 0x33, 0x34, 0x40, 0x35, 0x2a, 0x36,
		0x1f, 0x2b, 0x38, 0x3c, 0x3d, 0x49, 0x3e, 0x4a, 0x3f, 0x4b,
		0x4c, 0x41, 0x42, 0x4e, 0x37, 0x43, 0x44, 0x54, 0x55, 0x61,
		0x56, 0x62, 0x57, 0x63, 0x58, 0x4d, 0x59, 0x5a, 0x4f, 0x5b,
		0x50, 0x6c, 0x6d, 0x6e, 0x64, 0x70, 0x65, 0x71, 0x72, 0x73,
		0x67, 0x5c, 0x2d, 0x39, 0x45, 0x51, 0x5d, 0x69, 0x75, 0x68,
		0x74, 0x76, 0x0a, 0x16, 0x0b, 0x17, 0x22, 0x2e, 0x23, 0x2f,
		0x3a, 0x46, 0x3b, 0x52, 0x5e, 0x53, 0x5f, 0x6a, 0x6b, RYOS_LIGHT_LAYER_INDEX_INVALID };
static guint8 preset_numpad[18] = { 0x0a, 0x16, 0x0b, 0x17, 0x22, 0x2e, 0x23, 0x2f, 0x3a, 0x46,
		0x3b, 0x52, 0x5e, 0x53, 0x5f, 0x6a, 0x6b, RYOS_LIGHT_LAYER_INDEX_INVALID };
static guint8 preset_f_keys[17] = { 0x01, 0x02, 0x0e, 0x03, 0x0f, 0x04, 0x10, 0x05, 0x11, 0x06,
		0x07, 0x08, 0x14, 0x09, 0x15, 0x21, RYOS_LIGHT_LAYER_INDEX_INVALID };
static guint8 preset_m_keys[6] = { 0x00, 0x18, 0x30, 0x48, 0x60, RYOS_LIGHT_LAYER_INDEX_INVALID };
static guint8 preset_t_keys[4] = { 0x78, 0x79, 0x7a, RYOS_LIGHT_LAYER_INDEX_INVALID };
static guint8 preset_arrows[5] = { 0x75, 0x68, 0x74, 0x76, RYOS_LIGHT_LAYER_INDEX_INVALID };
static guint8 preset_wasd[5] = { 0x31, 0x3d, 0x49, 0x3e, RYOS_LIGHT_LAYER_INDEX_INVALID };
static guint8 preset_qwer[5] = { 0x25, 0x31, 0x26, 0x32, RYOS_LIGHT_LAYER_INDEX_INVALID };

static guint8 key_state(RyosconfigLayerIlluminationDialog *dialog, gboolean on) {
	guint8 value;

	value = 0;

	if (on) {
		roccat_set_bit8(&value, RYOS_LIGHT_LAYER_KEY_BIT_ON, TRUE);
		roccat_set_bit8(&value, RYOS_LIGHT_LAYER_KEY_BIT_BLINK, gtk_toggle_button_get_active(dialog->priv->blink_button));
	}

	return value;
}

static void ryos_custom_lights_set_from_array(RyosLightLayer *layer, guint8 *array, guint8 state) {
	while (*array != RYOS_LIGHT_LAYER_INDEX_INVALID)
		layer->keys[*array++] = state;
}

static void ryosconfig_layer_send(RyosconfigLayerIlluminationDialog *dialog) {
	RyosconfigLayerIlluminationDialogPrivate *priv = dialog->priv;
	GError *local_error = NULL;
	RyosCustomLights custom_lights;

	ryos_light_layer_to_custom_lights(priv->light_layer, &custom_lights, priv->periodic_blink_state);
	ryos_custom_lights_write(priv->ryos, &custom_lights, &local_error);
	if (local_error) {
		g_warning(_("Could not write custom lights: %s"), local_error->message);
		g_clear_error(&local_error);
	}
}

void ryosconfig_layer_illumination_dialog_get_layer(RyosconfigLayerIlluminationDialog *dialog, RyosLightLayer *light_layer) {
	ryos_light_layer_copy(light_layer, dialog->priv->light_layer);
}

void ryosconfig_layer_illumination_dialog_set_layer(RyosconfigLayerIlluminationDialog *dialog, RyosLightLayer const *light_layer) {
	ryos_light_layer_copy(dialog->priv->light_layer, light_layer);
	ryosconfig_layer_send(dialog);
}

GtkWidget *ryosconfig_layer_illumination_dialog_new(GtkWindow *parent, RoccatDevice *ryos) {
	RyosconfigLayerIlluminationDialog *layer_illumination_dialog;

	layer_illumination_dialog = g_object_new(RYOSCONFIG_LAYER_ILLUMINATION_DIALOG_TYPE, NULL);

	layer_illumination_dialog->priv->ryos = ryos;

	gtk_dialog_add_buttons(GTK_DIALOG(layer_illumination_dialog),
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			NULL);
	gtk_window_set_title(GTK_WINDOW(layer_illumination_dialog), _("Edit layer illumination"));
	gtk_window_set_transient_for(GTK_WINDOW(layer_illumination_dialog), parent);
	gtk_window_set_modal(GTK_WINDOW(layer_illumination_dialog), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(layer_illumination_dialog), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(layer_illumination_dialog), GDK_WINDOW_TYPE_HINT_NORMAL);

	return GTK_WIDGET(layer_illumination_dialog);
}

static void array_button_toggled_cb(GtkToggleButton *togglebutton, gpointer user_data) {
	RyosconfigLayerIlluminationDialog *dialog = RYOSCONFIG_LAYER_ILLUMINATION_DIALOG(user_data);
	RyosconfigLayerIlluminationDialogPrivate *priv = dialog->priv;
	guint8 value;
	guint8 *data;

	if (gtk_toggle_button_get_inconsistent(togglebutton)) {
		gtk_toggle_button_set_inconsistent(togglebutton, FALSE);
		gtk_toggle_button_set_active(togglebutton, TRUE);
	}

	value = key_state(dialog, gtk_toggle_button_get_active(togglebutton));
	data = g_object_get_data(G_OBJECT(togglebutton), array_key);
	ryos_custom_lights_set_from_array(priv->light_layer, data, value);

	ryosconfig_layer_send(dialog);
}

static void add_array_button(RyosconfigLayerIlluminationDialog *layer_illumination_dialog,
		GtkTable *table, gchar const *title, guint8 *data, guint left_attach, guint top_attach) {
	GtkWidget *button;
	button = gtk_check_button_new_with_label(title);
	gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(button), TRUE);
	g_object_set_data(G_OBJECT(button), array_key, data);
	g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(array_button_toggled_cb), layer_illumination_dialog);
	gtk_table_attach(table, button, left_attach, left_attach + 1, top_attach, top_attach + 1, GTK_EXPAND, GTK_EXPAND, 0, 0);
}

static gboolean key_press_event_cb(GtkWidget *dialog, GdkEventKey *event, gpointer user_data) {
	RyosconfigLayerIlluminationDialog *layer_illumination_dialog = RYOSCONFIG_LAYER_ILLUMINATION_DIALOG(dialog);
	RyosconfigLayerIlluminationDialogPrivate *priv = layer_illumination_dialog->priv;
	guint8 hid;
	guint8 index;
	gboolean on;

	// FIXME support remap

	hid = gaminggear_xkeycode_to_hid(event->hardware_keycode);
	index = ryos_hid_to_light_layer_index(hid);
	
	on = priv->light_layer->keys[index] != 0;
	priv->light_layer->keys[index] = key_state(layer_illumination_dialog, !on);

	ryosconfig_layer_send(layer_illumination_dialog);

	/* don't further process the event (disables return key to exit dialog) */
	return TRUE;
}

static void ryosconfig_layer_illumination_dialog_init(RyosconfigLayerIlluminationDialog *layer_illumination_dialog) {
	RyosconfigLayerIlluminationDialogPrivate *priv = RYOSCONFIG_LAYER_ILLUMINATION_DIALOG_GET_PRIVATE(layer_illumination_dialog);
	GtkVBox *content_area;
	GtkWidget *info_frame;
	GtkWidget *info_text;
	GtkWidget *effect_frame;
	GtkWidget *preset_frame;
	GtkTable *table;

	layer_illumination_dialog->priv = priv;
	priv->light_layer = g_malloc0(sizeof(RyosLightLayer));
	priv->periodic_blink_state = FALSE;

	content_area = GTK_VBOX(gtk_dialog_get_content_area(GTK_DIALOG(layer_illumination_dialog)));

	info_frame = gtk_frame_new(_("Info"));
	info_text = gtk_label_new(
			_("Press the keys you want to toggle.\n"
			"The keyboard reflects the settings, except the thumbster keys.\n"
			"Remapping and special keys are not supported."));

	gtk_container_add(GTK_CONTAINER(info_frame), GTK_WIDGET(info_text));

	effect_frame = gtk_frame_new(_("Key effect"));
	priv->blink_button = GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label(_("Blinking")));

	gtk_container_add(GTK_CONTAINER(effect_frame), GTK_WIDGET(priv->blink_button));

	preset_frame = gtk_frame_new(_("Presets"));
	table = GTK_TABLE(gtk_table_new(1, 1, TRUE));

	add_array_button(layer_illumination_dialog, table, _("WASD"), preset_wasd, 0, 0);
	add_array_button(layer_illumination_dialog, table, _("QWER"), preset_qwer, 0, 1);
	add_array_button(layer_illumination_dialog, table, _("Arrows"), preset_arrows, 0, 2);

	add_array_button(layer_illumination_dialog, table, _("F keys"), preset_f_keys, 1, 0);
	add_array_button(layer_illumination_dialog, table, _("M keys"), preset_m_keys, 1, 1);
	add_array_button(layer_illumination_dialog, table, _("T keys"), preset_t_keys, 1, 2);

	add_array_button(layer_illumination_dialog, table, _("Main Block"), preset_main_keys, 2, 0);
	add_array_button(layer_illumination_dialog, table, _("Numpad"), preset_numpad, 2, 1);
	add_array_button(layer_illumination_dialog, table, _("All keys"), preset_all_keys, 2, 2);

	gtk_container_add(GTK_CONTAINER(preset_frame), GTK_WIDGET(table));

	gtk_box_pack_start(GTK_BOX(content_area), info_frame, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(content_area), effect_frame, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(content_area), preset_frame, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(layer_illumination_dialog), "key-press-event", G_CALLBACK(key_press_event_cb), NULL);

	gtk_widget_show_all(GTK_WIDGET(content_area));
}

static void finalize(GObject *object) {
	RyosconfigLayerIlluminationDialogPrivate *priv = RYOSCONFIG_LAYER_ILLUMINATION_DIALOG(object)->priv;
	G_OBJECT_CLASS(ryosconfig_layer_illumination_dialog_parent_class)->finalize(object);
	g_free(priv->light_layer);
}

static void ryosconfig_layer_illumination_dialog_class_init(RyosconfigLayerIlluminationDialogClass *klass) {
	GObjectClass *gobject_class;

	gobject_class = (GObjectClass*)klass;
	gobject_class->finalize = finalize;

	g_type_class_add_private(klass, sizeof(RyosconfigLayerIlluminationDialogPrivate));
}

static gboolean timeout_cb(RyosconfigLayerIlluminationDialog *dialog) {
	RyosconfigLayerIlluminationDialogPrivate *priv = dialog->priv;

	priv->periodic_blink_state = !priv->periodic_blink_state;

	ryosconfig_layer_send(dialog);

	return TRUE;
}

static void ryosconfig_layer_illumination_dialog_start(RyosconfigLayerIlluminationDialog *dialog) {
	RyosconfigLayerIlluminationDialogPrivate *priv = dialog->priv;
	GError *local_error = NULL;

	ryos_light_control_custom(priv->ryos, TRUE, &local_error);
	if (local_error)
		(void)roccat_handle_error_dialog(gtk_roccat_widget_get_gtk_window(GTK_WIDGET(dialog)), _("Could not init custom mode"), &local_error);

	priv->source = g_timeout_add_seconds(1, (GSourceFunc)timeout_cb, dialog);
}

static void ryosconfig_layer_illumination_dialog_stop(RyosconfigLayerIlluminationDialog *dialog) {
	RyosconfigLayerIlluminationDialogPrivate *priv = dialog->priv;
	GError *local_error = NULL;

	g_source_remove(priv->source);

	ryos_light_control_custom(priv->ryos, FALSE, &local_error);
	if (local_error) {
		g_warning(_("Could not deinit custom mode: %s"), local_error->message);
		g_clear_error(&local_error);
	}
}

gboolean ryosconfig_layer_illumination_dialog_run(GtkWindow *parent, RoccatDevice *ryos, RyosLightLayer *light_layer) {
	RyosconfigLayerIlluminationDialog *dialog;
	gboolean retval = FALSE;


	dialog = RYOSCONFIG_LAYER_ILLUMINATION_DIALOG(ryosconfig_layer_illumination_dialog_new(parent, ryos));
	ryosconfig_layer_illumination_dialog_set_layer(dialog, light_layer);

	ryosconfig_layer_illumination_dialog_start(dialog);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		ryosconfig_layer_illumination_dialog_get_layer(dialog, light_layer);
		retval = TRUE;
	}

	ryosconfig_layer_illumination_dialog_stop(dialog);

	gtk_widget_destroy(GTK_WIDGET(dialog));

	return retval;
}
