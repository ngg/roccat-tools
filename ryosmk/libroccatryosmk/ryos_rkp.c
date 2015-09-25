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

#include "ryos_rkp.h"
#include "ryos_default_rkp.h"
#include "ryos_rkp_accessors.h"
#include "roccat_helper.h"
#include "roccat.h"
#include "i18n-lib.h"
#include <string.h>

static gchar *ryos_profile_dir(void) {
	gchar *base = roccat_profile_dir();
	gchar *dir = g_build_path("/", base, "ryosmk", NULL);
	g_free(base);
	return dir;
}

static gboolean ryos_profile_dir_create_if_needed(GError **error) {
	gchar *dir;
	gboolean retval;

	dir = ryos_profile_dir();
	retval = roccat_profile_dir_create_if_needed(dir, error);
	g_free(dir);
	return retval;
}

static gchar *ryos_build_config_rkp_path(guint profile_index) {
	gchar *dir = ryos_profile_dir();
	gchar *filename = g_strdup_printf("actual%i." ROCCAT_KEYBOARD_PROFILE_EXTENSION, profile_index);
	gchar *path = g_build_path("/", dir, filename, NULL);
	g_free(filename);
	g_free(dir);
	return path;
}

gboolean ryos_rkp_get_modified(RyosRkp const *rkp) {
	guint i;

	if (rkp->modified_key_mask ||
			rkp->modified_keys_easyzone ||
			rkp->modified_keys_extra ||
			rkp->modified_keys_function ||
			rkp->modified_keys_macro ||
			rkp->modified_keys_primary ||
			rkp->modified_keys_thumbster ||
			rkp->modified_light ||
			rkp->modified_light_macro ||
			rkp->modified_lights_automatic ||
			rkp->modified_lights_manual ||
			rkp->modified_rkp)
		return TRUE;

	for (i = 0; i < RYOS_RKP_KEYS_NUM; ++i)
		if (rkp->modified_macros[i])
			return TRUE;

	return FALSE;
}

static void ryos_rkp_set_modified_state(RyosRkp *rkp, gboolean state) {
	guint i;

	rkp->modified_key_mask = state;
	rkp->modified_keys_easyzone = state;
	rkp->modified_keys_extra = state;
	rkp->modified_keys_function = state;
	rkp->modified_keys_macro = state;
	rkp->modified_keys_primary = state;
	rkp->modified_keys_thumbster = state;
	rkp->modified_light = state;
	rkp->modified_light_macro = state;
	rkp->modified_lights_automatic = state;
	rkp->modified_lights_manual = state;
	rkp->modified_rkp = state;

	for (i = 0; i < RYOS_RKP_KEYS_NUM; ++i)
		rkp->modified_macros[i] = state;
}

void ryos_rkp_set_modified(RyosRkp *rkp) {
	ryos_rkp_set_modified_state(rkp, TRUE);
}

void ryos_rkp_set_unmodified(RyosRkp *rkp) {
	ryos_rkp_set_modified_state(rkp, FALSE);
}

RyosRkp *ryos_rkp_read_with_path(gchar const *path, GError **error) {
	RyosRkp *rkp;
	RyosRkpData *data;
	gsize length;
	gboolean result;

	result = g_file_get_contents(path, (gchar **)&data, &length, error);

	if (!result)
		return NULL;

	if (length != sizeof(RyosRkpData)) {
		g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, _("Data has wrong length, %zu instead of %zu"),
				(gsize)length, (gsize)sizeof(RyosRkpData));
		g_free(data);
		return NULL;
	}

	rkp = ryos_rkp_new();
	memcpy(&rkp->data, data, sizeof(RyosRkpData));
	g_free(data);

	return rkp;
}

gboolean ryos_rkp_write_with_path(gchar const *path, RyosRkp *rkp, GError **error) {
	return roccat_profile_write_with_path(path, (gchar const *)&rkp->data, sizeof(RyosRkpData), error);
}

gboolean ryos_rkp_save_actual(RyosRkp *rkp, guint profile_index, GError **error) {
	gchar *config_path;
	gboolean retval;

	ryos_profile_dir_create_if_needed(error);
	if (*error)
		return FALSE;

	config_path = ryos_build_config_rkp_path(profile_index);
	retval = ryos_rkp_write_with_path(config_path, rkp, error);
	g_free(config_path);
	rkp->modified_rkp = FALSE;

	return retval;
}

