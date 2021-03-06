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

#include "iskuconfig_profile_page.h"
#include "isku_capslock_frame.h"
#include "isku_easyzone_frame.h"
#include "isku_function_frame.h"
#include "isku_macro_frame.h"
#include "isku_media_frame.h"
#include "isku_thumbster_frame.h"
#include "isku_key_mask_selector.h"
#include "roccat_light_selector.h"
#include "roccat_gamefile_selector.h"
#include "i18n.h"

#define ISKUCONFIG_PROFILE_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ISKUCONFIG_PROFILE_PAGE_TYPE, IskuconfigProfilePageClass))
#define IS_ISKUCONFIG_PROFILE_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ISKUCONFIG_PROFILE_PAGE_TYPE))
#define ISKUCONFIG_PROFILE_PAGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), ISKUCONFIG_PROFILE_PAGE_TYPE, IskuconfigProfilePagePrivate))

typedef struct _IskuconfigProfilePageClass IskuconfigProfilePageClass;
typedef struct _IskuconfigProfilePagePrivate IskuconfigProfilePagePrivate;

struct _IskuconfigProfilePage {
	RoccatProfilePage parent;
	IskuconfigProfilePagePrivate *priv;
};

struct _IskuconfigProfilePageClass {
	RoccatProfilePageClass parent_class;
};

struct _IskuconfigProfilePagePrivate {
	IskuRkp *rkp;
	IskuCapslockFrame *capslock;
	IskuEasyzoneFrame *easyzone;
	IskuFunctionFrame *function;
	IskuMacroFrame *macro;
	IskuMediaFrame *media;
	IskuThumbsterFrame *thumbster;
	RoccatGamefileSelector *gamefiles;
	IskuKeyMaskSelector *key_mask;
	RoccatLightSelector *light;
};

G_DEFINE_TYPE(IskuconfigProfilePage, iskuconfig_profile_page, ROCCAT_PROFILE_PAGE_TYPE);

void iskuconfig_profile_page_update_brightness(IskuconfigProfilePage *profile_page, guint new_brightness) {
	IskuconfigProfilePagePrivate *priv = profile_page->priv;
	roccat_light_selector_set_brightness(priv->light, new_brightness);
	isku_rkp_set_light_brightness_without_modified(priv->rkp, new_brightness);
}

void iskuconfig_profile_page_update_key(IskuconfigProfilePage *profile_page, guint key_index, IskuRkpMacroKeyInfo *key_info) {
	IskuconfigProfilePagePrivate *priv = profile_page->priv;

	isku_capslock_frame_set_key(priv->capslock, key_index, key_info);
	isku_easyzone_frame_set_key(priv->easyzone, key_index, key_info);
	isku_function_frame_set_key(priv->function, key_index, key_info);
	isku_macro_frame_set_key(priv->macro, key_index, key_info);
	isku_media_frame_set_key(priv->media, key_index, key_info);
	isku_thumbster_frame_set_key(priv->thumbster, key_index, key_info);

	isku_rkp_set_macro_key_info_without_modified(priv->rkp, key_index, key_info);
}

GtkWidget *iskuconfig_profile_page_new(void) {
	return GTK_WIDGET(g_object_new(ISKUCONFIG_PROFILE_PAGE_TYPE, NULL));
}

static void set_from_rkp(IskuconfigProfilePage *profile_page) {
	IskuconfigProfilePagePrivate *priv = profile_page->priv;
	guint i;

	roccat_profile_page_set_name(ROCCAT_PROFILE_PAGE(profile_page), isku_rkp_get_profile_name(priv->rkp));

	isku_capslock_frame_set_from_rkp(priv->capslock, priv->rkp);
	isku_easyzone_frame_set_from_rkp(priv->easyzone, priv->rkp);
	isku_function_frame_set_from_rkp(priv->function, priv->rkp);
	isku_macro_frame_set_from_rkp(priv->macro, priv->rkp);
	isku_media_frame_set_from_rkp(priv->media, priv->rkp);
	isku_thumbster_frame_set_from_rkp(priv->thumbster, priv->rkp);
	isku_key_mask_selector_set_from_rkp(priv->key_mask, priv->rkp);

	roccat_light_selector_set_brightness(priv->light, isku_rkp_get_light_brightness(priv->rkp));
	roccat_light_selector_set_dimness(priv->light, isku_rkp_get_light_dimness(priv->rkp));
	roccat_light_selector_set_timeout(priv->light, isku_rkp_get_light_timeout(priv->rkp));

	for (i = 0; i < ISKU_GAMEFILE_NUM; ++i)
		roccat_gamefile_selector_set_text(priv->gamefiles, i, isku_rkp_get_game_file_name(priv->rkp, i));
}

