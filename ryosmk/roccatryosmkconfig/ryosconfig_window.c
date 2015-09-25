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

#include "ryos_rkp.h"
#include "ryos_dbus_services.h"
#include "ryos_reset.h"
#include "ryos_profile.h"
#include "ryos_info.h"
#include "ryos_rkp_accessors.h"
#include "ryosconfig_window.h"
#include "ryos_configuration_dialog.h"
#include "ryosconfig_profile_page.h"
#include "ryosconfig_layer_illumination_dialog.h"
#include "ryos_config.h"
#include "ryos_device.h"
#include "roccat_helper.h"
#include "roccat_warning_dialog.h"
#include "roccat_save_dialog.h"
#include "roccat_continue_dialog.h"
#include "roccat_info_dialog.h"
#include "roccat_update_assistant.h"
#include "roccat_firmware.h"
#include "g_roccat_helper.h"
#include "g_dbus_roccat_helper.h"
#include "roccat.h"
#include "i18n.h"

#define RYOSCONFIG_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOSCONFIG_WINDOW_TYPE, RyosconfigWindowClass))
#define IS_RYOSCONFIG_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOSCONFIG_WINDOW_TYPE))
#define RYOSCONFIG_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOSCONFIG_WINDOW_TYPE, RyosconfigWindowPrivate))

typedef struct _RyosconfigWindowClass RyosconfigWindowClass;
typedef struct _RyosconfigWindowPrivate RyosconfigWindowPrivate;

struct _RyosconfigWindow {
	RoccatConfigWindow parent;
	RyosconfigWindowPrivate *priv;
};

struct _RyosconfigWindowClass {
	RoccatConfigWindowClass parent_class;
};

struct _RyosconfigWindowPrivate {
	RoccatKeyFile *config;
	DBusGProxy *dbus_proxy;
};

G_DEFINE_TYPE(RyosconfigWindow, ryosconfig_window, ROCCAT_CONFIG_WINDOW_TYPE);

static gboolean change_profile_handler(gpointer user_data) {
	RoccatConfigWindow *roccat_window = ROCCAT_CONFIG_WINDOW(user_data);
	guint profile_index = roccat_config_window_get_active_page(roccat_window);
	GError *local_error = NULL;

	if (!roccat_config_window_warn_if_no_device(roccat_window))
		return FALSE;

	ryos_profile_write(roccat_config_window_get_device(roccat_window), profile_index, &local_error);
	if (!roccat_handle_error_dialog(GTK_WINDOW(roccat_window), _("Could not activate profile"), &local_error))
		return FALSE;
	ryos_dbus_emit_profile_changed_outside(RYOSCONFIG_WINDOW(roccat_window)->priv->dbus_proxy, profile_index);

	return FALSE;
}

static void window_active_page_changed_cb(RoccatConfigWindow *roccat_window, gpointer user_data) {
	g_idle_add(change_profile_handler, roccat_window);
}

static void actual_profile_changed_from_device_cb(DBusGProxy *proxy, guchar profile_index, gpointer user_data) {
	roccat_config_window_set_active_page(ROCCAT_CONFIG_WINDOW(user_data), profile_index);
}

static void brightness_changed_from_device_cb(DBusGProxy *proxy, guchar profile_index, guchar brightness_index, gpointer user_data) {
	RoccatConfigWindow *roccat_window = ROCCAT_CONFIG_WINDOW(user_data);
	ryosconfig_profile_page_update_brightness(RYOSCONFIG_PROFILE_PAGE(roccat_config_window_get_page(roccat_window, profile_index)), brightness_index);
}

static void macro_changed_from_device_cb(DBusGProxy *proxy, guchar profile_index, guchar button_index, gchar const *data, gpointer user_data) {
	RoccatConfigWindow *roccat_window = ROCCAT_CONFIG_WINDOW(user_data);
	RyosMacro *macro;
	gsize length;

	macro = (RyosMacro *)g_base64_decode(data, &length);
	if (length == sizeof(RyosMacro))
		ryosconfig_profile_page_update_key(RYOSCONFIG_PROFILE_PAGE(roccat_config_window_get_page(roccat_window, profile_index)), button_index, macro);
	g_free(macro);
}

