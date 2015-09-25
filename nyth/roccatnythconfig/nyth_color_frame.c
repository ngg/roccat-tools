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

#include "nyth_color_frame.h"
#include "roccat_color_selection_button.h"
#include "roccat_helper.h"
#include "i18n.h"

#define NYTH_COLOR_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), NYTH_COLOR_FRAME_TYPE, NythColorFrameClass))
#define IS_NYTH_COLOR_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), NYTH_COLOR_FRAME_TYPE))
#define NYTH_COLOR_FRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), NYTH_COLOR_FRAME_TYPE, NythColorFramePrivate))

typedef struct _NythColorFrameClass NythColorFrameClass;
typedef struct _NythColorFramePrivate NythColorFramePrivate;

enum {
	NYTH_COLORS_NUM = 33,
};

struct _NythColorFrame {
	GtkFrame parent;
	NythColorFramePrivate *priv;
};

struct _NythColorFrameClass {
	GtkFrameClass parent_class;
};

struct _NythColorFramePrivate {
	GtkToggleButton *light_on;
	RoccatColorSelectionButton *colors[NYTH_LIGHTS_NUM];
};

G_DEFINE_TYPE(NythColorFrame, nyth_color_frame, GTK_TYPE_FRAME);

static gchar const * const labels[NYTH_LIGHTS_NUM] = {
	N_("Logo"),
	N_("Bottom"),
};

static gchar const * const value_key = "value";

static GdkColor const colors[NYTH_COLORS_NUM] = {
	{0, 179 * 256,   0 * 256,   0 * 256},
	{0, 255 * 256,   0 * 256,   0 * 256},
	{0, 255 * 256,  71 * 256,   0 * 256},
	{0, 255 * 256, 106 * 256,   0 * 256},
	{0, 255 * 256, 157 * 256,  71 * 256},
	{0, 255 * 256, 186 * 256,  89 * 256},
	{0, 248 * 256, 232 * 256,   0 * 256},
	{0, 246 * 256, 255 * 256,  78 * 256},
	{0, 201 * 256, 255 * 256,  78 * 256},
	{0, 165 * 256, 255 * 256,  78 * 256},
	{0, 132 * 256, 255 * 256,  78 * 256},
	{0,   0 * 256, 255 * 256,   0 * 256},
	{0,   0 * 256, 207 * 256,  55 * 256},
	{0,   0 * 256, 166 * 256,  44 * 256},
	{0,   0 * 256, 207 * 256, 124 * 256},
	{0,   0 * 256, 207 * 256, 158 * 256},
	{0,   0 * 256, 203 * 256, 207 * 256},
	{0, 107 * 256, 255 * 256, 227 * 256},
	{0,  41 * 256, 197 * 256, 255 * 256},
	{0,  37 * 256, 162 * 256, 162 * 256},
	{0,  99 * 256, 158 * 256, 239 * 256},
	{0,  37 * 256, 132 * 256, 233 * 256},
	{0,   0 * 256,  72 * 256, 255 * 256},
	{0,  15 * 256,  15 * 256, 255 * 256},
	{0,  15 * 256,  15 * 256, 188 * 256},
	{0,  89 * 256,   7 * 256, 255 * 256},
	{0, 121 * 256,  12 * 256, 255 * 256},
	{0, 161 * 256,  12 * 256, 255 * 256},
	{0, 170 * 256, 108 * 256, 232 * 256},
	{0, 181 * 256,  10 * 256, 255 * 256},
	{0, 201 * 256,  12 * 256, 255 * 256},
	{0, 205 * 256,  10 * 256, 217 * 256},
	{0, 217 * 256,  10 * 256, 125 * 256},
};

static void nyth_light_to_color(NythLight const *light, GdkColor *color) {
	color->pixel = 0;
	color->red = light->red * 256;
	color->green = light->green * 256;
	color->blue = light->blue * 256;
}

