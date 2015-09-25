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
#include "ryosconfig_keyboard_selector.h"
#include "ryos_key_button.h"
#include "ryos_key_relations.h"
#include "ryos_fn_selector.h"
#include "ryos_standard_selector.h"
#include "ryos_capslock_selector.h"
#include "ryosconfig_illumination_frame.h"
#include "ryosconfig_key_illumination_selector.h"
#include "roccat_multiwidget.h"
#include "roccat_helper.h"
#include "ryos_rkp_accessors.h"
#include "g_cclosure_roccat_marshaller.h"
#include "i18n.h"

#define RYOSCONFIG_KEYBOARD_SELECTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOSCONFIG_KEYBOARD_SELECTOR_TYPE, RyosconfigKeyboardSelectorClass))
#define IS_RYOSCONFIG_KEYBOARD_SELECTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOSCONFIG_KEYBOARD_SELECTOR_TYPE))
#define RYOSCONFIG_KEYBOARD_SELECTOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), RYOSCONFIG_KEYBOARD_SELECTOR_TYPE, RyosconfigKeyboardSelectorClass))
#define RYOSCONFIG_KEYBOARD_SELECTOR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOSCONFIG_KEYBOARD_SELECTOR_TYPE, RyosconfigKeyboardSelectorPrivate))

typedef struct _RyosconfigKeyboardSelectorClass RyosconfigKeyboardSelectorClass;
typedef struct _RyosconfigKeyboardSelectorPrivate RyosconfigKeyboardSelectorPrivate;
typedef struct _RyosconfigKeyboardSelectorPrivateData RyosconfigKeyboardSelectorPrivateData;

struct _RyosconfigKeyboardSelector {
	GtkFrame parent;
	RyosconfigKeyboardSelectorPrivate *priv;
};

struct _RyosconfigKeyboardSelectorClass {
	GtkFrameClass parent_class;
};

enum {
	TABLE_HEIGHT = 30,
	TABLE_WIDTH = 95,
};

static gchar const * const index_key = "index";

struct _RyosconfigKeyboardSelectorPrivate {
	RyosKeyButton *keys[RYOS_KEY_RELATIONS_NUM];
	GtkScrolledWindow *scroll_window;
	GtkTable *table;
	
	RyosconfigIlluminationFrame *illumination_frame;
	GtkLabel *label;
	guint actual_index;

	RoccatMultiwidget *key_type_multiwidget;
	RyosFnSelector *fn_selector;
	RyosCapslockSelector *capslock_selector;
	RyosStandardSelector *standard_selector;
	GtkWidget *nothing_selector;

	RoccatMultiwidget *key_illumination_multiwidget;
	RyosconfigKeyIlluminationSelector *key_illumination_selector;
	GtkWidget *key_illumination_box;
	GtkWidget *key_illumination_nothing;

	/* private object data can only be 64kb in size */
	RyosconfigKeyboardSelectorPrivateData *data;

	gboolean periodic_blink_state;
	guint blink_timeout;
};

struct _RyosconfigKeyboardSelectorPrivateData {
	RyosKeyComboBoxData standard_datas[RYOS_KEY_RELATIONS_NUM];
	RyosKeyComboBoxData easyshift_datas[RYOS_KEY_RELATIONS_NUM];
	RyosStoredLights lights_automatic;
	RyosStoredLights lights_manual;
};

static void toggled_cb(GtkToggleButton *button, gpointer user_data);

G_DEFINE_TYPE(RyosconfigKeyboardSelector, ryosconfig_keyboard_selector, GTK_TYPE_FRAME);

enum {
	LAYER_EDITOR,
	LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL] = { 0 };

GtkWidget *ryosconfig_keyboard_selector_new(void) {
	return GTK_WIDGET(g_object_new(RYOSCONFIG_KEYBOARD_SELECTOR_TYPE,
			"label", _("Keys"),
			NULL));
}

static guint get_active_index(RyosconfigKeyboardSelector *selector) {
	return selector->priv->actual_index;
}

static void set_active_index(RyosconfigKeyboardSelector *selector, guint index) {
	selector->priv->actual_index = index;
}

static void selector_label_update(RyosconfigKeyboardSelector *selector, guint index) {
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;
	gchar *string;

	string = hid_to_beauty_keyname(ryos_key_relations[index].primary);
	gtk_label_set_text(priv->label, string);
	g_free(string);
}

