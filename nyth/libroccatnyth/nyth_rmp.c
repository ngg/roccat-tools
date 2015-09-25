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

#include "nyth_rmp.h"
#include "nyth_device.h"
#include "roccat_helper.h"
#include "g_roccat_helper.h"

typedef struct _NythRmpHeader NythRmpHeader;
typedef struct _NythRmpWindowsSettings NythRmpWindowsSettings;
typedef struct _NythRmpMacro NythRmpMacro;
typedef struct _NythRmpTimer NythRmpTimer;

enum {
	NYTH_RMP_PROFILE_NAME_LENGTH = 12, /* utf16 */
	NYTH_RMP_GAMEFILE_LENGTH = 256, /* utf16 */
};

struct _NythRmpHeader {
	guint16 profile_name[NYTH_RMP_PROFILE_NAME_LENGTH];
	guint8 unknown1[80];
	guint8 unknown2;
	guint8 unknown3;
	guint8 profile_autoswitch;
	guint8 unknown4[5];
} __attribute__ ((packed));

static void nyth_rmp_header_set_profile_name(NythRmpHeader *header, gchar const *name) {
	gunichar2 *string;
	glong items;
	string = g_utf8_to_utf16(name, -1L, NULL, &items, NULL);
	items = MIN(NYTH_RMP_PROFILE_NAME_LENGTH, items); /* does not need to be zero terminated */
	memcpy(&header->profile_name, string, items * sizeof(guint16));
	g_free(string);
}

static gchar *nyth_rmp_header_get_profile_name(NythRmpHeader const *header) {
	return g_utf16_to_utf8((gunichar2 const *)(&header->profile_name), NYTH_RMP_PROFILE_NAME_LENGTH, NULL, NULL, NULL);
}

struct _NythRmpWindowsSettings {
	guint8 unknown1;
	guint8 enable_mouse_acceleration;
	guint8 display_pointer_trails;
	guint8 vertical_scroll_page;
	guint8 vertical_scroll_speed;
	guint8 horizontal_tilt_speed;
	guint8 double_click_speed;
	guint8 windows_pointer_speed;
	guint8 unknown2;
	guint8 unknown3;
	guint8 unknown4;
} __attribute__ ((packed));

struct _NythRmpMacro {
	guint8 report_id;
	guint16 size;
	NythMacro macro;
	guint16 checksum;
} __attribute__ ((packed));

static void nyth_rmp_macro_set_size(NythRmpMacro *macro, guint16 value) {
	macro->size = GUINT16_TO_LE(value);
}

struct _NythRmpTimer {
	guint32 unknown;
	guint8 button_index;
	guint8 unused[6];
} __attribute__ ((packed));

static guint32 nyth_rmp_timer_get_unknown(NythRmpTimer const *timer) {
	return GUINT32_FROM_LE(timer->unknown);
}

static guint32 read_bigendian32(gconstpointer *data) {
	guint32 result = GUINT32_FROM_BE(*(guint32 const *)*data);
	*data += sizeof(guint32);
	return result;
}

static gchar *read_bigendian_string(gconstpointer *data) {
	guint32 size;
	gchar *utf8;

	size = read_bigendian32(data);
	if (size % 2) {
		g_warning("%u 2", size);
		return NULL;
	}
	utf8 = roccat_be_utf16_to_utf8((gunichar2 const *)*data, size / 2, NULL, NULL, NULL);
	*data += size;
	return utf8;
}

static gboolean read_header(NythProfileData *profile_data, gconstpointer *data) {
	guint32 size;
	NythRmpHeader const *header;
	gchar *string;
	
	size = read_bigendian32(data);
	if (size != sizeof(NythRmpHeader)) {
		g_warning("%u != sizeof(NythRmpHeader)", size);
		return FALSE;
	}
	
	header = (NythRmpHeader const *)*data;

	string = nyth_rmp_header_get_profile_name(header);
	nyth_profile_data_eventhandler_set_profile_name(&profile_data->eventhandler, string);
	g_free(string);
	
	*data += size;
	return TRUE;
}