static void device_add_cb(RoccatConfigWindow *roccat_window, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(roccat_window);
	RyosconfigWindowPrivate *priv = window->priv;
	GError *local_error = NULL;
	guint i;
	RyosRkp *rkp;
	RoccatDevice *device;
	RyosconfigProfilePage *profile_page;
	guint actual_profile_index;

	device = roccat_config_window_get_device(roccat_window);

	for (i = 0; i < RYOS_PROFILE_NUM; ++i) {
		profile_page = RYOSCONFIG_PROFILE_PAGE(roccat_config_window_get_page(roccat_window, i));
		ryosconfig_profile_page_set_device(profile_page, device);

		rkp = ryosconfig_profile_page_get_rkp(profile_page);
		ryos_rkp_update_from_device(device, rkp, i, &local_error);

		if (!roccat_handle_error_dialog(GTK_WINDOW(window), _("Could not read hardware profile data"), &local_error)) {
			ryos_rkp_free(rkp);
			break;
		}

		ryosconfig_profile_page_set_rkp(profile_page, rkp);

		ryos_rkp_free(rkp);
	}

	actual_profile_index = ryos_profile_read(device, &local_error);
	if (!roccat_handle_error_dialog(GTK_WINDOW(window), _("Could not read actual profile"), &local_error))
		return;
	roccat_config_window_set_active_page(roccat_window, actual_profile_index);

	dbus_g_proxy_connect_signal(priv->dbus_proxy, "ProfileChanged", G_CALLBACK(actual_profile_changed_from_device_cb), window, NULL);
	dbus_g_proxy_connect_signal(priv->dbus_proxy, "BrightnessChanged", G_CALLBACK(brightness_changed_from_device_cb), window, NULL);
	dbus_g_proxy_connect_signal(priv->dbus_proxy, "MacroChanged", G_CALLBACK(macro_changed_from_device_cb), window, NULL);
}

static void device_remove_cb(RoccatConfigWindow *roccat_window, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(roccat_window);
	RyosconfigWindowPrivate *priv = window->priv;

	dbus_g_proxy_disconnect_signal(priv->dbus_proxy, "ProfileChanged", G_CALLBACK(actual_profile_changed_from_device_cb), window);
	dbus_g_proxy_disconnect_signal(priv->dbus_proxy, "BrightnessChanged", G_CALLBACK(brightness_changed_from_device_cb), window);
	dbus_g_proxy_disconnect_signal(priv->dbus_proxy, "MacroChanged", G_CALLBACK(macro_changed_from_device_cb), window);
}

static RyosRkp *get_rkp(RyosconfigWindow *window, RyosconfigProfilePage *profile_page) {
	RyosRkp *rkp;
	gboolean moved;

	rkp = ryosconfig_profile_page_get_rkp(profile_page);
	moved = roccat_config_window_get_page_moved(ROCCAT_CONFIG_WINDOW(window), ROCCAT_PROFILE_PAGE(profile_page));

	if (moved)
		ryos_rkp_set_modified(rkp);

	return rkp;
}

static void set_rkp(RyosconfigWindow *window, guint profile_index, RyosRkp *rkp) {
	RyosconfigProfilePage *profile_page = RYOSCONFIG_PROFILE_PAGE(roccat_config_window_get_page(ROCCAT_CONFIG_WINDOW(window), profile_index));
	guint profile_number = profile_index + 1;
	ryosconfig_profile_page_set_rkp(profile_page, rkp);
	roccat_config_window_set_page_unmoved(ROCCAT_CONFIG_WINDOW(window), ROCCAT_PROFILE_PAGE(profile_page));
	ryos_dbus_emit_profile_data_changed_outside(window->priv->dbus_proxy, profile_number);
}