static void selector_show(RyosconfigKeyboardSelector *selector, guint index) {
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;

	switch (ryos_key_relations[index].keys_type) {
	case RYOS_KEY_RELATION_KEYS_TYPE_CAPSLOCK:
		ryos_capslock_selector_set_index(priv->capslock_selector, index, ryos_key_relations[index].mask, ryos_key_relations[index].default_normal);
		roccat_multiwidget_show(priv->key_type_multiwidget, GTK_WIDGET(priv->capslock_selector));
		break;
	case RYOS_KEY_RELATION_KEYS_TYPE_FN:
		roccat_multiwidget_show(priv->key_type_multiwidget, GTK_WIDGET(priv->fn_selector));
		break;
	case RYOS_KEY_RELATION_KEYS_TYPE_NOTHING:
		roccat_multiwidget_show(priv->key_type_multiwidget, priv->nothing_selector);
		break;
	case RYOS_KEY_RELATION_KEYS_TYPE_MACRO:
	case RYOS_KEY_RELATION_KEYS_TYPE_THUMBSTER:
	case RYOS_KEY_RELATION_KEYS_TYPE_EASYZONE:
		ryos_standard_selector_set_secondary_text(priv->standard_selector, _("Easyshift"));
		ryos_standard_selector_set_index(priv->standard_selector, index,
				ryos_key_relations[index].mask, ryos_key_relations[index].default_normal, ryos_key_relations[index].mask_easyshift, ryos_key_relations[index].default_easyshift);
		roccat_multiwidget_show(priv->key_type_multiwidget, GTK_WIDGET(priv->standard_selector));
		break;
	case RYOS_KEY_RELATION_KEYS_TYPE_FUNCTION:
		ryos_standard_selector_set_secondary_text(priv->standard_selector, _("FN"));
		ryos_standard_selector_set_index(priv->standard_selector, index,
				ryos_key_relations[index].mask, ryos_key_relations[index].default_normal, ryos_key_relations[index].mask_easyshift, ryos_key_relations[index].default_easyshift);
		roccat_multiwidget_show(priv->key_type_multiwidget, GTK_WIDGET(priv->standard_selector));
		break;
	default:
		// error
		break;
	}
}

static void button_deactivate_other(RyosconfigKeyboardSelector *selector, guint index) {
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;
	guint i;
	GtkToggleButton *toggle_button;

	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i) {
		if (!priv->keys[i])
			continue;
		
		if (i != index) {
			toggle_button = GTK_TOGGLE_BUTTON(priv->keys[i]);
			if (gtk_toggle_button_get_active(toggle_button)) {
				g_signal_handlers_block_by_func(G_OBJECT(toggle_button), toggled_cb, selector);
				gtk_toggle_button_set_active(toggle_button, FALSE);
				g_signal_handlers_unblock_by_func(G_OBJECT(toggle_button), toggled_cb, selector);
			}
		}
	}
}

static RyosLightLayer *get_active_layer(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	guint layer = ryosconfig_illumination_frame_get_layer(priv->illumination_frame);
	guint mode = ryosconfig_illumination_frame_get_mode(priv->illumination_frame);

	if (!priv->data)
		return NULL;

	if (mode == RYOS_ILLUMINATION_MODE_AUTOMATIC)
		return &priv->data->lights_automatic.layers[layer];
	else
		return &priv->data->lights_manual.layers[layer];
}

static void update_key_illumination(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	RyosLightLayer *active_layer;
	guint index;
	guint value;

	index = get_active_index(keyboard_selector);
	active_layer = get_active_layer(keyboard_selector);
	value = active_layer->keys[ryos_key_relations[index].primary_index];
	ryosconfig_key_illumination_selector_set_value_blocked(priv->key_illumination_selector, value);
}

static void button_update_active(RyosconfigKeyboardSelector *keyboard_selector) {
	guint index = get_active_index(keyboard_selector);

	selector_label_update(keyboard_selector, index);
	selector_show(keyboard_selector, index);
	update_key_illumination(keyboard_selector);
}

static void button_set_active(RyosconfigKeyboardSelector *selector, guint index) {
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;
	GtkToggleButton *toggle_button;

	set_active_index(selector, index);
	button_deactivate_other(selector, index);

	toggle_button = GTK_TOGGLE_BUTTON(priv->keys[index]);

	if (!gtk_toggle_button_get_active(toggle_button)) {
		g_signal_handlers_block_by_func(G_OBJECT(toggle_button), toggled_cb, selector);
		gtk_toggle_button_set_active(toggle_button, TRUE);
		g_signal_handlers_unblock_by_func(G_OBJECT(toggle_button), toggled_cb, selector);
	}

	button_update_active(selector);
}

static void toggled_cb(GtkToggleButton *button, gpointer user_data) {
	RyosconfigKeyboardSelector *keyboard_selector = RYOSCONFIG_KEYBOARD_SELECTOR(user_data);
	guint index;

	index = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(button), index_key));
	set_active_index(keyboard_selector, index);

	button_deactivate_other(keyboard_selector, index);
	button_update_active(keyboard_selector);
}

