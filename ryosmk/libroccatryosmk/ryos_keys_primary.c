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

#include "ryos_keys_primary.h"
#include "ryos_default_rkp.h"
#include "roccat_helper.h"
#include "ryos_device.h"
#include "i18n-lib.h"
#include <string.h>

static guint16 ryos_keys_primary_calc_checksum(RyosKeysPrimary const *keys_primary) {
	return ROCCAT_BYTESUM_PARTIALLY(keys_primary, RyosKeysPrimary, report_id, checksum);
}

static void ryos_keys_primary_set_checksum(RyosKeysPrimary *keys_primary, guint16 new_value) {
	keys_primary->checksum = GUINT16_TO_LE(new_value);
}

static void ryos_keys_primary_finalize(RyosKeysPrimary *keys_primary, guint profile_index) {
	keys_primary->report_id = RYOS_REPORT_ID_KEYS_PRIMARY;
	keys_primary->size = sizeof(RyosKeysPrimary);
	keys_primary->profile_index = profile_index;
	ryos_keys_primary_set_checksum(keys_primary, ryos_keys_primary_calc_checksum(keys_primary));
}

gboolean ryos_keys_primary_write(RoccatDevice *ryos, guint profile_index, RyosKeysPrimary *keys_primary, GError **error) {
	g_assert(profile_index < RYOS_PROFILE_NUM);
	ryos_keys_primary_finalize(keys_primary, profile_index);
	return ryos_device_write(ryos, (gchar const *)keys_primary, sizeof(RyosKeysPrimary), error);
}

RyosKeysPrimary *ryos_keys_primary_read(RoccatDevice *ryos, guint profile_index, GError **error) {
	RyosKeysPrimary *keys_primary;

	g_assert(profile_index < RYOS_PROFILE_NUM);

	gaminggear_device_lock(GAMINGGEAR_DEVICE(ryos));

	if (!ryos_select(ryos, profile_index, RYOS_CONTROL_REQUEST_KEYS_PRIMARY, error)) {
		gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));
		return NULL;
	}

	keys_primary = (RyosKeysPrimary *)ryos_device_read(ryos, RYOS_REPORT_ID_KEYS_PRIMARY, sizeof(RyosKeysPrimary), error);

	gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));

	return keys_primary;
}

gboolean ryos_keys_primary_equal(RyosKeysPrimary const *left, RyosKeysPrimary const *right) {
	gboolean equal;
	equal = ROCCAT_MEMCMP_PARTIALLY(left, right, RyosKeysPrimary, keys[0], checksum);
	return equal ? FALSE : TRUE;
}

void ryos_keys_primary_copy(RyosKeysPrimary *destination, RyosKeysPrimary const *source) {
	memcpy(destination, source, sizeof(RyosKeysPrimary));
}

static gint get_index(RyosKeysPrimary const *data, guint8 hid) {
	guint i;
	for (i = 0; i < RYOS_KEYS_PRIMARY_NUM; ++i)
		if (data->keys[i] == hid)
			return i;
	return -1;
}

guint8 ryos_keys_primary_correct_remapped(RyosKeysPrimary const *actual_keys, guint8 hid) {
	gint index;

	index = get_index(actual_keys, hid);
	if (index == -1) {
		g_warning(_("ryos_keys_primary_correct_remapped: can't find 0x%02x"), hid);
		return hid;
	}
	return ryos_rkp_default_data_static()->keys_primary.keys[index];
}
