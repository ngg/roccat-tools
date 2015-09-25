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

#include "ryostkl_ripple_c.h"
#include "ryos_custom_lights.h"
#include "ryos_light_control.h"
#include "g_roccat_helper.h"
#include "roccat_helper.h"
#include "i18n-lib.h"
#include <gaminggear/threads.h>

#define RYOSTKL_RIPPLE_C_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOSTKL_RIPPLE_C_TYPE, RyostklRippleCClass))
#define IS_RYOSTKL_RIPPLE_C_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOSTKL_RIPPLE_C_TYPE))
#define RYOSTKL_RIPPLE_C_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOSTKL_RIPPLE_C_TYPE, RyostklRippleCPrivate))

typedef struct _RyostklRippleCClass RyostklRippleCClass;

struct _RyostklRippleCClass {
	GObjectClass parent_class;
};

struct _RyostklRippleCPrivate {
	RoccatDevice *device;
	guint hid;
	GThread *worker;
	GaminggearMutex mutex;
	GaminggearCond start;
	gint changed;
};

enum {
	KEY_WIDTH = 4,
	VOID = 0xff,
	ROWS = 30,
	COLS = 73,
	KEYS_NUM = 104,
};

typedef struct {
	guint8 hid;
	guint8 row;
	guint8 col;
} Position;

/* thumbster keys don't send events */
static Position light_position_table[KEYS_NUM] = {
	/* keys_function */
	{0x3a,  2, 10}, {0x3b,  2, 14}, {0x3c,  2, 18}, {0x3d,  2, 22},
	{0x3e,  2, 28}, {0x3f,  2, 32}, {0x40,  2, 36}, {0x41,  2, 40},
	{0x42,  2, 46}, {0x43,  2, 50}, {0x44,  2, 54}, {0x45,  2, 58},
	{0x46,  2, 63}, {0x47,  2, 67}, {0x48,  2, 71},

	/* keys_easyzone */
	{0x35,  7,  2}, {0x1e,  7,  6}, {0x1f,  7, 10}, {0x20,  7, 14},
	{0x21,  7, 18}, {0x22,  7, 22}, {0x23,  7, 26}, {0x24,  7, 30},
	{0x25,  7, 34}, {0x26,  7, 38}, {0x27,  7, 42}, {0x2d,  7, 46},
	{0x2e,  7, 50}, {0x89,  7, 54}, {0x2a,  7, 58}, {0x49,  7, 63},
	{0x4a,  7, 67}, {0x4b,  7, 71},	{0x2b, 11,  3}, {0x14, 11,  8},
	{0x1a, 11, 12}, {0x08, 11, 16}, {0x15, 11, 20}, {0x17, 11, 24},
	{0x1c, 11, 28}, {0x18, 11, 32}, {0x0c, 11, 36}, {0x12, 11, 40},
	{0x13, 11, 44}, {0x2f, 11, 48}, {0x30, 11, 52}, {0x31, 11, 57},
	{0x4c, 11, 63}, {0x4d, 11, 67}, {0x4e, 11, 71},	{0x04, 15,  9},
	{0x16, 15, 13}, {0x07, 15, 17}, {0x09, 15, 21}, {0x0a, 15, 25},
	{0x0b, 15, 29}, {0x0d, 15, 33}, {0x0e, 15, 37}, {0x0f, 15, 41},
	{0x33, 15, 45}, {0x34, 15, 49}, {0x32, 15, 53}, {0x28, 15, 57},
	{0xe1, 19,  3},	{0x64, 19,  7}, {0x1d, 19, 11}, {0x1b, 19, 15},
	{0x06, 19, 19},	{0x19, 19, 23}, {0x05, 19, 27}, {0x11, 19, 31},
	{0x10, 19, 35},	{0x36, 19, 39}, {0x37, 19, 43}, {0x38, 19, 47},
	{0x87, 19, 51},	{0xe5, 19, 56}, {0x52, 19, 67},	{0xe0, 23,  3},
	{0xe3, 23,  8},	{0xe2, 23, 13}, {0x8b, 23, 17}, {0x2c, 23, 26},
	{0x8a, 23, 35},	{0x88, 23, 39}, {0xe6, 23, 44}, {0x65, 23, 52},
	{0xe4, 23, 57},	{0x50, 23, 63}, {0x51, 23, 67}, {0x4f, 23, 71},

	/* keys_extra */
	{0x39, 15,  4}, {0xf1, 23, 48}, {0x29,  2,  2},
};

