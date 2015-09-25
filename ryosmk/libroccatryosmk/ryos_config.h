#ifndef __ROCCAT_RYOS_CONFIG_H__
#define __ROCCAT_RYOS_CONFIG_H__

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

#include "roccat_config.h"

G_BEGIN_DECLS

RoccatKeyFile *ryos_configuration_load(void);
gchar *ryos_configuration_get_rkp_path(RoccatKeyFile *config);
void ryos_configuration_set_rkp_path(RoccatKeyFile *config, gchar const *path);
gint ryos_configuration_get_timer_notification_type(RoccatKeyFile *config);
void ryos_configuration_set_timer_notification_type(RoccatKeyFile *config, gint new_value);
gint ryos_configuration_get_profile_notification_type(RoccatKeyFile *config);
void ryos_configuration_set_profile_notification_type(RoccatKeyFile *config, gint new_value);
gint ryos_configuration_get_live_recording_notification_type(RoccatKeyFile *config);
void ryos_configuration_set_live_recording_notification_type(RoccatKeyFile *config, gint new_value);
gdouble ryos_configuration_get_notification_volume(RoccatKeyFile *config);
void ryos_configuration_set_notification_volume(RoccatKeyFile *config, gdouble new_value);
gint ryos_configuration_get_default_profile_number(RoccatKeyFile *config);
void ryos_configuration_set_default_profile_number(RoccatKeyFile *config, gint new_value);
gboolean ryos_configuration_get_use_lua(RoccatKeyFile *config);
void ryos_configuration_set_use_lua(RoccatKeyFile *config, gboolean new_value);
gchar *ryos_configuration_get_layout(RoccatKeyFile *config);
void ryos_configuration_set_layout(RoccatKeyFile *config, gchar const *layout);
gchar *ryos_configuration_get_ripple_module(RoccatKeyFile *config);
void ryos_configuration_set_ripple_module(RoccatKeyFile *config, gchar const *filename);

G_END_DECLS

#endif
