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

#include "ryos_ripple.h"

void ryos_ripple_start(RyosRipple *self, RoccatDevice *device, guint hid) {
	RYOS_RIPPLE_GET_INTERFACE(self)->start(self, device, hid);
}

G_DEFINE_INTERFACE(RyosRipple, ryos_ripple, G_TYPE_OBJECT);

static void ryos_ripple_default_init(RyosRippleInterface *iface) {
}
