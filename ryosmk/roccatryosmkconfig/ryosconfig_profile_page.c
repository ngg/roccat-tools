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

#include "ryosconfig_profile_page.h"
#include "roccat_gamefile_selector.h"
#include "ryosconfig_light_frame.h"
#include "ryosconfig_keyboard_selector.h"
#include "ryosconfig_led_feedback_frame.h"
#include "ryos_key_mask_selector.h"
#include "g_cclosure_roccat_marshaller.h"
#include "ryos_rkp_accessors.h"
#include "i18n.h"

#define RYOSCONFIG_PROFILE_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOSCONFIG_PROFILE_PAGE_TYPE, RyosconfigProfilePageClass))
#define IS_RYOSCONFIG_PROFILE_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOSCONFIG_PROFILE_PAGE_TYPE))
#define RYOSCONFIG_PROFILE_PAGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOSCONFIG_PROFILE_PAGE_TYPE, RyosconfigProfilePagePrivate))

typedef struct _RyosconfigProfilePageClass RyosconfigProfilePageClass;
typedef struct _RyosconfigProfilePagePrivate RyosconfigProfilePagePrivate;

struct _RyosconfigProfilePage {
	RoccatProfilePage parent;
	RyosconfigProfilePagePrivate *priv;
};

struct _RyosconfigProfilePageClass {
	RoccatProfilePageClass parent_class;
};

struct _RyosconfigProfilePagePrivate {
	RyosRkp *rkp;
	GtkBox *misc_box;
	RyosconfigLightFrame *light_frame;
	RyosconfigKeyboardSelector *keyboard_selector;
	RoccatGamefileSelector *gamefiles;
	RyosconfigLedFeedbackFrame *feedback_frame;
	RyosKeyMaskSelector *key_mask_selector;
};

G_DEFINE_TYPE(RyosconfigProfilePage, ryosconfig_profile_page, ROCCAT_PROFILE_PAGE_TYPE);

enum {
	LAYER_EDITOR,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

void ryosconfig_profile_page_set_device(RyosconfigProfilePage *profile_page, RoccatDevice const *device) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;
	ryosconfig_keyboard_selector_set_device_type(priv->keyboard_selector, device);
	ryosconfig_light_frame_set_device_type(priv->light_frame, device);
	ryosconfig_led_feedback_frame_set_device_type(priv->feedback_frame, device);
}

void ryosconfig_profile_page_update_brightness(RyosconfigProfilePage *profile_page, guint new_brightness) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;
	ryos_rkp_set_light_brightness_without_modified(priv->rkp, new_brightness);
	ryosconfig_light_frame_set_brightness(priv->light_frame, new_brightness);
}

void ryosconfig_profile_page_update_key(RyosconfigProfilePage *profile_page, guint macro_index, RyosMacro *macro) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;

	ryos_rkp_set_macro_without_modified(priv->rkp, macro_index, macro);
	ryos_rkp_set_key_to_macro_without_modified(priv->rkp, macro_index);

	ryosconfig_keyboard_selector_set_macro(priv->keyboard_selector, macro_index, macro);
}

GtkWidget *ryosconfig_profile_page_new(void) {
	return GTK_WIDGET(g_object_new(RYOSCONFIG_PROFILE_PAGE_TYPE, NULL));
}

static void set_from_rkp(RyosconfigProfilePage *profile_page) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;
	guint i;

	roccat_profile_page_set_name(ROCCAT_PROFILE_PAGE(profile_page), ryos_rkp_get_profile_name(priv->rkp));

	ryosconfig_light_frame_set_from_rkp(priv->light_frame, priv->rkp);
	ryosconfig_led_feedback_frame_set_from_rkp(priv->feedback_frame, priv->rkp);
	ryosconfig_keyboard_selector_set_from_rkp(priv->keyboard_selector, priv->rkp);
	ryos_key_mask_selector_set_from_rkp(priv->key_mask_selector, priv->rkp);

	for (i = 0; i < RYOS_RKP_PROFILE_GAMEFILE_NUM; ++i)
		roccat_gamefile_selector_set_text(priv->gamefiles, i, ryos_rkp_get_gamefile_name(priv->rkp, i));
}

static void update_rkp(RyosconfigProfilePage *profile_page, RyosRkp *rkp) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;
	gchar *profile_name;
	guint i;

	profile_name = roccat_profile_page_get_name(ROCCAT_PROFILE_PAGE(profile_page));
	ryos_rkp_set_profile_name(rkp, profile_name);
	g_free(profile_name);

	ryosconfig_light_frame_update_rkp(priv->light_frame, rkp);
	ryosconfig_led_feedback_frame_update_rkp(priv->feedback_frame, rkp);
	ryosconfig_keyboard_selector_update_rkp(priv->keyboard_selector, rkp);
	ryos_key_mask_selector_update_rkp(priv->key_mask_selector, rkp);

	for (i = 0; i < RYOS_RKP_PROFILE_GAMEFILE_NUM; ++i)
		ryos_rkp_set_gamefile_name(rkp, i, roccat_gamefile_selector_get_text(priv->gamefiles, i));
}