/* Fixing size request width
 * AspectFrame forces Table to shape, but does not correct it's size_request,
 * leaving an empty stripe on the side.
 */
static void table_size_request_cb(GtkWidget *table, GtkRequisition *requisition, gpointer user_data) {
	requisition->width = (gfloat)TABLE_WIDTH / (gfloat)TABLE_HEIGHT * (gfloat)requisition->height;
}

static gint find_index_by_type(guint type) {
	guint i;

	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i)
		if (ryos_key_relations[i].keys_type == type)
			return i;
	return -1;
}

static void set_fn_tooltip(RyosconfigKeyboardSelector *keyboard_selector, guint index) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	gchar *text;
	guint value;

	if (!priv->keys[index])
		return;

	value = ryos_fn_selector_get_value(priv->fn_selector);
	text = ryos_fn_selector_get_text_for_value(priv->fn_selector, value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(priv->keys[index]), text);
	g_free(text);
}

static void set_capslock_tooltip(RyosconfigKeyboardSelector *keyboard_selector, guint index) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	gchar *text;

	if (!priv->keys[index])
		return;

	text = ryos_capslock_selector_get_text_for_data(priv->capslock_selector, &priv->data->standard_datas[index]);
	gtk_widget_set_tooltip_text(GTK_WIDGET(priv->keys[index]), text);
	g_free(text);
}

static void set_standard_tooltip(RyosconfigKeyboardSelector *keyboard_selector, guint index) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	gchar *standard_text;
	gchar *easyshift_text;
	gchar *text;
	
	if (!priv->keys[index])
		return;

	standard_text = ryos_standard_selector_get_text_for_data(priv->standard_selector, &priv->data->standard_datas[index]);
	easyshift_text = ryos_standard_selector_get_text_for_data(priv->standard_selector, &priv->data->easyshift_datas[index]);
	text = g_strdup_printf(_("Standard: %s\nEasyshift: %s"), standard_text, easyshift_text);
	gtk_widget_set_tooltip_text(GTK_WIDGET(priv->keys[index]), text);
	g_free(standard_text);
	g_free(easyshift_text);
	g_free(text);
}

static void fn_key_changed_cb(RyosFnSelector *fn_selector, gpointer user_data) {
	gint index;

	/* assuming there's only one */
	index = find_index_by_type(RYOS_KEY_RELATION_KEYS_TYPE_FN);
	if (index < 0)
		return;

	set_fn_tooltip(RYOSCONFIG_KEYBOARD_SELECTOR(user_data), index);
}

static void capslock_key_changed_cb(RyosCapslockSelector *capslock_selector, gint index, gpointer user_data) {
	RyosconfigKeyboardSelector *keyboard_selector = RYOSCONFIG_KEYBOARD_SELECTOR(user_data);
	if (index >= 0)
		set_capslock_tooltip(keyboard_selector, index);
}

static void standard_key_changed_cb(RyosStandardSelector *standard_selector, gint index, gpointer user_data) {
	RyosconfigKeyboardSelector *keyboard_selector = RYOSCONFIG_KEYBOARD_SELECTOR(user_data);
	if (index >= 0)
		set_standard_tooltip(keyboard_selector, index);
}

static gint find_index_by_data_key_type(RyosKeyComboBoxData *datas, guint type) {
	guint i;
	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i)
		if (datas[i].key.type == type)
			return i;
	return -1;
}

static void remap_cb(RyosStandardSelector *standard_selector, gint source_index, guint old_hid, guint new_hid, gpointer user_data) {
	RyosconfigKeyboardSelector *keyboard_selector = RYOSCONFIG_KEYBOARD_SELECTOR(user_data);
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	gint target_index;

	/* find target before changing anything */
	target_index = find_index_by_data_key_type(priv->data->standard_datas, new_hid);
	if (target_index >= 0) {
		ryos_key_set_to_normal(&priv->data->standard_datas[target_index].key, old_hid);
		set_standard_tooltip(keyboard_selector, target_index);
	} else
		g_warning(_("Remap: Can't find target key with hid %u: just setting the key, no swapping"), new_hid);

	ryos_key_set_to_normal(&priv->data->standard_datas[source_index].key, new_hid);
	set_standard_tooltip(keyboard_selector, source_index);
	if (priv->actual_index == source_index)
		ryos_standard_selector_update(standard_selector);
}

static void update_keyboard_illumination(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	guint i;
	RyosLightLayer *active_layer;
	gboolean on;
	gboolean blink;
	gboolean state;

	active_layer = get_active_layer(keyboard_selector);

	if (!active_layer)
		return;

	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i) {
		if (!priv->keys[i])
			continue;
		
		on = roccat_get_bit8(active_layer->keys[ryos_key_relations[i].primary_index], RYOS_LIGHT_LAYER_KEY_BIT_ON);
		blink = roccat_get_bit8(active_layer->keys[ryos_key_relations[i].primary_index], RYOS_LIGHT_LAYER_KEY_BIT_BLINK);
		state = (blink && !priv->periodic_blink_state) ? FALSE : on;
		ryos_key_button_set_light(priv->keys[i], state);
	}
}