static gboolean ryostkl_ripple_get_center(guint8 hid, guint *row, guint *col) {
	guint i;

	for (i = 0; i < KEYS_NUM; ++i) {
		if (light_position_table[i].hid == hid) {
			*row = light_position_table[i].row;
			*col = light_position_table[i].col;
			return TRUE;
		}
	}

	return FALSE;
}

static guint8 light_index_table[ROWS][COLS] = {

	{ VOID, 0x01, 0x01, VOID, VOID, VOID, VOID, VOID, VOID, 0x02, 0x02, VOID, VOID, 0x0e, 0x0e, /* 0 */
	VOID, VOID, 0x03, 0x03, VOID, VOID, 0x0f, 0x0f, VOID, VOID, VOID, VOID, 0x04, 0x04, VOID, VOID, 0x10, 0x10, VOID, VOID,
	0x05, 0x05, VOID, VOID, 0x11, 0x11, VOID, VOID, VOID, VOID, 0x06, 0x06, VOID, VOID, 0x07, 0x07, VOID, VOID, 0x08, 0x08,
	VOID, VOID, 0x14, 0x14, VOID, VOID, VOID, 0x09, 0x09, VOID, VOID, 0x15, 0x15, VOID, VOID, 0x21, 0x21, VOID },

	{ 0x01, 0x01, 0x01, 0x01, VOID, VOID, VOID, VOID, 0x02, 0x02, 0x02, 0x02, 0x0e, 0x0e, 0x0e,
	0x0e, 0x03, 0x03, 0x03, 0x03, 0x0f, 0x0f, 0x0f, 0x0f, VOID, VOID, 0x04, 0x04, 0x04, 0x04, 0x10, 0x10, 0x10, 0x10, 0x05,
	0x05, 0x05, 0x05, 0x11, 0x11, 0x11, 0x11, VOID, VOID, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08,
	0x08, 0x14, 0x14, 0x14, 0x14, VOID, 0x09, 0x09, 0x09, 0x09, 0x15, 0x15, 0x15, 0x15, 0x21, 0x21, 0x21, 0x21 },

	{ 0x01, 0x01, 0x01, 0x01, VOID, VOID, VOID, VOID, 0x02, 0x02, 0x02, 0x02, 0x0e, 0x0e, 0x0e,
	0x0e, 0x03, 0x03, 0x03, 0x03, 0x0f, 0x0f, 0x0f, 0x0f, VOID, VOID, 0x04, 0x04, 0x04, 0x04, 0x10, 0x10, 0x10, 0x10, 0x05,
	0x05, 0x05, 0x05, 0x11, 0x11, 0x11, 0x11, VOID, VOID, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08,
	0x08, 0x14, 0x14, 0x14, 0x14, VOID, 0x09, 0x09, 0x09, 0x09, 0x15, 0x15, 0x15, 0x15, 0x21, 0x21, 0x21, 0x21 },

	{ VOID, 0x01, 0x01, VOID, VOID, VOID, VOID, VOID, VOID, 0x02, 0x02, VOID, VOID, 0x0e, 0x0e,
	VOID, VOID, 0x03, 0x03, VOID, VOID, 0x0f, 0x0f, VOID, VOID, VOID, VOID, 0x04, 0x04, VOID, VOID, 0x10, 0x10, VOID, VOID,
	0x05, 0x05, VOID, VOID, 0x11, 0x11, VOID, VOID, VOID, VOID, 0x06, 0x06, VOID, VOID, 0x07, 0x07, VOID, VOID, 0x08, 0x08,
	VOID, VOID, 0x14, 0x14, VOID, VOID, VOID, 0x09, 0x09, VOID, VOID, 0x15, 0x15, VOID, VOID, 0x21, 0x21, VOID },

	{ VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, 0x0c, 0x0c, VOID, VOID, 0x0d, 0x0d, VOID, VOID, 0x19, 0x19, VOID, VOID, 0x1a, 0x1a, /* 5 */
	VOID, VOID, 0x1b, 0x1b, VOID, VOID, 0x27, 0x27, VOID, VOID, 0x1c, 0x1c, VOID, VOID, 0x28, 0x28, VOID, VOID, 0x1d, 0x1d,
	VOID, VOID, 0x29, 0x29, VOID, VOID, 0x12, 0x12, VOID, VOID, 0x1e, 0x1e, VOID, VOID, 0x13, 0x13, VOID, VOID, 0x20, 0x20,
	VOID, VOID, 0x2c, 0x2c, VOID, VOID, VOID, 0x2d, 0x2d, VOID, VOID, 0x39, 0x39, VOID, VOID, 0x45, 0x45, VOID },

	{ 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x19, 0x19, 0x19, 0x19, 0x1a, 0x1a, 0x1a,
	0x1a, 0x1b, 0x1b, 0x1b, 0x1b, 0x27, 0x27, 0x27, 0x27, 0x1c, 0x1c, 0x1c, 0x1c, 0x28, 0x28, 0x28, 0x28, 0x1d, 0x1d, 0x1d,
	0x1d, 0x29, 0x29, 0x29, 0x29, 0x12, 0x12, 0x12, 0x12, 0x1e, 0x1e, 0x1e, 0x1e, 0x13, 0x13, 0x13, 0x13, 0x20, 0x20, 0x20,
	0x20, 0x2c, 0x2c, 0x2c, 0x2c, VOID, 0x2d, 0x2d, 0x2d, 0x2d, 0x39, 0x39, 0x39, 0x39, 0x45, 0x45, 0x45, 0x45 },

	{ 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x19, 0x19, 0x19, 0x19, 0x1a, 0x1a, 0x1a,
	0x1a, 0x1b, 0x1b, 0x1b, 0x1b, 0x27, 0x27, 0x27, 0x27, 0x1c, 0x1c, 0x1c, 0x1c, 0x28, 0x28, 0x28, 0x28, 0x1d, 0x1d, 0x1d,
	0x1d, 0x29, 0x29, 0x29, 0x29, 0x12, 0x12, 0x12, 0x12, 0x1e, 0x1e, 0x1e, 0x1e, 0x13, 0x13, 0x13, 0x13, 0x20, 0x20, 0x20,
	0x20, 0x2c, 0x2c, 0x2c, 0x2c, VOID, 0x2d, 0x2d, 0x2d, 0x2d, 0x39, 0x39, 0x39, 0x39, 0x45, 0x45, 0x45, 0x45 },

	{ VOID, 0x0c, 0x0c, VOID, VOID, 0x0d, 0x0d, VOID, VOID, 0x19, 0x19, VOID, VOID, 0x1a, 0x1a,
	VOID, VOID, 0x1b, 0x1b, VOID, VOID, 0x27, 0x27, VOID, VOID, 0x1c, 0x1c, VOID, VOID, 0x28, 0x28, VOID, VOID, 0x1d, 0x1d,
	VOID, VOID, 0x29, 0x29, VOID, VOID, 0x12, 0x12, VOID, VOID, 0x1e, 0x1e, VOID, VOID, 0x13, 0x13, VOID, VOID, 0x20, 0x20,
	VOID, VOID, 0x2c, 0x2c, VOID, VOID, VOID, 0x2d, 0x2d, VOID, VOID, 0x39, 0x39, VOID, VOID, 0x45, 0x45, VOID },

	{ VOID, VOID, 0x24, 0x24, VOID, VOID, VOID, 0x25, 0x25, VOID, VOID, 0x31, 0x31, VOID, VOID, /* 9 */
	0x26, 0x26, VOID, VOID, 0x32, 0x32, VOID, VOID, 0x33, 0x33, VOID, VOID, 0x34, 0x34, VOID, VOID, 0x40, 0x40, VOID, VOID,
	0x35, 0x35, VOID, VOID, 0x2a, 0x2a, VOID, VOID, 0x36, 0x36, VOID, VOID, 0x1f, 0x1f, VOID, VOID, 0x2b, 0x2b, VOID, VOID,
	VOID, 0x38, 0x38, VOID, VOID, VOID, VOID, 0x51, 0x51, VOID, VOID, 0x5d, 0x5d, VOID, VOID, 0x69, 0x69, VOID },

	{ VOID, 0x24, 0x24, 0x24, 0x24, VOID, 0x25, 0x25, 0x25, 0x25, 0x31, 0x31, 0x31, 0x31, 0x26,
	0x26, 0x26, 0x26, 0x32, 0x32, 0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34, 0x34, 0x40, 0x40, 0x40, 0x40, 0x35,
	0x35, 0x35, 0x35, 0x2a, 0x2a, 0x2a, 0x2a, 0x36, 0x36, 0x36, 0x36, 0x1f, 0x1f, 0x1f, 0x1f, 0x2b, 0x2b, 0x2b, 0x2b, VOID,
	0x38, 0x38, 0x38, 0x38, VOID, VOID, 0x51, 0x51, 0x51, 0x51, 0x5d, 0x5d, 0x5d, 0x5d, 0x69, 0x69, 0x69, 0x69 },

	{ VOID, 0x24, 0x24, 0x24, 0x24, VOID, 0x25, 0x25, 0x25, 0x25, 0x31, 0x31, 0x31, 0x31, 0x26,
	0x26, 0x26, 0x26, 0x32, 0x32, 0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34, 0x34, 0x40, 0x40, 0x40, 0x40, 0x35,
	0x35, 0x35, 0x35, 0x2a, 0x2a, 0x2a, 0x2a, 0x36, 0x36, 0x36, 0x36, 0x1f, 0x1f, 0x1f, 0x1f, 0x2b, 0x2b, 0x2b, 0x2b, VOID,
	0x38, 0x38, 0x38, 0x38, VOID, VOID, 0x51, 0x51, 0x51, 0x51, 0x5d, 0x5d, 0x5d, 0x5d, 0x69, 0x69, 0x69, 0x69 },

	{ VOID, VOID, 0x24, 0x24, VOID, VOID, VOID, 0x25, 0x25, VOID, VOID, 0x31, 0x31, VOID, VOID,
	0x26, 0x26, VOID, VOID, 0x32, 0x32, VOID, VOID, 0x33, 0x33, VOID, VOID, 0x34, 0x34, VOID, VOID, 0x40, 0x40, VOID, VOID,
	0x35, 0x35, VOID, VOID, 0x2a, 0x2a, VOID, VOID, 0x36, 0x36, VOID, VOID, 0x1f, 0x1f, VOID, VOID, 0x2b, 0x2b, VOID, VOID,
	VOID, 0x38, 0x38, VOID, VOID, VOID, VOID, 0x51, 0x51, VOID, VOID, 0x5d, 0x5d, VOID, VOID, 0x69, 0x69, VOID },

	{ VOID, VOID, VOID, 0x3c, 0x3c, VOID, VOID, VOID, 0x3d, 0x3d, VOID, VOID, 0x49, 0x49, VOID, /* 13 */
	VOID, 0x3e, 0x3e, VOID, VOID, 0x4a, 0x4a, VOID, VOID, 0x3f, 0x3f, VOID, VOID, 0x4b, 0x4b, VOID, VOID, 0x4c, 0x4c, VOID,
	VOID, 0x41, 0x41, VOID, VOID, 0x42, 0x42, VOID, VOID, 0x4e, 0x4e, VOID, VOID, 0x37, 0x37, VOID, VOID, 0x43, 0x43, VOID,
	VOID, 0x44, 0x44, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, 0x3c, 0x3c, 0x3c, 0x3c, VOID, 0x3d, 0x3d, 0x3d, 0x3d, 0x49, 0x49, 0x49, 0x49,
	0x3e, 0x3e, 0x3e, 0x3e, 0x4a, 0x4a, 0x4a, 0x4a, 0x3f, 0x3f, 0x3f, 0x3f, 0x4b, 0x4b, 0x4b, 0x4b, 0x4c, 0x4c, 0x4c, 0x4c,
	0x41, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42, 0x42, 0x4e, 0x4e, 0x4e, 0x4e, 0x37, 0x37, 0x37, 0x37, 0x43, 0x43, 0x43, 0x43,
	0x44, 0x44, 0x44, 0x44, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, 0x3c, 0x3c, 0x3c, 0x3c, VOID, 0x3d, 0x3d, 0x3d, 0x3d, 0x49, 0x49, 0x49, 0x49,
	0x3e, 0x3e, 0x3e, 0x3e, 0x4a, 0x4a, 0x4a, 0x4a, 0x3f, 0x3f, 0x3f, 0x3f, 0x4b, 0x4b, 0x4b, 0x4b, 0x4c, 0x4c, 0x4c, 0x4c,
	0x41, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42, 0x42, 0x4e, 0x4e, 0x4e, 0x4e, 0x37, 0x37, 0x37, 0x37, 0x43, 0x43, 0x43, 0x43,
	0x44, 0x44, 0x44, 0x44, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, VOID, 0x3c, 0x3c, VOID, VOID, VOID, 0x3d, 0x3d, VOID, VOID, 0x49, 0x49, VOID,
	VOID, 0x3e, 0x3e, VOID, VOID, 0x4a, 0x4a, VOID, VOID, 0x3f, 0x3f, VOID, VOID, 0x4b, 0x4b, VOID, VOID, 0x4c, 0x4c, VOID,
	VOID, 0x41, 0x41, VOID, VOID, 0x42, 0x42, VOID, VOID, 0x4e, 0x4e, VOID, VOID, 0x37, 0x37, VOID, VOID, 0x43, 0x43, VOID,
	VOID, 0x44, 0x44, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, 0x54, 0x54, VOID, VOID, 0x55, 0x55, VOID, VOID, 0x61, 0x61, VOID, VOID, 0x56, /* 17 */
	0x56, VOID, VOID, 0x62, 0x62, VOID, VOID, 0x57, 0x57, VOID, VOID, 0x63, 0x63, VOID, VOID, 0x58, 0x58, VOID, VOID, 0x4d,
	0x4d, VOID, VOID, 0x59, 0x59, VOID, VOID, 0x5a, 0x5a, VOID, VOID, 0x4f, 0x4f, VOID, VOID, 0x5b, 0x5b, VOID, VOID, VOID,
	0x50, 0x50, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x75, 0x75, VOID, VOID, VOID, VOID, VOID },

	{ VOID, 0x54, 0x54, 0x54, 0x54, 0x55, 0x55, 0x55, 0x55, 0x61, 0x61, 0x61, 0x61, 0x56, 0x56,
	0x56, 0x56, 0x62, 0x62, 0x62, 0x62, 0x57, 0x57, 0x57, 0x57, 0x63, 0x63, 0x63, 0x63, 0x58, 0x58, 0x58, 0x58, 0x4d, 0x4d,
	0x4d, 0x4d, 0x59, 0x59, 0x59, 0x59, 0x5a, 0x5a, 0x5a, 0x5a, 0x4f, 0x4f, 0x4f, 0x4f, 0x5b, 0x5b, 0x5b, 0x5b, VOID, 0x50,
	0x50, 0x50, 0x50, VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x75, 0x75, 0x75, 0x75, VOID, VOID, VOID, VOID },

	{ VOID, 0x54, 0x54, 0x54, 0x54, 0x55, 0x55, 0x55, 0x55, 0x61, 0x61, 0x61, 0x61, 0x56, 0x56,
	0x56, 0x56, 0x62, 0x62, 0x62, 0x62, 0x57, 0x57, 0x57, 0x57, 0x63, 0x63, 0x63, 0x63, 0x58, 0x58, 0x58, 0x58, 0x4d, 0x4d,
	0x4d, 0x4d, 0x59, 0x59, 0x59, 0x59, 0x5a, 0x5a, 0x5a, 0x5a, 0x4f, 0x4f, 0x4f, 0x4f, 0x5b, 0x5b, 0x5b, 0x5b, VOID, 0x50,
	0x50, 0x50, 0x50, VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x75, 0x75, 0x75, 0x75, VOID, VOID, VOID, VOID },

	{ VOID, VOID, 0x54, 0x54, VOID, VOID, 0x55, 0x55, VOID, VOID, 0x61, 0x61, VOID, VOID, 0x56,
	0x56, VOID, VOID, 0x62, 0x62, VOID, VOID, 0x57, 0x57, VOID, VOID, 0x63, 0x63, VOID, VOID, 0x58, 0x58, VOID, VOID, 0x4d,
	0x4d, VOID, VOID, 0x59, 0x59, VOID, VOID, 0x5a, 0x5a, VOID, VOID, 0x4f, 0x4f, VOID, VOID, 0x5b, 0x5b, VOID, VOID, VOID,
	0x50, 0x50, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x75, 0x75, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, 0x6c, 0x6c, VOID, VOID, VOID, 0x6d, 0x6d, VOID, VOID, VOID, 0x6e, 0x6e, VOID, /* 21 */
	VOID, 0x64, 0x64, VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x70, 0x70, VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x65,
	0x65, VOID, VOID, 0x71, 0x71, VOID, VOID, VOID, 0x72, 0x72, VOID, VOID, 0x73, 0x73, VOID, VOID, 0x67, 0x67, VOID, VOID,
	VOID, 0x5c, 0x5c, VOID, VOID, VOID, VOID, 0x68, 0x68, VOID, VOID, 0x74, 0x74, VOID, VOID, 0x76, 0x76, VOID },

	{ VOID, 0x6c, 0x6c, 0x6c, 0x6c, VOID, 0x6d, 0x6d, 0x6d, 0x6d, VOID, 0x6e, 0x6e, 0x6e, 0x6e,
	0x64, 0x64, 0x64, 0x64, VOID, VOID, VOID, VOID, VOID, 0x70, 0x70, 0x70, 0x70, VOID, VOID, VOID, VOID, VOID, 0x65, 0x65,
	0x65, 0x65, 0x71, 0x71, 0x71, 0x71, VOID, 0x72, 0x72, 0x72, 0x72, 0x73, 0x73, 0x73, 0x73, 0x67, 0x67, 0x67, 0x67, VOID,
	0x5c, 0x5c, 0x5c, 0x5c, VOID, VOID, 0x68, 0x68, 0x68, 0x68, 0x74, 0x74, 0x74, 0x74, 0x76, 0x76, 0x76, 0x76 },

	{ VOID, 0x6c, 0x6c, 0x6c, 0x6c, VOID, 0x6d, 0x6d, 0x6d, 0x6d, VOID, 0x6e, 0x6e, 0x6e, 0x6e,
	0x64, 0x64, 0x64, 0x64, VOID, VOID, VOID, VOID, VOID, 0x70, 0x70, 0x70, 0x70, VOID, VOID, VOID, VOID, VOID, 0x65, 0x65,
	0x65, 0x65, 0x71, 0x71, 0x71, 0x71, VOID, 0x72, 0x72, 0x72, 0x72, 0x73, 0x73, 0x73, 0x73, 0x67, 0x67, 0x67, 0x67, VOID,
	0x5c, 0x5c, 0x5c, 0x5c, VOID, VOID, 0x68, 0x68, 0x68, 0x68, 0x74, 0x74, 0x74, 0x74, 0x76, 0x76, 0x76, 0x76 },

	{ VOID, VOID, 0x6c, 0x6c, VOID, VOID, VOID, 0x6d, 0x6d, VOID, VOID, VOID, 0x6e, 0x6e, VOID,
	VOID, 0x64, 0x64, VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x70, 0x70, VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x65,
	0x65, VOID, VOID, 0x71, 0x71, VOID, VOID, VOID, 0x72, 0x72, VOID, VOID, 0x73, 0x73, VOID, VOID, 0x67, 0x67, VOID, VOID,
	VOID, 0x5c, 0x5c, VOID, VOID, VOID, VOID, 0x68, 0x68, VOID, VOID, 0x74, 0x74, VOID, VOID, 0x76, 0x76, VOID },

	{ VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, /* 26 */
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x78, 0x78, VOID, VOID, 0x79, 0x79, VOID, VOID, 0x7a, 0x7a, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, 0x78, 0x78, 0x78, 0x78, 0x79, 0x79, 0x79, 0x79, 0x7a, 0x7a, 0x7a, 0x7a, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, 0x78, 0x78, 0x78, 0x78, 0x79, 0x79, 0x79, 0x79, 0x7a, 0x7a, 0x7a, 0x7a, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },

	{ VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, 0x78, 0x78, VOID, VOID, 0x79, 0x79, VOID, VOID, 0x7a, 0x7a, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID,
	VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID },
};

