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

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "ryostkl_ripple_lua.h"
#include "ryos_custom_lights.h"
#include "ryos_light_control.h"
#include "g_roccat_helper.h"
#include "roccat_helper.h"
#include "config.h"
#include "i18n-lib.h"
#include <gaminggear/threads.h>

#define RYOSTKL_RIPPLE_LUA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOSTKL_RIPPLE_LUA_TYPE, RyostklRippleLuaClass))
#define IS_RYOSTKL_RIPPLE_LUA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOSTKL_RIPPLE_LUA_TYPE))

typedef struct _RyostklRippleLuaClass RyostklRippleLuaClass;

struct _RyostklRippleLuaClass {
	GObjectClass parent_class;
};

enum {
	PROP_0,
	PROP_MODULE,
};

static RyosCustomLights custom_lights;
static RoccatDevice *device;
static GThread *thread;
static GaminggearMutex mutex;
static GaminggearCond start;
static gchar *module_name;
static lua_State *state;

static guint8 hid_to_bit[] = {
/*	   0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
	0xff, 0xff, 0xff, 0xff, 0x3d, 0x63, 0x62, 0x3e, 0x26, 0x4a, 0x3f, 0x4b, 0x35, 0x4c, 0x41, 0x42, /* 0 */
	0x4d, 0x58, 0x2a, 0x36, 0x25, 0x32, 0x49, 0x33, 0x40, 0x57, 0x31, 0x56, 0x34, 0x61, 0x0d, 0x19, /* 1 */
	0x1a, 0x1b, 0x27, 0x1c, 0x28, 0x1d, 0x29, 0x12, 0x44, 0x01, 0x2c, 0x24, 0x70, 0x1e, 0x13, 0x1f, /* 2 */
	0x2b, 0x38, 0x43, 0x4e, 0x37, 0x0c, 0x59, 0x5a, 0x4f, 0x3c, 0x02, 0x0e, 0x03, 0x0f, 0x04, 0x10, /* 3 */
	0x05, 0x11, 0x06, 0x07, 0x08, 0x14, 0x09, 0x15, 0x21, 0x2d, 0x39, 0x45, 0x51, 0x5d, 0x69, 0x76, /* 4 */
	0x68, 0x74, 0x75, 0x0a, 0x16, 0x0b, 0x17, 0x2f, 0x5f, 0x52, 0x5e, 0x53, 0x3a, 0x46, 0x3b, 0x22, /* 5 */
	0x2e, 0x23, 0x6a, 0x6b, 0x55, 0x67, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 6 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 7 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5b, 0x71, 0x20, 0x65, 0x64, 0xff, 0xff, 0xff, 0xff, /* 8 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 9 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x18, 0x30, 0x48, 0x60, 0xff, /* a */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* b */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* c */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* d */
	0x6c, 0x54, 0x6e, 0x6d, 0x5c, 0x50, 0x72, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* e */
	0xff, 0x73, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* f */
};

static int ryosmk_modify_key(lua_State *L, gboolean state) {
	lua_Integer integer;

	switch (lua_type(L, -1)) {
	case LUA_TTABLE:
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			/* stack:
			 * -3 table
			 * -2 key
			 * -1 value
			 */
			integer = luaL_checkinteger(L, -2);
			roccat_bitfield_set_bit(custom_lights.data, hid_to_bit[integer], state);
			/* removes value, keeps key for next iteration */
			lua_pop(L, 1);
		}
		return 0;
		break;
	case LUA_TNUMBER:
		integer = luaL_checkinteger(L, -1);
		roccat_bitfield_set_bit(custom_lights.data, hid_to_bit[integer], state);
		return 0;
		break;
	default:
		lua_pushfstring(L, "table or integer expected, got %s", luaL_typename(L, -1));
		return luaL_argerror(L, -1, lua_tostring(L, -1));
		break;
	}
}

static int ryosmk_set_key(lua_State *L) {
	return ryosmk_modify_key(L, TRUE);
}

static int ryosmk_clear_key(lua_State *L) {
	return ryosmk_modify_key(L, FALSE);
}

static int ryosmk_get_key(lua_State *L) {
	lua_Integer hid = luaL_checkinteger(L, 1);
	lua_pushboolean(L, roccat_bitfield_get_bit(custom_lights.data, hid_to_bit[hid]) ? 1 : 0);
	return 1;
}

static int ryosmk_clear(lua_State *L) {
	memset(custom_lights.data, 0, RYOS_CUSTOM_LIGHTS_DATA_LENGTH);
	return 0;
}

static int ryosmk_empty(lua_State *L) {
	lua_pushboolean(L, ryos_custom_lights_empty(&custom_lights) ? 1 : 0);
	return 1;
}

static gint lua_call_tick(void) {
	int retval;

	lua_getglobal(state, "tick");
	retval = lua_pcall(state, 0, 1, 0);
	if (retval) {
		g_warning(_("lua_pcall(tick): %s"), lua_tostring(state, -1));
		lua_pop(state, 1);
		return -1;
	}

	if (lua_isnil(state, -1))
		return -1;

	return lua_tointeger(state, -1);
}

static void lua_call_start(guint hid) {
	int retval;

	lua_getglobal(state, "start");
	lua_pushinteger(state, hid);
	retval = lua_pcall(state, 1, 0, 0);
	if (retval) {
		g_warning(_("lua_pcall(start): %s"), lua_tostring(state, -1));
		lua_pop(state, 1);
	}
}