static void change_illumination_pointer(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	RyosLightLayer *active_layer;

	active_layer = get_active_layer(keyboard_selector);
	ryosconfig_illumination_frame_set_pointer_blocked(priv->illumination_frame, active_layer);
	update_keyboard_illumination(keyboard_selector);
	update_key_illumination(keyboard_selector);
}

static void illumination_changed_cb(RyosconfigIlluminationFrame *illumination_frame, gpointer user_data) {
	change_illumination_pointer(RYOSCONFIG_KEYBOARD_SELECTOR(user_data));
}

static gboolean layer_editor_cb(RyosconfigIlluminationFrame *illumination_frame, gpointer light_layer, gpointer user_data) {
	RyosconfigKeyboardSelector *keyboard_selector = RYOSCONFIG_KEYBOARD_SELECTOR(user_data);
	gboolean result;
	g_signal_emit((gpointer)keyboard_selector, signals[LAYER_EDITOR], 0, light_layer, &result);
	return result;
}

static void key_illumination_changed_cb(RyosconfigKeyIlluminationSelector *selector, gpointer user_data) {
	RyosconfigKeyboardSelector *keyboard_selector = RYOSCONFIG_KEYBOARD_SELECTOR(user_data);
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	guint index = get_active_index(keyboard_selector);
	RyosLightLayer *active_layer;
	guint value;
	gboolean on;

	value = ryosconfig_key_illumination_selector_get_value(priv->key_illumination_selector);

	active_layer = get_active_layer(keyboard_selector);
	active_layer->keys[ryos_key_relations[index].primary_index] = value;

	on = roccat_get_bit8(value, RYOS_LIGHT_LAYER_KEY_BIT_ON);
	ryos_key_button_set_light(priv->keys[index], on);
}

static void create_key_type_multiwidget(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;

	priv->key_type_multiwidget = ROCCAT_MULTIWIDGET(roccat_multiwidget_new());
	priv->fn_selector = RYOS_FN_SELECTOR(ryos_fn_selector_new());
	priv->capslock_selector = RYOS_CAPSLOCK_SELECTOR(ryos_capslock_selector_new(priv->data->standard_datas));
	priv->standard_selector = RYOS_STANDARD_SELECTOR(ryos_standard_selector_new(priv->data->standard_datas, priv->data->easyshift_datas));
	priv->nothing_selector = gtk_label_new(_("This key does not allow settings"));

	gtk_label_set_line_wrap(GTK_LABEL(priv->nothing_selector), TRUE);

	g_signal_connect(G_OBJECT(priv->fn_selector), "key-changed", G_CALLBACK(fn_key_changed_cb), keyboard_selector);
	g_signal_connect(G_OBJECT(priv->capslock_selector), "key-changed", G_CALLBACK(capslock_key_changed_cb), keyboard_selector);
	g_signal_connect(G_OBJECT(priv->standard_selector), "key-changed", G_CALLBACK(standard_key_changed_cb), keyboard_selector);
	g_signal_connect(G_OBJECT(priv->standard_selector), "remap", G_CALLBACK(remap_cb), keyboard_selector);

	roccat_multiwidget_add(priv->key_type_multiwidget, GTK_WIDGET(priv->fn_selector));
	roccat_multiwidget_add(priv->key_type_multiwidget, GTK_WIDGET(priv->capslock_selector));
	roccat_multiwidget_add(priv->key_type_multiwidget, GTK_WIDGET(priv->standard_selector));
	roccat_multiwidget_add(priv->key_type_multiwidget, priv->nothing_selector);
}

static void create_key_illumination_multiwidget(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;

	priv->key_illumination_multiwidget = ROCCAT_MULTIWIDGET(roccat_multiwidget_new());
	priv->key_illumination_box = gtk_hbox_new(FALSE, 0);
	priv->key_illumination_selector = RYOSCONFIG_KEY_ILLUMINATION_SELECTOR(ryosconfig_key_illumination_selector_new());
	priv->key_illumination_nothing = gtk_label_new(_("This model does not support per-key light settings"));

	gtk_label_set_line_wrap(GTK_LABEL(priv->key_illumination_nothing), TRUE);

	g_signal_connect(G_OBJECT(priv->key_illumination_selector), "changed", G_CALLBACK(key_illumination_changed_cb), keyboard_selector);

	gtk_box_pack_start(GTK_BOX(priv->key_illumination_box), gtk_label_new(_("Light")), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(priv->key_illumination_box), GTK_WIDGET(priv->key_illumination_selector), TRUE, TRUE, 0);

	roccat_multiwidget_add(priv->key_illumination_multiwidget, priv->key_illumination_box);
	roccat_multiwidget_add(priv->key_illumination_multiwidget, priv->key_illumination_nothing);
}

