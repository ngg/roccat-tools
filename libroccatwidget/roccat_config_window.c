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

#include "roccat_config_window.h"
#include "roccat_profile_page_tab_label.h"
#include "roccat_about_dialog.h"
#include "roccat_timer_editor_dialog.h"
#include "gaminggear/gaminggear_macro_editor_dialog.h"
#include "roccat_warning_dialog.h"
#include "gtk_roccat_helper.h"
#include "g_roccat_helper.h"
#include "g_cclosure_roccat_marshaller.h"
#include "i18n-lib.h"

#define ROCCAT_CONFIG_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), ROCCAT_CONFIG_WINDOW_TYPE, RoccatConfigWindowPrivate))

struct _RoccatConfigWindowPrivate {
	GtkNotebook *notebook;
	GtkMenuBar *menu_bar;
	GtkMenu *edit_menu;
	GtkMenu *device_menu;
	gchar *device_name;
	RoccatDeviceScannerInterface *device_scanner_interface;
	RoccatDevice *device;
	gulong device_remove_handler_id;
	guint active_page;
	guint variable_pages;
	GtkButton *add_button;
};

G_DEFINE_TYPE(RoccatConfigWindow, roccat_config_window, GTK_TYPE_WINDOW);

enum {
	PROP_0,
	PROP_DEVICE_NAME,
	PROP_VARIABLE_PAGES,
};

enum {
	SAVE_ALL,
	ACTIVE_CHANGED,
	DEVICE_ADDED,
	DEVICE_REMOVED,
	ADD_PAGE,
	REMOVE_PAGE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void roccat_config_window_set_title(RoccatConfigWindow *config_window, gboolean connected, gchar const *device_name) {
	gchar *title;

	title = g_strdup_printf(_("%s configuration - %s connected"),
			config_window->priv->device_name,
			connected ? device_name : _("No device"));
	gtk_window_set_title(GTK_WINDOW(config_window), title);
	g_free(title);
}

static void roccat_config_window_set_title_disconnected(RoccatConfigWindow *config_window) {
	roccat_config_window_set_title(config_window, FALSE, "");
}

static void roccat_config_window_set_title_connected(RoccatConfigWindow *config_window, gchar const *device_name) {
	roccat_config_window_set_title(config_window, TRUE, device_name);
}

GtkWidget *roccat_config_window_new(gchar const *device_name) {
	return GTK_WIDGET(g_object_new(ROCCAT_CONFIG_WINDOW_TYPE,
			"device-name", device_name,
			NULL));
}

static void menu_file_save_all_cb(GtkMenuItem *item, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	g_signal_emit((gpointer)config_window, signals[SAVE_ALL], 0);
}

static void menu_file_quit_cb(GtkMenuItem *item, gpointer user_data) {
	GdkEvent *event;
	gboolean retval;

	/* close window like the close button has been pressed
	 * which leads to user being asked to save work
	 */
	event = gdk_event_new(GDK_DELETE);
	g_signal_emit_by_name((gpointer)user_data, "delete-event", event, &retval);
	gdk_event_free(event);
	if (!retval)
		gtk_widget_destroy(GTK_WIDGET(user_data));
}

static void menu_edit_timers_cb(GtkMenuItem *item, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	roccat_timer_editor_dialog(GTK_WINDOW(config_window));
}

static void menu_edit_macros_cb(GtkMenuItem *item, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	gaminggear_macro_editor_dialog(GTK_WINDOW(config_window));
}

static void menu_help_about_cb(GtkMenuItem *item, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	gchar *title;

	title = g_strdup_printf(_("Application to manipulate profiles and settings of Roccat %s devices"),
			config_window->priv->device_name);
	roccat_about_dialog(GTK_WINDOW(config_window), NULL, title);
	g_free(title);
}

static GtkWidget *menu_bar_new(RoccatConfigWindow *config_window) {
	RoccatConfigWindowPrivate *priv = config_window->priv;
	GtkWidget *menu_bar;
	GtkWidget *menu_item;
	GtkWidget *menu;

	menu_bar = gtk_menu_bar_new();

	menu_item = gtk_menu_item_new_with_label(_("File"));
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu_item = gtk_image_menu_item_new_with_label(_("Store all in device"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_file_save_all_cb), config_window);

	/* separator */
	menu_item = gtk_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_file_quit_cb), config_window);


	menu_item = gtk_menu_item_new_with_label(_("Edit"));
	priv->edit_menu = GTK_MENU(gtk_menu_new());
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), GTK_WIDGET(priv->edit_menu));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu_item = gtk_image_menu_item_new_with_label(_("Macros"));
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->edit_menu), menu_item);
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_edit_macros_cb), config_window);

	menu_item = gtk_image_menu_item_new_with_label(_("Timers"));
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->edit_menu), menu_item);
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_edit_timers_cb), config_window);


	menu_item = gtk_menu_item_new_with_label(_("Device"));
	priv->device_menu = GTK_MENU(gtk_menu_new());
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), GTK_WIDGET(priv->device_menu));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);


	menu_item = gtk_menu_item_new_with_label(_("Help"));
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(menu_help_about_cb), config_window);

	return menu_bar;
}

