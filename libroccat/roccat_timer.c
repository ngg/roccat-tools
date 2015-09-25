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

#include "roccat_timer.h"

void roccat_timer_set_name(RoccatTimer *timer, gchar const *new_name) {
	g_free(timer->name);
	timer->name = g_strdup(new_name);
}

RoccatTimer *roccat_timer_new(void) {
	RoccatTimer *timer;
	timer = g_malloc0(sizeof(RoccatTimer));
	return timer;
}

void roccat_timer_free(RoccatTimer *timer) {
	if (timer == NULL)
		return;
	g_free(timer->name);
	g_free(timer);
}