static gboolean ryos_rkp_save_macro_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, guint key_index, guint key_type, GError **error) {
	if (rkp->modified_macros[key_index] && key_type == RYOS_KEY_TYPE_MACRO) {
		if (!ryos_macro_write(ryos, profile_index, key_index, &rkp->data.macros[key_index], error))
			return FALSE;
		rkp->modified_macros[key_index] = FALSE;
	} else if (key_type == RYOS_KEY_TYPE_LED_MACRO) {
		/* led macro uses rkp's macro space but not stored in hardware */
		rkp->modified_macros[key_index] = FALSE;
	}
	return TRUE;
}

static gboolean ryos_rkp_save_keys_primary_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	if (rkp->modified_keys_primary) {
		if (!ryos_keys_primary_write(ryos, profile_index, &rkp->data.keys_primary, error))
			return FALSE;
		rkp->modified_keys_primary = FALSE;
	}
	return TRUE;
}

static gboolean ryos_rkp_save_keys_function_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	if (rkp->modified_keys_function) {
		if (!ryos_keys_function_write(ryos, profile_index, &rkp->data.keys_function, error))
			return FALSE;
		rkp->modified_keys_function = FALSE;
	}
	return TRUE;
}

static gboolean ryos_rkp_save_keys_macro_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	guint i;

	for (i = 0; i < RYOS_KEYS_MACRO_NUM * 2; ++i) {
		if (!ryos_rkp_save_macro_when_needed(ryos, rkp, profile_index, ryos_keys_macro_index_to_macro_index(i), rkp->data.keys_macro.keys[i].type, error))
			return FALSE;
	}

	if (rkp->modified_keys_macro) {
		if (!ryos_keys_macro_write(ryos, profile_index, &rkp->data.keys_macro, error))
			return FALSE;
		rkp->modified_keys_macro = FALSE;
	}

	return TRUE;
}

static gboolean ryos_rkp_save_keys_thumbster_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	guint i;

	for (i = 0; i < RYOS_KEYS_THUMBSTER_NUM * 2; ++i) {
		if (!ryos_rkp_save_macro_when_needed(ryos, rkp, profile_index, ryos_keys_thumbster_index_to_macro_index(i), rkp->data.keys_thumbster.keys[i].type, error))
			return FALSE;
	}

	if (rkp->modified_keys_thumbster) {
		if (!ryos_keys_thumbster_write(ryos, profile_index, &rkp->data.keys_thumbster, error))
			return FALSE;
		rkp->modified_keys_thumbster = FALSE;
	}

	return TRUE;
}

static gboolean ryos_rkp_save_keys_extra_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	gboolean retval = TRUE;
	if (rkp->modified_keys_extra) {
		retval = ryos_keys_extra_write(ryos, profile_index, &rkp->data.keys_extra, error);
		if (retval)
			rkp->modified_keys_extra = FALSE;
	}
	return retval;
}

static gboolean ryos_rkp_save_keys_easyzone_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	guint i;

	for (i = 0; i < RYOS_KEYS_EASYZONE_NUM; ++i) {
		if (!ryos_rkp_save_macro_when_needed(ryos, rkp, profile_index, ryos_keys_easyzone_index_to_macro_index(i), rkp->data.keys_easyzone.keys[i].type, error))
			return FALSE;
	}

	if (rkp->modified_keys_easyzone) {
		if (!ryos_keys_easyzone_write(ryos, profile_index, &rkp->data.keys_easyzone, error))
			return FALSE;
		rkp->modified_keys_easyzone = FALSE;
	}

	return TRUE;
}

static gboolean ryos_rkp_save_key_mask_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	gboolean retval = TRUE;
	if (rkp->modified_key_mask) {
		retval = ryos_key_mask_write(ryos, profile_index, &rkp->data.key_mask, error);
		if (retval)
			rkp->modified_key_mask = FALSE;
	}
	return retval;
}

static gboolean ryos_rkp_save_light_macro_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	gboolean retval = TRUE;
	if (rkp->modified_light_macro &&
			gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(ryos)) == USB_DEVICE_ID_ROCCAT_RYOS_MK_PRO) {
		retval = ryos_light_macro_write(ryos, profile_index, &rkp->data.light_macro, error);
		if (retval)
			rkp->modified_light_macro = FALSE;
	}
	return retval;
}

