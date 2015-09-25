#ifndef __ROCCAT_SAVUCONFIG_WINDOW_H__
#define __ROCCAT_SAVUCONFIG_WINDOW_H__

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

#include "roccat_config_window.h"

G_BEGIN_DECLS

#define SAVUCONFIG_WINDOW_TYPE (savuconfig_window_get_type())
#define SAVUCONFIG_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SAVUCONFIG_WINDOW_TYPE, SavuconfigWindow))
#define IS_SAVUCONFIG_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SAVUCONFIG_WINDOW))

typedef struct _SavuconfigWindow SavuconfigWindow;

GType savuconfig_window_get_type(void);
GtkWidget *savuconfig_window_new(void);

G_END_DECLS

#endif
