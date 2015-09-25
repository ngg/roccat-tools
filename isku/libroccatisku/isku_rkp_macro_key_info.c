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

#include "isku_rkp.h"
#include "roccat.h"
#include "roccat_helper.h"
#include "i18n-lib.h"
#include <string.h>

guint8 isku_rkp_macro_key_info_calc_checksum(IskuRkpMacroKeyInfo const *key_info) {
	return ROCCAT_BYTESUM_PARTIALLY(key_info, IskuRkpMacroKeyInfo, index, checksum);
}

void isku_rkp_macro_key_info_set_checksum(IskuRkpMacroKeyInfo *key_info) {
	key_info->checksum = isku_rkp_macro_key_info_calc_checksum(key_info);
}

static void isku_rkp_macro_key_info_set_macroset_name(IskuRkpMacroKeyInfo *key_info, gchar const *new_name) {
	g_strlcpy((gchar *)(key_info->macroset_name), new_name, ISKU_RKP_MACRO_KEY_INFO_MACROSET_NAME_LENGTH);
}

static void isku_rkp_macro_key_info_set_macro_name(IskuRkpMacroKeyInfo *key_info, gchar const *new_name) {
	g_strlcpy((gchar *)(key_info->macro_name), new_name, ISKU_RKP_MACRO_KEY_INFO_MACRO_NAME_LENGTH);
}

static void isku_rkp_macro_key_info_set_filename(IskuRkpMacroKeyInfo *key_info, gchar const *new_name) {
	g_strlcpy((gchar *)(key_info->filename), new_name, ISKU_RKP_MACRO_KEY_INFO_FILENAME_LENGTH);
}

static void isku_rkp_macro_key_info_set_timer_name(IskuRkpMacroKeyInfo *key_info, gchar const *new_name) {
	g_strlcpy((gchar *)(key_info->timer_name), new_name, ISKU_RKP_MACRO_KEY_INFO_TIMER_NAME_LENGTH);
}

static void gaminggear_macro_keystroke_to_isku_keystroke(GaminggearMacroKeystroke const *from, IskuKeystroke *to) {
	to->key = from->key;
	isku_keystroke_set_period(to, gaminggear_macro_keystroke_get_period(from));

	if (from->action == GAMINGGEAR_MACRO_KEYSTROKE_ACTION_PRESS)
		to->action = ISKU_KEYSTROKE_ACTION_PRESS;
	else
		to->action = ISKU_KEYSTROKE_ACTION_RELEASE;
}

static void isku_keystroke_to_gaminggear_macro_keystroke(IskuKeystroke const *from, GaminggearMacroKeystroke *to) {
	to->key = from->key;
	gaminggear_macro_keystroke_set_period(to, isku_keystroke_get_period(from));

	if (from->action == ISKU_KEYSTROKE_ACTION_PRESS)
		to->action = GAMINGGEAR_MACRO_KEYSTROKE_ACTION_PRESS;
	else
		to->action = GAMINGGEAR_MACRO_KEYSTROKE_ACTION_RELEASE;
}

IskuRkpMacroKeyInfo *isku_rkp_macro_key_info_new(void) {
	return (IskuRkpMacroKeyInfo *)g_malloc0(sizeof(IskuRkpMacroKeyInfo));
}

void isku_rkp_macro_key_info_free(IskuRkpMacroKeyInfo *key_info) {
	g_free(key_info);
}

IskuRkpMacroKeyInfo *isku_macro_to_rkp_macro_key_info(IskuMacro const *macro) {
	IskuRkpMacroKeyInfo *result;
	result = isku_rkp_macro_key_info_new();
	guint i, count;

	result->type = ISKU_KEY_TYPE_MACRO;
	isku_rkp_macro_key_info_set_macroset_name(result, (gchar const *)macro->macroset_name);
	isku_rkp_macro_key_info_set_macro_name(result, (gchar const *)macro->macro_name);
	isku_rkp_macro_key_info_set_loop(result, macro->loop);

	count = isku_macro_get_count(macro);
	count = MIN(count, ISKU_RKP_MACRO_KEY_INFO_KEYSTROKES_NUM);
	isku_rkp_macro_key_info_set_count(result, count);

	for (i = 0; i < count; ++i) {
		result->keystrokes[i] = macro->keystrokes[i];
	}

	return result;
}

IskuRkpMacroKeyInfo *gaminggear_macro_to_isku_rkp_macro_key_info(GaminggearMacro const *gaminggear_macro, GError **error) {
	IskuRkpMacroKeyInfo *result;
	guint i, count;

	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	count = gaminggear_macro_keystrokes_get_count(&gaminggear_macro->keystrokes);
	if (count > ISKU_RKP_MACRO_KEY_INFO_KEYSTROKES_NUM) {
		g_set_error(error, ROCCAT_ERROR, ROCCAT_ERROR_MACRO_TOO_LONG, _("Macro is too long for this device"));
		return NULL;
	}

	result = isku_rkp_macro_key_info_new();
	result->type = ISKU_KEY_TYPE_MACRO;
	isku_rkp_macro_key_info_set_count(result, count);
	isku_rkp_macro_key_info_set_macroset_name(result, gaminggear_macro->macroset);
	isku_rkp_macro_key_info_set_macro_name(result, gaminggear_macro->macro);
	isku_rkp_macro_key_info_set_loop(result, gaminggear_macro->keystrokes.loop);

	for (i = 0; i < count; ++i) {
		gaminggear_macro_keystroke_to_isku_keystroke(&gaminggear_macro->keystrokes.keystrokes[i], &result->keystrokes[i]);
	}

	return result;
}

