#ifndef __RYOS_KEY_BUTTON_H__
#define __RYOS_KEY_BUTTON_H__

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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define RYOS_KEY_BUTTON_TYPE (ryos_key_button_get_type())
#define RYOS_KEY_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOS_KEY_BUTTON_TYPE, RyosKeyButton))
#define IS_RYOS_KEY_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOS_KEY_BUTTON_TYPE))

typedef struct _RyosKeyButton RyosKeyButton;

GType ryos_key_button_get_type(void);
GtkWidget *ryos_key_button_new(guint hid_usage_id);
void ryos_key_button_set_label(RyosKeyButton *key_button, guint hid_usage_id);
void ryos_key_button_set_light(RyosKeyButton *key_button, gboolean state);

gchar *hid_to_beauty_keyname(guint8 hid);

G_END_DECLS

#endif
