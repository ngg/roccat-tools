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

#include "ryos_sdk.h"
#include "ryos_custom_lights.h"
#include "ryos_light.h"
#include "ryos_profile.h"
#include "ryos_light_control.h"
#include "roccat_helper.h"
#include <string.h>

enum {
	RYOS_SDK_KEY_NUM = 110,
};

static guint table_sdk_to_light_index[RYOS_SDK_KEY_NUM] = {
	0x01, 0x02, 0x0E, 0x03, 0x0F, 0x04, 0x10, 0x05, 0x11, 0x06,
	0x07, 0x08, 0x14, 0x09, 0x15, 0x21, 0x00, 0x0C, 0x0D, 0x19,
	0x1A, 0x1B, 0x27, 0x1C, 0x28, 0x1D, 0x29, 0x12, 0x1E, 0x13,
	0x2C, 0x2D, 0x39, 0x45, 0x0A, 0x16, 0x0B, 0x17, 0x18, 0x24,
	0x25, 0x31, 0x26, 0x32, 0x33, 0x34, 0x40, 0x35, 0x2A, 0x36,
	0x1F, 0x2B, 0x44, 0x51, 0x5D, 0x69, 0x22, 0x2E, 0x23, 0x2F,
	0x30, 0x3C, 0x3D, 0x49, 0x3E, 0x4A, 0x3F, 0x4B, 0x4C, 0x41,
	0x42, 0x4E, 0x37, 0x43, 0x3A, 0x46, 0x3B, 0x48, 0x54, 0x55,
	0x61, 0x56, 0x62, 0x57, 0x63, 0x58, 0x4D, 0x59, 0x5A, 0x4F,
	0x50, 0x75, 0x52, 0x5E, 0x53, 0x5F, 0x60, 0x6C, 0x6D, 0x6E,
	0x70, 0x72, 0x73, 0x67, 0x5C, 0x68, 0x74, 0x76, 0x6A, 0x6B,
};

static RyosCustomLights sdk_custom_lights = { 0 };

static gboolean sdk_write(RoccatDevice *ryos, GError **error) {
	return ryos_custom_lights_write(ryos, &sdk_custom_lights, error);
}

gboolean ryos_sdk_set_kb_sdk_mode(RoccatDevice *ryos, gboolean state, GError **error) {
	gboolean retval;
	RyosLightLayer *light_layer;
	
	retval = ryos_light_control_custom(ryos, state, error);
	if (!retval)
		return retval;

	if (!state)
		return retval;

	light_layer = ryos_get_active_light_layer(ryos, error);
	if (!light_layer)
		return FALSE;
	
	ryos_light_layer_to_custom_lights(light_layer, &sdk_custom_lights, TRUE);
	g_free(light_layer);
	
	return sdk_write(ryos, error);
}

gboolean ryos_sdk_set_all_leds(RoccatDevice *ryos, GArray const *data, GError **error) {
	guint i;
	gboolean value;

	for (i = 0; i < RYOS_SDK_KEY_NUM; ++i) {
		if (i < data->len)
			value = g_array_index(data, guchar, i) ? TRUE : FALSE;
		else
			value = FALSE;
		roccat_bitfield_set_bit(&sdk_custom_lights.data[0], table_sdk_to_light_index[i], value);
	}

	return sdk_write(ryos, error);
}

gboolean ryos_sdk_turn_on_all_leds(RoccatDevice *ryos, GError **error) {
	memset(&sdk_custom_lights.data[0], 0xff, RYOS_CUSTOM_LIGHTS_DATA_LENGTH);
	return sdk_write(ryos, error);
}

gboolean ryos_sdk_turn_off_all_leds(RoccatDevice *ryos, GError **error) {
	memset(&sdk_custom_lights.data[0], 0x00, RYOS_CUSTOM_LIGHTS_DATA_LENGTH);
	return sdk_write(ryos, error);
}

gboolean ryos_sdk_set_led_on(RoccatDevice *ryos, guint8 index, GError **error) {
	roccat_bitfield_set_bit(&sdk_custom_lights.data[0], table_sdk_to_light_index[index], TRUE);
	return sdk_write(ryos, error);
}

gboolean ryos_sdk_set_led_off(RoccatDevice *ryos, guint8 index, GError **error) {
	roccat_bitfield_set_bit(&sdk_custom_lights.data[0], table_sdk_to_light_index[index], FALSE);
	return sdk_write(ryos, error);
}

// TODO threaded?
gboolean ryos_sdk_all_key_blinking(RoccatDevice *ryos, guint interval, guint count, GError **error) {
	guint i;

	for (i = 0; i < count; ++i) {
		if (!ryos_sdk_turn_on_all_leds(ryos, error))
			return FALSE;
		g_usleep(interval * 1000);
		if (!ryos_sdk_turn_off_all_leds(ryos, error))
			return FALSE;
		g_usleep(interval * 1000);
	}

	return TRUE;
}
