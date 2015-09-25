#ifndef __RYOS_KEY_RELATIONS_H__
#define __RYOS_KEY_RELATIONS_H__

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
#include "ryos_key_combo_box.h"
 
G_BEGIN_DECLS

typedef struct _RyosKeyRelation RyosKeyRelation;
typedef struct _RyosKeyPosition RyosKeyPosition;

enum {
	RYOS_KEY_RELATIONS_NUM = 119,
	RYOS_KEY_RELATIONS_INDEX_INVALID = 0xff,
	RYOS_KEY_POSITIONS_END = 0xff,
};

struct _RyosKeyRelation {
	guint8 primary;
	guint8 talk_index;
	guint8 macro_index; /* values range from 0x00-0x6f */
	guint8 macro_index_easyshift;
	guint8 primary_index; /* extended with thumbster keys */
	guint8 keys_type;
	guint8 keys_index;
	guint8 keys_index_easyshift;
	guint mask;
	guint mask_easyshift;
	guint8 default_normal;
	guint8 default_easyshift;
};

extern RyosKeyRelation const ryos_key_relations[RYOS_KEY_RELATIONS_NUM];

typedef enum {
	RYOS_KEY_RELATION_KEYS_TYPE_NOTHING,
	RYOS_KEY_RELATION_KEYS_TYPE_FN,
	RYOS_KEY_RELATION_KEYS_TYPE_CAPSLOCK,
	RYOS_KEY_RELATION_KEYS_TYPE_MACRO,
	RYOS_KEY_RELATION_KEYS_TYPE_THUMBSTER,
	RYOS_KEY_RELATION_KEYS_TYPE_FUNCTION,
	RYOS_KEY_RELATION_KEYS_TYPE_EASYZONE,
} RyosKeyRelationKeysType;

struct _RyosKeyPosition {
	guint8 relation_index;
	guint8 row;
	guint8 height;
	guint8 column;
	guint8 width;
};

extern RyosKeyPosition const ryos_key_positions_macro[];
extern RyosKeyPosition const ryos_key_positions_thumbster[];
extern RyosKeyPosition const ryos_key_positions_function[];
extern RyosKeyPosition const ryos_key_positions_escape[];
extern RyosKeyPosition const ryos_key_positions_numpad[];
extern RyosKeyPosition const ryos_key_positions_arrows[];
extern RyosKeyPosition const ryos_key_positions_block_above_arrows[];

RyosKeyPosition const *ryos_key_positions_get_main(gchar const *layout);

G_END_DECLS

#endif
