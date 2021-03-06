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

#include "koneplusconfig_profile_page.h"
#include "koneplus_buttons_frame.h"
#include "koneplus_light_effects_frame.h"
#include "koneplus_color_frame.h"
#include "koneplus_cpi_selector.h"
#include "roccat_sensitivity_frame.h"
#include "roccat_polling_rate_frame.h"
#include "roccat_gamefile_selector.h"
#include "i18n.h"

#define KONEPLUSCONFIG_PROFILE_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KONEPLUSCONFIG_PROFILE_PAGE_TYPE, KoneplusconfigProfilePageClass))
#define IS_KONEPLUSCONFIG_PROFILE_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KONEPLUSCONFIG_PROFILE_PAGE_TYPE))
#define KONEPLUSCONFIG_PROFILE_PAGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KONEPLUSCONFIG_PROFILE_PAGE_TYPE, KoneplusconfigProfilePagePrivate))

typedef struct _KoneplusconfigProfilePageClass KoneplusconfigProfilePageClass;
typedef struct _KoneplusconfigProfilePagePrivate KoneplusconfigProfilePagePrivate;

struct _KoneplusconfigProfilePage {
	RoccatProfilePage parent;
	KoneplusconfigProfilePagePrivate *priv;
};

struct _KoneplusconfigProfilePageClass {
	RoccatProfilePageClass parent_class;
};

struct _KoneplusconfigProfilePagePrivate {
	KoneplusRmp *rmp;
	KoneplusButtonsFrame *buttons;
	RoccatGamefileSelector *gamefiles;
	RoccatPollingRateFrame *polling_rate;
	KoneplusCpiSelector *cpi;
	RoccatSensitivityFrame *sensitivity;
	KoneplusLightEffectsFrame *light_effects;
	KoneplusColorFrame *colors;
};

G_DEFINE_TYPE(KoneplusconfigProfilePage, koneplusconfig_profile_page, ROCCAT_PROFILE_PAGE_TYPE);

GtkWidget *koneplusconfig_profile_page_new(void) {
	return GTK_WIDGET(g_object_new(KONEPLUSCONFIG_PROFILE_PAGE_TYPE, NULL));
}

static void set_from_rmp(KoneplusconfigProfilePage *profile_page) {
	KoneplusconfigProfilePagePrivate *priv = profile_page->priv;
	guint i;

	roccat_profile_page_set_name(ROCCAT_PROFILE_PAGE(profile_page), koneplus_rmp_get_profile_name(priv->rmp));

	koneplus_buttons_frame_set_from_rmp(priv->buttons, priv->rmp);
	koneplus_light_effects_frame_set_from_rmp(priv->light_effects, priv->rmp);
	koneplus_color_frame_set_from_rmp(priv->colors, priv->rmp);
	koneplus_cpi_selector_set_from_rmp(priv->cpi, priv->rmp);
	roccat_polling_rate_frame_set_value(priv->polling_rate, koneplus_rmp_get_polling_rate(priv->rmp));

	roccat_sensitivity_frame_set_advanced(priv->sensitivity, koneplus_rmp_get_xy_synchronous(priv->rmp));
	if (koneplus_rmp_get_xy_synchronous(priv->rmp) == ROCCAT_SENSITIVITY_ADVANCED_ON) {
		roccat_sensitivity_frame_set_x_value(priv->sensitivity, koneplus_rmp_get_sensitivity_x(priv->rmp));
		roccat_sensitivity_frame_set_y_value(priv->sensitivity, koneplus_rmp_get_sensitivity_y(priv->rmp));
	} else
		roccat_sensitivity_frame_set_x_value(priv->sensitivity, koneplus_rmp_get_sensitivity(priv->rmp));

	for (i = 0; i < KONEPLUS_GAMEFILE_NUM; ++i)
		roccat_gamefile_selector_set_text(priv->gamefiles, i, koneplus_rmp_get_game_file_name(priv->rmp, i));
}