static gboolean timeout_cb(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;

	priv->periodic_blink_state = !priv->periodic_blink_state;

	update_keyboard_illumination(keyboard_selector);

	return TRUE;
}

static void start_blinking(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	if (priv->blink_timeout)
		return;
	priv->blink_timeout = g_timeout_add_seconds(1, (GSourceFunc)timeout_cb, keyboard_selector);
}

static void stop_blinking(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	if (!priv->blink_timeout)
		return;
	g_source_remove(priv->blink_timeout);
	priv->blink_timeout = 0;
}

static void ryosconfig_keyboard_selector_init(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = RYOSCONFIG_KEYBOARD_SELECTOR_GET_PRIVATE(keyboard_selector);
	keyboard_selector->priv = priv;
	GtkWidget *keyboard_frame;
	GtkWidget *vbox;
	GtkWidget *key_frame;
	GtkWidget *hbox;
	GtkWidget *key_vbox;

	priv->data = (RyosconfigKeyboardSelectorPrivateData *)g_malloc(sizeof(RyosconfigKeyboardSelectorPrivateData));
	priv->periodic_blink_state = FALSE;
	priv->blink_timeout = 0;
	
	vbox = gtk_vbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(keyboard_selector), vbox);

	priv->scroll_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
	gtk_scrolled_window_set_policy(priv->scroll_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->scroll_window), TRUE, TRUE, 0);

	keyboard_frame = gtk_aspect_frame_new(NULL, 0.0, 0.0, (gfloat)TABLE_WIDTH / (gfloat)TABLE_HEIGHT, FALSE);
	gtk_frame_set_shadow_type(GTK_FRAME(keyboard_frame), GTK_SHADOW_NONE);
	gtk_scrolled_window_add_with_viewport(priv->scroll_window, keyboard_frame);

	priv->table = GTK_TABLE(gtk_table_new(TABLE_HEIGHT, TABLE_WIDTH, TRUE));
	g_signal_connect(G_OBJECT(priv->table), "size-request", G_CALLBACK(table_size_request_cb), NULL);
	gtk_container_add(GTK_CONTAINER(keyboard_frame), GTK_WIDGET(priv->table));

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	key_frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), key_frame, TRUE, TRUE, 0);

	key_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(key_frame), key_vbox);

	priv->label = GTK_LABEL(gtk_label_new(NULL));
	gtk_frame_set_label_widget(GTK_FRAME(key_frame), GTK_WIDGET(priv->label));

	create_key_type_multiwidget(keyboard_selector);
	create_key_illumination_multiwidget(keyboard_selector);

	gtk_box_pack_start(GTK_BOX(key_vbox), GTK_WIDGET(priv->key_type_multiwidget), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(key_vbox), GTK_WIDGET(priv->key_illumination_multiwidget), TRUE, TRUE, 0);

	priv->illumination_frame = RYOSCONFIG_ILLUMINATION_FRAME(ryosconfig_illumination_frame_new());
	g_signal_connect(G_OBJECT(priv->illumination_frame), "mode-changed", G_CALLBACK(illumination_changed_cb), keyboard_selector);
	g_signal_connect(G_OBJECT(priv->illumination_frame), "layer-changed", G_CALLBACK(illumination_changed_cb), keyboard_selector);
	g_signal_connect(G_OBJECT(priv->illumination_frame), "layer-editor", G_CALLBACK(layer_editor_cb), keyboard_selector);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(priv->illumination_frame), TRUE, TRUE, 0);

	gtk_widget_show_all(GTK_WIDGET(keyboard_selector));
}

static void finalize(GObject *object) {
	RyosconfigKeyboardSelector *selector = RYOSCONFIG_KEYBOARD_SELECTOR(object);
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;
	G_OBJECT_CLASS(ryosconfig_keyboard_selector_parent_class)->finalize(object);
	stop_blinking(selector);
	g_free(priv->data);
}

static void ryosconfig_keyboard_selector_class_init(RyosconfigKeyboardSelectorClass *klass) {
	GObjectClass *gobject_class;

	gobject_class = (GObjectClass*)klass;
	gobject_class->finalize = finalize;

	g_type_class_add_private(klass, sizeof(RyosconfigKeyboardSelectorPrivate));

	signals[LAYER_EDITOR] = g_signal_new("layer-editor",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_roccat_marshal_BOOLEAN__POINTER, G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);
}

