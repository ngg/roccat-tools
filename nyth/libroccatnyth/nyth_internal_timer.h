#ifndef __ROCCAT_NYTH_INTERNAL_TIMER_H__
#define __ROCCAT_NYTH_INTERNAL_TIMER_H__

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with roccat-tools. If not, see <http://www.gnu.org/licenses/>.
 */

#include "nyth.h"
#include "roccat_timer.h"

G_BEGIN_DECLS

typedef struct _NythInternalTimer NythInternalTimer;

enum {
	NYTH_INTERNAL_TIMER_NAME_LENGTH = 24,
};

struct _NythInternalTimer {
	guint8 name[NYTH_INTERNAL_TIMER_NAME_LENGTH];
	guint8 duration;
};

void nyth_internal_timer_set_name(NythInternalTimer *timer, gchar const *new_name);
void roccat_timer_to_nyth_internal_timer(RoccatTimer const *roccat_timer, NythInternalTimer *nyth_timer);
gboolean nyth_internal_timer_equal(NythInternalTimer const *left, NythInternalTimer const *right);
void nyth_internal_timer_copy(NythInternalTimer *destination, NythInternalTimer const *source);
NythInternalTimer *nyth_internal_timer_dup(NythInternalTimer const *source);

G_END_DECLS

#endif
