#ifndef __ROCCAT_LUACONFIG_POLLING_RATE_FRAME_H__
#define __ROCCAT_LUACONFIG_POLLING_RATE_FRAME_H__

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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LUACONFIG_POLLING_RATE_FRAME_TYPE (luaconfig_polling_rate_frame_get_type())
#define LUACONFIG_POLLING_RATE_FRAME(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), LUACONFIG_POLLING_RATE_FRAME_TYPE, LuaconfigPollingRateFrame))
#define IS_LUACONFIG_POLLING_RATE_FRAME(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), LUACONFIG_POLLING_RATE_FRAME_TYPE))

typedef struct _LuaconfigPollingRateFrame LuaconfigPollingRateFrame;

GType luaconfig_polling_rate_frame_get_type(void);
GtkWidget *luaconfig_polling_rate_frame_new(void);
void luaconfig_polling_rate_frame_set_value(LuaconfigPollingRateFrame *frame, guint rmp_value);
guint luaconfig_polling_rate_frame_get_value(LuaconfigPollingRateFrame *frame);

G_END_DECLS

#endif