/* returns TRUE if position is in table range */
static gboolean set_led(RyosCustomLights *custom_lights, gint row, gint column, gboolean value) {
	guint8 bit;

	if (row >= ROWS || row < 0 || column >= COLS || column < 0)
		return FALSE;

	bit = light_index_table[row][column];
	if (bit != VOID)
		roccat_bitfield_set_bit(custom_lights->data, bit, value);

	return TRUE;
}

/*
 * QUOTE Midpoint circle algorithm copied from Perone's programming pad
 * http://www.willperone.net/Code/codecircle.php
 * Copyright (c) Will Perone
 */
static gboolean ryostkl_ripple_draw_circle(RyosCustomLights *custom_lights, gint center_row, gint center_col, gint radius, gboolean value) {
	int x, y, d, delta_e, delta_se;
	gboolean go_on = FALSE;

	radius *= KEY_WIDTH,

	x = 0;
	y = radius;
	d = 1 - radius;
	delta_e = 3;
	delta_se = 5 - radius * 2;

	if (radius < 1)
		return TRUE;

	if (set_led(custom_lights, center_row - y, center_col, value)) go_on = TRUE;
	if (set_led(custom_lights, center_row + y, center_col, value)) go_on = TRUE;
	if (set_led(custom_lights, center_row, center_col - y, value)) go_on = TRUE;
	if (set_led(custom_lights, center_row, center_col + y, value)) go_on = TRUE;

	while (y > x) {
		if (d < 0) {
			d += delta_e;
			delta_se += 2;
		} else {
			d += delta_se;
			delta_se += 4;
			y--;
		}
		delta_e += 2;
		x++;

		if (set_led(custom_lights, center_row - y, center_col - x, value)) go_on = TRUE;
		if (set_led(custom_lights, center_row - x, center_col - y, value)) go_on = TRUE;
		if (set_led(custom_lights, center_row - x, center_col + y, value)) go_on = TRUE;
		if (set_led(custom_lights, center_row - y, center_col + x, value)) go_on = TRUE;
		if (set_led(custom_lights, center_row + y, center_col - x, value)) go_on = TRUE;
		if (set_led(custom_lights, center_row + x, center_col - y, value)) go_on = TRUE;
		if (set_led(custom_lights, center_row + x, center_col + y, value)) go_on = TRUE;
		if (set_led(custom_lights, center_row + y, center_col + x, value)) go_on = TRUE;
	}
	return go_on;
}

