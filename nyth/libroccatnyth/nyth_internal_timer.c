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

#include "nyth_internal_timer.h"
#include <string.h>

void nyth_internal_timer_set_name(NythInternalTimer *timer, gchar const *new_name) {
	g_strlcpy((gchar *)(timer->name), new_name, NYTH_INTERNAL_TIMER_NAME_LENGTH);
}

void roccat_timer_to_nyth_internal_timer(RoccatTimer const *roccat_timer, NythInternalTimer *nyth_timer) {
	memset(nyth_timer, 0, sizeof(NythInternalTimer));
	nyth_internal_timer_set_name(nyth_timer, roccat_timer->name);
	nyth_timer->duration = roccat_timer_get_seconds(roccat_timer);
	// TODO error handling;
}

gboolean nyth_internal_timer_equal(NythInternalTimer const *left, NythInternalTimer const *right) {
	return memcmp(left, right, sizeof(NythInternalTimer)) ? FALSE : TRUE;
}

void nyth_internal_timer_copy(NythInternalTimer *destination, NythInternalTimer const *source) {
	memcpy(destination, source, sizeof(NythInternalTimer));
}

NythInternalTimer *nyth_internal_timer_dup(NythInternalTimer const *source) {
	return (NythInternalTimer *)g_memdup(source, sizeof(NythInternalTimer));
}
