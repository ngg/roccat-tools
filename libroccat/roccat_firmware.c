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

#include "roccat.h"
#include "roccat_firmware.h"
#include "roccat_device_hidraw.h"
#include "roccat_control.h"
#include "i18n-lib.h"
#include <string.h>
#include <errno.h>

typedef struct _RoccatFirmwareWrite RoccatFirmwareWrite;

struct _RoccatFirmwareWrite {
	guint8 report_id;
	guint8 number;
	guint8 data[ROCCAT_FIRMWARE_CHUNK_SIZE];
} __attribute__ ((packed));

enum {
	ROCCAT_REPORT_ID_FIRMWARE_WRITE = 0x1b,
	ROCCAT_REPORT_ID_FIRMWARE_WRITE_CONTROL = 0x1c,
};

typedef enum {
	ROCCAT_FIRMWARE_WRITE_TYPE_0 = 0x00,
	ROCCAT_FIRMWARE_WRITE_TYPE_1 = 0x01,
	ROCCAT_FIRMWARE_WRITE_TYPE_2 = 0x02,
	ROCCAT_FIRMWARE_WRITE_TYPE_3 = 0x03,
	ROCCAT_FIRMWARE_WRITE_TYPE_4 = 0x04,
	ROCCAT_FIRMWARE_WRITE_TYPE_F = 0x0f,
} RoccatFirmwareWriteType;

struct _RoccatFirmwareState {
	RoccatDevice *device;
	guint update_state;
	guint chunk_number;
	gboolean chunk_init;
	guint firmware_size;
	guint firmware_chunks;
	guint8 *firmware;
	guint wait_after_0;
	guint wait_after_1;
	guint wait_after_2;
	guint wait_after_34f;
	guint8 number;
};

static guint16 calc_chunk_bytesum(guint8 const *data) {
	guint i;
	guint16 result = 0;

	for (i = 0; i < ROCCAT_FIRMWARE_CHUNK_SIZE; ++i)
		result += data[i];

	return result;
}

static gboolean roccat_firmware_write(RoccatDevice *roccat_device, RoccatFirmwareWrite const *buffer,
		guint check_wait, GError **error) {
	gboolean retval;

	retval = roccat_device_hidraw_write(roccat_device, (gchar const *)buffer, sizeof(RoccatFirmwareWrite), error);
	if (!retval)
		return FALSE;

	return roccat_check_write(roccat_device, ROCCAT_REPORT_ID_FIRMWARE_WRITE_CONTROL, check_wait, 110, error);
}

static void report_init(RoccatFirmwareWrite *report, guint8 number) {
	report->report_id = ROCCAT_REPORT_ID_FIRMWARE_WRITE;
	report->number = number;
	memset(&report->data, 0, ROCCAT_FIRMWARE_CHUNK_SIZE);
}

static void report_store16(RoccatFirmwareWrite *report, guint index, guint16 value) {
	value = GUINT16_TO_LE(value);
	report->data[index] = (guint8)value;
	report->data[index + 1] = (guint8)(value >> 8);
}

static void report_store32(RoccatFirmwareWrite *report, guint index, guint32 value) {
	value = GUINT32_TO_LE(value);
	report->data[index] = (guint8)value;
	report->data[index + 1] = (guint8)(value >> 8);
	report->data[index + 2] = (guint8)(value >> 16);
	report->data[index + 3] = (guint8)(value >> 24);
}

static gboolean roccat_firmware_write_0(RoccatFirmwareState const *state, GError **error) {
	RoccatFirmwareWrite report;

	report_init(&report, state->number | ROCCAT_FIRMWARE_WRITE_TYPE_0);
	report_store32(&report, 2, 0x1032AC08);
	report_store32(&report, 6, 0x0100CB03);

	return roccat_firmware_write(state->device, &report, state->wait_after_0, error);
}

static gboolean roccat_firmware_write_1(RoccatFirmwareState const *state, GError **error) {
	RoccatFirmwareWrite report;

	report_init(&report, state->number | ROCCAT_FIRMWARE_WRITE_TYPE_1);
	report_store32(&report, 2, state->firmware_size);
	report_store32(&report, 6, 0x8822AC08);

	return roccat_firmware_write(state->device, &report, state->wait_after_1, error);
}

static gboolean roccat_firmware_write_2(RoccatFirmwareState const *state, GError **error) {
	RoccatFirmwareWrite report;

	report_init(&report, state->number | ROCCAT_FIRMWARE_WRITE_TYPE_2);
	report.data[2] = 0xEC;

	/* Play it save with increased waiting time
	 * At least Savu might bail out with 110
	 */
	return roccat_firmware_write(state->device, &report, state->wait_after_2, error);
}

static gboolean roccat_firmware_write_3(RoccatFirmwareState const *state, GError **error) {
	RoccatFirmwareWrite report;
	guint16 bytesum;
	guint8 const *chunk;

	chunk = state->firmware + state->chunk_number * ROCCAT_FIRMWARE_CHUNK_SIZE;
	bytesum = calc_chunk_bytesum(chunk);

	report_init(&report, state->number | ROCCAT_FIRMWARE_WRITE_TYPE_3);
	report_store16(&report, 2, state->chunk_number);
	report_store16(&report, 4, bytesum);
	report_store32(&report, 6, 0x8822AC08);

	return roccat_firmware_write(state->device, &report, state->wait_after_34f, error);
}

static gboolean roccat_firmware_write_4(RoccatFirmwareState const *state, GError **error) {
	RoccatFirmwareWrite report;
	guint8 const *chunk;

	chunk = state->firmware + state->chunk_number * ROCCAT_FIRMWARE_CHUNK_SIZE;
	
	report_init(&report, state->number | ROCCAT_FIRMWARE_WRITE_TYPE_4);
	memcpy(&report.data, chunk, ROCCAT_FIRMWARE_CHUNK_SIZE);

	return roccat_firmware_write(state->device, &report, state->wait_after_34f, error);
}

