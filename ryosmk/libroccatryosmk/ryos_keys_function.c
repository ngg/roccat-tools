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

#include "ryos_keys_function.h"
#include "roccat_helper.h"
#include "ryos_device.h"
#include <string.h>

static guint16 ryos_keys_function_calc_checksum(RyosKeysFunction const *keys_function) {
	return ROCCAT_BYTESUM_PARTIALLY(keys_function, RyosKeysFunction, report_id, checksum);
}

static void ryos_keys_function_set_checksum(RyosKeysFunction *keys_function, guint16 new_value) {
	keys_function->checksum = GUINT16_TO_LE(new_value);
}

static void ryos_keys_function_finalize(RyosKeysFunction *keys_function, guint profile_index) {
	keys_function->report_id = RYOS_REPORT_ID_KEYS_FUNCTION;
	keys_function->size = sizeof(RyosKeysFunction);
	keys_function->profile_index = profile_index;
	ryos_keys_function_set_checksum(keys_function, ryos_keys_function_calc_checksum(keys_function));
}

gboolean ryos_keys_function_write(RoccatDevice *ryos, guint profile_index, RyosKeysFunction *keys_function, GError **error) {
	g_assert(profile_index < RYOS_PROFILE_NUM);
	ryos_keys_function_finalize(keys_function, profile_index);
	return ryos_device_write(ryos, (gchar const *)keys_function, sizeof(RyosKeysFunction), error);
}

RyosKeysFunction *ryos_keys_function_read(RoccatDevice *ryos, guint profile_index, GError **error) {
	RyosKeysFunction *keys_function;

	g_assert(profile_index < RYOS_PROFILE_NUM);

	gaminggear_device_lock(GAMINGGEAR_DEVICE(ryos));

	if (!ryos_select(ryos, profile_index, RYOS_CONTROL_REQUEST_KEYS_FUNCTION, error)) {
		gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));
		return NULL;
	}

	keys_function = (RyosKeysFunction *)ryos_device_read(ryos, RYOS_REPORT_ID_KEYS_FUNCTION, sizeof(RyosKeysFunction), error);

	gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));

	return keys_function;
}

gboolean ryos_keys_function_equal(RyosKeysFunction const *left, RyosKeysFunction const *right) {
	gboolean equal;
	equal = ROCCAT_MEMCMP_PARTIALLY(left, right, RyosKeysFunction, keys[0], checksum);
	return equal ? FALSE : TRUE;
}

void ryos_keys_function_copy(RyosKeysFunction *destination, RyosKeysFunction const *source) {
	memcpy(destination, source, sizeof(RyosKeysFunction));
}
