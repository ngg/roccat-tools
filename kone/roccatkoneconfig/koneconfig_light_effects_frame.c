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

#include "koneconfig_light_effects_frame.h"
#include "gtk_roccat_helper.h"
#include <gaminggear/gaminggear_hscale.h>
#include "i18n.h"

#define KONECONFIG_LIGHT_EFFECTS_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KONECONFIG_LIGHT_EFFECTS_FRAME_TYPE, KoneconfigLightEffectsFrameClass))
#define IS_KONECONFIG_LIGHT_EFFECTS_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KONECONFIG_LIGHT_EFFECTS_FRAME_TYPE))
#define KONECONFIG_LIGHT_EFFECTS_FRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KONECONFIG_LIGHT_EFFECTS_FRAME_TYPE, KoneconfigLightEffectsFramePrivate))

typedef struct _KoneconfigLightEffectsFrameClass KoneconfigLightEffectsFrameClass;
typedef struct _KoneconfigLightEffectsFramePrivate KoneconfigLightEffectsFramePrivate;

struct _KoneconfigLightEffectsFrame {
	GtkFrame parent;
	KoneconfigLightEffectsFramePrivate *priv;
};

struct _KoneconfigLightEffectsFrameClass {
	GtkFrameClass parent_class;
};

struct _KoneconfigLightEffectsFramePrivate {
	GSList *radios_1;
	GSList *radios_2;
	GSList *radios_3;
	GaminggearHScale *speed;
};

G_DEFINE_TYPE(KoneconfigLightEffectsFrame, koneconfig_light_effects_frame, GTK_TYPE_FRAME);

static gchar const * const value_key = "value";

static void pack_radio_button(gpointer button, gpointer parent) {
	gtk_box_pack_start(parent, button, TRUE, TRUE, 0);
}

static GtkRadioButton *find_radio(GSList *list, guint wanted) {
	GSList *child;
	guint value;

	for (child = list; child; child = g_slist_next(child)) {
		value = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(child->data), value_key));
		if (value == wanted)
			return child->data;
	}
	return NULL;
}

void koneconfig_light_effects_frame_set_from_rmp(KoneconfigLightEffectsFrame *frame, KoneRMP *rmp) {
	KoneconfigLightEffectsFramePrivate *priv = frame->priv;
	GtkRadioButton *radio;

	radio = find_radio(priv->radios_1, kone_rmp_get_light_effect_1(rmp));
	if (radio)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);

	radio = find_radio(priv->radios_2, kone_rmp_get_light_effect_2(rmp));
	if (radio)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);

	radio = find_radio(priv->radios_3, kone_rmp_get_light_effect_3(rmp));
	if (radio)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);

	gaminggear_hscale_set_value(priv->speed, kone_rmp_get_light_effect_speed(rmp));
}

void koneconfig_light_effects_frame_update_rmp(KoneconfigLightEffectsFrame *frame, KoneRMP *rmp) {
	KoneconfigLightEffectsFramePrivate *priv = frame->priv;
	GtkWidget *active;
	guint value;

	active = gtk_roccat_radio_button_group_get_active(priv->radios_1);
	value = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(active), value_key));
	kone_rmp_set_light_effect_1(rmp, value);

	active = gtk_roccat_radio_button_group_get_active(priv->radios_2);
	value = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(active), value_key));
	kone_rmp_set_light_effect_2(rmp, value);

	active = gtk_roccat_radio_button_group_get_active(priv->radios_3);
	value = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(active), value_key));
	kone_rmp_set_light_effect_3(rmp, value);

	kone_rmp_set_light_effect_speed(rmp, gaminggear_hscale_get_value(priv->speed));
}

GtkWidget *koneconfig_light_effects_frame_new(void) {
	KoneconfigLightEffectsFrame *frame;

	frame = KONECONFIG_LIGHT_EFFECTS_FRAME(g_object_new(KONECONFIG_LIGHT_EFFECTS_FRAME_TYPE,
			NULL));

	return GTK_WIDGET(frame);
}