static RoccatProfilePageTabLabel *get_tab_label_from_profile_page(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page) {
	return ROCCAT_PROFILE_PAGE_TAB_LABEL(gtk_notebook_get_tab_label(config_window->priv->notebook, GTK_WIDGET(profile_page)));
}

static RoccatProfilePageTabLabel *get_tab_label(RoccatConfigWindow *config_window, gint index) {
	return get_tab_label_from_profile_page(config_window, roccat_config_window_get_page(config_window, index));
}

static RoccatProfilePage *get_profile_page_from_tab_label(RoccatConfigWindow *config_window, RoccatProfilePageTabLabel const *tab_label) {
	RoccatProfilePage *profile_page;
	guint i;
	guint count;
	
	count = roccat_config_window_get_num_pages(config_window);
	for (i = 0; i < count; ++i) {
		profile_page = roccat_config_window_get_page(config_window, i);
		if (get_tab_label_from_profile_page(config_window, profile_page) == tab_label)
			return profile_page;
	}
	
	return NULL;
}

gboolean roccat_config_window_get_page_moved(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page) {
	gint is_index = roccat_config_window_get_page_index(config_window, profile_page);
	gint was_index = roccat_profile_page_tab_label_get_index(get_tab_label_from_profile_page(config_window, profile_page));
	return (is_index == was_index) ? FALSE : TRUE;
}

void roccat_config_window_set_page_unmoved(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page) {
	gint is_index = roccat_config_window_get_page_index(config_window, profile_page);
	roccat_profile_page_tab_label_set_index(get_tab_label_from_profile_page(config_window, profile_page), is_index);
}

static void page_reordered_cb(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	roccat_config_window_set_active_page(config_window, roccat_config_window_get_active_page(config_window));
}

static void destroy_event_cb(GtkWidget *window, gpointer user_data) {
	gtk_main_quit();
}

static void button_apply_cb(GtkButton *button, gpointer user_data) {
	g_signal_emit((gpointer)user_data, signals[SAVE_ALL], 0);
}

static void button_ok_cb(GtkButton *button, gpointer user_data) {
	/* Data should be saved without asking user and window should be closed
	 * without triggering question again. */
	g_signal_emit((gpointer)user_data, signals[SAVE_ALL], 0);
	gtk_widget_destroy(GTK_WIDGET(user_data));
}

static void button_cancel_cb(GtkButton *button, gpointer user_data) {
	gtk_widget_destroy(GTK_WIDGET(user_data));
}

static GtkWidget *buttons_new(RoccatConfigWindow *config_window) {
	GtkWidget *hbox1, *hbox2, *button;

	hbox1 = gtk_hbox_new(FALSE, 0);
	hbox2 = gtk_hbox_new(TRUE, 0);

	gtk_box_pack_end(GTK_BOX(hbox1), hbox2, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cancel_cb), config_window);
	gtk_box_pack_start(GTK_BOX(hbox2), button, TRUE, TRUE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_apply_cb), config_window);
	gtk_box_pack_start(GTK_BOX(hbox2), button, TRUE, TRUE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_ok_cb), config_window);
	gtk_box_pack_start(GTK_BOX(hbox2), button, TRUE, TRUE, 0);

	return hbox1;
}

static void add_button_clicked_cb(GtkButton *button, gpointer user_data) {
	g_signal_emit(user_data, signals[ADD_PAGE], 0);
}

