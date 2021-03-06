/*
 * This file is part of roccat-tools.
 *
 * roccat-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Buttons Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * roccat-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Buttons Public License for more details.
 *
 * You should have received a copy of the GNU Buttons Public License
 * along with roccat-tools. If not, see <http://www.gnu.org/licenses/>.
 */

#include "savu.h"
#include "roccat_helper.h"

void savu_button_set_to_shortcut(SavuButton *button, guint hid_usage_id, guint8 modifier) {
	button->type = SAVU_BUTTON_TYPE_SHORTCUT;
	button->modifier = modifier;
	button->key = hid_usage_id;
}

void savu_button_set_to_normal(SavuButton *button, guint type) {
	button->type = type;
	button->modifier = 0;
	button->key = 0;
}

guint8 savu_button_build_shortcut_modifier(gboolean ctrl, gboolean shift, gboolean alt, gboolean win) {
	guint8 result = 0;

	roccat_set_bit8(&result, SAVU_BUTTON_MODIFIER_BIT_CTRL, ctrl);
	roccat_set_bit8(&result, SAVU_BUTTON_MODIFIER_BIT_SHIFT, shift);
	roccat_set_bit8(&result, SAVU_BUTTON_MODIFIER_BIT_ALT, alt);
	roccat_set_bit8(&result, SAVU_BUTTON_MODIFIER_BIT_WIN, win);

	return result;
}
