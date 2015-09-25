#ifndef __ROCCAT_TYON_SROM_H__
#define __ROCCAT_TYON_SROM_H__

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

#include <glib.h>

G_BEGIN_DECLS

typedef struct _TyonSRom TyonSRom;

enum {
	TYON_SROM_SIZE = 3072, /* TODO clarify if it's 3070 bytes extended with 0s */
	TYON_SROM_CHUNK_COUNT = 3,
	TYON_SROM_CHUNK_SIZE = 1024,
};

struct _TyonSRom {
	guint8 report_id; /* TYON_REPORT_ID_SROM */
	guint8 number;
	guint8 data[TYON_SROM_CHUNK_SIZE];
	guint16 checksum;
};

G_END_DECLS

#endif
