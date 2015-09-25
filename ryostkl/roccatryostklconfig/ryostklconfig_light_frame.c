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

#include "ryostklconfig_light_frame.h"
#include "roccat_light_selector.h"
#include "ryostklconfig_pro_dimness_selector.h"
#include "roccat_multiwidget.h"
#include "i18n.h"

#define RYOSTKLCONFIG_LIGHT_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOSTKLCONFIG_LIGHT_FRAME_TYPE, RyostklconfigLightFrameClass))
#define IS_RYOSTKLCONFIG_LIGHT_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOSTKLCONFIG_LIGHT_FRAME_TYPE))
#define RYOSTKLCONFIG_LIGHT_FRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOSTKLCONFIG_LIGHT_FRAME_TYPE, RyostklconfigLightFramePrivate))

typedef struct _RyostklconfigLightFrameClass RyostklconfigLightFrameClass;
typedef struct _RyostklconfigLightFramePrivate RyostklconfigLightFramePrivate;

struct _RyostklconfigLightFrame {
	GtkFrame parent;
	RyostklconfigLightFramePrivate *priv;
};

struct _RyostklconfigLightFrameClass {
	GtkFrameClass parent_class;
};

struct _RyostklconfigLightFramePrivate {
	RoccatMultiwidget *multiwidget;
	RoccatLightSelector *light_selector;
	RyostklconfigProDimnessSelector *pro_dimness_selector;
	GtkLabel *nothing;
};

G_DEFINE_TYPE(RyostklconfigLightFrame, ryostklconfig_light_frame, GTK_TYPE_FRAME);

GtkWidget *ryostklconfig_light_frame_new(void) {
	return GTK_WIDGET(g_object_new(RYOSTKLCONFIG_LIGHT_FRAME_TYPE, NULL));
}

static void ryostklconfig_light_frame_init(RyostklconfigLightFrame *light_frame) {
	RyostklconfigLightFramePrivate *priv = RYOSTKLCONFIG_LIGHT_FRAME_GET_PRIVATE(light_frame);
	light_frame->priv = priv;
	guint rows;

	priv->multiwidget = ROCCAT_MULTIWIDGET(roccat_multiwidget_new());
	priv->light_selector = ROCCAT_LIGHT_SELECTOR(roccat_light_selector_new_with_range(RYOS_LIGHT_BRIGHTNESS_MIN, RYOS_LIGHT_BRIGHTNESS_MAX, 1));
	priv->nothing = GTK_LABEL(gtk_label_new(_("This model does not support light settings")));
	priv->pro_dimness_selector = RYOSTKLCONFIG_PRO_DIMNESS_SELECTOR(ryostklconfig_pro_dimness_selector_new());

	gtk_label_set_line_wrap(GTK_LABEL(priv->nothing), TRUE);

	roccat_multiwidget_add(priv->multiwidget, GTK_WIDGET(priv->light_selector));
	roccat_multiwidget_add(priv->multiwidget, GTK_WIDGET(priv->nothing));

	gtk_container_add(GTK_CONTAINER(light_frame), GTK_WIDGET(priv->multiwidget));

#if (GTK_CHECK_VERSION(2, 22, 0))
	gtk_table_get_size(GTK_TABLE(priv->light_selector), &rows, NULL);
#else
	g_object_get(G_OBJECT(priv->light_selector), "n-rows", &rows, NULL);
#endif

	gtk_table_attach(GTK_TABLE(priv->light_selector), gtk_label_new(_("Function")), 0, 1, rows, rows + 1, GTK_SHRINK, GTK_EXPAND, 0, 0);
	gtk_table_attach(GTK_TABLE(priv->light_selector), GTK_WIDGET(priv->pro_dimness_selector), 1, 2, rows, rows + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);

	gtk_frame_set_label(GTK_FRAME(light_frame), _("Light"));
}

static void ryostklconfig_light_frame_class_init(RyostklconfigLightFrameClass *klass) {
	g_type_class_add_private(klass, sizeof(RyostklconfigLightFramePrivate));
}

void ryostklconfig_light_frame_set_device_type(RyostklconfigLightFrame *light_frame, RoccatDevice const *device) {
	RyostklconfigLightFramePrivate *priv = light_frame->priv;
	switch (gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(device))) {
	case USB_DEVICE_ID_ROCCAT_RYOS_TKL:
		roccat_multiwidget_show(priv->multiwidget, GTK_WIDGET(priv->nothing));
		break;
	case USB_DEVICE_ID_ROCCAT_RYOS_TKL_PRO:
		roccat_multiwidget_show(priv->multiwidget, GTK_WIDGET(priv->light_selector));
		break;
	default:
		break;
	}
	ryostklconfig_pro_dimness_selector_set_device_type(priv->pro_dimness_selector, device);
}

void ryostklconfig_light_frame_set_brightness(RyostklconfigLightFrame *light_frame, guint new_brightness) {
	roccat_light_selector_set_brightness(light_frame->priv->light_selector, new_brightness);
}

void ryostklconfig_light_frame_set_from_profile_data(RyostklconfigLightFrame *light_frame, RyostklProfileData const *profile_data) {
	RyostklconfigLightFramePrivate *priv = light_frame->priv;
	roccat_light_selector_set_brightness(priv->light_selector, profile_data->hardware.light.brightness);
	roccat_light_selector_set_dimness(priv->light_selector, profile_data->hardware.light.dimness);
	roccat_light_selector_set_timeout(priv->light_selector, profile_data->hardware.light.timeout);
	ryostklconfig_pro_dimness_selector_set_from_profile_data(priv->pro_dimness_selector, profile_data);
}

void ryostklconfig_light_frame_update_profile_data(RyostklconfigLightFrame *light_frame, RyostklProfileData *profile_data) {
	RyostklconfigLightFramePrivate *priv = light_frame->priv;
	guint8 brightness = roccat_light_selector_get_brightness(priv->light_selector);
	guint8 dimness = roccat_light_selector_get_dimness(priv->light_selector);
	guint8 timeout = roccat_light_selector_get_timeout(priv->light_selector);

	if (profile_data->hardware.light.brightness != brightness) {
		profile_data->hardware.light.brightness = brightness;
		profile_data->hardware.modified_light = TRUE;
	}

	if (profile_data->hardware.light.dimness != dimness) {
		profile_data->hardware.light.dimness = dimness;
		profile_data->hardware.modified_light = TRUE;
	}

	if (profile_data->hardware.light.timeout != timeout) {
		profile_data->hardware.light.timeout = timeout;
		profile_data->hardware.modified_light = TRUE;
	}

	ryostklconfig_pro_dimness_selector_update_profile_data(priv->pro_dimness_selector, profile_data);
}
