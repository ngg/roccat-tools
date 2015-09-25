#ifndef __RYOSTKL_RIPPLE_H__
#define __RYOSTKL_RIPPLE_H__

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

#define RYOSTKL_RIPPLE_TYPE (ryostkl_ripple_get_type())
#define RYOSTKL_RIPPLE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOSTKL_RIPPLE_TYPE, RyostklRipple))
#define IS_RYOSTKL_RIPPLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOSTKL_RIPPLE_TYPE))
#define RYOSTKL_RIPPLE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), RYOSTKL_RIPPLE_TYPE, RyostklRippleInterface))

typedef struct _RyostklRipple RyostklRipple;
typedef struct _RyostklRippleInterface RyostklRippleInterface;

struct _RyostklRippleInterface {
	GTypeInterface parent;

	void (*start)(RyostklRipple *self, RoccatDevice *device, guint hid);
};

GType ryostkl_ripple_get_type(void);

void ryostkl_ripple_start(RyostklRipple *self, RoccatDevice *device, guint hid);

G_END_DECLS

#endif