static void create_key(RyosconfigKeyboardSelector *keyboard_selector, RyosKeyPosition const *position,
		guint column_offset, guint row_offset) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	guint relation_index = position->relation_index;

	priv->keys[relation_index] = RYOS_KEY_BUTTON(ryos_key_button_new(ryos_key_relations[relation_index].primary));
	g_object_set_data(G_OBJECT(priv->keys[relation_index]), index_key, GUINT_TO_POINTER(relation_index));
	g_signal_connect(G_OBJECT(priv->keys[relation_index]), "toggled", G_CALLBACK(toggled_cb), keyboard_selector);

	gtk_table_attach(priv->table, GTK_WIDGET(priv->keys[relation_index]),
			position->column + column_offset, position->column + column_offset + position->width,
			position->row + row_offset, position->row + row_offset + position->height,
			GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
}

static void create_block(RyosconfigKeyboardSelector *keyboard_selector, RyosKeyPosition const *positions,
		guint column_offset, guint row_offset) {
	guint i;

	for (i = 0; positions[i].relation_index != RYOS_KEY_POSITIONS_END; ++i)
		create_key(keyboard_selector, &positions[i], column_offset, row_offset);
}

static void clear_keys(RyosconfigKeyboardSelector *keyboard_selector) {
	RyosconfigKeyboardSelectorPrivate *priv = keyboard_selector->priv;
	guint i;

	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i) {
		if (priv->keys[i]) {
			gtk_widget_destroy(GTK_WIDGET(priv->keys[i]));
			priv->keys[i] = NULL;
		}
	}
}

static void set_tooltips(RyosconfigKeyboardSelector *keyboard_selector) {
	guint i;
	
	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i) {
		switch (ryos_key_relations[i].keys_type) {
		case RYOS_KEY_RELATION_KEYS_TYPE_MACRO:
		case RYOS_KEY_RELATION_KEYS_TYPE_EASYZONE:
		case RYOS_KEY_RELATION_KEYS_TYPE_FUNCTION:
		case RYOS_KEY_RELATION_KEYS_TYPE_THUMBSTER:
			set_standard_tooltip(keyboard_selector, i);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_CAPSLOCK:
			set_capslock_tooltip(keyboard_selector, i);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_FN:
			set_fn_tooltip(keyboard_selector, i);
			break;
		default:
			break;
		};
	}
}

void ryosconfig_keyboard_selector_set_layout(RyosconfigKeyboardSelector *keyboard_selector, gchar const *layout) {
	clear_keys(keyboard_selector);
	create_block(keyboard_selector, ryos_key_positions_escape, 5, 0);
	create_block(keyboard_selector, ryos_key_positions_function, 13, 0);
	create_block(keyboard_selector, ryos_key_positions_macro, 0, 5);
	create_block(keyboard_selector, ryos_key_positions_get_main(layout), 5, 5);
	create_block(keyboard_selector, ryos_key_positions_block_above_arrows, 66, 0);
	create_block(keyboard_selector, ryos_key_positions_arrows, 66, 17);
	create_block(keyboard_selector, ryos_key_positions_numpad, 79, 5);
	create_block(keyboard_selector, ryos_key_positions_thumbster, 26, 26);

	set_tooltips(keyboard_selector);
	gtk_widget_show_all(GTK_WIDGET(keyboard_selector->priv->table));
	button_set_active(keyboard_selector, ryos_key_positions_escape[0].relation_index);
}

