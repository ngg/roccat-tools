#ifndef __ROCCAT_NYTH_DCU_FRAME_H__
#define __ROCCAT_NYTH_DCU_FRAME_H__

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

#define NYTH_DCU_FRAME_TYPE (nyth_dcu_frame_get_type())
#define NYTH_DCU_FRAME(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), NYTH_DCU_FRAME_TYPE, NythDcuFrame))
#define IS_NYTH_DCU_FRAME(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), NYTH_DCU_FRAME_TYPE))

typedef struct _NythDcuFrame NythDcuFrame;

GType nyth_dcu_frame_get_type(void);
GtkWidget *nyth_dcu_frame_new(void);

guint nyth_dcu_frame_get_value(NythDcuFrame *frame);

/* does not emit changed signal */
void nyth_dcu_frame_set_value_blocked(NythDcuFrame *frame, guint value);

G_END_DECLS

#endif