static gpointer worker(gpointer user_data) {
	RyostklRippleCPrivate *priv = RYOSTKL_RIPPLE_C(user_data)->priv;
	GError *local_error = NULL;
	guint center_row = 0;
	guint center_col = 0;
	guint radius = 0;
	RoccatDevice *device;
	RyosCustomLights custom_lights;

	while (TRUE) {
		gaminggear_mutex_lock(&priv->mutex);
		gaminggear_cond_wait(&priv->start, &priv->mutex);
		device = priv->device;
		gaminggear_mutex_unlock(&priv->mutex);

		if (!ryos_light_control_custom(device, TRUE, &local_error)) {
			g_warning(_("Could not init custom mode: %s"), local_error->message);
			g_clear_error(&local_error);
			continue; // Try again next time its started
		}

		while (TRUE) {
			gaminggear_mutex_lock(&priv->mutex);
			if (priv->changed) {
				if (!ryostkl_ripple_get_center(priv->hid, &center_row, &center_col)) {
					g_warning(_("Could not find a key for hid 0x%02x"), priv->hid);
					gaminggear_mutex_unlock(&priv->mutex);
					break;
				}
				if (device != priv->device) {
					gaminggear_mutex_unlock(&priv->mutex);
					/* If device changes due to replug first event gets lost,
					 * because threat waits again for start at outer loop */
					break;
				}
				device = priv->device;
				radius = 1;
				memset(&custom_lights.data, 0, RYOS_CUSTOM_LIGHTS_DATA_LENGTH);
				priv->changed = FALSE;
			}
			gaminggear_mutex_unlock(&priv->mutex);

			if (!ryostkl_ripple_draw_circle(&custom_lights, center_row, center_col, radius, TRUE))
				break;

			if (!ryos_custom_lights_write(device, &custom_lights, &local_error)) {
				g_warning(_("Could not write custom lights: %s"), local_error->message);
				g_clear_error(&local_error);
				break;
			}

			if (g_atomic_int_get(&priv->changed))
				continue;

			g_usleep(50 * G_ROCCAT_USEC_PER_MSEC);

			if (g_atomic_int_get(&priv->changed))
				continue;

			ryostkl_ripple_draw_circle(&custom_lights, center_row, center_col, radius - 1 , FALSE);

			if (ryos_custom_lights_empty(&custom_lights))
				break;

			if (!ryos_custom_lights_write(device, &custom_lights, &local_error)) {
				g_warning(_("Could not write custom lights: %s"), local_error->message);
				g_clear_error(&local_error);
				break;
			}

			++radius;
		}

		if (!ryos_light_control_custom(device, FALSE, &local_error)) {
			g_warning(_("Could not deinit custom mode: %s"), local_error->message);
			g_clear_error(&local_error);
		}
	}

	return NULL;
}