static gboolean save_single(RyosconfigWindow *window, RyosRkp *rkp, guint profile_index, GError **error) {
	RoccatDevice *device = roccat_config_window_get_device(ROCCAT_CONFIG_WINDOW(window));

	if (device)
		ryos_rkp_save(device, rkp, profile_index, error);
	else
		ryos_rkp_save_actual(rkp, profile_index, error);

	if (*error)
		return FALSE;
	else {
		set_rkp(window, profile_index, rkp);
		return TRUE;
	}
}

static gboolean save_all(RyosconfigWindow *window, gboolean ask) {
	RoccatConfigWindow *roccat_window = ROCCAT_CONFIG_WINDOW(window);
	RyosRkp *rkps[RYOS_PROFILE_NUM];
	RyosconfigProfilePage *profile_page;
	GError *local_error = NULL;
	gboolean modified;
	gboolean retval;
	guint i;

	modified = FALSE;
	for (i = 0; i < RYOS_PROFILE_NUM; ++i) {
		profile_page = RYOSCONFIG_PROFILE_PAGE(roccat_config_window_get_page(roccat_window, i));
		rkps[i] = get_rkp(window, profile_page);
		if (ryos_rkp_get_modified(rkps[i]))
			modified = TRUE;
	}
	if (!modified) {
		retval = TRUE;
		goto exit;
	}

	if (ask) {
		switch (roccat_save_unsaved_dialog(GTK_WINDOW(window), TRUE)) {
		case GTK_RESPONSE_CANCEL:
			/* not saving by user request */
			retval = FALSE;
			goto exit;
			break;
		case GTK_RESPONSE_ACCEPT:
			break;
		case GTK_RESPONSE_REJECT:
			/* discarding by user request */
			retval = TRUE;
			goto exit;
			break;
		default:
			break;
		}
	}

	roccat_config_window_warn_if_no_device(roccat_window);

	retval = TRUE;
	for (i = 0; i < RYOS_PROFILE_NUM; ++i) {
		if (!save_single(window, rkps[i], i, &local_error)) {
			retval = FALSE;
			break;
		}
	}
	roccat_handle_error_dialog(GTK_WINDOW(window), _("Could not save unsaved data"), &local_error);

exit:
	for (i = 0; i < RYOS_PROFILE_NUM; ++i)
		ryos_rkp_free(rkps[i]);

	return retval;
}

static gboolean delete_event_cb(GtkWidget *window, GdkEvent *event, gpointer user_data) {
	return !save_all(RYOSCONFIG_WINDOW(window), TRUE);
}

static void add_file_filter_to_file_chooser(GtkFileChooser *chooser) {
	GtkFileFilter *filter;
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Ryos MK Windows profiles"));
	gtk_file_filter_add_pattern(filter, "*." ROCCAT_KEYBOARD_PROFILE_EXTENSION);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
}

static gchar *ryos_create_filename_proposition(RyosRkp const *rkp, guint profile_index) {
	gchar *profile_name;
	gchar *filename;

	profile_name = ryos_rkp_get_profile_name(rkp);
	filename = roccat_profile_filename_proposition(profile_name, profile_index, ROCCAT_KEYBOARD_PROFILE_EXTENSION);
	g_free(profile_name);

	return filename;
}