static void roccat_config_window_init(RoccatConfigWindow *config_window) {
	RoccatConfigWindowPrivate *priv = ROCCAT_CONFIG_WINDOW_GET_PRIVATE(config_window);
	GtkWidget *vbox;
	GtkWidget *image;

	config_window->priv = priv;
	priv->device_name = NULL;

	gtk_window_resize(GTK_WINDOW(config_window), 640, 480);

	gtk_window_set_default_icon_name("roccat");
	g_signal_connect(G_OBJECT(config_window), "destroy", G_CALLBACK(destroy_event_cb), NULL);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(config_window), GTK_WIDGET(vbox));

	priv->menu_bar = GTK_MENU_BAR(menu_bar_new(config_window));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->menu_bar), FALSE, FALSE, 0);

	priv->notebook = GTK_NOTEBOOK(gtk_notebook_new());
	g_signal_connect(G_OBJECT(priv->notebook), "page-reordered", G_CALLBACK(page_reordered_cb), config_window);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->notebook), TRUE, TRUE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
	priv->add_button = GTK_BUTTON(gtk_button_new());
	gtk_button_set_image(priv->add_button, image);
	gtk_button_set_relief(priv->add_button, GTK_RELIEF_NONE);
	g_signal_connect(G_OBJECT(priv->add_button), "clicked", G_CALLBACK(add_button_clicked_cb), config_window);
	gtk_notebook_set_action_widget(priv->notebook, GTK_WIDGET(priv->add_button), GTK_PACK_END);

	gtk_box_pack_start(GTK_BOX(vbox), buttons_new(config_window), FALSE, FALSE, 0);

	gtk_widget_show_all(vbox);
}

static void update_state_first_page(RoccatConfigWindow *config_window) {
	RoccatProfilePageTabLabel *tab_label;

	tab_label = get_tab_label(config_window, 0);

	if (config_window->priv->variable_pages && (roccat_config_window_get_num_pages(config_window) > 1))
		roccat_profile_page_tab_label_set_closeable(tab_label, TRUE);
	else
		roccat_profile_page_tab_label_set_closeable(tab_label, FALSE);
}

static void update_state_show_tabs(RoccatConfigWindow *config_window) {
	RoccatConfigWindowPrivate *priv = config_window->priv;
	gboolean show_tabs = (priv->variable_pages > 2) || (roccat_config_window_get_num_pages(config_window) > 1);
	gtk_notebook_set_show_tabs(priv->notebook, show_tabs);
}

static void update_state_add_button(RoccatConfigWindow *config_window) {
	RoccatConfigWindowPrivate *priv = config_window->priv;
	if (priv->variable_pages && (roccat_config_window_get_num_pages(config_window) < priv->variable_pages))
		gtk_widget_show(GTK_WIDGET(priv->add_button));
	else
		gtk_widget_hide(GTK_WIDGET(priv->add_button));
}

static GObject *constructor(GType gtype, guint n_properties, GObjectConstructParam *properties) {
	GObject *obj;
	RoccatConfigWindow *config_window;

	obj = G_OBJECT_CLASS(roccat_config_window_parent_class)->constructor(gtype, n_properties, properties);
	config_window = ROCCAT_CONFIG_WINDOW(obj);

	update_state_show_tabs(config_window);
	update_state_add_button(config_window);
	roccat_config_window_set_title_disconnected(config_window);

	return obj;
}