void ryosconfig_keyboard_selector_set_from_rkp(RyosconfigKeyboardSelector *selector, RyosRkp const *rkp) {
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;
	RyosconfigKeyboardSelectorPrivateData *data = priv->data;
	RyosKeyComboBoxData *standarddata;
	RyosKeyComboBoxData *easyshiftdata;
	RyosRkpData const *rkpdata;
	guint i;
	RyosKeyRelation const *key;

	rkpdata = &rkp->data;

	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i) {
		standarddata = &data->standard_datas[i];
		easyshiftdata = &data->easyshift_datas[i];
		key = &ryos_key_relations[i];

		if (key->macro_index != RYOS_KEY_RELATIONS_INDEX_INVALID) {
			ryos_led_macro_copy(&standarddata->led_macro, &rkpdata->led_macros[key->macro_index]);
			ryos_macro_copy(&standarddata->macro, &rkpdata->macros[key->macro_index]);
			ryos_rkp_quicklaunch_copy(&standarddata->quicklaunch, &rkpdata->launchers[key->macro_index]);
			ryos_rkp_timer_copy(&standarddata->timer, &rkpdata->timers[key->macro_index]);
		}

		if (key->macro_index_easyshift != RYOS_KEY_RELATIONS_INDEX_INVALID) {
			ryos_led_macro_copy(&easyshiftdata->led_macro, &rkpdata->led_macros[key->macro_index_easyshift]);
			ryos_macro_copy(&easyshiftdata->macro, &rkpdata->macros[key->macro_index_easyshift]);
			ryos_rkp_quicklaunch_copy(&easyshiftdata->quicklaunch, &rkpdata->launchers[key->macro_index_easyshift]);
			ryos_rkp_timer_copy(&easyshiftdata->timer, &rkpdata->timers[key->macro_index_easyshift]);
		}

		switch (key->keys_type) {
		case RYOS_KEY_RELATION_KEYS_TYPE_MACRO:
			ryos_key_copy(&standarddata->key, &rkpdata->keys_macro.keys[key->keys_index]);
			ryos_key_copy(&easyshiftdata->key, &rkpdata->keys_macro.keys[key->keys_index_easyshift]);
			ryos_rkp_talk_copy(&standarddata->talk, &rkpdata->talks[key->talk_index]);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_EASYZONE:
			ryos_key_set_to_normal(&standarddata->key, rkpdata->keys_primary.keys[key->primary_index]);
			ryos_key_copy(&easyshiftdata->key, &rkpdata->keys_easyzone.keys[key->keys_index_easyshift]);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_FUNCTION:
			ryos_key_copy(&standarddata->key, &rkpdata->keys_function.keys[key->keys_index]);
			ryos_key_copy(&easyshiftdata->key, &rkpdata->keys_function.keys[key->keys_index_easyshift]);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_THUMBSTER:
			ryos_key_copy(&standarddata->key, &rkpdata->keys_thumbster.keys[key->keys_index]);
			ryos_key_copy(&easyshiftdata->key, &rkpdata->keys_thumbster.keys[key->keys_index_easyshift]);
			ryos_rkp_talk_copy(&standarddata->talk, &rkpdata->talks[key->talk_index]);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_CAPSLOCK:
			ryos_key_set_to_normal(&standarddata->key, rkpdata->keys_extra.capslock);
			ryos_rkp_talk_copy(&standarddata->talk, &rkpdata->talks[key->talk_index]);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_FN:
			ryos_fn_selector_set_value(RYOS_FN_SELECTOR(priv->fn_selector), rkpdata->keys_extra.fn);
			break;
		default:
			break;
		};
	}

	set_tooltips(selector);
	
	ryos_stored_lights_copy(&data->lights_automatic, &rkpdata->lights_automatic);
	ryos_stored_lights_copy(&data->lights_manual, &rkpdata->lights_manual);

	ryosconfig_illumination_frame_set_mode_blocked(priv->illumination_frame, rkp->data.light.illumination_mode);
	change_illumination_pointer(selector);

	button_update_active(selector);
}

