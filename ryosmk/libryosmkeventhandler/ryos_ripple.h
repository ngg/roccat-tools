#ifndef __RYOS_RIPPLE_H__
#define __RYOS_RIPPLE_H__

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

#include "ryos.h"

G_BEGIN_DECLS

#define RYOS_RIPPLE_TYPE (ryos_ripple_get_type())
#define RYOS_RIPPLE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOS_RIPPLE_TYPE, RyosRipple))
#define IS_RYOS_RIPPLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOS_RIPPLE_TYPE))
#define RYOS_RIPPLE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), RYOS_RIPPLE_TYPE, RyosRippleInterface))

typedef struct _RyosRipple RyosRipple;
typedef struct _RyosRippleInterface RyosRippleInterface;

struct _RyosRippleInterface {
	GTypeInterface parent;

	void (*start)(RyosRipple *self, RoccatDevice *device, guint hid);
};

GType ryos_ripple_get_type(void);

void ryos_ripple_start(RyosRipple *self, RoccatDevice *device, guint hid);

G_END_DECLS

#endif