static gboolean roccat_firmware_write_f(RoccatFirmwareState const *state, GError **error) {
	RoccatFirmwareWrite report;

	report_init(&report, state->number | ROCCAT_FIRMWARE_WRITE_TYPE_F);

	return roccat_firmware_write(state->device, &report, state->wait_after_34f, error);
}

guint roccat_firmware_fractions(RoccatFirmwareState const *state) {
	return state->firmware_chunks * 2 + 4;
}

void roccat_firmware_state_reset(RoccatFirmwareState *state) {
	state->update_state = 0;
}

void roccat_firmware_state_set_wait_0(RoccatFirmwareState *state, guint wait) {
	state->wait_after_0 = wait;
}

void roccat_firmware_state_set_wait_1(RoccatFirmwareState *state, guint wait) {
	state->wait_after_1 = wait;
}

void roccat_firmware_state_set_wait_2(RoccatFirmwareState *state, guint wait) {
	state->wait_after_2 = wait;
}

void roccat_firmware_state_set_wait_34f(RoccatFirmwareState *state, guint wait) {
	state->wait_after_34f = wait;
}

static void set_firmware_size(RoccatFirmwareState *state, guint firmware_size) {
	state->firmware_size = firmware_size;
	state->firmware_chunks = (firmware_size + ROCCAT_FIRMWARE_CHUNK_SIZE - 1) / ROCCAT_FIRMWARE_CHUNK_SIZE;
}

RoccatFirmwareState *roccat_firmware_state_new(RoccatDevice *device, guint firmware_size, guint8 number) {
	RoccatFirmwareState *state;
	state = (RoccatFirmwareState *)g_malloc(sizeof(RoccatFirmwareState));
	state->device = device;
	set_firmware_size(state, firmware_size);
	state->firmware = NULL;
	state->number = number & 0xf0;
	roccat_firmware_state_set_wait_0(state, ROCCAT_FIRMWARE_UPDATE_WAIT_0_DEFAULT);
	roccat_firmware_state_set_wait_1(state, ROCCAT_FIRMWARE_UPDATE_WAIT_1_DEFAULT);
	roccat_firmware_state_set_wait_2(state, ROCCAT_FIRMWARE_UPDATE_WAIT_2_DEFAULT);
	roccat_firmware_state_set_wait_34f(state, ROCCAT_FIRMWARE_UPDATE_WAIT_34f_DEFAULT);
	roccat_firmware_state_reset(state);
	return state;
}

/* loads firmware and reallocs it into @chunks * @chunk_size memory */
gboolean roccat_firmware_state_read_firmware(RoccatFirmwareState *state, gchar const *path, GError **error) {
	gchar *result;
	gsize bytes;
	guint i;

	if (!g_file_get_contents(path, &result, &bytes, error))
		return FALSE;

	if (state->firmware_size == 0) {
			set_firmware_size(state, bytes);
	} else {
		if (bytes != state->firmware_size) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, _("Firmware image has wrong size: %zu bytes instead of %zu"), bytes, (gsize)state->firmware_size);
			return FALSE;
		}
	}

	result = g_realloc(result, ROCCAT_FIRMWARE_CHUNK_SIZE * state->firmware_chunks);

	for (i = state->firmware_size; i < ROCCAT_FIRMWARE_CHUNK_SIZE * state->firmware_chunks; ++i)
		result[i] = 0;

	g_free(state->firmware);
	state->firmware = (guint8 *)result;
	return TRUE;
}

gboolean roccat_firmware_state_tick(RoccatFirmwareState *state, GError **error) {
	GError *local_error = NULL;

	switch (state->update_state) {
	case 0:
		if (!roccat_firmware_write_0(state, error))
			return FALSE;
		++state->update_state;
		break;
	case 1:
		if (!roccat_firmware_write_1(state, error))
			return FALSE;
		++state->update_state;
		break;
	case 2:
		if (!roccat_firmware_write_2(state, error))
			return FALSE;
		++state->update_state;
		state->chunk_number = 0;
		state->chunk_init = 1;
		break;
	case 3:
		if (state->chunk_number < state->firmware_chunks) {
			if (state->chunk_init) {
				if (!roccat_firmware_write_3(state, error))
					return FALSE;
				state->chunk_init = FALSE;
			} else {
				if (!roccat_firmware_write_4(state, error))
					return FALSE;
				++state->chunk_number;
				state->chunk_init = TRUE;
			}
		} else
			++state->update_state;
		break;
	case 4:
		if (!roccat_firmware_write_f(state, &local_error)) {
			if (g_error_matches(local_error, ROCCAT_ERRNO_ERROR, EPIPE) ||
					g_error_matches(local_error, ROCCAT_ERRNO_ERROR, EPROTO)) {
				/* It is normal for e.g. Tyon to reset without even answering to f */
				g_debug(_("Firmware update step 0xf: Catching errno %i: %s"), local_error->code, local_error->message);
				g_clear_error(&local_error);
			} else {
				g_propagate_error(error, local_error);
				return FALSE;
			}
		}
		++state->update_state;
		return FALSE;
		break;
	default:
		return FALSE;
		break;
	}

	return TRUE;
}

void roccat_firmware_state_free(RoccatFirmwareState *state) {
	g_free(state->firmware);
	g_free(state);
}

gchar *roccat_firmware_version_to_string(guint firmware_version) {
	return g_strdup_printf("%i.%02i", firmware_version / 100, firmware_version % 100);
}