static gboolean read_gamefiles(NythProfileData *profile_data, gconstpointer *data) {
	guint32 size;
	gchar *gamefile;
	guint i;

	size = read_bigendian32(data);
	if (size != NYTH_GAMEFILE_NUM * NYTH_RMP_GAMEFILE_LENGTH * 2) {
		g_warning("%u != NYTH_GAMEFILE_NUM * NYTH_RMP_GAMEFILE_LENGTH * 2", size);
		return FALSE;
	}

	for (i = 0; i < NYTH_GAMEFILE_NUM; ++i) {
		gamefile = g_utf16_to_utf8((gunichar2 const *)*data, NYTH_RMP_GAMEFILE_LENGTH, NULL, NULL, NULL);
		nyth_profile_data_eventhandler_set_gamefile_name(&profile_data->eventhandler, i, gamefile);
		g_free(gamefile);
		*data += NYTH_RMP_GAMEFILE_LENGTH * 2;
	}

	return TRUE;
}

static gboolean read_sound_feedback(NythProfileData *profile_data, gconstpointer *data) {
	guint32 size;
	guint16 sound_feedback;

	size = read_bigendian32(data);
	if (size != 2) {
		g_warning("%u != sizeof(sound_feedback)", size);
		return FALSE;
	}

	sound_feedback = *(guint16 const *)data;
	// TODO unused

	*data += size;
	return TRUE;
}

static NythInternalTimer const default_timer = { .name = "Dummy", .duration = 15 };

static gboolean read_timer(NythProfileData *profile_data, gconstpointer *data) {
	NythRmpTimer const *rmp_timer;
	guint32 count;
	guint32 button_index;
	guint32 size;
	guint i;
	
	count = read_bigendian32(data);
	if (count > NYTH_PROFILE_BUTTON_NUM) {
		g_warning("%u > NYTH_PROFILE_BUTTON_NUM", count);
		return FALSE;
	}

	for (i = 0; i < count; ++i) {
		button_index = read_bigendian32(data);
		size = read_bigendian32(data);
		if (size != sizeof(NythRmpTimer)) {
			g_warning("%u != sizeof(NythRmpTimer)", size);
			return FALSE;
		}

		rmp_timer = (NythRmpTimer const *)*data;
		g_debug("Timer has value 0x%08x", nyth_rmp_timer_get_unknown(rmp_timer));

		/* Windows version implementation looks broken.
		 * Name and duration are not stored directly which makes rmp not
		 * transferable between systems.
		 * NythRmpTimer.unknown either encodes at least the duration or it's
		 * just a reference inside entry TimerDatas in file "Nyth_Black".
		 * FIXME Importing a dummy timer instead.
		 */
		nyth_profile_data_eventhandler_set_timer(&profile_data->eventhandler, button_index, &default_timer);
		*data += size;
	}

	return TRUE;
}

static gboolean read_unknown(NythProfileData *profile_data, gconstpointer *data) {
	/* keep this in two lines to prevent pointer change problem */
	guint32 size = read_bigendian32(data);
	*data += size;
	return TRUE;
}

static gboolean read_unknown_with_count(NythProfileData *profile_data, gconstpointer *data) {
	guint32 count;
	guint i;

	count = read_bigendian32(data);
	for (i = 0; i < count; ++i)
		read_unknown(profile_data, data);

	return TRUE;
}

static gboolean read_openers(NythProfileData *profile_data, gconstpointer *data) {
	guint32 count;
	guint32 button_index;
	guint i;
	gchar *string;

	count = read_bigendian32(data);
	if (count > NYTH_PROFILE_BUTTON_NUM) {
		g_warning("%u > NYTH_PROFILE_BUTTON_NUM", count);
		return FALSE;
	}

	for (i = 0; i < count; ++i) {
		button_index = read_bigendian32(data);

		string = read_bigendian_string(data);
		nyth_profile_data_eventhandler_set_opener(&profile_data->eventhandler, button_index, string);
		g_free(string);
	}
	return TRUE;
}

static gboolean read_windows_settings(NythProfileData *profile_data, gconstpointer *data) {
	guint32 size;

	size = read_bigendian32(data);
	if (size != sizeof(NythRmpWindowsSettings)) {
		g_warning("%u != sizeof(NythRmpWindowsSettings)", size);
		return FALSE;
	}

	*data += size;
	return TRUE;
}