void nyth_color_frame_set_from_profile_data(NythColorFrame *frame, NythProfileData const *profile_data) {
	NythColorFramePrivate *priv = frame->priv;
	NythProfileSettings const *profile_settings = &profile_data->hardware.profile_settings;
	GdkColor color;
	guint i;
	gboolean custom;

	gtk_toggle_button_set_active(priv->light_on, profile_settings->lights_enabled & NYTH_PROFILE_SETTINGS_LIGHTS_ENABLED_MASK);

	for (i = 0; i < NYTH_LIGHTS_NUM; ++i) {
		nyth_light_to_color(&profile_settings->lights[i], &color);
		custom = roccat_get_bit8(profile_settings->lights_enabled, i + NYTH_PROFILE_SETTINGS_LIGHTS_ENABLED_BIT_CUSTOM_COLOR_LOGO);
		roccat_color_selection_button_set_custom_color(priv->colors[i], &color);
		roccat_color_selection_button_set_palette_index(priv->colors[i], profile_settings->lights[i].index);
		roccat_color_selection_button_set_active_type(priv->colors[i], custom ? ROCCAT_COLOR_SELECTION_DIALOG_TYPE_CUSTOM : ROCCAT_COLOR_SELECTION_DIALOG_TYPE_PALETTE);
	}
}

void nyth_color_frame_update_profile_data(NythColorFrame *frame, NythProfileData *profile_data) {
	NythColorFramePrivate *priv = frame->priv;
	NythProfileSettings *profile_settings = &profile_data->hardware.profile_settings;
	guint8 lights_enabled;
	NythLight light;
	GdkColor color;
	guint i;

	lights_enabled = gtk_toggle_button_get_active(priv->light_on) ? NYTH_PROFILE_SETTINGS_LIGHTS_ENABLED_ON : NYTH_PROFILE_SETTINGS_LIGHTS_ENABLED_OFF;

	for (i = 0; i < NYTH_LIGHTS_NUM; ++i) {
		roccat_set_bit8(&lights_enabled, i + NYTH_PROFILE_SETTINGS_LIGHTS_ENABLED_BIT_CUSTOM_COLOR_LOGO,
				roccat_color_selection_button_get_active_type(priv->colors[i]) == ROCCAT_COLOR_SELECTION_DIALOG_TYPE_CUSTOM);

		roccat_color_selection_button_get_custom_color(priv->colors[i], &color);

		light.index = roccat_color_selection_button_get_palette_index(priv->colors[i]);
		light.red = color.red / 256;
		light.green = color.green / 256;
		light.blue = color.blue / 256;

		if (!nyth_light_equal(&light, &profile_settings->lights[i])) {
			nyth_light_copy(&profile_settings->lights[i], &light);
			profile_data->hardware.modified_profile_settings = TRUE;
		}
	}

	if (lights_enabled != profile_settings->lights_enabled) {
		profile_settings->lights_enabled = lights_enabled;
		profile_data->hardware.modified_profile_settings = TRUE;
	}
}

GtkWidget *nyth_color_frame_new(void) {
	return GTK_WIDGET(g_object_new(NYTH_COLOR_FRAME_TYPE, NULL));
}

static void nyth_color_frame_init(NythColorFrame *frame) {
	NythColorFramePrivate *priv = NYTH_COLOR_FRAME_GET_PRIVATE(frame);
	GtkTable *table;
	GtkVBox *vbox;
	guint i;

	frame->priv = priv;

	vbox = GTK_VBOX(gtk_vbox_new(FALSE, 0));
	table = GTK_TABLE(gtk_table_new(NYTH_LIGHTS_NUM, 2, FALSE));

	priv->light_on = GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label(_("Lights on")));

	for (i = 0; i < NYTH_LIGHTS_NUM; ++i) {
		gtk_table_attach(table, gtk_label_new(_N(labels[i])), 0, 1, 0 + i, 1 + i, GTK_EXPAND, GTK_EXPAND, 0, 0);

		priv->colors[i] = ROCCAT_COLOR_SELECTION_BUTTON(roccat_color_selection_button_new());
		roccat_color_selection_button_set_view_type(priv->colors[i], ROCCAT_COLOR_SELECTION_DIALOG_TYPE_BOTH);
		roccat_color_selection_button_set_palette_colors(priv->colors[i], colors, NYTH_COLORS_NUM, 11);
		gtk_table_attach(table, GTK_WIDGET(priv->colors[i]), 1, 2, 0 + i, 1 + i, GTK_EXPAND, GTK_EXPAND, 0, 0);
	}

	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->light_on), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(table), TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(vbox));
	gtk_frame_set_label(GTK_FRAME(frame), _("Light"));
}

static void nyth_color_frame_class_init(NythColorFrameClass *klass) {
	g_type_class_add_private(klass, sizeof(NythColorFramePrivate));
}