static void add_radios_1(GtkBox *parent, KoneconfigLightEffectsFrame *frame) {
	KoneconfigLightEffectsFramePrivate *priv = frame->priv;
	GtkWidget *radio1, *radio2, *radio3;

	radio3 = gtk_radio_button_new_with_label(NULL, _("Random multi"));
	g_object_set_data(G_OBJECT(radio3), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_1_RANDOM_MULTI));

	radio2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio3), _("Random single"));
	g_object_set_data(G_OBJECT(radio2), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_1_RANDOM_SINGLE));

	radio1 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio3), _("Selected color"));
	g_object_set_data(G_OBJECT(radio1), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_1_SELECTED_COLOR));

	priv->radios_1 = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1));
	g_slist_foreach(priv->radios_1, pack_radio_button, parent);
}

static void add_radios_2(GtkBox *parent, KoneconfigLightEffectsFrame *frame) {
	KoneconfigLightEffectsFramePrivate *priv = frame->priv;
	GtkWidget *radio1, *radio2, *radio3, *radio4, *radio5;

	radio5 = gtk_radio_button_new_with_label(NULL, _("Move horizontal"));
	g_object_set_data(G_OBJECT(radio5), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_2_MOVE_HORIZONTAL));

	radio4 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio5), _("Move vertical"));
	g_object_set_data(G_OBJECT(radio4), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_2_MOVE_VERTICAL));

	radio3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio5), _("Rotate counterclockwise"));
	g_object_set_data(G_OBJECT(radio3), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_2_ROTATE_COUNTERCLOCKWISE));

	radio2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio5), _("Rotate clockwise"));
	g_object_set_data(G_OBJECT(radio2), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_2_ROTATE_CLOCKWISE));

	radio1 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio5), _("Fixed color"));
	g_object_set_data(G_OBJECT(radio1), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_2_FIXED_COLOR));

	priv->radios_2 = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1));
	g_slist_foreach(priv->radios_2, pack_radio_button, parent);
}

static void add_radios_3(GtkBox *parent, KoneconfigLightEffectsFrame *frame) {
	KoneconfigLightEffectsFramePrivate *priv = frame->priv;
	GtkWidget *radio1, *radio2, *radio3, *radio4;

	radio4 = gtk_radio_button_new_with_label(NULL, _("Heartbeat"));
	g_object_set_data(G_OBJECT(radio4), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_3_HEARTBEAT));

	radio3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio4), _("Blinking"));
	g_object_set_data(G_OBJECT(radio3), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_3_BLINKING));

	radio2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio4), _("Pulsating"));
	g_object_set_data(G_OBJECT(radio2), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_3_PULSATING));

	radio1 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio4), _("Fully lighted"));
	g_object_set_data(G_OBJECT(radio1), value_key, GUINT_TO_POINTER(KONE_LIGHT_EFFECT_3_FULL_LIGHTENED));

	priv->radios_3 = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1));
	g_slist_foreach(priv->radios_3, pack_radio_button, parent);
}

static void add_speed(GtkBox *parent, KoneconfigLightEffectsFrame *frame) {
	KoneconfigLightEffectsFramePrivate *priv = frame->priv;
	GtkWidget *hbox;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(parent, hbox, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Effect speed")), FALSE, FALSE, 0);

	priv->speed = GAMINGGEAR_HSCALE(gaminggear_hscale_new_with_range(KONE_LIGHT_EFFECT_SPEED_MIN, KONE_LIGHT_EFFECT_SPEED_MAX, 1.0));
	gaminggear_hscale_set_draw_spin(priv->speed, FALSE);

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(priv->speed), TRUE, TRUE, 0);
}

static void koneconfig_light_effects_frame_init(KoneconfigLightEffectsFrame *frame) {
	KoneconfigLightEffectsFramePrivate *priv = KONECONFIG_LIGHT_EFFECTS_FRAME_GET_PRIVATE(frame);
	GtkBox *box;

	frame->priv = priv;

	box = GTK_BOX(gtk_vbox_new(FALSE, 0));

	add_radios_1(box, frame);
	gtk_box_pack_start(box, gtk_hseparator_new(), TRUE, TRUE, 0);
	add_radios_2(box, frame);
	gtk_box_pack_start(box, gtk_hseparator_new(), TRUE, TRUE, 0);
	add_radios_3(box, frame);
	gtk_box_pack_start(box, gtk_hseparator_new(), TRUE, TRUE, 0);
	add_speed(box, frame);

	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(box));

	gtk_frame_set_label(GTK_FRAME(frame), _("Light effects"));
}

static void koneconfig_light_effects_frame_class_init(KoneconfigLightEffectsFrameClass *klass) {
	g_type_class_add_private(klass, sizeof(KoneconfigLightEffectsFramePrivate));
}
