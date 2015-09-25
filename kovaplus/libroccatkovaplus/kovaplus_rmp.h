#ifndef __ROCCAT_KOVAPLUS_RMP_H__
#define __ROCCAT_KOVAPLUS_RMP_H__

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

#include "kovaplus.h"

G_BEGIN_DECLS

struct _KovaplusRmp {
	gboolean modified_rmp;
	gboolean modified_profile_buttons;
	gboolean modified_profile_settings;
	GKeyFile *key_file;
};

G_END_DECLS

#endif
