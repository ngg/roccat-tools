#ifndef __RYOSTKL_RIPPLE_LUA_H__
#define __RYOSTKL_RIPPLE_LUA_H__

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

#define RYOSTKL_RIPPLE_LUA_TYPE (ryostkl_ripple_lua_get_type())
#define RYOSTKL_RIPPLE_LUA(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOSTKL_RIPPLE_LUA_TYPE, RyostklRippleLua))
#define IS_RYOSTKL_RIPPLE_LUA(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOSTKL_RIPPLE_LUA_TYPE))

typedef struct _RyostklRippleLua RyostklRippleLua;

struct _RyostklRippleLua {
	GObject parent;
};

GType ryostkl_ripple_lua_get_type(void);
RyostklRippleLua *ryostkl_ripple_lua_new(gchar const *module_name);

G_END_DECLS

#endif
