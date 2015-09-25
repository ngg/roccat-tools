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

#include "koneplus_color_frame.h"
#include "roccat_color_selection_button.h"
#include "i18n-lib.h"

#define KONEPLUS_COLOR_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KONEPLUS_COLOR_FRAME_TYPE, KoneplusColorFrameClass))
#define IS_KONEPLUS_COLOR_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KONEPLUS_COLOR_FRAME_TYPE))
#define KONEPLUS_COLOR_FRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KONEPLUS_COLOR_FRAME_TYPE, KoneplusColorFramePrivate))

typedef struct _KoneplusColorFrameClass KoneplusColorFrameClass;
typedef struct _KoneplusColorFramePrivate KoneplusColorFramePrivate;

struct _KoneplusColorFrame {
	GtkFrame parent;
	KoneplusColorFramePrivate *priv;
};

struct _KoneplusColorFrameClass {
	GtkFrameClass parent_class;
};

struct _KoneplusColorFramePrivate {
	RoccatColorSelectionButton *colors[KONEPLUS_PROFILE_SETTING_LIGHTS_NUM];
	GtkToggleButton *states[KONEPLUS_PROFILE_SETTING_LIGHTS_NUM];
};

G_DEFINE_TYPE(KoneplusColorFrame, koneplus_color_frame, GTK_TYPE_FRAME);

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

void koneplus_color_frame_set_from_rmp(KoneplusColorFrame *frame, KoneplusRmp *rmp) {
	KoneplusColorFramePrivate *priv = frame->priv;
	KoneplusRmpLightInfo *light_info;
	guint i;

	for (i = 0; i < KONEPLUS_PROFILE_SETTING_LIGHTS_NUM; ++i) {
		light_info = koneplus_rmp_get_rmp_light_info(rmp, i);
		roccat_color_selection_button_set_palette_index(priv->colors[i], light_info->index);
		gtk_toggle_button_set_active(priv->states[i], (light_info->state == KONEPLUS_RMP_LIGHT_INFO_STATE_ON));
		g_free(light_info);
	}
}

void koneplus_color_frame_update_rmp(KoneplusColorFrame *frame, KoneplusRmp *rmp) {
	KoneplusColorFramePrivate *priv = frame->priv;
	KoneplusRmpLightInfo light_info;
	KoneplusRmpLightInfo const *standard_light_info;
	guint color_index, state;
	guint i;

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
	}
}

GtkWidget *koneplus_color_frame_new(void) {
	KoneplusColorFrame *frame;

	frame = KONEPLUS_COLOR_FRAME(g_object_new(KONEPLUS_COLOR_FRAME_TYPE,
			NULL));

	return GTK_WIDGET(frame);
}

static void koneplus_color_frame_init(KoneplusColorFrame *frame) {
	KoneplusColorFramePrivate *priv = KONEPLUS_COLOR_FRAME_GET_PRIVATE(frame);
	GtkWidget *hbox;
	GtkWidget *table;
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

	table = gtk_table_new(2, 2, TRUE);
	gtk_container_add(GTK_CONTAINER(frame), table);

	for (i = 0; i < KONEPLUS_PROFILE_SETTING_LIGHTS_NUM; ++i) {
		hbox = gtk_hbox_new(FALSE, 0);
		priv->colors[i] = ROCCAT_COLOR_SELECTION_BUTTON(roccat_color_selection_button_new());
		roccat_color_selection_button_set_view_type(priv->colors[i], ROCCAT_COLOR_SELECTION_DIALOG_TYPE_PALETTE);
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

	gtk_frame_set_label(GTK_FRAME(frame), _("Colors"));
}

static void koneplus_color_frame_class_init(KoneplusColorFrameClass *klass) {
	g_type_class_add_private(klass, sizeof(KoneplusColorFramePrivate));
}
