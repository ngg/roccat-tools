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

#include "konextd_color_frame.h"
#include "roccat_color_selection_button.h"
#include "i18n-lib.h"

#define KONEXTD_COLOR_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KONEXTD_COLOR_FRAME_TYPE, KonextdColorFrameClass))
#define IS_KONEXTD_COLOR_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KONEXTD_COLOR_FRAME_TYPE))
#define KONEXTD_COLOR_FRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KONEXTD_COLOR_FRAME_TYPE, KonextdColorFramePrivate))

typedef struct _KonextdColorFrameClass KonextdColorFrameClass;
typedef struct _KonextdColorFramePrivate KonextdColorFramePrivate;

struct _KonextdColorFrame {
	GtkFrame parent;
	KonextdColorFramePrivate *priv;
};

struct _KonextdColorFrameClass {
	GtkFrameClass parent_class;
};

struct _KonextdColorFramePrivate {
	GtkToggleButton *use_palette;
	GtkToggleButton *use_custom;
	RoccatColorSelectionButton *colors[KONEPLUS_PROFILE_SETTING_LIGHTS_NUM];
	GtkToggleButton *states[KONEPLUS_PROFILE_SETTING_LIGHTS_NUM];
};

/* these are the colors for on screen display, which differ from the device colors */
static GdkColor const colors[KONEPLUS_RMP_LIGHT_INFO_COLORS_NUM] = {
	{0, 0xb4 * 256, 0x00 * 256, 0x01 * 256},
	{0, 0xfe * 256, 0x00 * 256, 0x00 * 256},
	{0, 0xff * 256, 0x47 * 256, 0x00 * 256},
	{0, 0xff * 256, 0x6b * 256, 0x01 * 256},
	{0, 0xff * 256, 0x9d * 256, 0x48 * 256},
	{0, 0xff * 256, 0xbb * 256, 0x5a * 256},
	{0, 0xf9 * 256, 0xe9 * 256, 0x00 * 256},
	{0, 0xf6 * 256, 0xff * 256, 0x4e * 256},
	{0, 0xc9 * 256, 0xff * 256, 0x4f * 256},
	{0, 0xa5 * 256, 0xff * 256, 0x4f * 256},
	{0, 0x84 * 256, 0xff * 256, 0x4e * 256},
	{0, 0x00 * 256, 0xff * 256, 0x01 * 256},
	{0, 0x00 * 256, 0xcf * 256, 0x37 * 256},
	{0, 0x01 * 256, 0xa6 * 256, 0x2d * 256},
	{0, 0x00 * 256, 0xd0 * 256, 0x7c * 256},
	{0, 0x00 * 256, 0xd0 * 256, 0x9f * 256},
	{0, 0x00 * 256, 0xca * 256, 0xce * 256},
	{0, 0x6b * 256, 0xe1 * 256, 0xe3 * 256},
	{0, 0x29 * 256, 0xc5 * 256, 0xfe * 256},
	{0, 0x4c * 256, 0xb3 * 256, 0xee * 256},
	{0, 0x63 * 256, 0x9d * 256, 0xef * 256},
	{0, 0x24 * 256, 0x84 * 256, 0xea * 256},
	{0, 0x00 * 256, 0x48 * 256, 0xff * 256},
	{0, 0x10 * 256, 0x0f * 256, 0xff * 256},
	{0, 0x0f * 256, 0x0f * 256, 0xbd * 256},
	{0, 0x59 * 256, 0x07 * 256, 0xff * 256},
	{0, 0x79 * 256, 0x0c * 256, 0xff * 256},
	{0, 0xa1 * 256, 0x0c * 256, 0xfe * 256},
	{0, 0xaa * 256, 0x6c * 256, 0xe9 * 256},
	{0, 0xb3 * 256, 0x0a * 256, 0xd7 * 256},
	{0, 0xc9 * 256, 0x0b * 256, 0xff * 256},
	{0, 0xce * 256, 0x0a * 256, 0xda * 256},
	{0, 0xd9 * 256, 0x0a * 256, 0x7c * 256},
};

G_DEFINE_TYPE(KonextdColorFrame, konextd_color_frame, GTK_TYPE_FRAME);

static void koneplus_rmp_light_info_to_color(KoneplusRmpLightInfo const *light_info, GdkColor *color) {
	color->pixel = 0;
	color->red = light_info->red * 256;
	color->green = light_info->green * 256;
	color->blue = light_info->blue * 256;
}

static void koneplus_color_to_rmp_light_info(GdkColor const *color, KoneplusRmpLightInfo *light_info, guint index, guint state) {
	light_info->index = index;
	light_info->state = state;
	light_info->red = color->red / 256;
	light_info->green = color->green / 256;
	light_info->blue = color->blue / 256;
}

static void update(KonextdColorFrame *frame) {
	KonextdColorFramePrivate *priv = frame->priv;
	gboolean use_palette = gtk_toggle_button_get_active(priv->use_palette);
	guint i;

	for (i = 0; i < KONEPLUS_PROFILE_SETTING_LIGHTS_NUM; ++i)
		roccat_color_selection_button_set_view_type(priv->colors[i], (use_palette) ? ROCCAT_COLOR_SELECTION_DIALOG_TYPE_PALETTE : ROCCAT_COLOR_SELECTION_DIALOG_TYPE_CUSTOM);
}