static gpointer worker(gpointer user_data) {
	GError *local_error = NULL;
	RoccatDevice *local_device;
	gint msecs;

	while (TRUE) {
		gaminggear_mutex_lock(&mutex);
		gaminggear_cond_wait(&start, &mutex);
		local_device = device;
		gaminggear_mutex_unlock(&mutex);

		if (!ryos_light_control_custom(local_device, TRUE, &local_error)) {
			g_warning(_("Could not init custom mode: %s"), local_error->message);
			g_clear_error(&local_error);
			continue; // Try again next time its started
		}

		while (TRUE) {
			gaminggear_mutex_lock(&mutex);
			msecs = lua_call_tick();
			gaminggear_mutex_unlock(&mutex);

			if (msecs == -1)
				break;

			if (!ryos_custom_lights_write(local_device, &custom_lights, &local_error)) {
				g_warning(_("Could not write custom lights: %s"), local_error->message);
				g_clear_error(&local_error);
				break;
			}

			g_usleep(msecs * G_ROCCAT_USEC_PER_MSEC);
		}

		if (!ryos_light_control_custom(local_device, FALSE, &local_error)) {
			g_warning(_("Could not deinit custom mode: %s"), local_error->message);
			g_clear_error(&local_error);
		}
	}

	return NULL;
}

static void ryostkl_ripple_lua_start(RyostklRipple *self, RoccatDevice *device_arg, guint hid) {
	gaminggear_mutex_lock(&mutex);
	device = device_arg;
	memset(&custom_lights.data, 0, RYOS_CUSTOM_LIGHTS_DATA_LENGTH);
	lua_call_start(hid);
	gaminggear_cond_signal(&start);
	gaminggear_mutex_unlock(&mutex);
}

static void ryostkl_ripple_lua_interface_init(RyostklRippleInterface *iface) {
	iface->start = ryostkl_ripple_lua_start;
}

G_DEFINE_TYPE_WITH_CODE(RyostklRippleLua, ryostkl_ripple_lua, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(RYOSTKL_RIPPLE_TYPE, ryostkl_ripple_lua_interface_init));

RyostklRippleLua *ryostkl_ripple_lua_new(gchar const *module_name) {
	return RYOSTKL_RIPPLE_LUA(g_object_new(RYOSTKL_RIPPLE_LUA_TYPE,
			"module", module_name,
			NULL));
}

static void ryostkl_ripple_lua_init(RyostklRippleLua *ripple_lua) {}

static GObject *ryostkl_ripple_lua_constructor(GType gtype, guint n_properties, GObjectConstructParam *properties) {
	GError *local_error = NULL;
	GObject *obj;
	gchar *module;
	int retval;

	obj = G_OBJECT_CLASS(ryostkl_ripple_lua_parent_class)->constructor(gtype, n_properties, properties);

	state = luaL_newstate();
	luaL_openlibs(state);

	lua_newtable(state);
	lua_pushcfunction(state, ryosmk_set_key);
	lua_setfield(state, -2, "set_key");
	lua_pushcfunction(state, ryosmk_get_key);
	lua_setfield(state, -2, "get_key");
	lua_pushcfunction(state, ryosmk_clear_key);
	lua_setfield(state, -2, "clear_key");
	lua_pushcfunction(state, ryosmk_clear);
	lua_setfield(state, -2, "clear");
	lua_pushcfunction(state, ryosmk_empty);
	lua_setfield(state, -2, "empty");
	lua_setglobal(state, "ryosmk");

	module = g_build_filename(RYOSTKL_RIPPLE_MODULES_PATH, module_name, NULL);
	retval = luaL_dofile(state, module);
	if (retval) {
		g_warning(_("luaL_dofile(%s): %s"), module, lua_tostring(state, -1));
		lua_pop(state, 1);
	}
	g_free(module);

	gaminggear_mutex_init(&mutex);
	gaminggear_cond_init(&start);
	thread = gaminggear_thread_try_new("RyostklRipple", worker, NULL, &local_error);
	if (local_error) {
		g_warning(_("Could not create ripple thread: %s"), local_error->message);
		g_clear_error(&local_error);
	}

	return obj;
}

static void ryostkl_ripple_lua_finalize(GObject *object) {
	if (thread)
		g_thread_join(thread);
	gaminggear_mutex_clear(&mutex);
	gaminggear_cond_clear(&start);

	if (state)
		lua_close(state);

	g_free(module_name);

	G_OBJECT_CLASS(ryostkl_ripple_lua_parent_class)->finalize(object);
}

static void ryostkl_ripple_lua_set_property(GObject *object, guint prop_id, GValue const *value, GParamSpec *pspec) {
	switch(prop_id) {
	case PROP_MODULE:
		module_name = g_strdup(g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void ryostkl_ripple_lua_class_init(RyostklRippleLuaClass *klass) {
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->constructor = ryostkl_ripple_lua_constructor;
	gobject_class->set_property = ryostkl_ripple_lua_set_property;
	gobject_class->finalize = ryostkl_ripple_lua_finalize;

	g_object_class_install_property(gobject_class, PROP_MODULE,
			g_param_spec_string("module",
					"Lua module",
					"Lua module",
					"ripple.lua",
					G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}
