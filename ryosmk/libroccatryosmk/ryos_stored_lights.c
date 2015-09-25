/*
 * This file is part of roccat-tools.
 *
 * roccat-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Macro Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * roccat-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Macro Public License for more details.
 *
 * You should have received a copy of the GNU Macro Public License
 * along with roccat-tools. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ryos_stored_lights.h"
#include "roccat_helper.h"
#include "ryos_device.h"
#include "ryos_light.h"
#include "ryos_profile.h"
#include <string.h>

static guint16 ryos_stored_lights_calc_checksum(RyosStoredLights const *stored_lights) {
	return ROCCAT_BYTESUM_PARTIALLY(stored_lights, RyosStoredLights, report_id, checksum);
}

static void ryos_stored_lights_set_checksum(RyosStoredLights *stored_lights, guint16 new_value) {
	stored_lights->checksum = GUINT16_TO_LE(new_value);
}

static void ryos_stored_lights_finalize(RyosStoredLights *stored_lights, guint profile_index, guint mode) {
	stored_lights->report_id = RYOS_REPORT_ID_STORED_LIGHTS;
	stored_lights->size = sizeof(RyosStoredLights);
	stored_lights->profile_index = profile_index;
	stored_lights->illumination_mode = mode;
	ryos_stored_lights_set_checksum(stored_lights, ryos_stored_lights_calc_checksum(stored_lights));
}

gboolean ryos_stored_lights_write(RoccatDevice *ryos, guint profile_index, guint mode, RyosStoredLights *stored_lights, GError **error) {
	g_assert(profile_index < RYOS_PROFILE_NUM);
	ryos_stored_lights_finalize(stored_lights, profile_index, mode);
	return ryos_device_write(ryos, (gchar const *)stored_lights, sizeof(RyosStoredLights), error);
}

static RyosStoredLights *ryos_stored_lights_read(RoccatDevice *ryos, guint profile_index, guint request, GError **error) {
	RyosStoredLights *stored_lights;

	g_assert(profile_index < RYOS_PROFILE_NUM);

	gaminggear_device_lock(GAMINGGEAR_DEVICE(ryos));

	if (!ryos_select(ryos, profile_index, request, error)) {
		gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));
		return NULL;
	}

	stored_lights = (RyosStoredLights *)ryos_device_read(ryos, RYOS_REPORT_ID_STORED_LIGHTS, sizeof(RyosStoredLights), error);

	gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));

	return stored_lights;
}

RyosStoredLights *ryos_stored_lights_read_manual(RoccatDevice *ryos, guint profile_index, GError **error) {
	return ryos_stored_lights_read(ryos, profile_index, RYOS_CONTROL_REQUEST_STORED_LIGHTS_MANUAL, error);
}

RyosStoredLights *ryos_stored_lights_read_automatic(RoccatDevice *ryos, guint profile_index, GError **error) {
	return ryos_stored_lights_read(ryos, profile_index, RYOS_CONTROL_REQUEST_STORED_LIGHTS_AUTOMATIC, error);
}

gboolean ryos_stored_lights_equal(RyosStoredLights const *left, RyosStoredLights const *right) {
	gboolean equal;
	equal = ROCCAT_MEMCMP_PARTIALLY(left, right, RyosStoredLights, illumination_mode, checksum);
	return equal ? FALSE : TRUE;
}

void ryos_stored_lights_copy(RyosStoredLights *destination, RyosStoredLights const *source) {
	memcpy(destination, source, sizeof(RyosStoredLights));
}

void ryos_light_layer_copy(RyosLightLayer *destination, RyosLightLayer const *source) {
	memcpy(destination, source, sizeof(RyosLightLayer));
}

RyosLightLayer *ryos_get_active_light_layer(RoccatDevice *ryos, GError **error) {
	GError *local_error = NULL;
	guint profile_index;
	RyosLight *light;
	guint illumination_mode;
	RyosStoredLights *stored_lights;
	RyosLightLayer *result;
	
	profile_index = ryos_profile_read(ryos, &local_error);
	if (local_error) {
		g_propagate_error(error, local_error);
		return NULL;
	}

	light = ryos_light_read(ryos, profile_index, error);
	if (light == NULL)
		return NULL;
	illumination_mode = light->illumination_mode;
	g_free(light);

	if (illumination_mode == RYOS_ILLUMINATION_MODE_AUTOMATIC)
		stored_lights = ryos_stored_lights_read_automatic(ryos, profile_index, error);
	else
		stored_lights = ryos_stored_lights_read_manual(ryos, profile_index, error);
	if (stored_lights == NULL)
		return NULL;

	result = (RyosLightLayer *)g_malloc(sizeof(RyosLightLayer));
	*result = stored_lights->layers[RYOS_STORED_LIGHTS_LAYER_NORMAL];
	
	g_free(stored_lights);
	return result;
}

enum {
	KEYS_NUM = 119,
};

static struct {
	guint8 primary;
	guint8 lights_index;
} hid_to_index_table[KEYS_NUM] = {
	/* 0 keys_macro */
	{0xaa, 0x00},
	{0xab, 0x18},
	{0xac, 0x30},
	{0xad, 0x48},
	{0xae, 0x60},

	/* 5 keys_function */
	{0x29, 0x01},
	{0x3a, 0x02},
	{0x3b, 0x0e},
	{0x3c, 0x03},
	{0x3d, 0x0f},
	{0x3e, 0x04},
	{0x3f, 0x10},
	{0x40, 0x05},
	{0x41, 0x11},
	{0x42, 0x06},
	{0x43, 0x07},
	{0x44, 0x08},
	{0x45, 0x14},
	{0x46, 0x09},
	{0x47, 0x15},
	{0x48, 0x21},

	/* 21 keys_thumbster */
	{RYOS_LIGHT_LAYER_INDEX_INVALID, 0x78},
	{RYOS_LIGHT_LAYER_INDEX_INVALID, 0x79},
	{RYOS_LIGHT_LAYER_INDEX_INVALID, 0x7a},

	/* 24 keys_main */
	{0x35, 0x0c},
	{0x1e, 0x0d},
	{0x1f, 0x19},
	{0x20, 0x1a},
	{0x21, 0x1b},
	{0x22, 0x27},
	{0x23, 0x1c},
	{0x24, 0x28},
	{0x25, 0x1d},
	{0x26, 0x29},
	{0x27, 0x12},
	{0x2d, 0x1e},
	{0x2e, 0x13},
	{0x89, 0x20},
	{0x2a, 0x2c},
	{0x2b, 0x24},
	{0x14, 0x25},
	{0x1a, 0x31},
	{0x08, 0x26},
	{0x15, 0x32},
	{0x17, 0x33},
	{0x1c, 0x34},
	{0x18, 0x40},
	{0x0c, 0x35},
	{0x12, 0x2a},
	{0x13, 0x36},
	{0x2f, 0x1f},
	{0x30, 0x2b},
	{0x31, 0x38},
	{0x39, 0x3c},
	{0x04, 0x3d},
	{0x16, 0x49},
	{0x07, 0x3e},
	{0x09, 0x4a},
	{0x0a, 0x3f},
	{0x0b, 0x4b},
	{0x0d, 0x4c},
	{0x0e, 0x41},
	{0x0f, 0x42},
	{0x33, 0x4e},
	{0x34, 0x37},
	{0x32, 0x43},
	{0x28, 0x44},
	{0xe1, 0x54},
	{0x64, 0x55},
	{0x1d, 0x61},
	{0x1b, 0x56},
	{0x06, 0x62},
	{0x19, 0x57},
	{0x05, 0x63},
	{0x11, 0x58},
	{0x10, 0x4d},
	{0x36, 0x59},
	{0x37, 0x5a},
	{0x38, 0x4f},
	{0x87, 0x5b},
	{0xe5, 0x50},
	{0xe0, 0x6c},
	{0xe3, 0x6d},
	{0xe2, 0x6e},
	{0x8b, 0x64},
	{0x2c, 0x70},
	{0x8a, 0x65},
	{0x88, 0x71},
	{0xe6, 0x72},
	{0xf1, 0x73}, /* FN key */
	{0x65, 0x67},
	{0xe4, 0x5c},
	{0x49, 0x2d},
	{0x4a, 0x39},
	{0x4b, 0x45},
	{0x4c, 0x51},
	{0x4d, 0x5d},
	{0x4e, 0x69},

	/* 98 keys_arrow */
	{0x52, 0x75},
	{0x50, 0x68},
	{0x51, 0x74},
	{0x4f, 0x76},

	/* 102 num_block */
	{0x53, 0x0a},
	{0x54, 0x16},
	{0x55, 0x0b},
	{0x56, 0x17},
	{0x5f, 0x22},
	{0x60, 0x2e},
	{0x61, 0x23},
	{0x57, 0x2f},
	{0x5c, 0x3a},
	{0x5d, 0x46},
	{0x5e, 0x3b},
	{0x59, 0x52},
	{0x5a, 0x5e},
	{0x5b, 0x53},
	{0x58, 0x5f},
	{0x62, 0x6a},
	{0x63, 0x6b},
};

guint8 ryos_hid_to_light_layer_index(guint8 hid) {
	guint i;

	for (i = 0; i < KEYS_NUM; ++i)
		if (hid_to_index_table[i].primary == hid)
			return hid_to_index_table[i].lights_index;

	return RYOS_LIGHT_LAYER_INDEX_INVALID;
}
