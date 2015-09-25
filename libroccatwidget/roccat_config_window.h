#ifndef __ROCCAT_CONFIG_WINDOW_H__
#define __ROCCAT_CONFIG_WINDOW_H__

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

#include "roccat_profile_page.h"
#include "roccat_device_scanner_interface.h"

G_BEGIN_DECLS

#define ROCCAT_CONFIG_WINDOW_TYPE (roccat_config_window_get_type())
#define ROCCAT_CONFIG_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ROCCAT_CONFIG_WINDOW_TYPE, RoccatConfigWindowClass))
#define IS_ROCCAT_CONFIG_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ROCCAT_CONFIG_WINDOW_TYPE))
#define ROCCAT_CONFIG_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ROCCAT_CONFIG_WINDOW_TYPE, RoccatConfigWindow))
#define IS_ROCCAT_CONFIG_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ROCCAT_CONFIG_WINDOW_TYPE))

typedef struct _RoccatConfigWindowClass RoccatConfigWindowClass;
typedef struct _RoccatConfigWindow RoccatConfigWindow;
typedef struct _RoccatConfigWindowPrivate RoccatConfigWindowPrivate;

struct _RoccatConfigWindowClass {
	GtkWindowClass parent_class;
};

struct _RoccatConfigWindow {
	GtkWindow window;
	RoccatConfigWindowPrivate *priv;
};

GType roccat_config_window_get_type(void);
GtkWidget *roccat_config_window_new(gchar const *device_name);

void roccat_config_window_append_page(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page);
void roccat_config_window_remove_pages(RoccatConfigWindow *config_window);
gint roccat_config_window_get_page_index(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page);
RoccatProfilePage *roccat_config_window_get_page(RoccatConfigWindow *config_window, guint index);
guint roccat_config_window_get_num_pages(RoccatConfigWindow *config_window);

gboolean roccat_config_window_get_page_moved(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page);
void roccat_config_window_set_page_unmoved(RoccatConfigWindow *config_window, RoccatProfilePage *profile_page);

void roccat_config_window_menu_insert(RoccatConfigWindow *config_window, GtkMenuItem *menu_item);
void roccat_config_window_edit_menu_append(RoccatConfigWindow *config_window, GtkMenuItem *menu_item);
void roccat_config_window_device_menu_append(RoccatConfigWindow *config_window, GtkMenuItem *menu_item);

void roccat_config_window_set_active_page(RoccatConfigWindow *config_window, guint index);
gint roccat_config_window_get_active_page(RoccatConfigWindow *config_window);

void roccat_config_window_set_device_scanner(RoccatConfigWindow *config_window, RoccatDeviceScannerInterface *device_scanner_interface);
RoccatDevice *roccat_config_window_get_device(RoccatConfigWindow *config_window);
gboolean roccat_config_window_warn_if_no_device(RoccatConfigWindow *config_window);

G_END_DECLS

#endif