static void update_rmp(KoneplusconfigProfilePage *profile_page, KoneplusRmp *rmp) {
	KoneplusconfigProfilePagePrivate *priv = profile_page->priv;
	gchar *profile_name;
	guint i;

	profile_name = roccat_profile_page_get_name(ROCCAT_PROFILE_PAGE(profile_page));
	koneplus_rmp_set_profile_name(rmp, profile_name);
	g_free(profile_name);

	koneplus_buttons_frame_update_rmp(priv->buttons, rmp);
	koneplus_light_effects_frame_update_rmp(priv->light_effects, rmp);
	koneplus_color_frame_update_rmp(priv->colors, rmp);
	koneplus_cpi_selector_update_rmp(priv->cpi, rmp);
	koneplus_rmp_set_polling_rate(rmp, roccat_polling_rate_frame_get_value(priv->polling_rate));

	koneplus_rmp_set_xy_synchronous(rmp, roccat_sensitivity_frame_get_advanced(priv->sensitivity));
	if (roccat_sensitivity_frame_get_advanced(priv->sensitivity) == ROCCAT_SENSITIVITY_ADVANCED_ON) {
		koneplus_rmp_set_sensitivity_x(rmp, roccat_sensitivity_frame_get_x_value(priv->sensitivity));
		koneplus_rmp_set_sensitivity_y(rmp, roccat_sensitivity_frame_get_y_value(priv->sensitivity));
	} else
		koneplus_rmp_set_sensitivity(rmp, roccat_sensitivity_frame_get_x_value(priv->sensitivity));

	for (i = 0; i < KONEPLUS_GAMEFILE_NUM; ++i)
		koneplus_rmp_set_game_file_name(rmp, i, roccat_gamefile_selector_get_text(priv->gamefiles, i));
}

static void reset_cb(RoccatProfilePage *profile_page, gpointer user_data) {
	set_from_rmp(KONEPLUSCONFIG_PROFILE_PAGE(profile_page));
}

static void koneplusconfig_profile_page_init(KoneplusconfigProfilePage *profile_page) {
	KoneplusconfigProfilePagePrivate *priv = KONEPLUSCONFIG_PROFILE_PAGE_GET_PRIVATE(profile_page);
	GtkWidget *content;
	GtkWidget *left;
	GtkWidget *light;
	GtkWidget *right;

	profile_page->priv = priv;

	content = gtk_hbox_new(FALSE, 0);
	left = gtk_vbox_new(FALSE, 0);
	light = gtk_hbox_new(FALSE, 0);
	right = gtk_vbox_new(FALSE, 0);
	priv->buttons = KONEPLUS_BUTTONS_FRAME(koneplus_buttons_frame_new());
	priv->gamefiles = ROCCAT_GAMEFILE_SELECTOR(roccat_gamefile_selector_new(KONEPLUS_GAMEFILE_NUM));
	priv->polling_rate = ROCCAT_POLLING_RATE_FRAME(roccat_polling_rate_frame_new());
	priv->cpi = KONEPLUS_CPI_SELECTOR(koneplus_cpi_selector_new());
	priv->sensitivity = ROCCAT_SENSITIVITY_FRAME(roccat_sensitivity_frame_new());
	priv->light_effects = KONEPLUS_LIGHT_EFFECTS_FRAME(koneplus_light_effects_frame_new());
	priv->colors = KONEPLUS_COLOR_FRAME(koneplus_color_frame_new());

	gtk_box_pack_start(GTK_BOX(light), GTK_WIDGET(priv->colors), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(light), GTK_WIDGET(priv->light_effects), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(right), GTK_WIDGET(priv->buttons), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(right), GTK_WIDGET(priv->sensitivity), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(right), GTK_WIDGET(priv->polling_rate), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(right), GTK_WIDGET(priv->gamefiles), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(left), light, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(left), GTK_WIDGET(priv->cpi), TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(content), left);
	gtk_container_add(GTK_CONTAINER(content), right);

	gtk_box_pack_start(GTK_BOX(profile_page), content, TRUE, TRUE, 0);

	gtk_widget_show_all(content);
	g_signal_connect(G_OBJECT(profile_page), "reset", G_CALLBACK(reset_cb), NULL);
}

static void koneplusconfig_profile_page_class_init(KoneplusconfigProfilePageClass *klass) {
	g_type_class_add_private(klass, sizeof(KoneplusconfigProfilePagePrivate));
}

void koneplusconfig_profile_page_set_rmp(KoneplusconfigProfilePage *profile_page, KoneplusRmp *rmp) {
	KoneplusconfigProfilePagePrivate *priv = profile_page->priv;
	if (priv->rmp)
		koneplus_rmp_free(priv->rmp);
	priv->rmp = koneplus_rmp_dup(rmp);
	set_from_rmp(profile_page);
}

KoneplusRmp *koneplusconfig_profile_page_get_rmp(KoneplusconfigProfilePage *profile_page) {
	KoneplusconfigProfilePagePrivate *priv = profile_page->priv;
	KoneplusRmp *rmp;
	rmp = koneplus_rmp_dup(priv->rmp);
	update_rmp(profile_page, rmp);
	return rmp;
}
