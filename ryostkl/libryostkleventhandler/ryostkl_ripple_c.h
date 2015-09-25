#ifndef __RYOSTKL_RIPPLE_C_H__
#define __RYOSTKL_RIPPLE_C_H__

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

#include "ryostkl_ripple.h"

G_BEGIN_DECLS

#define RYOSTKL_RIPPLE_C_TYPE (ryostkl_ripple_c_get_type())
#define RYOSTKL_RIPPLE_C(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOSTKL_RIPPLE_C_TYPE, RyostklRippleC))
#define IS_RYOSTKL_RIPPLE_C(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOSTKL_RIPPLE_C_TYPE))

typedef struct _RyostklRippleC RyostklRippleC;
typedef struct _RyostklRippleCPrivate RyostklRippleCPrivate;

struct _RyostklRippleC {
	GObject parent;
	RyostklRippleCPrivate *priv;
};

GType ryostkl_ripple_c_get_type(void);
RyostklRippleC *ryostkl_ripple_c_new(void);

G_END_DECLS

#endif