static gboolean ryos_rkp_save_light_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	gboolean retval = TRUE;

	if (rkp->modified_light) {
		retval = ryos_light_write(ryos, profile_index, &rkp->data.light, error);
		if (retval)
			rkp->modified_light = FALSE;
	}

	if (!retval)
		return retval;

	if (rkp->data.light.dimness_type == RYOS_LIGHT_DIMNESS_TYPE_MACRO)
		retval = ryos_rkp_save_light_macro_when_needed(ryos, rkp, profile_index, error);

	return retval;
}

static gboolean ryos_rkp_save_lights_automatic_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	gboolean retval = TRUE;
	if (rkp->modified_lights_automatic) {
		retval = ryos_stored_lights_write(ryos, profile_index, RYOS_ILLUMINATION_MODE_AUTOMATIC, &rkp->data.lights_automatic, error);
		if (retval)
			rkp->modified_lights_automatic = FALSE;
	}
	return retval;
}

static gboolean ryos_rkp_save_lights_manual_when_needed(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	gboolean retval = TRUE;
	if (rkp->modified_lights_manual) {
		retval = ryos_stored_lights_write(ryos, profile_index, RYOS_ILLUMINATION_MODE_MANUAL, &rkp->data.lights_manual, error);
		if (retval)
			rkp->modified_lights_manual = FALSE;
	}
	return retval;
}

