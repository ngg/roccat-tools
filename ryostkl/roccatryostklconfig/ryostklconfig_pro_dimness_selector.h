#ifndef __RYOSTKLCONFIG_PRO_DIMNESS_SELECTOR_H__
#define __RYOSTKLCONFIG_PRO_DIMNESS_SELECTOR_H__

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

#include "ryostkl_profile_data.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define RYOSTKLCONFIG_PRO_DIMNESS_SELECTOR_TYPE (ryostklconfig_pro_dimness_selector_get_type())
#define RYOSTKLCONFIG_PRO_DIMNESS_SELECTOR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOSTKLCONFIG_PRO_DIMNESS_SELECTOR_TYPE, RyostklconfigProDimnessSelector))
#define IS_RYOSTKLCONFIG_PRO_DIMNESS_SELECTOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOSTKLCONFIG_PRO_DIMNESS_SELECTOR_TYPE))

typedef struct _RyostklconfigProDimnessSelector RyostklconfigProDimnessSelector;

GType ryostklconfig_pro_dimness_selector_get_type(void);
GtkWidget *ryostklconfig_pro_dimness_selector_new(void);

void ryostklconfig_pro_dimness_selector_set_device_type(RyostklconfigProDimnessSelector *dimness_selector, RoccatDevice const *device);

void ryostklconfig_pro_dimness_selector_set_from_profile_data(RyostklconfigProDimnessSelector *dimness_selector, RyostklProfileData const *profile_data);
void ryostklconfig_pro_dimness_selector_update_profile_data(RyostklconfigProDimnessSelector *dimness_selector, RyostklProfileData *profile_data);

G_END_DECLS

#endif