void ryosconfig_keyboard_selector_update_rkp(RyosconfigKeyboardSelector *selector, RyosRkp *rkp) {
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;
	RyosconfigKeyboardSelectorPrivateData *data = priv->data;
	RyosKeysPrimary *keys_primary;
	RyosKeysMacro *keys_macro;
	RyosKeysThumbster *keys_thumbster;
	RyosKeysEasyzone *keys_easyzone;
	RyosKeysFunction *keys_function;
	RyosKeysExtra *keys_extra;
	RyosKeyComboBoxData *standarddata;
	RyosKeyComboBoxData *easyshiftdata;
	guint i;
	RyosKeyRelation const *key;

	keys_primary = ryos_rkp_get_keys_primary(rkp);
	keys_macro = ryos_rkp_get_keys_macro(rkp);
	keys_thumbster = ryos_rkp_get_keys_thumbster(rkp);
	keys_easyzone = ryos_rkp_get_keys_easyzone(rkp);
	keys_function = ryos_rkp_get_keys_function(rkp);
	keys_extra = ryos_rkp_get_keys_extra(rkp);

	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i) {
		standarddata = &data->standard_datas[i];
		easyshiftdata = &data->easyshift_datas[i];
		key = &ryos_key_relations[i];

		switch (key->keys_type) {
		case RYOS_KEY_RELATION_KEYS_TYPE_MACRO:
			ryos_key_copy(&keys_macro->keys[key->keys_index], &standarddata->key);
			ryos_key_copy(&keys_macro->keys[key->keys_index_easyshift], &easyshiftdata->key);
			ryos_rkp_set_talk(rkp, key->talk_index, &standarddata->talk);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_EASYZONE:
			keys_primary->keys[key->primary_index] = standarddata->key.type;
			ryos_key_copy(&keys_easyzone->keys[key->keys_index_easyshift], &easyshiftdata->key);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_FUNCTION:
			ryos_key_copy(&keys_function->keys[key->keys_index], &standarddata->key);
			ryos_key_copy(&keys_function->keys[key->keys_index_easyshift], &easyshiftdata->key);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_THUMBSTER:
			ryos_key_copy(&keys_thumbster->keys[key->keys_index], &standarddata->key);
			ryos_key_copy(&keys_thumbster->keys[key->keys_index_easyshift], &easyshiftdata->key);
			ryos_rkp_set_talk(rkp, key->talk_index, &standarddata->talk);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_CAPSLOCK:
			keys_extra->capslock = standarddata->key.type;
			ryos_rkp_set_talk(rkp, key->talk_index, &standarddata->talk);
			break;
		case RYOS_KEY_RELATION_KEYS_TYPE_FN:
			keys_extra->fn = ryos_fn_selector_get_value(RYOS_FN_SELECTOR(priv->fn_selector));
			break;
		};

		if (key->macro_index != RYOS_KEY_RELATIONS_INDEX_INVALID) {
			ryos_rkp_set_led_macro(rkp, key->macro_index, &standarddata->led_macro);
			ryos_rkp_set_macro(rkp, key->macro_index, &standarddata->macro);
			ryos_rkp_set_quicklaunch(rkp, key->macro_index, &standarddata->quicklaunch);
			ryos_rkp_set_timer(rkp, key->macro_index, &standarddata->timer);
		}

		if (key->macro_index_easyshift != RYOS_KEY_RELATIONS_INDEX_INVALID) {
			ryos_rkp_set_led_macro(rkp, key->macro_index_easyshift, &easyshiftdata->led_macro);
			ryos_rkp_set_macro(rkp, key->macro_index_easyshift, &easyshiftdata->macro);
			ryos_rkp_set_quicklaunch(rkp, key->macro_index_easyshift, &easyshiftdata->quicklaunch);
			ryos_rkp_set_timer(rkp, key->macro_index_easyshift, &easyshiftdata->timer);
		}
	}

	ryos_rkp_set_keys_primary(rkp, keys_primary);
	ryos_rkp_set_keys_macro(rkp, keys_macro);
	ryos_rkp_set_keys_thumbster(rkp, keys_thumbster);
	ryos_rkp_set_keys_easyzone(rkp, keys_easyzone);
	ryos_rkp_set_keys_function(rkp, keys_function);
	ryos_rkp_set_keys_extra(rkp, keys_extra);

	g_free(keys_primary);
	g_free(keys_macro);
	g_free(keys_thumbster);
	g_free(keys_easyzone);
	g_free(keys_function);
	g_free(keys_extra);

	ryos_rkp_set_lights_automatic(rkp, &priv->data->lights_automatic);
	ryos_rkp_set_lights_manual(rkp, &priv->data->lights_manual);
	ryos_rkp_set_light_illumination_mode(rkp, ryosconfig_illumination_frame_get_mode(priv->illumination_frame));
}

void ryosconfig_keyboard_selector_set_macro(RyosconfigKeyboardSelector *selector, guint macro_index, RyosMacro *macro) {
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;
	guint i;
	for (i = 0; i < RYOS_KEY_RELATIONS_NUM; ++i) {
		if (macro_index == ryos_key_relations[i].macro_index) {
			ryos_key_set_to_normal(&priv->data->standard_datas[i].key, RYOS_KEY_TYPE_MACRO);
			ryos_macro_copy(&priv->data->standard_datas[i].macro, macro);
			break;
		} else if (macro_index == ryos_key_relations[i].macro_index_easyshift) {
			ryos_key_set_to_normal(&priv->data->easyshift_datas[i].key, RYOS_KEY_TYPE_MACRO);
			ryos_macro_copy(&priv->data->easyshift_datas[i].macro, macro);
			break;
		}
	}

	if (i >= RYOS_KEY_RELATIONS_NUM)
		return;

	set_standard_tooltip(selector, i);
	if (get_active_index(selector) == i)
		ryos_standard_selector_update(priv->standard_selector);
}

void ryosconfig_keyboard_selector_set_device_type(RyosconfigKeyboardSelector *selector, RoccatDevice const *device) {
	RyosconfigKeyboardSelectorPrivate *priv = selector->priv;
	switch (gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(device))) {
	case USB_DEVICE_ID_ROCCAT_RYOS_MK:
	case USB_DEVICE_ID_ROCCAT_RYOS_MK_GLOW:
		roccat_multiwidget_show(priv->key_illumination_multiwidget, GTK_WIDGET(priv->key_illumination_nothing));
		stop_blinking(selector);
		break;
	case USB_DEVICE_ID_ROCCAT_RYOS_MK_PRO:
		roccat_multiwidget_show(priv->key_illumination_multiwidget, GTK_WIDGET(priv->key_illumination_box));
		start_blinking(selector);
		break;
	default:
		break;
	}
	ryosconfig_illumination_frame_set_device_type(selector->priv->illumination_frame, device);
}