static void update_rkp(IskuconfigProfilePage *profile_page, IskuRkp *rkp) {
	IskuconfigProfilePagePrivate *priv = profile_page->priv;
	gchar *profile_name;
	guint i;

	profile_name = roccat_profile_page_get_name(ROCCAT_PROFILE_PAGE(profile_page));
	isku_rkp_set_profile_name(rkp, profile_name);
	g_free(profile_name);

	isku_capslock_frame_update_rkp(priv->capslock, rkp);
	isku_easyzone_frame_update_rkp(priv->easyzone, rkp);
	isku_function_frame_update_rkp(priv->function, rkp);
	isku_macro_frame_update_rkp(priv->macro, rkp);
	isku_media_frame_update_rkp(priv->media, rkp);
	isku_thumbster_frame_update_rkp(priv->thumbster, rkp);
	isku_key_mask_selector_update_rkp(priv->key_mask, rkp);

	isku_rkp_set_light_brightness(rkp, roccat_light_selector_get_brightness(priv->light));
	isku_rkp_set_light_dimness(rkp, roccat_light_selector_get_dimness(priv->light));
	isku_rkp_set_light_timeout(rkp, roccat_light_selector_get_timeout(priv->light));

	for (i = 0; i < ISKU_GAMEFILE_NUM; ++i)
		isku_rkp_set_game_file_name(rkp, i, roccat_gamefile_selector_get_text(priv->gamefiles, i));
}

static void reset_cb(RoccatProfilePage *profile_page, gpointer user_data) {
	set_from_rkp(ISKUCONFIG_PROFILE_PAGE(profile_page));
}

static void iskuconfig_profile_page_init(IskuconfigProfilePage *profile_page) {
	IskuconfigProfilePagePrivate *priv = ISKUCONFIG_PROFILE_PAGE_GET_PRIVATE(profile_page);
	GtkWidget *notebook;
	GtkWidget *page1;
	GtkWidget *page2;
	GtkWidget *page3;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *light_frame;

	profile_page->priv = priv;

	notebook = gtk_notebook_new();
	hbox = gtk_hbox_new(FALSE, 0);
	vbox = gtk_vbox_new(FALSE, 0);
	page1 = gtk_vbox_new(FALSE, 0);
	page2 = gtk_vbox_new(FALSE, 0);
	page3 = gtk_hbox_new(FALSE, 0);
	light_frame = gtk_frame_new(_("Light"));
	priv->easyzone = ISKU_EASYZONE_FRAME(isku_easyzone_frame_new());
	priv->function = ISKU_FUNCTION_FRAME(isku_function_frame_new());
	priv->media = ISKU_MEDIA_FRAME(isku_media_frame_new());
	priv->capslock = ISKU_CAPSLOCK_FRAME(isku_capslock_frame_new());
	priv->macro = ISKU_MACRO_FRAME(isku_macro_frame_new());
	priv->thumbster = ISKU_THUMBSTER_FRAME(isku_thumbster_frame_new());
	priv->gamefiles = ROCCAT_GAMEFILE_SELECTOR(roccat_gamefile_selector_new(ISKU_GAMEFILE_NUM));
	priv->key_mask = ISKU_KEY_MASK_SELECTOR(isku_key_mask_selector_new());
	priv->light = ROCCAT_LIGHT_SELECTOR(roccat_light_selector_new_with_range(ISKU_LIGHT_BRIGHTNESS_MIN, ISKU_LIGHT_BRIGHTNESS_MAX, 1));

	gtk_container_add(GTK_CONTAINER(light_frame), GTK_WIDGET(priv->light));

	gtk_box_pack_start(GTK_BOX(vbox), light_frame, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->capslock), TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(priv->key_mask), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(page1), hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page1), GTK_WIDGET(priv->macro), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page1), GTK_WIDGET(priv->thumbster), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page1), GTK_WIDGET(priv->gamefiles), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page2), GTK_WIDGET(priv->easyzone), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page3), GTK_WIDGET(priv->function), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page3), GTK_WIDGET(priv->media), TRUE, TRUE, 0);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page1, gtk_label_new(_("Main")));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page2, gtk_label_new(_("Easyzone")));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page3, gtk_label_new(_("Media/F keys")));

	gtk_box_pack_start(GTK_BOX(profile_page), notebook, TRUE, TRUE, 0);

	gtk_widget_show_all(notebook);
	g_signal_connect(G_OBJECT(profile_page), "reset", G_CALLBACK(reset_cb), NULL);
}

static void iskuconfig_profile_page_class_init(IskuconfigProfilePageClass *klass) {
	g_type_class_add_private(klass, sizeof(IskuconfigProfilePagePrivate));
}

void iskuconfig_profile_page_set_rkp(IskuconfigProfilePage *profile_page, IskuRkp *rkp) {
	IskuconfigProfilePagePrivate *priv = profile_page->priv;
	if (priv->rkp)
		isku_rkp_free(priv->rkp);
	priv->rkp = isku_rkp_dup(rkp);
	set_from_rkp(profile_page);
}

IskuRkp *iskuconfig_profile_page_get_rkp(IskuconfigProfilePage *profile_page) {
	IskuconfigProfilePagePrivate *priv = profile_page->priv;
	IskuRkp *rkp;
	rkp = isku_rkp_dup(priv->rkp);
	update_rkp(profile_page, rkp);
	return rkp;
}
