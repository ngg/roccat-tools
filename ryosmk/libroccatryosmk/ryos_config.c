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

#include "ryos_config.h"
#include "g_roccat_helper.h"
#include "roccat_notificator.h"

static gchar const * const group_name = "RyosGUI";
static gchar const * const eventhandler_group_name = "Eventhandler";
static gchar const * const rkp_path_key = "RkpFilePath";
static gchar const * const timer_notification_type_key = "TimerNotificationType";
static gchar const * const profile_notification_type_key = "ProfileNotificationType";
static gchar const * const live_recording_notification_type_key = "LiveRecordingNotificationType";
static gchar const * const default_profile_number_key = "DefaultProfileNumber";
static gchar const * const notification_volume_key = "NotificationVolume";
static gchar const * const ripple_module_key = "RippleModule";
static gchar const * const use_lua_key = "UseLua";
static gchar const * const layout_key = "Layout";

static gchar *configuration_path(void) {
	gchar *dir = roccat_configuration_dir();
	gchar *path = g_build_filename(dir, "ryosmk.ini", NULL);
	g_free(dir);
	return path;
}

RoccatKeyFile *ryos_configuration_load(void) {
	RoccatKeyFile *config;
	gchar *path;

	path = configuration_path();
	config = roccat_key_file_load(path);
	g_free(path);

	return config;
}

gchar *ryos_configuration_get_rkp_path(RoccatKeyFile *config) {
	return roccat_key_file_get_string_with_default(config, group_name, rkp_path_key, g_get_home_dir());
}

void ryos_configuration_set_rkp_path(RoccatKeyFile *config, gchar const *path) {
	roccat_key_file_set_string(config, group_name, rkp_path_key, path);
}

gint ryos_configuration_get_timer_notification_type(RoccatKeyFile *config) {
	return roccat_key_file_get_integer_with_default(config, group_name, timer_notification_type_key, ROCCAT_NOTIFICATION_TYPE_OSD);
}

void ryos_configuration_set_timer_notification_type(RoccatKeyFile *config, gint new_value) {
	roccat_key_file_set_integer(config, group_name, timer_notification_type_key, new_value);
}

gint ryos_configuration_get_profile_notification_type(RoccatKeyFile *config) {
	return roccat_key_file_get_integer_with_default(config, group_name, profile_notification_type_key, ROCCAT_NOTIFICATION_TYPE_OSD);
}

void ryos_configuration_set_profile_notification_type(RoccatKeyFile *config, gint new_value) {
	roccat_key_file_set_integer(config, group_name, profile_notification_type_key, new_value);
}

gint ryos_configuration_get_live_recording_notification_type(RoccatKeyFile *config) {
	return roccat_key_file_get_integer_with_default(config, group_name, live_recording_notification_type_key, ROCCAT_NOTIFICATION_TYPE_OSD);
}

void ryos_configuration_set_live_recording_notification_type(RoccatKeyFile *config, gint new_value) {
	roccat_key_file_set_integer(config, group_name, live_recording_notification_type_key, new_value);
}

gdouble ryos_configuration_get_notification_volume(RoccatKeyFile *config) {
	return roccat_key_file_get_double_with_default(config, group_name, notification_volume_key, 0);
}

void ryos_configuration_set_notification_volume(RoccatKeyFile *config, gdouble new_value) {
	roccat_key_file_set_double(config, group_name, notification_volume_key, new_value);
}

gint ryos_configuration_get_default_profile_number(RoccatKeyFile *config) {
	return roccat_key_file_get_integer_with_default(config, group_name, default_profile_number_key, 0);
}

void ryos_configuration_set_default_profile_number(RoccatKeyFile *config, gint new_value) {
	roccat_key_file_set_integer(config, group_name, default_profile_number_key, new_value);
}

gboolean ryos_configuration_get_use_lua(RoccatKeyFile *config) {
	return roccat_key_file_get_boolean_with_default(config, eventhandler_group_name, use_lua_key, FALSE);
}

void ryos_configuration_set_use_lua(RoccatKeyFile *config, gboolean new_value) {
	roccat_key_file_set_boolean(config, eventhandler_group_name, use_lua_key, new_value);
}

gchar *ryos_configuration_get_layout(RoccatKeyFile *config) {
	return roccat_key_file_get_string_with_default(config, group_name, layout_key, "");
}

void ryos_configuration_set_layout(RoccatKeyFile *config, gchar const *layout) {
	roccat_key_file_set_string(config, group_name, layout_key, layout);
}

gchar *ryos_configuration_get_ripple_module(RoccatKeyFile *config) {
	return roccat_key_file_get_string_with_default(config, eventhandler_group_name, ripple_module_key, "ripple.lc");
}

void ryos_configuration_set_ripple_module(RoccatKeyFile *config, gchar const *filename) {
	roccat_key_file_set_string(config, eventhandler_group_name, ripple_module_key, filename);
}
