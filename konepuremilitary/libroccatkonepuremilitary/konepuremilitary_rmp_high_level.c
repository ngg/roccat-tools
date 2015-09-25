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

#include "konepuremilitary.h"
#include "roccat_helper.h"
#include "koneplus_rmp.h"
#include "roccat.h"
#include "i18n-lib.h"
#include <string.h>

static gchar *konepuremilitary_profile_dir(void) {
	gchar *base = roccat_profile_dir();
	gchar *dir = g_build_path("/", base, "konepuremilitary", NULL);
	g_free(base);
	return dir;
}

static gboolean konepuremilitary_profile_dir_create_if_needed(GError **error) {
	gchar *dir;
	gboolean retval;

	dir = konepuremilitary_profile_dir();
	retval = roccat_profile_dir_create_if_needed(dir, error);
	g_free(dir);
	return retval;
}

static gchar *konepuremilitary_build_config_rmp_path(guint profile_index) {
	gchar *dir = konepuremilitary_profile_dir();
	gchar *filename = g_strdup_printf("actual%i." ROCCAT_MOUSE_PROFILE_EXTENSION, profile_index + 1);
	gchar *path = g_build_path("/", dir, filename, NULL);
	g_free(filename);
	g_free(dir);
	return path;
}

static gboolean koneplus_rmp_profile_buttons_modified(KoneplusRmp const *rmp) {
	guint i;

	for (i = 0; i < KONEPLUS_PROFILE_BUTTON_NUM; ++i) {
		if (rmp->modified_macros[i])
			return TRUE;
	}
	return FALSE;
}

KoneplusRmp *konepuremilitary_rmp_load_actual(guint profile_index) {
	KoneplusRmp *rmp;
	gchar *config_path;

	config_path = konepuremilitary_build_config_rmp_path(profile_index);
	rmp = koneplus_rmp_read_with_path(config_path, konepuremilitary_rmp_defaults(), NULL);
	g_free(config_path);
	if (!rmp) {
		rmp = konepuremilitary_default_rmp();
		rmp->modified_rmp = TRUE;
	}
	return rmp;
}

static void konepure_rmp_update_with_hardware_macro(KoneplusRmp *rmp, RoccatDevice *device, guint profile_index, guint profile_buttons_index) {
	KoneplusMacro *hardware_macro;
	guint rmp_index;

	rmp_index = konepure_profile_button_index_to_rmp_index(profile_buttons_index);
	if (rmp_index == -1)
		return;

	hardware_macro = koneplus_macro_read(device, profile_index, profile_buttons_index, NULL);
	if (!hardware_macro)
		return;

	koneplus_rmp_update_with_macro(rmp, rmp_index, hardware_macro);
	g_free(hardware_macro);
}

void konepuremilitary_rmp_update_from_device(KoneplusRmp *rmp, RoccatDevice *device, guint profile_index) {
	KonepureProfileButtons *hardware_profile_buttons, *rmp_profile_buttons;
	KonepuremilitaryProfileSettings *hardware_profile_settings, *rmp_profile_settings;
	GError *error = NULL;
	guint i;

	gaminggear_device_lock(GAMINGGEAR_DEVICE(device));

	/* hardware takes precedence */
	hardware_profile_settings = konepuremilitary_profile_settings_read(device, profile_index, &error);

	if (hardware_profile_settings) {
		rmp_profile_settings = konepuremilitary_rmp_to_profile_settings(rmp);

		if (!konepuremilitary_profile_settings_equal(hardware_profile_settings, rmp_profile_settings)) {
			g_warning(_("Hardware data and rmp for profile %u differ"), profile_index + 1);
			konepuremilitary_rmp_update_with_profile_settings(rmp, hardware_profile_settings);
		}
		g_free(rmp_profile_settings);
		g_free(hardware_profile_settings);
	} else
		g_clear_error(&error);

	hardware_profile_buttons = konepure_profile_buttons_read(device, profile_index, &error);

	if (hardware_profile_buttons) {
		rmp_profile_buttons = konepure_rmp_to_profile_buttons(rmp);

		if (!konepure_profile_buttons_equal(hardware_profile_buttons, rmp_profile_buttons)) {
			g_warning(_("Hardware data and rmp for profile %u differ"), profile_index + 1);
			konepure_rmp_update_with_profile_buttons(rmp, hardware_profile_buttons);
		}

		g_free(rmp_profile_buttons);

		for (i = 0; i < KONEPURE_PROFILE_BUTTON_NUM; ++i) {
			if (hardware_profile_buttons->buttons[i].type == KONEPLUS_PROFILE_BUTTON_TYPE_MACRO)
				konepure_rmp_update_with_hardware_macro(rmp, device, profile_index, i);
		}

		g_free(hardware_profile_buttons);
	} else
		g_clear_error(&error);

	gaminggear_device_unlock(GAMINGGEAR_DEVICE(device));
}

KoneplusRmp *konepuremilitary_rmp_load(RoccatDevice *device, guint profile_index, GError **error) {
	KoneplusRmp *rmp;

	g_assert(profile_index < KONEPLUS_PROFILE_NUM);

	rmp = konepuremilitary_rmp_load_actual(profile_index);

	konepuremilitary_rmp_update_from_device(rmp, device, profile_index);

	return rmp;
}

