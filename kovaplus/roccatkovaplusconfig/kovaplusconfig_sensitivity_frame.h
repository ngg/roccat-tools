#ifndef __KOVAPLUSCONFIG_SENSITIVITY_FRAME_H__
#define __KOVAPLUSCONFIG_SENSITIVITY_FRAME_H__

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

#include "kovaplus.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KOVAPLUSCONFIG_SENSITIVITY_FRAME_TYPE (kovaplusconfig_sensitivity_frame_get_type())
#define KOVAPLUSCONFIG_SENSITIVITY_FRAME(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KOVAPLUSCONFIG_SENSITIVITY_FRAME_TYPE, KovaplusconfigSensitivityFrame))
#define IS_KOVAPLUSCONFIG_SENSITIVITY_FRAME(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KOVAPLUSCONFIG_SENSITIVITY_FRAME_TYPE))

typedef struct _KovaplusconfigSensitivityFrame KovaplusconfigSensitivityFrame;

GType kovaplusconfig_sensitivity_frame_get_type(void);
GtkWidget *kovaplusconfig_sensitivity_frame_new(void);

void kovaplusconfig_sensitivity_frame_set_from_rmp(KovaplusconfigSensitivityFrame *frame, KovaplusRmp *rmp);
void kovaplusconfig_sensitivity_frame_update_rmp(KovaplusconfigSensitivityFrame *frame, KovaplusRmp *rmp);

G_END_DECLS

#endif