static gboolean read_profile_buttons(NythProfileData *profile_data, gconstpointer *data) {
	guint32 size;

	size = read_bigendian32(data);
	if (size != sizeof(NythProfileButtons)) {
		g_warning("%u != sizeof(NythProfileButtons)", size);
		return FALSE;
	}

	nyth_profile_data_hardware_set_profile_buttons(&profile_data->hardware, (NythProfileButtons const *)*data);

	*data += size;
	return TRUE;
}

static gboolean read_profile_settings(NythProfileData *profile_data, gconstpointer *data) {
	guint32 size;

	size = read_bigendian32(data);
	if (size != sizeof(NythProfileSettings)) {
		g_warning("%u != sizeof(NythProfileSettings)", size);
		return FALSE;
	}

	nyth_profile_data_hardware_set_profile_settings(&profile_data->hardware, (NythProfileSettings const *)*data);

	*data += size;
	return TRUE;
}

static gboolean read_button_set(NythProfileData *profile_data, gconstpointer *data) {
	guint32 size;

	size = read_bigendian32(data);
	if (size != sizeof(NythButtonSet)) {
		g_warning("%u != sizeof(NythButtonSet)", size);
		return FALSE;
	}

	nyth_profile_data_hardware_set_button_set(&profile_data->hardware, (NythButtonSet const *)*data);

	*data += size;
	return TRUE;
}

static gboolean read_macros(NythProfileData *profile_data, gconstpointer *data) {
	NythRmpMacro const *rmp_macro;
	guint32 count;
	guint32 size;
	guint i;

	count = read_bigendian32(data);
	if (count != NYTH_PROFILE_BUTTON_NUM) {
		g_warning("%u != NYTH_PROFILE_BUTTON_NUM", count);
		return FALSE;
	}
	
	for (i = 0; i < count; ++i) {
		size = read_bigendian32(data);
		if (size != sizeof(NythRmpMacro)) {
			g_warning("%u != sizeof(NythRmpMacro)", size);
			return FALSE;
		}
		
		rmp_macro = (NythRmpMacro const *)*data;
		nyth_profile_data_hardware_set_macro(&profile_data->hardware, i, &rmp_macro->macro);
		*data += size;
	}

	return TRUE;
}

NythProfileData *nyth_rmp_to_profile_data(gconstpointer rmp, gsize length) {
	NythProfileData *profile_data;
	gconstpointer pointer;
	
	profile_data = nyth_profile_data_new();
	pointer = rmp;
	
	if (!read_header(profile_data, &pointer)) goto error;
	if (!read_gamefiles(profile_data, &pointer)) goto error;
	if (!read_sound_feedback(profile_data, &pointer)) goto error;
	if (!read_timer(profile_data, &pointer)) goto error;
	if (!read_unknown_with_count(profile_data, &pointer)) goto error;
	if (!read_unknown(profile_data, &pointer)) goto error;
	if (!read_openers(profile_data, &pointer)) goto error;
	if (!read_windows_settings(profile_data, &pointer)) goto error;
	if (!read_profile_buttons(profile_data, &pointer)) goto error;
	if (!read_profile_settings(profile_data, &pointer)) goto error;
	if (!read_button_set(profile_data, &pointer)) goto error;
	if (!read_unknown(profile_data, &pointer)) goto error;
	if (!read_macros(profile_data, &pointer)) goto error;

	if (pointer > rmp + length)
		g_warning("%lu > %lu", pointer - rmp, length);

	return profile_data;
error:
	g_free(profile_data);
	return NULL;
}

static void write_bigendian32(gpointer *data, gsize *length, guint32 value) {
	guint32 *start = (guint32 *)roccat_realloc(data, length, sizeof(guint32));
	*start = GUINT32_TO_BE(value);
}

static gpointer write_with_size(gpointer *data, gsize *length, guint32 size) {
	write_bigendian32(data, length, size);
	return roccat_realloc(data, length, size);
}