gboolean ryos_rkp_save(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	gboolean retval = FALSE;

	g_assert(profile_index < RYOS_PROFILE_NUM);

	if (!ryos_rkp_get_modified(rkp)) return TRUE;

	gaminggear_device_lock(GAMINGGEAR_DEVICE(ryos));

	if (!ryos_rkp_save_keys_primary_when_needed(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_save_keys_function_when_needed(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_save_keys_macro_when_needed(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_save_keys_thumbster_when_needed(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_save_keys_extra_when_needed(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_save_keys_easyzone_when_needed(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_save_key_mask_when_needed(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_save_light_when_needed(ryos, rkp, profile_index, error)) goto error;

	if (gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(ryos)) == USB_DEVICE_ID_ROCCAT_RYOS_MK_PRO) {
		if (!ryos_rkp_save_lights_automatic_when_needed(ryos, rkp, profile_index, error)) goto error;
		if (!ryos_rkp_save_lights_manual_when_needed(ryos, rkp, profile_index, error)) goto error;
		if (!ryos_rkp_save_light_macro_when_needed(ryos, rkp, profile_index, error)) goto error;
	}

	if (!ryos_rkp_save_actual(rkp, profile_index, error)) goto error;

	retval = TRUE;
error:
	gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));
	return retval;
}

RyosRkp *ryos_rkp_load_save_after_reset(RoccatDevice *ryos, guint profile_index, GError **error) {
	RyosRkp *rkp;

	rkp = ryos_rkp_dup(ryos_rkp_default());
	ryos_rkp_update_from_device(ryos, rkp, profile_index, NULL);
	ryos_rkp_set_unmodified(rkp);
	ryos_rkp_save_actual(rkp, profile_index, error);
	return rkp;
}

RyosRkp *ryos_rkp_load_actual(guint profile_index) {
	RyosRkp *rkp;
	gchar *config_path;
	config_path = ryos_build_config_rkp_path(profile_index);
	rkp = ryos_rkp_read_with_path(config_path, NULL);
	g_free(config_path);
	if (!rkp) {
		rkp = ryos_rkp_dup(ryos_rkp_default());
		rkp->modified_rkp = TRUE;
	}
	return rkp;
}

/* Hardware data takes precedence. Difference has to be saved only in rkp */
static void correct_modified(RyosRkp *rkp, gboolean *mod) {
	if (*mod) {
		*mod = FALSE;
		rkp->modified_rkp = TRUE;
	}
}

static gboolean ryos_rkp_update_with_hardware_keys_primary(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosKeysPrimary *hardware_keys_primary = ryos_keys_primary_read(ryos, profile_index, error);
	if (!hardware_keys_primary)
		return FALSE;
	ryos_rkp_set_keys_primary(rkp, hardware_keys_primary);
	correct_modified(rkp, &rkp->modified_keys_primary);
	g_free(hardware_keys_primary);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_keys_function(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosKeysFunction *hardware_keys_function = ryos_keys_function_read(ryos, profile_index, error);
	if (!hardware_keys_function)
		return FALSE;
	ryos_rkp_set_keys_function(rkp, hardware_keys_function);
	correct_modified(rkp, &rkp->modified_keys_function);
	g_free(hardware_keys_function);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_macro(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, guint macro_index, GError **error) {
	RyosMacro *hardware_macro = ryos_macro_read(ryos, profile_index, macro_index, error);
	if (!hardware_macro)
		return FALSE;
	ryos_rkp_set_macro(rkp, macro_index, hardware_macro);
	correct_modified(rkp, &rkp->modified_macros[macro_index]);
	g_free(hardware_macro);
	return TRUE;
}

/* gets macro if needed or resets key to rkp value if data is missing */
static gboolean set_key(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, guint macro_index,
		RyosKey *rkp_key, RyosKey const *hardware_key, gboolean *mod, GError **error) {
	switch (hardware_key->type) {
	case RYOS_KEY_TYPE_QUICKLAUNCH:
	case RYOS_KEY_TYPE_TIMER:
	case RYOS_KEY_TYPE_TALK_EASYSHIFT:
	case RYOS_KEY_TYPE_TALK_EASYSHIFT_LOCK:
	case RYOS_KEY_TYPE_TALK_EASYAIM_1:
	case RYOS_KEY_TYPE_TALK_EASYAIM_2:
	case RYOS_KEY_TYPE_TALK_EASYAIM_3:
	case RYOS_KEY_TYPE_TALK_EASYAIM_4:
	case RYOS_KEY_TYPE_TALK_EASYAIM_5:
	case RYOS_KEY_TYPE_LED_MACRO:
		if (hardware_key->type != rkp_key->type) {
			g_warning(_("Profile %i: Key (0x%02x) differs from rkp (0x%02x) with missing data, so rkp takes precedence"),
					profile_index, hardware_key->type, rkp_key->type);
			*mod = TRUE; /* now hardware differs */
		}
		break;
	case RYOS_KEY_TYPE_MACRO:
		if (!ryos_rkp_update_with_hardware_macro(ryos, rkp, profile_index, macro_index, error))
			return FALSE;
		*rkp_key = *hardware_key;
		break;
	default:
		*rkp_key = *hardware_key;
		break;
	}
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_keys_macro(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosKeysMacro *hardware_keys_macro;
	guint i;

	hardware_keys_macro = ryos_keys_macro_read(ryos, profile_index, error);
	if (!hardware_keys_macro)
		return FALSE;

	for (i = 0; i < RYOS_KEYS_MACRO_NUM * 2; ++i) {
		if (!set_key(ryos, rkp, profile_index,
				ryos_keys_macro_index_to_macro_index(i),
				&rkp->data.keys_macro.keys[i],
				&hardware_keys_macro->keys[i],
				&rkp->modified_keys_macro, error))
			return FALSE;
	}

	g_free(hardware_keys_macro);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_keys_thumbster(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosKeysThumbster *hardware_keys_thumbster;
	guint i;

	hardware_keys_thumbster = ryos_keys_thumbster_read(ryos, profile_index, error);
	if (!hardware_keys_thumbster)
		return FALSE;

	for (i = 0; i < RYOS_KEYS_THUMBSTER_NUM * 2; ++i) {
		if (!set_key(ryos, rkp, profile_index,
				ryos_keys_thumbster_index_to_macro_index(i),
				&rkp->data.keys_thumbster.keys[i],
				&hardware_keys_thumbster->keys[i],
				&rkp->modified_keys_thumbster, error))
			return FALSE;
	}

	g_free(hardware_keys_thumbster);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_keys_extra(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosKeysExtra *hardware_keys_extra = ryos_keys_extra_read(ryos, profile_index, error);
	if (!hardware_keys_extra)
		return FALSE;
	ryos_rkp_set_keys_extra(rkp, hardware_keys_extra);
	correct_modified(rkp, &rkp->modified_keys_extra);
	g_free(hardware_keys_extra);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_keys_easyzone(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosKeysEasyzone *hardware_keys_easyzone;
	guint i;

	hardware_keys_easyzone = ryos_keys_easyzone_read(ryos, profile_index, error);
	if (!hardware_keys_easyzone)
		return FALSE;

	for (i = 0; i < RYOS_KEYS_EASYZONE_NUM; ++i) {
		if (!set_key(ryos, rkp, profile_index,
				ryos_keys_easyzone_index_to_macro_index(i),
				&rkp->data.keys_easyzone.keys[i],
				&hardware_keys_easyzone->keys[i],
				&rkp->modified_keys_easyzone, error))
			return FALSE;
	}

	g_free(hardware_keys_easyzone);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_key_mask(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosKeyMask *hardware_key_mask = ryos_key_mask_read(ryos, profile_index, error);
	if (!hardware_key_mask)
		return FALSE;
	ryos_rkp_set_key_mask(rkp, hardware_key_mask);
	correct_modified(rkp, &rkp->modified_key_mask);
	g_free(hardware_key_mask);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_light_macro(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosMacro *hardware_light_macro;

	if (gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(ryos)) != USB_DEVICE_ID_ROCCAT_RYOS_MK_PRO)
		return TRUE;

	hardware_light_macro = ryos_light_macro_read(ryos, profile_index, error);
	if (!hardware_light_macro)
		return FALSE;

	ryos_rkp_set_light_macro(rkp, hardware_light_macro);
	correct_modified(rkp, &rkp->modified_light_macro);

	g_free(hardware_light_macro);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_light(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosLight *hardware_light = ryos_light_read(ryos, profile_index, error);
	gboolean retval = TRUE;

	if (!hardware_light)
		return FALSE;

	ryos_rkp_set_light(rkp, hardware_light);
	correct_modified(rkp, &rkp->modified_light);

	if (hardware_light->dimness_type == RYOS_LIGHT_DIMNESS_TYPE_MACRO) {
		if (!ryos_rkp_update_with_hardware_light_macro(ryos, rkp, profile_index, error))
			retval = FALSE;
	}

	g_free(hardware_light);
	return retval;
}

static gboolean ryos_rkp_update_with_hardware_lights_automatic(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosStoredLights *hardware_lights;

	if (gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(ryos)) != USB_DEVICE_ID_ROCCAT_RYOS_MK_PRO)
		return TRUE;

	hardware_lights = ryos_stored_lights_read_automatic(ryos, profile_index, error);
	if (!hardware_lights)
		return FALSE;

	ryos_rkp_set_lights_automatic(rkp, hardware_lights);
	correct_modified(rkp, &rkp->modified_lights_automatic);

	g_free(hardware_lights);
	return TRUE;
}

static gboolean ryos_rkp_update_with_hardware_lights_manual(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	RyosStoredLights *hardware_lights;

	if (gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(ryos)) != USB_DEVICE_ID_ROCCAT_RYOS_MK_PRO)
		return TRUE;

	hardware_lights = ryos_stored_lights_read_manual(ryos, profile_index, error);
	if (!hardware_lights)
		return FALSE;

	ryos_rkp_set_lights_manual(rkp, hardware_lights);
	correct_modified(rkp, &rkp->modified_lights_manual);

	g_free(hardware_lights);
	return TRUE;
}

gboolean ryos_rkp_update_from_device(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error) {
	gboolean retval = FALSE;

	gaminggear_device_lock(GAMINGGEAR_DEVICE(ryos));

	if (!ryos_rkp_update_with_hardware_keys_primary(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_keys_function(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_keys_macro(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_keys_thumbster(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_keys_extra(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_keys_easyzone(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_key_mask(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_light(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_lights_automatic(ryos, rkp, profile_index, error)) goto error;
	if (!ryos_rkp_update_with_hardware_lights_manual(ryos, rkp, profile_index, error)) goto error;

	retval = TRUE;
error:
	gaminggear_device_unlock(GAMINGGEAR_DEVICE(ryos));
	return retval;
}

static void ryos_rkp_differ_warning(RyosRkp const *rkp, guint profile_index) {
	if (ryos_rkp_get_modified(rkp))
		g_warning(_("Hardware data and rkp for profile %u differ"), profile_index + 1);
	// TODO expand
}

RyosRkp *ryos_rkp_load(RoccatDevice *ryos, guint profile_index, GError **error) {
	RyosRkp *rkp;

	g_assert(profile_index < RYOS_PROFILE_NUM);

	rkp = ryos_rkp_load_actual(profile_index);
	ryos_rkp_update_from_device(ryos, rkp, profile_index, error);
	ryos_rkp_differ_warning(rkp, profile_index);

	return rkp;
}

RyosRkp *ryos_rkp_new(void) {
	return (RyosRkp *)g_malloc0(sizeof(RyosRkp));
}

void ryos_rkp_free(RyosRkp *rkp) {
	g_free(rkp);
}

RyosRkp *ryos_rkp_dup(RyosRkp const *other) {
	return (RyosRkp *)g_memdup((gconstpointer)other, sizeof(RyosRkp));
}
