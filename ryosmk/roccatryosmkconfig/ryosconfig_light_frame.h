#ifndef __RYOSCONFIG_LIGHT_FRAME_H__
#define __RYOSCONFIG_LIGHT_FRAME_H__

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
#include "ryos_rkp.h"

G_BEGIN_DECLS

#define RYOSCONFIG_LIGHT_FRAME_TYPE (ryosconfig_light_frame_get_type())
#define RYOSCONFIG_LIGHT_FRAME(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOSCONFIG_LIGHT_FRAME_TYPE, RyosconfigLightFrame))
#define IS_RYOSCONFIG_LIGHT_FRAME(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOSCONFIG_LIGHT_FRAME_TYPE))

typedef struct _RyosconfigLightFrame RyosconfigLightFrame;

GType ryosconfig_light_frame_get_type(void);
GtkWidget *ryosconfig_light_frame_new(void);

void ryosconfig_light_frame_set_device_type(RyosconfigLightFrame *light_frame, RoccatDevice const *device);

void ryosconfig_light_frame_set_brightness(RyosconfigLightFrame *light_frame, guint new_brightness);

void ryosconfig_light_frame_set_from_rkp(RyosconfigLightFrame *light_frame, RyosRkp const *rkp);
void ryosconfig_light_frame_update_rkp(RyosconfigLightFrame *light_frame, RyosRkp *rkp);

G_END_DECLS

#endif