gboolean konepuremilitary_rmp_save_actual(KoneplusRmp *rmp, guint profile_index, GError **error) {
	gchar *config_path;
	gboolean retval;

	konepuremilitary_profile_dir_create_if_needed(error);
	if (*error)
		return FALSE;

	config_path = konepuremilitary_build_config_rmp_path(profile_index);
	retval = koneplus_rmp_write_with_path(config_path, rmp, error);
	g_free(config_path);

	return retval;
}

static gboolean konepuremilitary_rmp_save_profile_settings(RoccatDevice *device, KoneplusRmp *rmp, guint profile_index, GError **error) {
	KonepuremilitaryProfileSettings *profile_settings;
	gboolean retval;

	profile_settings = konepuremilitary_rmp_to_profile_settings(rmp);
	retval = konepuremilitary_profile_settings_write(device, profile_index, profile_settings, error);
	g_free(profile_settings);

	return retval;
}

static gboolean konepuremilitary_rmp_save_profile_buttons(RoccatDevice *device, KoneplusRmp *rmp, guint profile_index, GError **error) {
	KonepureProfileButtons *profile_buttons;
	gboolean retval;

	profile_buttons = konepure_rmp_to_profile_buttons(rmp);
	retval = konepure_profile_buttons_write(device, profile_index, profile_buttons, error);
	g_free(profile_buttons);

	return retval;
}

static gboolean konepure_rmp_save_macro(RoccatDevice *device, KoneplusRmp *rmp, guint profile_index, guint key_index, guint rmp_index, GError **error) {
	KoneplusRmpMacroKeyInfo *macro_key_info;
	KoneplusMacro *macro;
	guint retval = TRUE;

	macro_key_info = koneplus_rmp_get_macro_key_info(rmp, rmp_index);
	macro = koneplus_rmp_macro_key_info_to_macro(macro_key_info);
	g_free(macro_key_info);
	if (macro) {
		retval = koneplus_macro_write(device, profile_index, key_index, macro, error);
		g_free(macro);
	}

	return retval;
}

static gboolean konepuremilitary_rmp_save_actual_when_needed(KoneplusRmp *rmp, guint profile_index, GError **error) {
	gboolean retval = TRUE;

	if (koneplus_rmp_get_modified(rmp)) {
		retval = konepuremilitary_rmp_save_actual(rmp, profile_index, error);
		rmp->modified_rmp = FALSE;
	}

	return retval;
}

static gboolean konepuremilitary_rmp_save_profile_settings_when_needed(RoccatDevice *device, KoneplusRmp *rmp, guint profile_index, GError **error) {
	gboolean retval = TRUE;

	if (rmp->modified_settings) {
		retval = konepuremilitary_rmp_save_profile_settings(device, rmp, profile_index, error);
		rmp->modified_settings = FALSE;
	}

	return retval;
}

static gboolean konepure_rmp_save_macro_when_needed(RoccatDevice *device, KoneplusRmp *rmp, guint profile_index, guint key_index, GError **error) {
	gboolean retval = TRUE;
	gint rmp_index = konepure_profile_button_index_to_rmp_index(key_index);

	if (rmp_index < 0) {
		g_set_error(error, ROCCAT_ERROR, ROCCAT_ERROR_INVAL, _("invalid key index"));
		return FALSE;
	}

	if (rmp->modified_macros[rmp_index]) {
		retval = konepure_rmp_save_macro(device, rmp, profile_index, key_index, rmp_index, error);
		rmp->modified_macros[rmp_index] = FALSE;
	}

	return retval;
}

static gboolean konepure_rmp_save_profile_buttons_when_needed(RoccatDevice *device, KoneplusRmp *rmp, guint profile_index, GError **error) {
	gboolean retval = TRUE;
	guint i;

	if (koneplus_rmp_profile_buttons_modified(rmp)) {
		retval = konepuremilitary_rmp_save_profile_buttons(device, rmp, profile_index, error);
		if (!retval)
			return FALSE;

		for (i = 0; i < KONEPURE_PROFILE_BUTTON_NUM; ++i) {
			if (!konepure_rmp_save_macro_when_needed(device, rmp, profile_index, i, error))
				return FALSE;
		}
	}
	return retval;
}

gboolean konepuremilitary_rmp_save(RoccatDevice *device, KoneplusRmp *rmp, guint profile_index, GError **error) {
	gboolean retval = FALSE;

	g_assert(profile_index < KONEPLUS_PROFILE_NUM);

	if (!konepuremilitary_rmp_save_actual_when_needed(rmp, profile_index, error)) return FALSE;

	gaminggear_device_lock(GAMINGGEAR_DEVICE(device));

	if (!konepure_rmp_save_profile_buttons_when_needed(device, rmp, profile_index, error)) goto error;
	if (!konepuremilitary_rmp_save_profile_settings_when_needed(device, rmp, profile_index, error)) goto error;

	retval = TRUE;
error:
	gaminggear_device_unlock(GAMINGGEAR_DEVICE(device));
	return retval;
}

KoneplusRmp *konepuremilitary_rmp_load_save_after_reset(RoccatDevice *device, guint profile_index, GError **error) {
	KoneplusRmp *rmp;

	rmp = konepuremilitary_default_rmp();

	/* Hardware profiles differ in light-color and -effects */
	konepuremilitary_rmp_update_from_device(rmp, device, profile_index);
	koneplus_rmp_set_unmodified(rmp);

	konepuremilitary_rmp_save_actual(rmp, profile_index, error);
	return rmp;
}
