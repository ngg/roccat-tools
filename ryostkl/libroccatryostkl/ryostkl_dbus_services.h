#ifndef __ROCCAT_RYOSTKL_DBUS_SERVICES_H__
#define __ROCCAT_RYOSTKL_DBUS_SERVICES_H__

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with roccat-tools. If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

#define RYOSTKL_DBUS_SERVER_PATH "/org/roccat/Ryostkl"
#define RYOSTKL_DBUS_SERVER_INTERFACE "org.roccat.Ryostkl"

DBusGProxy *ryostkl_dbus_proxy_new(void);
gboolean ryostkl_dbus_emit_profile_data_changed_outside(DBusGProxy *proxy, guint profile_index);
gboolean ryostkl_dbus_emit_profile_data_changed_outside_instant(guint profile_index);
gboolean ryostkl_dbus_emit_profile_changed_outside(DBusGProxy *proxy, guint profile_index);
gboolean ryostkl_dbus_emit_profile_changed_outside_instant(guint profile_index);
gboolean ryostkl_dbus_emit_configuration_changed_outside(DBusGProxy *proxy);
gboolean ryostkl_dbus_emit_configuration_changed_outside_instant(void);

G_END_DECLS

#endif
