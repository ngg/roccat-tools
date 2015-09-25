#ifndef __ROCCAT_RYOS_RKP_ACCESSORS_H__
#define __ROCCAT_RYOS_RKP_ACCESSORS_H__

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with roccat-tools. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ryos_rkp.h"
#include "ryos_key_mask.h"
#include "ryos_keys_easyzone.h"
#include "ryos_keys_extra.h"
#include "ryos_keys_function.h"
#include "ryos_keys_macro.h"
#include "ryos_keys_primary.h"
#include "ryos_keys_thumbster.h"
#include "ryos_led_macro.h"

G_BEGIN_DECLS

gchar *ryos_rkp_get_profile_name(RyosRkp const *rkp);
gchar *ryos_rkp_get_gamefile_name(RyosRkp const *rkp, guint index);
RyosKeysPrimary *ryos_rkp_get_keys_primary(RyosRkp const *rkp);
RyosKeysFunction *ryos_rkp_get_keys_function(RyosRkp const *rkp);
RyosKeysMacro *ryos_rkp_get_keys_macro(RyosRkp const *rkp);
RyosKeysThumbster *ryos_rkp_get_keys_thumbster(RyosRkp const *rkp);
RyosKeysExtra *ryos_rkp_get_keys_extra(RyosRkp const *rkp);
RyosKeysEasyzone *ryos_rkp_get_keys_easyzone(RyosRkp const *rkp);
RyosKeyMask *ryos_rkp_get_key_mask(RyosRkp const *rkp);
RyosLight *ryos_rkp_get_light(RyosRkp const *rkp);
RyosMacro *ryos_rkp_get_light_macro(RyosRkp const *rkp);
RyosMacro *ryos_rkp_get_macro(RyosRkp const *rkp, guint index);
RyosRkpTimer *ryos_rkp_get_timer(RyosRkp const *rkp, guint index);
RyosRkpQuicklaunch *ryos_rkp_get_quicklaunch(RyosRkp const *rkp, guint index);
RyosRkpTalk *ryos_rkp_get_talk(RyosRkp const *rkp, guint index);
RyosStoredLights *ryos_rkp_get_lights_automatic(RyosRkp const *rkp);
RyosStoredLights *ryos_rkp_get_lights_manual(RyosRkp const *rkp);

void ryos_rkp_set_profile_name(RyosRkp *rkp, gchar const *name);
void ryos_rkp_set_gamefile_name(RyosRkp *rkp, guint index, gchar const *name);
void ryos_rkp_set_keys_primary(RyosRkp *rkp, RyosKeysPrimary const *keys_primary);
void ryos_rkp_set_keys_function(RyosRkp *rkp, RyosKeysFunction const *keys_function);
void ryos_rkp_set_keys_macro(RyosRkp *rkp, RyosKeysMacro const *keys_macro);
void ryos_rkp_set_keys_thumbster(RyosRkp *rkp, RyosKeysThumbster const *keys_thumbster);
void ryos_rkp_set_keys_extra(RyosRkp *rkp, RyosKeysExtra const *keys_extra);
void ryos_rkp_set_keys_easyzone(RyosRkp *rkp, RyosKeysEasyzone const *keys_easyzone);
void ryos_rkp_set_key_to_macro_without_modified(RyosRkp *rkp, guint macro_index);
void ryos_rkp_set_key_to_macro(RyosRkp *rkp, guint macro_index);
void ryos_rkp_set_key_mask(RyosRkp *rkp, RyosKeyMask const *key_mask);
void ryos_rkp_set_light(RyosRkp *rkp, RyosLight const *light);
void ryos_rkp_set_light_macro(RyosRkp *rkp, RyosMacro const *light_macro);
void ryos_rkp_set_light_brightness_without_modified(RyosRkp *rkp, guint8 new_brightness);
void ryos_rkp_set_light_brightness(RyosRkp *rkp, guint8 new_brightness);
void ryos_rkp_set_light_dimness(RyosRkp *rkp, guint8 new_dimness);
void ryos_rkp_set_light_dimness_type(RyosRkp *rkp, guint8 new_type);
void ryos_rkp_set_light_timeout(RyosRkp *rkp, guint8 new_timeout);
void ryos_rkp_set_light_macro_exec_feedback(RyosRkp *rkp, guint8 new_feedback);
void ryos_rkp_set_light_illumination_mode(RyosRkp *rkp, guint8 new_illumination_mode);
void ryos_rkp_set_macro_without_modified(RyosRkp *rkp, guint index, RyosMacro const *macro);
void ryos_rkp_set_macro(RyosRkp *rkp, guint index, RyosMacro const *macro);
void ryos_rkp_set_led_macro(RyosRkp *rkp, guint index, RyosLedMacro const *led_macro);
void ryos_rkp_set_timer(RyosRkp *rkp, guint index, RyosRkpTimer const *timer);
void ryos_rkp_set_quicklaunch(RyosRkp *rkp, guint index, RyosRkpQuicklaunch const *quicklaunch);
void ryos_rkp_set_talk(RyosRkp *rkp, guint index, RyosRkpTalk const *talk);
void ryos_rkp_set_led_volume_indicator(RyosRkp *rkp, guint8 new_value);
void ryos_rkp_set_lights_automatic(RyosRkp *rkp, RyosStoredLights *lights);
void ryos_rkp_set_lights_manual(RyosRkp *rkp, RyosStoredLights *lights);

G_END_DECLS

#endif
