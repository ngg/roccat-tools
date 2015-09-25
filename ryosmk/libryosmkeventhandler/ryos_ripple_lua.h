#ifndef __RYOS_RIPPLE_LUA_H__
#define __RYOS_RIPPLE_LUA_H__

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

G_BEGIN_DECLS

#define RYOS_RIPPLE_LUA_TYPE (ryos_ripple_lua_get_type())
#define RYOS_RIPPLE_LUA(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOS_RIPPLE_LUA_TYPE, RyosRippleLua))
#define IS_RYOS_RIPPLE_LUA(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOS_RIPPLE_LUA_TYPE))

typedef struct _RyosRippleLua RyosRippleLua;

struct _RyosRippleLua {
	GObject parent;
};

GType ryos_ripple_lua_get_type(void);
RyosRippleLua *ryos_ripple_lua_new(gchar const *module_name);

G_END_DECLS

#endif