GaminggearMacro *isku_rkp_macro_key_info_to_gaminggear_macro(IskuRkpMacroKeyInfo const *key_info) {
	GaminggearMacro *gaminggear_macro;
	guint i, count;
	GaminggearMacroKeystroke gaminggear_macro_keystroke;

	if (key_info->type != ISKU_KEY_TYPE_MACRO)
		return NULL;

	gaminggear_macro = gaminggear_macro_new((gchar const *)key_info->macroset_name, (gchar const *)key_info->macro_name, NULL);

	gaminggear_macro->keystrokes.loop = isku_rkp_macro_key_info_get_loop(key_info);

	count = isku_rkp_macro_key_info_get_count(key_info);

	/* truncate to the smallest: source or target */
	count = MIN(count, ISKU_RKP_MACRO_KEY_INFO_KEYSTROKES_NUM);
	count = MIN(count, GAMINGGEAR_MACRO_KEYSTROKES_NUM);

	for (i = 0; i < count; ++i) {
		isku_keystroke_to_gaminggear_macro_keystroke(&key_info->keystrokes[i], &gaminggear_macro_keystroke);
		gaminggear_macro_keystrokes_add(&gaminggear_macro->keystrokes, &gaminggear_macro_keystroke);
	}
	return gaminggear_macro;
}

IskuRkpMacroKeyInfo *isku_rkp_macro_key_info_new_quicklaunch(gchar const *filename) {
	IskuRkpMacroKeyInfo *result = isku_rkp_macro_key_info_new();
	result->type = ISKU_KEY_TYPE_QUICKLAUNCH;
	isku_rkp_macro_key_info_set_filename(result, filename);
	return result;
}

IskuRkpMacroKeyInfo *isku_rkp_macro_key_info_new_special(IskuKeyType type) {
	IskuRkpMacroKeyInfo *result = isku_rkp_macro_key_info_new();
	result->type = type;
	return result;
}

IskuRkpMacroKeyInfo *isku_rkp_macro_key_info_new_timer(gchar const *name, guint seconds) {
	IskuRkpMacroKeyInfo *result = isku_rkp_macro_key_info_new();
	result->type = ISKU_KEY_TYPE_TIMER;
	isku_rkp_macro_key_info_set_timer_name(result, name);
	isku_rkp_macro_key_info_set_timer_length(result, seconds);
	return result;
}

IskuRkpMacroKeyInfo *isku_rkp_macro_key_info_new_talk(IskuKeyType type, guint talk_device) {
	IskuRkpMacroKeyInfo *result = isku_rkp_macro_key_info_new();
	result->type = type;
	isku_rkp_macro_key_info_set_talk_device(result, talk_device);
	return result;

}

guint8 isku_rkp_macro_key_info_build_shortcut_modifier(gboolean ctrl, gboolean shift, gboolean alt, gboolean win) {
	guint8 result = 0;

	roccat_set_bit8(&result, ISKU_KEYSTROKE_SHORTCUT_ACTION_MODIFIER_BIT_CTRL, ctrl);
	roccat_set_bit8(&result, ISKU_KEYSTROKE_SHORTCUT_ACTION_MODIFIER_BIT_SHIFT, shift);
	roccat_set_bit8(&result, ISKU_KEYSTROKE_SHORTCUT_ACTION_MODIFIER_BIT_ALT, alt);
	roccat_set_bit8(&result, ISKU_KEYSTROKE_SHORTCUT_ACTION_MODIFIER_BIT_WIN, win);

	return result;
}

IskuRkpMacroKeyInfo *isku_rkp_macro_key_info_new_shortcut(guint hid_usage_id,
		guint8 modifier) {
	IskuRkpMacroKeyInfo *result = isku_rkp_macro_key_info_new();
	result->type = ISKU_KEY_TYPE_SHORTCUT;
	result->keystrokes[0].key = hid_usage_id;
	isku_rkp_macro_key_info_set_count(result, 1);
	result->keystrokes[0].action = modifier;

	return result;
}

gboolean isku_rkp_macro_key_info_equal(IskuRkpMacroKeyInfo const *left, IskuRkpMacroKeyInfo const *right) {
	gboolean equal;
	equal = ROCCAT_MEMCMP_PARTIALLY(left, right, IskuRkpMacroKeyInfo, type, checksum);
	return equal ? FALSE : TRUE;
}

IskuRkpMacroKeyInfo *isku_rkp_macro_key_info_dup(IskuRkpMacroKeyInfo const *source) {
	IskuRkpMacroKeyInfo *result;
	result = isku_rkp_macro_key_info_new();
	memcpy(result, source, sizeof(IskuRkpMacroKeyInfo));
	return result;
}
