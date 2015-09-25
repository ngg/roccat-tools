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

#include "ryos.h"
#include "roccat_helper.h"
#include <string.h>

void ryos_key_set_to_shortcut(RyosKey *key, guint hid_usage_id, guint8 modifier) {
	key->type = RYOS_KEY_TYPE_SHORTCUT;
	key->modifier = modifier;
	key->key = hid_usage_id;
}

void ryos_key_set_to_normal(RyosKey *key, guint type) {
	key->type = type;
	key->modifier = 0;
	key->key = 0;
}

guint8 ryos_key_build_shortcut_modifier(gboolean ctrl, gboolean shift, gboolean alt, gboolean win) {
	guint8 result = 0;

	roccat_set_bit8(&result, RYOS_KEY_MODIFIER_BIT_CTRL, ctrl);
	roccat_set_bit8(&result, RYOS_KEY_MODIFIER_BIT_SHIFT, shift);
	roccat_set_bit8(&result, RYOS_KEY_MODIFIER_BIT_ALT, alt);
	roccat_set_bit8(&result, RYOS_KEY_MODIFIER_BIT_WIN, win);

	return result;
}

gboolean ryos_key_equal(RyosKey const *left, RyosKey const *right) {
	gboolean equal;
	equal = memcmp(left, right, sizeof(RyosKey));
	return equal ? FALSE : TRUE;
}

void ryos_key_copy(RyosKey *destination, RyosKey const *source) {
	memcpy(destination, source, sizeof(RyosKey));
}