static void save_profile_to_file_cb(RoccatProfilePage *profile_page, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(user_data);
	RyosconfigWindowPrivate *priv = window->priv;
	GtkWidget *dialog;
	gchar *filename, *path;
	gchar *temp_filename;
	GError *error = NULL;
	guint profile_index = roccat_config_window_get_page_index(ROCCAT_CONFIG_WINDOW(window), profile_page);
	RyosRkp *rkp;

	rkp = ryosconfig_profile_page_get_rkp(RYOSCONFIG_PROFILE_PAGE(profile_page));

	dialog = gtk_file_chooser_dialog_new(_("Save profile"),
			GTK_WINDOW(window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
			NULL);

	add_file_filter_to_file_chooser(GTK_FILE_CHOOSER(dialog));

	path = ryos_configuration_get_rkp_path(priv->config);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);
	g_free(path);

	filename = ryos_create_filename_proposition(rkp, profile_index);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);
	g_free(filename);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		temp_filename = roccat_create_filename_with_extension(filename, ROCCAT_KEYBOARD_PROFILE_EXTENSION);
		g_free(filename);

		path = g_path_get_dirname(temp_filename);
		ryos_configuration_set_rkp_path(priv->config, path);
		g_free(path);

		ryos_rkp_write_with_path(temp_filename, rkp, &error);
		g_free(temp_filename);
		if (error) {
			roccat_warning_dialog(GTK_WINDOW(dialog), _("Could not save profile"), error->message);
			g_error_free(error);
		}
	}

	gtk_widget_destroy(dialog);
	ryos_rkp_free(rkp);
}