static void set_property(GObject *object, guint prop_id, GValue const *value, GParamSpec *pspec) {
	RoccatConfigWindowPrivate *priv = ROCCAT_CONFIG_WINDOW(object)->priv;
	switch(prop_id) {
	case PROP_DEVICE_NAME:
		g_free(priv->device_name);
		priv->device_name = g_value_dup_string(value);
		break;
	case PROP_VARIABLE_PAGES:
		priv->variable_pages = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
	RoccatConfigWindowPrivate *priv = ROCCAT_CONFIG_WINDOW(object)->priv;
	switch(prop_id) {
	case PROP_DEVICE_NAME:
		g_value_set_string(value, priv->device_name);
		break;
	case PROP_VARIABLE_PAGES:
		g_value_set_uint(value, priv->variable_pages);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void finalize(GObject *object) {
	RoccatConfigWindowPrivate *priv = ROCCAT_CONFIG_WINDOW(object)->priv;

	g_clear_object(&priv->device_scanner_interface);

	G_OBJECT_CLASS(roccat_config_window_parent_class)->finalize(object);
}

static void roccat_config_window_class_init(RoccatConfigWindowClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->constructor = constructor;
	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;
	gobject_class->finalize = finalize;

	g_type_class_add_private(klass, sizeof(RoccatConfigWindowPrivate));

	g_object_class_install_property(gobject_class, PROP_DEVICE_NAME,
			g_param_spec_string("device-name",
					"Device name",
					"Name of device",
					"Undefined",
					G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	 g_object_class_install_property(gobject_class, PROP_VARIABLE_PAGES,
			 g_param_spec_uint("variable-pages",
					 "Variable page count",
					 "Variable page count",
					 0, G_MAXUINT, 0,
					 G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	signals[SAVE_ALL] = g_signal_new("save-all",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0,
			NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	signals[ACTIVE_CHANGED] = g_signal_new("active-changed",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0,
			NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	signals[DEVICE_ADDED] = g_signal_new("device-added",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0,
			NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	signals[DEVICE_REMOVED] = g_signal_new("device-removed",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0,
			NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	signals[ADD_PAGE] = g_signal_new("add-page",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0,
			NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	signals[REMOVE_PAGE] = g_signal_new("remove-page",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0,
			NULL, NULL, g_cclosure_roccat_marshal_BOOLEAN__UINT, G_TYPE_BOOLEAN, 1, G_TYPE_UINT);
}

void roccat_config_window_set_active_page(RoccatConfigWindow *config_window, guint index) {
	RoccatProfilePageTabLabel *tab_label = get_tab_label(config_window, index);
	if (tab_label) {
		roccat_profile_page_tab_label_set_active(tab_label);
		config_window->priv->active_page = index;
	}
}

gint roccat_config_window_get_active_page(RoccatConfigWindow *config_window) {
	return config_window->priv->active_page;
}

static void active_changed_cb(RoccatProfilePageTabLabel *tab_label, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	RoccatConfigWindowPrivate *priv = ROCCAT_CONFIG_WINDOW(user_data)->priv;
	guint new_active;

	new_active = roccat_config_window_get_page_index(config_window, get_profile_page_from_tab_label(config_window, tab_label));
	if (priv->active_page != new_active) {
		priv->active_page = new_active;
		g_signal_emit(user_data, signals[ACTIVE_CHANGED], 0);
	}
}

static void remove_page(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page) {
	gint index = roccat_config_window_get_page_index(config_window, profile_page);
	gboolean should_remove;
	g_signal_emit((gpointer)config_window, signals[REMOVE_PAGE], 0, index, &should_remove);
	if (should_remove) {
		gtk_notebook_remove_page(config_window->priv->notebook, index);
		roccat_config_window_set_active_page(config_window, roccat_config_window_get_active_page(config_window));
		update_state_first_page(config_window);
		update_state_add_button(config_window);
	}
}

void roccat_config_window_remove_pages(RoccatConfigWindow *config_window) {
	while (roccat_config_window_get_page(config_window, 0))
		gtk_notebook_remove_page(config_window->priv->notebook, 0);
	update_state_show_tabs(config_window);
	update_state_add_button(config_window);
}

static void tab_label_close_cb(RoccatProfilePageTabLabel *tab_label, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	RoccatProfilePage *profile_page;

	if (roccat_config_window_get_num_pages(config_window) > 1) {
		profile_page = get_profile_page_from_tab_label(config_window, tab_label);
		remove_page(config_window, profile_page);
	}
}

RoccatProfilePage *roccat_config_window_get_page(RoccatConfigWindow *config_window, guint index) {
	return ROCCAT_PROFILE_PAGE(gtk_notebook_get_nth_page(config_window->priv->notebook, index));
}

gint roccat_config_window_get_page_index(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page) {
	return gtk_notebook_page_num(config_window->priv->notebook, GTK_WIDGET(profile_page));
}

guint roccat_config_window_get_num_pages(RoccatConfigWindow *config_window) {
	return gtk_notebook_get_n_pages(config_window->priv->notebook);
}

static void profile_renamed_cb(RoccatProfilePage *profile_page, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	RoccatProfilePageTabLabel *tab_label = get_tab_label_from_profile_page(config_window, profile_page);
	gchar *name = roccat_profile_page_get_name(profile_page);
	roccat_profile_page_tab_label_set_name(tab_label, name);
	g_free(name);
}

void roccat_config_window_append_page(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page) {
	RoccatConfigWindowPrivate *priv = config_window->priv;
	RoccatProfilePageTabLabel *tab_label;
	gint index;
	
	tab_label = ROCCAT_PROFILE_PAGE_TAB_LABEL(roccat_profile_page_tab_label_new());
	
	index = gtk_notebook_append_page(priv->notebook, GTK_WIDGET(profile_page), GTK_WIDGET(tab_label));
	gtk_notebook_set_tab_reorderable(priv->notebook, GTK_WIDGET(profile_page), TRUE);

	if (index > 0)
		roccat_profile_page_tab_label_set_group(tab_label, roccat_profile_page_tab_label_get_group(get_tab_label(config_window, 0)));
	roccat_profile_page_tab_label_set_index(tab_label, index);
	roccat_profile_page_tab_label_set_closeable(tab_label, priv->variable_pages > 1);

	g_signal_connect(G_OBJECT(tab_label), "active-changed", G_CALLBACK(active_changed_cb), config_window);
	g_signal_connect(G_OBJECT(tab_label), "close", G_CALLBACK(tab_label_close_cb), config_window);
	
	g_signal_connect(G_OBJECT(profile_page), "renamed", G_CALLBACK(profile_renamed_cb), config_window);

	update_state_show_tabs(config_window);
	update_state_add_button(config_window);
	update_state_first_page(config_window);
}

void roccat_config_window_menu_insert(RoccatConfigWindow *config_window, GtkMenuItem *menu_item) {
	RoccatConfigWindowPrivate *priv = config_window->priv;
	gint position;

	/* second to last */
	position = gtk_roccat_container_get_n_children(GTK_CONTAINER(priv->menu_bar)) - 1;
	if (position < 0)
		position = 0;

	gtk_menu_shell_insert(GTK_MENU_SHELL(priv->menu_bar), GTK_WIDGET(menu_item), position);
}

void roccat_config_window_edit_menu_append(RoccatConfigWindow *config_window, GtkMenuItem *menu_item) {
	RoccatConfigWindowPrivate *priv = config_window->priv;
	gtk_widget_show(GTK_WIDGET(menu_item));
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->edit_menu), GTK_WIDGET(menu_item));
}

void roccat_config_window_device_menu_append(RoccatConfigWindow *config_window, GtkMenuItem *menu_item) {
	RoccatConfigWindowPrivate *priv = config_window->priv;
	gtk_widget_show(GTK_WIDGET(menu_item));
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->device_menu), GTK_WIDGET(menu_item));
}

static void remove_device_remove_handler(RoccatConfigWindow *config_window) {
	RoccatConfigWindowPrivate *priv = config_window->priv;

	if (!priv->device_remove_handler_id)
		return;

	g_signal_handler_disconnect(G_OBJECT(priv->device_scanner_interface), priv->device_remove_handler_id);
	priv->device_remove_handler_id = 0;
}

static void device_remove_cb(RoccatDeviceScannerInterface *interface, RoccatDevice *device, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	RoccatConfigWindowPrivate *priv = config_window->priv;

	if (!priv->device || !gaminggear_device_equal(GAMINGGEAR_DEVICE(device), GAMINGGEAR_DEVICE(priv->device)))
		return;

	remove_device_remove_handler(config_window);

	g_clear_object(&priv->device);

	roccat_config_window_set_title_disconnected(config_window);

	g_signal_emit((gpointer)config_window, signals[DEVICE_REMOVED], 0);
}

static void set_device_remove_handler(RoccatConfigWindow *config_window) {
	RoccatConfigWindowPrivate *priv = config_window->priv;

	if (priv->device_remove_handler_id)
		return;

	priv->device_remove_handler_id = g_signal_connect(G_OBJECT(priv->device_scanner_interface),
			"device-removed", G_CALLBACK(device_remove_cb), config_window);
}

static void device_add_cb(RoccatDeviceScannerInterface *interface, RoccatDevice *device, gpointer user_data) {
	RoccatConfigWindow *config_window = ROCCAT_CONFIG_WINDOW(user_data);
	RoccatConfigWindowPrivate *priv = config_window->priv;

	roccat_device_debug(device);

	if (priv->device) {
		roccat_multiple_device_warning_dialog(GTK_WINDOW(config_window), roccat_device_get_name_static(device));
		return;
	}

	priv->device = device;
	g_object_ref(G_OBJECT(priv->device));

	roccat_config_window_set_title_connected(config_window, roccat_device_get_name_static(priv->device));
	set_device_remove_handler(config_window);

	g_signal_emit((gpointer)config_window, signals[DEVICE_ADDED], 0);
}

void roccat_config_window_set_device_scanner(RoccatConfigWindow *config_window, RoccatDeviceScannerInterface *device_scanner_interface) {
	RoccatConfigWindowPrivate *priv = config_window->priv;

	if (priv->device_scanner_interface)
		return;

	priv->device_scanner_interface = device_scanner_interface;
	g_object_ref(G_OBJECT(priv->device_scanner_interface));

	g_signal_connect(G_OBJECT(priv->device_scanner_interface), "device-added", G_CALLBACK(device_add_cb), config_window);
	roccat_device_scanner_interface_start(priv->device_scanner_interface);
}

RoccatDevice *roccat_config_window_get_device(RoccatConfigWindow *config_window) {
	return config_window->priv->device;
}

gboolean roccat_config_window_warn_if_no_device(RoccatConfigWindow *config_window) {
	RoccatConfigWindowPrivate *priv = config_window->priv;

	return roccat_no_device_warning_dialog(GTK_WINDOW(config_window),
			priv->device, priv->device_name);
}