void konextd_color_frame_set_from_rmp(KonextdColorFrame *frame, KoneplusRmp *rmp) {
	KonextdColorFramePrivate *priv = frame->priv;
	KoneplusRmpLightInfo *light_info;
	GdkColor color;
	guint i;

	if (konextd_rmp_get_light_chose_type(rmp) == KONEXTD_RMP_LIGHT_CHOSE_TYPE_TABLE)
		gtk_toggle_button_set_active(priv->use_palette, TRUE);
	else
		gtk_toggle_button_set_active(priv->use_custom, TRUE);

	for (i = 0; i < KONEPLUS_PROFILE_SETTING_LIGHTS_NUM; ++i) {
		light_info = koneplus_rmp_get_rmp_light_info(rmp, i);
		koneplus_rmp_light_info_to_color(light_info, &color);
		roccat_color_selection_button_set_custom_color(priv->colors[i], &color);
		roccat_color_selection_button_set_palette_index(priv->colors[i], light_info->index);
		gtk_toggle_button_set_active(priv->states[i], (light_info->state == KONEPLUS_RMP_LIGHT_INFO_STATE_ON));
		g_free(light_info);
	}

	update(frame);
}

void konextd_color_frame_update_rmp(KonextdColorFrame *frame, KoneplusRmp *rmp) {
	KonextdColorFramePrivate *priv = frame->priv;
	KoneplusRmpLightInfo light_info;
	KoneplusRmpLightInfo const *standard_light_info;
	guint color_index, state;
	guint i;
	GdkColor color;

	konextd_rmp_set_light_chose_type(rmp, gtk_toggle_button_get_active(priv->use_palette) ? KONEXTD_RMP_LIGHT_CHOSE_TYPE_TABLE : KONEXTD_RMP_LIGHT_CHOSE_TYPE_CUSTOM);

	for (i = 0; i < KONEPLUS_PROFILE_SETTING_LIGHTS_NUM; ++i) {
		color_index = roccat_color_selection_button_get_palette_index(priv->colors[i]);
		state = gtk_toggle_button_get_active(priv->states[i]) ?
			KONEPLUS_RMP_LIGHT_INFO_STATE_ON : KONEPLUS_RMP_LIGHT_INFO_STATE_OFF;

		standard_light_info = koneplus_rmp_light_info_get_standard(color_index);
		light_info.index = standard_light_info->index;
		light_info.state = state;
		light_info.red = standard_light_info->red;
		light_info.green = standard_light_info->green;
		light_info.blue = standard_light_info->blue;
		koneplus_rmp_set_rmp_light_info(rmp, i, &light_info);

		roccat_color_selection_button_get_custom_color(priv->colors[i], &color);
		koneplus_color_to_rmp_light_info(&color, &light_info, KONEPLUS_LIGHT_INFO_INDEX_CUSTOM, state);
		konextd_rmp_set_custom_light_info(rmp, i, &light_info);
	}
}

static void use_palette_cb(GtkToggleButton *toggle_button, gpointer user_data) {
	update(KONEXTD_COLOR_FRAME(user_data));
}

GtkWidget *konextd_color_frame_new(void) {
	return GTK_WIDGET(g_object_new(KONEXTD_COLOR_FRAME_TYPE, NULL));
}

static void konextd_color_frame_init(KonextdColorFrame *frame) {
	KonextdColorFramePrivate *priv = KONEXTD_COLOR_FRAME_GET_PRIVATE(frame);
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *vbox;
	guint i;

	frame->priv = priv;

	/* order is in KoneplusProfileSettingsLightIndex */
	struct {
		guint left_attach, top_attach;
	} lights[KONEPLUS_PROFILE_SETTING_LIGHTS_NUM] = {
			{0, 0},
			{0, 1},
			{1, 0},
			{1, 1}
	};

	vbox = gtk_vbox_new(FALSE, 0);
	table = gtk_table_new(2, 2, TRUE);

	priv->use_palette = GTK_TOGGLE_BUTTON(gtk_radio_button_new_with_label(NULL, _("Palette")));
	g_signal_connect(G_OBJECT(priv->use_palette), "toggled", G_CALLBACK(use_palette_cb), frame);
	priv->use_custom = GTK_TOGGLE_BUTTON(gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(priv->use_palette), _("Custom")));

	for (i = 0; i < KONEPLUS_PROFILE_SETTING_LIGHTS_NUM; ++i) {
		hbox = gtk_hbox_new(FALSE, 0);
		priv->colors[i] = ROCCAT_COLOR_SELECTION_BUTTON(roccat_color_selection_button_new());
		roccat_color_selection_button_set_palette_colors(priv->colors[i], colors, KONEPLUS_RMP_LIGHT_INFO_COLORS_NUM, 0);
		priv->states[i] = GTK_TOGGLE_BUTTON(gtk_check_button_new());

		gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(priv->colors[i]), TRUE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(priv->states[i]), TRUE, FALSE, 0);
		gtk_table_attach(GTK_TABLE(table), hbox,
				lights[i].left_attach, lights[i].left_attach + 1,
				lights[i].top_attach, lights[i].top_attach + 1,
				GTK_EXPAND, GTK_EXPAND, 0, 0
		);
	}

	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->use_palette), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->use_custom), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_frame_set_label(GTK_FRAME(frame), _("Colors"));
}

static void konextd_color_frame_class_init(KonextdColorFrameClass *klass) {
	g_type_class_add_private(klass, sizeof(KonextdColorFramePrivate));
}