static void ryostkl_ripple_c_start(RyostklRipple *self, RoccatDevice *device, guint hid) {
	RyostklRippleCPrivate *priv = RYOSTKL_RIPPLE_C(self)->priv;
	gaminggear_mutex_lock(&priv->mutex);
	priv->device = device;
	priv->hid = hid;
	priv->changed = TRUE;
	gaminggear_cond_signal(&priv->start);
	gaminggear_mutex_unlock(&priv->mutex);
}

static void ryostkl_ripple_c_interface_init(RyostklRippleInterface *iface) {
	iface->start = ryostkl_ripple_c_start;
}

G_DEFINE_TYPE_WITH_CODE(RyostklRippleC, ryostkl_ripple_c, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(RYOSTKL_RIPPLE_TYPE, ryostkl_ripple_c_interface_init));

RyostklRippleC *ryostkl_ripple_c_new(void) {
	return RYOSTKL_RIPPLE_C(g_object_new(RYOSTKL_RIPPLE_C_TYPE, NULL));
}

static void ryostkl_ripple_c_init(RyostklRippleC *ripple_c) {
	RyostklRippleCPrivate *priv = RYOSTKL_RIPPLE_C_GET_PRIVATE(ripple_c);
	GError *local_error = NULL;

	ripple_c->priv = priv;

	gaminggear_mutex_init(&priv->mutex);
	gaminggear_cond_init(&priv->start);
	priv->changed = FALSE;

	priv->worker = gaminggear_thread_try_new("RyostklRipple", worker, ripple_c, &local_error);
	if (local_error) {
		g_warning(_("Could not create ripple thread: %s"), local_error->message);
		g_clear_error(&local_error);
		priv->worker = NULL;
	}
}

static void ryostkl_ripple_c_finalize(GObject *object) {
	RyostklRippleCPrivate *priv = RYOSTKL_RIPPLE_C(object)->priv;
	if (priv->worker)
		g_thread_join(priv->worker);
	gaminggear_mutex_clear(&priv->mutex);
	gaminggear_cond_clear(&priv->start);
	G_OBJECT_CLASS(ryostkl_ripple_c_parent_class)->finalize(object);
}

static void ryostkl_ripple_c_class_init(RyostklRippleCClass *klass) {
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->finalize = ryostkl_ripple_c_finalize;

	g_type_class_add_private(klass, sizeof(RyostklRippleCPrivate));
}