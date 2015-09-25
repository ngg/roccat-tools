#ifndef __KONECONFIG_COLOR_FRAME_H__
#define __KONECONFIG_COLOR_FRAME_H__

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

#include "kone.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KONECONFIG_COLOR_FRAME_TYPE (koneconfig_color_frame_get_type())
#define KONECONFIG_COLOR_FRAME(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KONECONFIG_COLOR_FRAME_TYPE, KoneconfigColorFrame))
#define IS_KONECONFIG_COLOR_FRAME(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KONECONFIG_COLOR_FRAME_TYPE))

typedef struct _KoneconfigColorFrame KoneconfigColorFrame;

GType koneconfig_color_frame_get_type(void);
GtkWidget *koneconfig_color_frame_new(void);

void koneconfig_color_frame_set_from_rmp(KoneconfigColorFrame *frame, KoneRMP *rmp);
void koneconfig_color_frame_update_rmp(KoneconfigColorFrame *frame, KoneRMP *rmp);

G_END_DECLS

#endif
