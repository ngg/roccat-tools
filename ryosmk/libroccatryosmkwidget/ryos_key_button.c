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
#include "ryos_key_button.h"

#define RYOS_KEY_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOS_KEY_BUTTON_TYPE, RyosKeyButtonClass))
#define IS_RYOS_KEY_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOS_KEY_BUTTON_TYPE))
#define RYOS_KEY_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOS_KEY_BUTTON_TYPE, RyosKeyButtonPrivate))

typedef struct _RyosKeyButtonClass RyosKeyButtonClass;
typedef struct _RyosKeyButtonPrivate RyosKeyButtonPrivate;

struct _RyosKeyButton {
	GtkToggleButton parent;
	RyosKeyButtonPrivate *priv;
};

struct _RyosKeyButtonClass {
	GtkToggleButtonClass parent_class;
};

struct _RyosKeyButtonPrivate {
	gchar *label_text;
	GtkLabel *label;
	gboolean light_on;
};

G_DEFINE_TYPE(RyosKeyButton, ryos_key_button, GTK_TYPE_TOGGLE_BUTTON);

gchar *hid_to_beauty_keyname(guint8 hid) {
	gchar *text;

	switch(hid) {
	case 0x28: // return
		text = g_strdup("\xe2\x86\xb5");
		break;
	case 0x29:
		text = g_strdup("ESC");
		break;
	case 0x2a: // backspace
		text = g_strdup("\xe2\x9f\xb5");
		break;
	case 0x2b: // tab
		text = g_strdup("\xe2\x86\xb9");
		break;
	case 0x2c:
		text = g_strdup("Space");
		break;
	case 0x39: // capslock
		text = g_strdup("\xe2\x87\xa9");
		break;
	case 0x47:
		text = g_strdup("Scroll\nLock");
		break;
	case 0x4b: // page up
		text = g_strdup("Pg\xe2\x86\x91");
		break;
	case 0x4e: // page down
		text = g_strdup("Pg\xe2\x86\x93");
		break;
	case 0x4f: // arrow right
		text = g_strdup("\xe2\x86\x92");
		break;
	case 0x50: // arrow left
		text = g_strdup("\xe2\x86\x90");
		break;
	case 0x51: // arrow down
		text = g_strdup("\xe2\x86\x93");
		break;
	case 0x52: // arrow up
		text = g_strdup("\xe2\x86\x91");
		break;
	case 0x53:
		text = g_strdup("Num\nLock");
		break;
	case 0x58: // kp enter
		text = g_strdup("Enter");
		break;
	case 0x59:
		text = g_strdup("1");
		break;
	case 0x5a:
		text = g_strdup("2");
		break;
	case 0x5b:
		text = g_strdup("3");
		break;
	case 0x5c:
		text = g_strdup("4");
		break;
	case 0x5d:
		text = g_strdup("5");
		break;
	case 0x5e:
		text = g_strdup("6");
		break;
	case 0x5f:
		text = g_strdup("7");
		break;
	case 0x60:
		text = g_strdup("8");
		break;
	case 0x61:
		text = g_strdup("9");
		break;
	case 0x62:
		text = g_strdup("0");
		break;
	case 0xaa:
		text = g_strdup("M1");
		break;
	case 0xab:
		text = g_strdup("M2");
		break;
	case 0xac:
		text = g_strdup("M3");
		break;
	case 0xad:
		text = g_strdup("M4");
		break;
	case 0xae:
		text = g_strdup("M5");
		break;
	case 0xe0:
	case 0xe4:
		text = g_strdup("CTRL");
		break;
	case 0xe1:
	case 0xe5: // shift
		text = g_strdup("\xe2\x87\xa7");
		break;
	case 0xe2:
	case 0xe6:
		text = g_strdup("ALT");
		break;
	case 0xe3:
		text = g_strdup("GUI");
		break;
	case 0xf1:
		text = g_strdup("FN");
		break;
	case 0xf2:
		text = g_strdup("T1");
		break;
	case 0xf3:
		text = g_strdup("T2");
		break;
	case 0xf4:
		text = g_strdup("T3");
		break;
	default:
		text = gaminggear_hid_to_keyname(hid);
		break;
	}
	return text;
}

static void button_size_request_cb(GtkWidget *button, GtkRequisition *requisition, gpointer user_data) {
	requisition->width = requisition->height;
}

static void set_markup(RyosKeyButton *key_button) {
	RyosKeyButtonPrivate *priv = key_button->priv;
	gchar *markup;

	if (!priv->label_text)
		return;

	if (priv->light_on) {
		markup = g_markup_printf_escaped("<b>%s</b>", priv->label_text);
		gtk_label_set_markup(priv->label, markup);
		g_free(markup);
	} else {
		gtk_label_set_text(priv->label, priv->label_text);
	}

}

void ryos_key_button_set_light(RyosKeyButton *key_button, gboolean state) {
	key_button->priv->light_on = state;
	set_markup(key_button);
}

void ryos_key_button_set_label(RyosKeyButton *key_button, guint hid_usage_id) {
	RyosKeyButtonPrivate *priv = key_button->priv;

	g_free(priv->label_text);
	priv->label_text = hid_to_beauty_keyname(hid_usage_id);
	set_markup(key_button);
}

GtkWidget *ryos_key_button_new(guint hid_usage_id) {
	RyosKeyButton *key_button;

	key_button = RYOS_KEY_BUTTON(g_object_new(RYOS_KEY_BUTTON_TYPE, NULL));

	g_signal_connect(G_OBJECT(key_button), "size-request", G_CALLBACK(button_size_request_cb), NULL);
	ryos_key_button_set_label(key_button, hid_usage_id);

	return GTK_WIDGET(key_button);
}

static void ryos_key_button_init(RyosKeyButton *key_button) {
	RyosKeyButtonPrivate *priv = RYOS_KEY_BUTTON_GET_PRIVATE(key_button);
	key_button->priv = priv;

	priv->label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_justify(priv->label, GTK_JUSTIFY_CENTER);
	gtk_label_set_width_chars(priv->label, 3);
	gtk_label_set_max_width_chars(priv->label, 3);
	gtk_container_add(GTK_CONTAINER(key_button), GTK_WIDGET(priv->label));

	priv->label_text = NULL;
}

static void finalize(GObject *object) {
	RyosKeyButtonPrivate *priv = RYOS_KEY_BUTTON(object)->priv;
	G_OBJECT_CLASS(ryos_key_button_parent_class)->finalize(object);
	g_free(priv->label_text);
}

static void ryos_key_button_class_init(RyosKeyButtonClass *klass) {
	GObjectClass *gobject_class;

	gobject_class = (GObjectClass*)klass;
	gobject_class->finalize = finalize;

	g_type_class_add_private(klass, sizeof(RyosKeyButtonPrivate));
}
