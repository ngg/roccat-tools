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

#include <gaminggear/hid_uid.h>
#include <gaminggear/input_events.h>
#include "roccat.h"
#include "pyra.h"
#include "roccat_process_helper.h"
#include "roccat_helper.h"
#include "g_roccat_helper.h"
#include <unistd.h>
#include <errno.h>

gboolean pyra_play_shortcut_press(PyraRMPMacroKeyInfo const *macro_key_info) {
	PyraRMPKeystroke const *keystroke;
	gint hids[5];
	gsize length;

	if (macro_key_info->type != PYRA_RMP_MACRO_KEY_INFO_TYPE_SHORTCUT)
		return FALSE;

	keystroke = &macro_key_info->keystrokes[0];
	length = 0;

	if (roccat_get_bit8(keystroke->action, PYRA_RMP_KEYSTROKE_SHORTCUT_ACTION_SHIFT))
		hids[length++] = HID_UID_KB_LEFT_SHIFT;

	if (roccat_get_bit8(keystroke->action, PYRA_RMP_KEYSTROKE_SHORTCUT_ACTION_CTRL))
		hids[length++] = HID_UID_KB_LEFT_CONTROL;

	if (roccat_get_bit8(keystroke->action, PYRA_RMP_KEYSTROKE_SHORTCUT_ACTION_ALT))
		hids[length++] = HID_UID_KB_LEFT_ALT;

	if (roccat_get_bit8(keystroke->action, PYRA_RMP_KEYSTROKE_SHORTCUT_ACTION_WIN))
		hids[length++] = HID_UID_KB_LEFT_GUI;

	hids[length++] = keystroke->key;

	gaminggear_input_event_write_keyboard_multi(hids, length, GAMINGGEAR_INPUT_EVENT_VALUE_PRESS);

	return TRUE;
}

gboolean pyra_play_shortcut_release(PyraRMPMacroKeyInfo const *macro_key_info) {
	PyraRMPKeystroke const *keystroke;
	gint hids[5];
	gsize length;

	if (macro_key_info->type != PYRA_RMP_MACRO_KEY_INFO_TYPE_SHORTCUT)
		return FALSE;

	keystroke = &macro_key_info->keystrokes[0];
	length = 0;

	hids[length++] = keystroke->key;

	if (roccat_get_bit8(keystroke->action, PYRA_RMP_KEYSTROKE_SHORTCUT_ACTION_WIN))
		hids[length++] = HID_UID_KB_LEFT_GUI;

	if (roccat_get_bit8(keystroke->action, PYRA_RMP_KEYSTROKE_SHORTCUT_ACTION_ALT))
		hids[length++] = HID_UID_KB_LEFT_ALT;

	if (roccat_get_bit8(keystroke->action, PYRA_RMP_KEYSTROKE_SHORTCUT_ACTION_CTRL))
		hids[length++] = HID_UID_KB_LEFT_CONTROL;

	if (roccat_get_bit8(keystroke->action, PYRA_RMP_KEYSTROKE_SHORTCUT_ACTION_SHIFT))
		hids[length++] = HID_UID_KB_LEFT_SHIFT;

	gaminggear_input_event_write_keyboard_multi(hids, length, GAMINGGEAR_INPUT_EVENT_VALUE_RELEASE);

	return TRUE;
}

gboolean pyra_play_quicklaunch_forked(PyraRMPMacroKeyInfo const *macro_key_info) {
	if (macro_key_info->type != PYRA_RMP_MACRO_KEY_INFO_TYPE_QUICKLAUNCH)
		return FALSE;
	return roccat_double_fork((gchar const *)macro_key_info->filename);
}
