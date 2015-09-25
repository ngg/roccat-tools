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

#include "ryos_keys_easyzone.h"
#include "roccat_helper.h"
#include "ryos_device.h"
#include <string.h>

static guint16 ryos_keys_easyzone_calc_checksum(RyosKeysEasyzone const *keys_easyzone) {
	return ROCCAT_BYTESUM_PARTIALLY(keys_easyzone, RyosKeysEasyzone, report_id, checksum);
}

static void ryos_keys_easyzone_set_checksum(RyosKeysEasyzone *keys_easyzone, guint16 new_value) {
	keys_easyzone->checksum = GUINT16_TO_LE(new_value);
}

static void ryos_keys_easyzone_finalize(RyosKeysEasyzone *keys_easyzone, guint profile_index) {
	keys_easyzone->report_id = RYOS_REPORT_ID_KEYS_EASYZONE;
	keys_easyzone->size = sizeof(RyosKeysEasyzone);
	keys_easyzone->profile_index = profile_index;
	ryos_keys_easyzone_set_checksum(keys_easyzone, ryos_keys_easyzone_calc_checksum(keys_easyzone));
}

gboolean ryos_keys_easyzone_write(RoccatDevice *ryos, guint profile_index, RyosKeysEasyzone *keys_easyzone, GError **error) {
	g_assert(profile_index < RYOS_PROFILE_NUM);
	ryos_keys_easyzone_finalize(keys_easyzone, profile_index);
	return ryos_device_write(ryos, (gchar const *)keys_easyzone, sizeof(RyosKeysEasyzone), error);
}

RyosKeysEasyzone *ryos_keys_easyzone_read(RoccatDevice *ryos, guint profile_index, GError **error) {
	RyosKeysEasyzone *keys_easyzone;

	g_assert(profile_index < RYOS_PROFILE_NUM);

	gaminggear_device_lock(GAMINGGEAR_DEVICE(ryos));

	if (!ryos_select(ryos, profile_index, RYOS_CONTROL_REQUEST_KEYS_EASYZONE, error)) {
		gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));
		return NULL;
	}

	keys_easyzone = (RyosKeysEasyzone *)ryos_device_read(ryos, RYOS_REPORT_ID_KEYS_EASYZONE, sizeof(RyosKeysEasyzone), error);

	gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));

	return keys_easyzone;
}

gboolean ryos_keys_easyzone_equal(RyosKeysEasyzone const *left, RyosKeysEasyzone const *right) {
	gboolean equal;
	equal = ROCCAT_MEMCMP_PARTIALLY(left, right, RyosKeysEasyzone, keys[0], checksum);
	return equal ? FALSE : TRUE;
}

void ryos_keys_easyzone_copy(RyosKeysEasyzone *destination, RyosKeysEasyzone const *source) {
	memcpy(destination, source, sizeof(RyosKeysEasyzone));
}

guint ryos_keys_easyzone_index_to_macro_index(guint easyzone_index) {
	return easyzone_index + 0x10;
}

guint ryos_macro_index_to_keys_easyzone_index(guint macro_index) {
	return macro_index - 0x10;
}

gboolean ryos_macro_index_is_keys_easyzone(guint macro_index) {
	return macro_index >= 0x10 && macro_index <= 0x6f;
}
