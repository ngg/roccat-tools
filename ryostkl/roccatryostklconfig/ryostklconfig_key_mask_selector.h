#ifndef __RYOSTKLCONFIG_KEY_MASK_SELECTOR_H__
#define __RYOSTKLCONFIG_KEY_MASK_SELECTOR_H__

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

#define RYOSTKLCONFIG_KEY_MASK_SELECTOR_TYPE (ryostklconfig_key_mask_selector_get_type())
#define RYOSTKLCONFIG_KEY_MASK_SELECTOR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOSTKLCONFIG_KEY_MASK_SELECTOR_TYPE, RyostklconfigKeyMaskSelector))
#define IS_RYOSTKLCONFIG_KEY_MASK_SELECTOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOSTKLCONFIG_KEY_MASK_SELECTOR_TYPE))

typedef struct _RyostklconfigKeyMaskSelector RyostklconfigKeyMaskSelector;

GType ryostklconfig_key_mask_selector_get_type(void);
GtkWidget *ryostklconfig_key_mask_selector_new(void);

void ryostklconfig_key_mask_selector_set_from_profile_data(RyostklconfigKeyMaskSelector *key_mask, RyostklProfileData const *profile_data);
void ryostklconfig_key_mask_selector_update_profile_data(RyostklconfigKeyMaskSelector *key_mask, RyostklProfileData *profile_data);

G_END_DECLS

#endif