static void load_profile_from_file_cb(RoccatProfilePage *profile_page, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(user_data);
	RyosconfigWindowPrivate *priv = window->priv;
	GtkWidget *dialog;
	gchar *filename, *path;
	GError *error = NULL;
	RyosRkp *rkp;

	dialog = gtk_file_chooser_dialog_new(_("Load profile"),
			GTK_WINDOW(window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);

	add_file_filter_to_file_chooser(GTK_FILE_CHOOSER(dialog));
	path = ryos_configuration_get_rkp_path(priv->config);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);
	g_free(path);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		path = g_path_get_dirname(filename);
		ryos_configuration_set_rkp_path(priv->config, path);
		g_free(path);

		rkp = ryos_rkp_read_with_path(filename, &error);
		if (error) {
			roccat_warning_dialog(GTK_WINDOW(dialog), _("Could not load profile"), error->message);
			g_error_free(error);
		} else {
			ryos_rkp_set_modified(rkp);
			ryosconfig_profile_page_set_rkp(RYOSCONFIG_PROFILE_PAGE(profile_page), rkp);
			ryos_rkp_free(rkp);
		}

		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

static void save_all_cb(RoccatConfigWindow *roccat_window, gpointer user_data) {
	save_all(RYOSCONFIG_WINDOW(roccat_window), FALSE);
}

static void set_keyboard_layout(RyosconfigWindow *window, gchar const *layout) {
	RyosconfigProfilePage *profile_page;
	guint i;

	for (i = 0; i < RYOS_PROFILE_NUM; ++i) {
		profile_page = RYOSCONFIG_PROFILE_PAGE(roccat_config_window_get_page(ROCCAT_CONFIG_WINDOW(window), i));
		ryosconfig_profile_page_set_keyboard_layout(profile_page, layout);
	}
}

static void menu_edit_preferences_cb(GtkMenuItem *item, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(user_data);
	RyosconfigWindowPrivate *priv = window->priv;
	ryos_configuration_dialog(GTK_WINDOW(window), priv->config);
	if (TRUE) { // FIXME only if changed!
		set_keyboard_layout(window, ryos_configuration_get_layout(priv->config));
		ryos_dbus_emit_configuration_changed_outside(priv->dbus_proxy);
	}
}

static void menu_device_firmware_update_cb(GtkMenuItem *item, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(user_data);
	GError *local_error = NULL;
	RoccatDevice *ryos;
	GtkWidget *assistant;
	guint version;
	gchar *version_string;

	if (!roccat_config_window_warn_if_no_device(ROCCAT_CONFIG_WINDOW(window)))
		return;

	ryos = roccat_config_window_get_device(ROCCAT_CONFIG_WINDOW(window));
	version = ryos_firmware_version_read(ryos, &local_error);
	if (local_error) {
		roccat_warning_dialog(GTK_WINDOW(window), _("Could not read version info"), local_error->message);
		g_error_free(local_error);
		return;
	}

	version_string = roccat_firmware_version_to_string(version);
	assistant = roccat_update_assistant_new(GTK_WINDOW(window), ryos);
	roccat_update_assistant_add_firmware(ROCCAT_UPDATE_ASSISTANT(assistant), _("Firmware"), version_string, RYOS_FIRMWARE_SIZE, ROCCAT_FIRMWARE_NUMBER_DEFAULT);
	g_free(version_string);
	gtk_widget_show_all(assistant);
}

static void menu_device_info_cb(GtkMenuItem *item, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(user_data);
	RoccatInfoDialog *dialog;
	GError *local_error = NULL;
	RyosInfo *info;
	gchar *firmware;
	gchar *dfu;

	if (!roccat_config_window_warn_if_no_device(ROCCAT_CONFIG_WINDOW(window)))
		return;

	info = ryos_info_read(roccat_config_window_get_device(ROCCAT_CONFIG_WINDOW(window)), &local_error);
	if (!roccat_handle_error_dialog(GTK_WINDOW(window), _("Could not read informations"), &local_error))
		return;

	dialog = ROCCAT_INFO_DIALOG(roccat_info_dialog_new(GTK_WINDOW(window)));

	firmware = roccat_firmware_version_to_string(info->firmware_version);
	roccat_info_dialog_add_line(dialog, _("Firmware version"), gtk_label_new(firmware));
	g_free(firmware);

	dfu = roccat_firmware_version_to_string(info->dfu_version);
	roccat_info_dialog_add_line(dialog, _("DFU version"), gtk_label_new(dfu));
	g_free(dfu);

	g_free(info);

	(void)gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void menu_device_reset_cb(GtkMenuItem *item, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(user_data);
	RyosconfigWindowPrivate *priv = window->priv;
	RyosRkp *rkp;
	guint i;
	GError *error = NULL;
	RoccatDevice *device;
	RyosconfigProfilePage *profile_page;

	if (!roccat_config_window_warn_if_no_device(ROCCAT_CONFIG_WINDOW(window)))
		return;

	if (!roccat_continue_dialog(GTK_WINDOW(window), _("This resets the device to factory defaults.")))
		return;

	device = roccat_config_window_get_device(ROCCAT_CONFIG_WINDOW(window));
	ryos_reset(device, RYOS_RESET_FUNCTION_RESET, &error);
	if (!roccat_handle_error_dialog(GTK_WINDOW(window), _("Could not reset device"), &error))
		return;

	for (i = 0; i < RYOS_PROFILE_NUM; ++i) {
		profile_page = RYOSCONFIG_PROFILE_PAGE(roccat_config_window_get_page(ROCCAT_CONFIG_WINDOW(window), i));
		rkp = ryos_rkp_load_save_after_reset(device, i, &error);
		ryosconfig_profile_page_set_rkp(profile_page, rkp);
		ryos_rkp_free(rkp);
		ryos_dbus_emit_profile_data_changed_outside(priv->dbus_proxy, i);
	}
}

static gboolean layer_editor_cb(RyosconfigProfilePage *profile_page, gpointer light_layer, gpointer user_data) {
	RyosconfigWindow *window = RYOSCONFIG_WINDOW(user_data);
	return ryosconfig_layer_illumination_dialog_run(GTK_WINDOW(window),
			roccat_config_window_get_device(ROCCAT_CONFIG_WINDOW(window)), light_layer);
}

GtkWidget *ryosconfig_window_new(void) {
	return GTK_WIDGET(g_object_new(RYOSCONFIG_WINDOW_TYPE,
			"device-name", RYOS_DEVICE_NAME_COMBINED,
			NULL));
}

static GObject *constructor(GType gtype, guint n_properties, GObjectConstructParam *properties) {
	GObject *obj;
	RyosconfigWindow *window;
	RoccatConfigWindow *roccat_window;
	RyosconfigWindowPrivate *priv;
	RyosRkp *rkp;
	guint i;
	GtkMenuItem *menu_item;
	GtkWidget *profile_page;

	obj = G_OBJECT_CLASS(ryosconfig_window_parent_class)->constructor(gtype, n_properties, properties);
	window = RYOSCONFIG_WINDOW(obj);
	roccat_window = ROCCAT_CONFIG_WINDOW(obj);
	priv = window->priv;

	priv->config = ryos_configuration_load();

	for (i = 0; i < RYOS_PROFILE_NUM; ++i) {
		profile_page = ryosconfig_profile_page_new();
		roccat_config_window_append_page(roccat_window, ROCCAT_PROFILE_PAGE(profile_page));

		rkp = ryos_rkp_load_actual(i);
		ryosconfig_profile_page_set_rkp(RYOSCONFIG_PROFILE_PAGE(profile_page), rkp);
		ryos_rkp_free(rkp);

		g_signal_connect(G_OBJECT(profile_page), "load-from-file", G_CALLBACK(load_profile_from_file_cb), window);
		g_signal_connect(G_OBJECT(profile_page), "save-to-file", G_CALLBACK(save_profile_to_file_cb), window);
		g_signal_connect(G_OBJECT(profile_page), "layer-editor", G_CALLBACK(layer_editor_cb), window);
	}

	set_keyboard_layout(window, ryos_configuration_get_layout(priv->config));
	
	menu_item = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL));
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_edit_preferences_cb), window);
	roccat_config_window_edit_menu_append(roccat_window, menu_item);

	menu_item = GTK_MENU_ITEM(gtk_image_menu_item_new_with_label(_("Firmware update")));
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_device_firmware_update_cb), window);
	roccat_config_window_device_menu_append(roccat_window, menu_item);

	menu_item = GTK_MENU_ITEM(gtk_image_menu_item_new_with_label(_("Reset")));
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_device_reset_cb), window);
	roccat_config_window_device_menu_append(roccat_window, menu_item);

	menu_item = GTK_MENU_ITEM(gtk_image_menu_item_new_with_label(_("Info")));
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_device_info_cb), window);
	roccat_config_window_device_menu_append(roccat_window, menu_item);

	g_signal_connect(G_OBJECT(roccat_window), "active-changed", G_CALLBACK(window_active_page_changed_cb), NULL);
	g_signal_connect(G_OBJECT(roccat_window), "delete-event", G_CALLBACK(delete_event_cb), NULL);
	g_signal_connect(G_OBJECT(roccat_window), "save-all", G_CALLBACK(save_all_cb), NULL);
	g_signal_connect(G_OBJECT(roccat_window), "device-added", G_CALLBACK(device_add_cb), NULL);
	g_signal_connect(G_OBJECT(roccat_window), "device-removed", G_CALLBACK(device_remove_cb), NULL);

	/* keep this order */
	priv->dbus_proxy = ryos_dbus_proxy_new();

	roccat_config_window_set_device_scanner(roccat_window, ROCCAT_DEVICE_SCANNER_INTERFACE(ryos_device_scanner_new()));

	gtk_widget_show(GTK_WIDGET(window));

	/* if a device is already plugged in, add_cb has already been executed */
	roccat_config_window_warn_if_no_device(roccat_window);

	return obj;
}

static void ryosconfig_window_init(RyosconfigWindow *window) {
	window->priv = RYOSCONFIG_WINDOW_GET_PRIVATE(window);
}

static void finalize(GObject *object) {
	RyosconfigWindowPrivate *priv = RYOSCONFIG_WINDOW(object)->priv;

	ryos_configuration_dialog_save(GTK_WINDOW(object), priv->config);
	ryos_dbus_emit_configuration_changed_outside(priv->dbus_proxy);
	g_clear_pointer(&priv->config, roccat_configuration_free);
	g_clear_pointer(&priv->dbus_proxy, dbus_roccat_proxy_free);

	G_OBJECT_CLASS(ryosconfig_window_parent_class)->finalize(object);
}

static void ryosconfig_window_class_init(RyosconfigWindowClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->constructor = constructor;
	gobject_class->finalize = finalize;

	g_type_class_add_private(klass, sizeof(RyosconfigWindowClass));
}
