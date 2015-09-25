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

#include "roccat_color_selection_button.h"
#include "i18n-lib.h"

#define ROCCAT_COLOR_SELECTION_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), ROCCAT_COLOR_SELECTION_BUTTON_TYPE, RoccatColorSelectionButtonPrivate))

struct _RoccatColorSelectionButtonPrivate {
	RoccatColorSelectionDialog *dialog;
	RoccatColorSelectionDialogType active_type;
	GdkColor custom_color;
	guint palette_index;
};

G_DEFINE_TYPE(RoccatColorSelectionButton, roccat_color_selection_button, ROCCAT_COLOR_BUTTON_TYPE);

void roccat_color_selection_button_set_palette_colors(RoccatColorSelectionButton *button, GdkColor const *colors, size_t length, guint width) {
	roccat_color_palette_selector_set_colors(roccat_color_selection_dialog_get_palette_selector(button->priv->dialog), colors, length, width);
}

static void update(RoccatColorSelectionButton *button) {
	RoccatColorSelectionButtonPrivate *priv = button->priv;
	GdkColor color;

	switch (priv->active_type) {
	case ROCCAT_COLOR_SELECTION_DIALOG_TYPE_PALETTE:
		roccat_color_palette_selector_get_color_for_index(roccat_color_selection_dialog_get_palette_selector(button->priv->dialog), priv->palette_index, &color);
		break;
	case ROCCAT_COLOR_SELECTION_DIALOG_TYPE_CUSTOM:
	case ROCCAT_COLOR_SELECTION_DIALOG_TYPE_BOTH:
		color = priv->custom_color;
		break;
	default:
		break;
	}

	roccat_color_button_set_color(ROCCAT_COLOR_BUTTON(button), &color);
}

void roccat_color_selection_button_set_view_type(RoccatColorSelectionButton *button, RoccatColorSelectionDialogType type) {
	RoccatColorSelectionButtonPrivate *priv = button->priv;
	priv->active_type = type;
	roccat_color_selection_dialog_set_view_type(priv->dialog, type);
	update(button);
}

void roccat_color_selection_button_set_active_type(RoccatColorSelectionButton *button, RoccatColorSelectionDialogType type) {
	button->priv->active_type = type;
	update(button);
}

RoccatColorSelectionDialogType roccat_color_selection_button_get_active_type(RoccatColorSelectionButton *button) {
	return button->priv->active_type;
}

guint roccat_color_selection_button_get_palette_index(RoccatColorSelectionButton *button) {
	return button->priv->palette_index;
}

void roccat_color_selection_button_set_palette_index(RoccatColorSelectionButton *button, guint index) {
	RoccatColorSelectionButtonPrivate *priv = button->priv;
	if (priv->active_type != ROCCAT_COLOR_SELECTION_DIALOG_TYPE_PALETTE || priv->palette_index != index) {
		priv->active_type = ROCCAT_COLOR_SELECTION_DIALOG_TYPE_PALETTE;
		priv->palette_index = index;
		update(button);
		g_signal_emit_by_name((gpointer)button, "value-changed");
	}
}

void roccat_color_selection_button_get_custom_color(RoccatColorSelectionButton *button, GdkColor *color) {
	*color = button->priv->custom_color;
}

void roccat_color_selection_button_set_custom_color(RoccatColorSelectionButton *button, GdkColor const *color) {
	RoccatColorSelectionButtonPrivate *priv = button->priv;
	if (priv->active_type != ROCCAT_COLOR_SELECTION_DIALOG_TYPE_CUSTOM || !gdk_color_equal(color, &priv->custom_color)) {
		priv->active_type = ROCCAT_COLOR_SELECTION_DIALOG_TYPE_CUSTOM;
		priv->custom_color = *color;
		update(button);
		g_signal_emit_by_name((gpointer)button, "value-changed");
	}
}

static void button_clicked_cb(RoccatColorSelectionButton *button, gpointer user_data) {
	RoccatColorSelectionButtonPrivate *priv = button->priv;
	GtkColorSelection *custom_selector;
	RoccatColorPaletteSelector *palette_selector;
	GdkColor color;

	custom_selector = GTK_COLOR_SELECTION(roccat_color_selection_dialog_get_custom_selector(priv->dialog));
	palette_selector = roccat_color_selection_dialog_get_palette_selector(priv->dialog);

	gtk_color_selection_set_current_color(custom_selector, &priv->custom_color);
	gtk_color_selection_set_previous_color(custom_selector, &priv->custom_color);

	if (gtk_dialog_run(GTK_DIALOG(priv->dialog)) == GTK_RESPONSE_OK) {
		switch (roccat_color_selection_dialog_get_active_type(priv->dialog)) {
		case ROCCAT_COLOR_SELECTION_DIALOG_TYPE_PALETTE:
			roccat_color_selection_button_set_palette_index(button, roccat_color_palette_selector_get_index(palette_selector));
			break;
		case ROCCAT_COLOR_SELECTION_DIALOG_TYPE_CUSTOM:
			gtk_color_selection_get_current_color(custom_selector, &color);
			roccat_color_selection_button_set_custom_color(button, &color);
			break;
		default:
			break;
		}
	}
	gtk_widget_hide(GTK_WIDGET(priv->dialog));
}

GtkWidget *roccat_color_selection_button_new(void) {
	return GTK_WIDGET(g_object_new(ROCCAT_COLOR_SELECTION_BUTTON_TYPE, NULL));
}

static void roccat_color_selection_button_init(RoccatColorSelectionButton *button) {
	RoccatColorSelectionButtonPrivate *priv = ROCCAT_COLOR_SELECTION_BUTTON_GET_PRIVATE(button);

	button->priv = priv;

	priv->dialog = ROCCAT_COLOR_SELECTION_DIALOG(roccat_color_selection_dialog_new(NULL));

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_clicked_cb), NULL);
}

static void roccat_color_selection_button_finalize(GObject *object) {
	RoccatColorSelectionButtonPrivate *priv = ROCCAT_COLOR_SELECTION_BUTTON(object)->priv;
	gtk_widget_destroy(GTK_WIDGET(priv->dialog));
	G_OBJECT_CLASS(roccat_color_selection_button_parent_class)->finalize(object);
}

static void roccat_color_selection_button_class_init(RoccatColorSelectionButtonClass *klass) {
	GObjectClass *gobject_class = (GObjectClass*)klass;

	gobject_class->finalize = roccat_color_selection_button_finalize;

	g_type_class_add_private(klass, sizeof(RoccatColorSelectionButtonPrivate));

	g_signal_new("value-changed",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}