static void reset_cb(RoccatProfilePage *profile_page, gpointer user_data) {
	set_from_rkp(RYOSCONFIG_PROFILE_PAGE(profile_page));
}

static gboolean layer_editor_cb(RyosconfigKeyboardSelector *keyboard_selector, gpointer light_layer, gpointer user_data) {
	RyosconfigProfilePage *profile_page = RYOSCONFIG_PROFILE_PAGE(user_data);
	gboolean result;
	g_signal_emit((gpointer)profile_page, signals[LAYER_EDITOR], 0, light_layer, &result);
	return result;
}

static void append_keys_page(RyosconfigProfilePage *profile_page, GtkNotebook *notebook) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;
	GtkWidget *vbox;

	vbox = gtk_vbox_new(FALSE, 0);
	priv->keyboard_selector = RYOSCONFIG_KEYBOARD_SELECTOR(ryosconfig_keyboard_selector_new());

	g_signal_connect(G_OBJECT(priv->keyboard_selector), "layer-editor", G_CALLBACK(layer_editor_cb), profile_page);

	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->keyboard_selector), TRUE, TRUE, 0);
	gtk_notebook_append_page(notebook, vbox, gtk_label_new(_("Keys")));
}

static void append_misc_page(RyosconfigProfilePage *profile_page, GtkNotebook *notebook) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;
	GtkWidget *hbox;
	GtkWidget *vbox;

	hbox = gtk_hbox_new(FALSE, 0);
	vbox = gtk_vbox_new(FALSE, 0);
	priv->misc_box = GTK_BOX(gtk_vbox_new(FALSE, 0));
	priv->light_frame = RYOSCONFIG_LIGHT_FRAME(ryosconfig_light_frame_new());
	priv->feedback_frame = RYOSCONFIG_LED_FEEDBACK_FRAME(ryosconfig_led_feedback_frame_new());
	priv->gamefiles = ROCCAT_GAMEFILE_SELECTOR(roccat_gamefile_selector_new(RYOS_RKP_PROFILE_GAMEFILE_NUM));
	priv->key_mask_selector = RYOS_KEY_MASK_SELECTOR(ryos_key_mask_selector_new());

	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->light_frame), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->feedback_frame), TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(priv->key_mask_selector), TRUE, TRUE, 0);

	gtk_box_pack_start(priv->misc_box, hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(priv->misc_box, GTK_WIDGET(priv->gamefiles), TRUE, TRUE, 0);

	gtk_notebook_append_page(notebook, GTK_WIDGET(priv->misc_box), gtk_label_new(_("Misc")));
}

static void ryosconfig_profile_page_init(RyosconfigProfilePage *profile_page) {
	RyosconfigProfilePagePrivate *priv = RYOSCONFIG_PROFILE_PAGE_GET_PRIVATE(profile_page);
	GtkWidget *notebook;

	profile_page->priv = priv;

	priv->rkp = NULL;

	notebook = gtk_notebook_new();

	append_keys_page(profile_page, GTK_NOTEBOOK(notebook));
	append_misc_page(profile_page, GTK_NOTEBOOK(notebook));

	gtk_box_pack_start(GTK_BOX(profile_page), notebook, TRUE, TRUE, 0);

	gtk_widget_show_all(notebook);
	g_signal_connect(G_OBJECT(profile_page), "reset", G_CALLBACK(reset_cb), NULL);
}

static void ryosconfig_profile_page_class_init(RyosconfigProfilePageClass *klass) {
	g_type_class_add_private(klass, sizeof(RyosconfigProfilePagePrivate));

	signals[LAYER_EDITOR] = g_signal_new("layer-editor",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_roccat_marshal_BOOLEAN__POINTER, G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);
}

void ryosconfig_profile_page_set_keyboard_layout(RyosconfigProfilePage *profile_page, gchar const *layout) {
	ryosconfig_keyboard_selector_set_layout(profile_page->priv->keyboard_selector, layout);
}

void ryosconfig_profile_page_set_rkp(RyosconfigProfilePage *profile_page, RyosRkp const *rkp) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;
	if (priv->rkp)
		ryos_rkp_free(priv->rkp);
	priv->rkp = ryos_rkp_dup(rkp);
	set_from_rkp(profile_page);
}

RyosRkp *ryosconfig_profile_page_get_rkp(RyosconfigProfilePage *profile_page) {
	RyosconfigProfilePagePrivate *priv = profile_page->priv;
	RyosRkp *rkp;
	rkp = ryos_rkp_dup(priv->rkp);
	update_rkp(profile_page, rkp);
	return rkp;
}