static void write_header(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	NythRmpHeader *header;
	
	header = (NythRmpHeader *)write_with_size(data, length, sizeof(NythRmpHeader));
	nyth_rmp_header_set_profile_name(header, profile_data->eventhandler.profile_name);
	/* default values */
	header->unknown2 = 50;
	header->profile_autoswitch = 0;

}

static void write_windows_settings(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	NythRmpWindowsSettings *settings;

	settings = (NythRmpWindowsSettings *)write_with_size(data, length, sizeof(NythRmpWindowsSettings));
	/* default values */
	settings->enable_mouse_acceleration = 0;
	settings->display_pointer_trails = 0;
	settings->vertical_scroll_page = 0;
	settings->vertical_scroll_speed = 3;
	settings->horizontal_tilt_speed = 3;
	settings->double_click_speed = 7;
	settings->windows_pointer_speed = 6;
	settings->unknown2 = 6;
	settings->unknown3 = 6;
	settings->unknown4 = 6;
}

static void write_profile_settings(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	NythProfileSettings *settings;
	
	settings = (NythProfileSettings *)write_with_size(data, length, sizeof(NythProfileSettings));
	nyth_profile_settings_copy(settings, &profile_data->hardware.profile_settings);
}

static void write_profile_buttons(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	NythProfileButtons *buttons;
	
	buttons = (NythProfileButtons *)write_with_size(data, length, sizeof(NythProfileButtons));
	nyth_profile_buttons_copy(buttons, &profile_data->hardware.profile_buttons);
}

static void write_button_set(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	NythButtonSet *button_set;
	
	button_set = (NythButtonSet *)write_with_size(data, length, sizeof(NythButtonSet));
	nyth_button_set_copy(button_set, &profile_data->hardware.button_set);
}

static void write_unknown(gpointer *data, gsize *length, gsize size) {
	(void)write_with_size(data, length, size);
}

static void write_sound_feedback(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	guint16 *sound_feedback;

	sound_feedback = (guint16 *)write_with_size(data, length, sizeof(guint16));
	*sound_feedback = 0; /* default value */
}

static void write_gamefiles(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	guint16 *pointer;
	gunichar2 *gamefile;
	guint i;
	gsize size;
	
	size = NYTH_GAMEFILE_NUM * NYTH_RMP_GAMEFILE_LENGTH * sizeof(guint16);
	pointer = (guint16 *)write_with_size(data, length, size);
	for (i = 0; i < NYTH_GAMEFILE_NUM; ++i) {
		gamefile = g_utf8_to_utf16(profile_data->eventhandler.gamefile_names[i], NYTH_GAMEFILE_LENGTH, NULL, NULL, NULL);
		memcpy(pointer, gamefile, NYTH_RMP_GAMEFILE_LENGTH * sizeof(guint16));
		g_free(gamefile);
		pointer += NYTH_RMP_GAMEFILE_LENGTH;
	}
}

static void write_timer(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	NythRmpTimer *rmp_timer;
	guint count;
	guint i;
	
	count = 0;
	for (i = 0; i < NYTH_PROFILE_BUTTON_NUM; ++i)
		if (profile_data->hardware.profile_buttons.buttons[i].type == NYTH_BUTTON_TYPE_TIMER)
			++count;
	write_bigendian32(data, length, count);

	for (i = 0; i < NYTH_PROFILE_BUTTON_NUM; ++i) {
		if (profile_data->hardware.profile_buttons.buttons[i].type == NYTH_BUTTON_TYPE_TIMER) {
			write_bigendian32(data, length, i);
			rmp_timer = (NythRmpTimer *)write_with_size(data, length, sizeof(NythRmpTimer));
			/* FIXME I don't know what to export in NythRmpTimer.unknown
			 * Just don't export timers.
			 */
			rmp_timer->button_index = i;
		}
	}
}

static void write_macros(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	NythRmpMacro *rmp_macro;
	guint i;
	
	/* The Windows version doesn't support the compressed format with button_index
	 * for macro section (nor button_index contained in macro structure).
	 * Always all macros are written, regardless of usage which increases the filesize massively.
	 */
	write_bigendian32(data, length, NYTH_PROFILE_BUTTON_NUM);
	for (i = 0; i < NYTH_PROFILE_BUTTON_NUM; ++i) {
		rmp_macro = (NythRmpMacro *)write_with_size(data, length, sizeof(NythRmpMacro));

		/* Windows version writes the wrong report id 0x0e used by Isku* and Ryos* */
		rmp_macro->report_id = NYTH_REPORT_ID_MACRO;

		nyth_rmp_macro_set_size(rmp_macro, sizeof(NythRmpMacro));
		memcpy(&rmp_macro->macro, &profile_data->hardware.macros[i], sizeof(NythMacro));
	}
}

static void write_openers(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	guint type;
	guint count;
	guint i;
	gunichar2 *be_utf16;
	glong items_written;
	gpointer pointer;
	
	count = 0;
	for (i = 0; i < NYTH_PROFILE_BUTTON_NUM; ++i) {
		type = profile_data->hardware.profile_buttons.buttons[i].type;
		if (type == NYTH_BUTTON_TYPE_OPEN_APPLICATION || type == NYTH_BUTTON_TYPE_OPEN_DOCUMENT || type == NYTH_BUTTON_TYPE_OPEN_FOLDER || type == NYTH_BUTTON_TYPE_OPEN_WEBSITE) {
			++count;
		}
	}
	write_bigendian32(data, length, count);

	for (i = 0; i < NYTH_PROFILE_BUTTON_NUM; ++i) {
		type = profile_data->hardware.profile_buttons.buttons[i].type;
		if (type == NYTH_BUTTON_TYPE_OPEN_APPLICATION || type == NYTH_BUTTON_TYPE_OPEN_DOCUMENT || type == NYTH_BUTTON_TYPE_OPEN_FOLDER || type == NYTH_BUTTON_TYPE_OPEN_WEBSITE) {
			write_bigendian32(data, length, i);
			be_utf16 = roccat_utf8_to_be_utf16((gchar const *)profile_data->eventhandler.openers[i], NYTH_GAMEFILE_LENGTH, NULL, &items_written, NULL);
			pointer = write_with_size(data, length, items_written * sizeof(guint16));
			memcpy(pointer, be_utf16, items_written * sizeof(guint16));
			g_free(be_utf16);
		}
	}
}

static void write_unknown_with_count(NythProfileData const *profile_data, gpointer *data, gsize *length) {
	guint8 *pointer;
	write_bigendian32(data, length, 1);
	pointer = (guint8 *)write_with_size(data, length, 4);
	pointer[0] = 7;
	pointer[1] = 0;
	pointer[2] = 0;
	pointer[3] = 0;
}

gpointer nyth_profile_data_to_rmp(NythProfileData const *profile_data, gsize *length) {
	gpointer rmp = NULL;
	gsize local_length = 0;
	
	write_header(profile_data, &rmp, &local_length);
	write_gamefiles(profile_data, &rmp, &local_length);
	write_sound_feedback(profile_data, &rmp, &local_length);
	write_timer(profile_data, &rmp, &local_length);
	write_unknown_with_count(profile_data, &rmp, &local_length);
	write_unknown(&rmp, &local_length, 0);
	write_openers(profile_data, &rmp, &local_length);
	write_windows_settings(profile_data, &rmp, &local_length);
	write_profile_buttons(profile_data, &rmp, &local_length);
	write_profile_settings(profile_data, &rmp, &local_length);
	write_button_set(profile_data, &rmp, &local_length);
	write_unknown(&rmp, &local_length, 48);
	write_macros(profile_data, &rmp, &local_length);
	
	*length = local_length;
	return rmp;
}

gpointer nyth_rmp_read_with_path(gchar const *path, gsize *length, GError **error) {
	guint8 *rmp;
	gboolean result;
	result = g_file_get_contents(path, (gchar **)&rmp, length, error);
	if (!result)
		return NULL;
	return rmp;
}

gboolean nyth_rmp_write_with_path(gchar const *path, gconstpointer rmp, gsize length, GError **error) {
	return roccat_profile_write_with_path(path, (gchar const *)rmp, length, error);
}
