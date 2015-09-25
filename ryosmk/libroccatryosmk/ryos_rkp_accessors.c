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

#include "ryos_rkp_accessors.h"
#include "roccat_helper.h"
#include <string.h>

gchar *ryos_rkp_get_profile_name(RyosRkp const *rkp) {
	return g_utf16_to_utf8((gunichar2 const *)(rkp->data.profile_name), RYOS_RKP_PROFILE_NAME_LENGTH, NULL, NULL, NULL);
}

void ryos_rkp_set_profile_name(RyosRkp *rkp, gchar const *name) {
	gunichar2 *string;
	glong items;

	string = g_utf8_to_utf16(name, -1L, NULL, &items, NULL);
	if (string == NULL)
		return;

	/* make sure target has trailing 0 */
	items = MIN(RYOS_RKP_PROFILE_NAME_LENGTH - 1, items);
	string[items] = 0;

	if (memcmp(&rkp->data.profile_name, string, (items + 1) * 2)) {
		memset(&rkp->data.profile_name, 0, RYOS_RKP_PROFILE_NAME_LENGTH * 2);
		memcpy(&rkp->data.profile_name, string, items * 2);
		rkp->modified_rkp = TRUE;
	}
	g_free(string);
}

gchar *ryos_rkp_get_gamefile_name(RyosRkp const *rkp, guint index) {
	return g_utf16_to_utf8((gunichar2 const *)(rkp->data.gamefile_names[index]), RYOS_RKP_PROFILE_GAMEFILE_LENGTH, NULL, NULL, NULL);
}

void ryos_rkp_set_gamefile_name(RyosRkp *rkp, guint index, gchar const *name) {
	gunichar2 *string;
	glong items;

	string = g_utf8_to_utf16(name, -1L, NULL, &items, NULL);
	if (string == NULL)
		return;

	items = MIN(RYOS_RKP_PROFILE_GAMEFILE_LENGTH - 1, items);
	string[items] = 0;

	if (memcmp(&rkp->data.gamefile_names[index], string, (items + 1) * 2)) {
		memset(&rkp->data.gamefile_names[index], 0, RYOS_RKP_PROFILE_GAMEFILE_LENGTH * 2);
		memcpy(&rkp->data.gamefile_names[index], string, items * 2);
		rkp->modified_rkp = TRUE;
	}
	g_free(string);
}

RyosKeysPrimary *ryos_rkp_get_keys_primary(RyosRkp const *rkp) {
	RyosKeysPrimary *keys_primary;
	keys_primary = (RyosKeysPrimary *)g_malloc(sizeof(RyosKeysPrimary));
	ryos_keys_primary_copy(keys_primary, &rkp->data.keys_primary);
	return keys_primary;
}

void ryos_rkp_set_keys_primary(RyosRkp *rkp, RyosKeysPrimary const *keys_primary) {
	if (!ryos_keys_primary_equal(&rkp->data.keys_primary, keys_primary)) {
		ryos_keys_primary_copy(&rkp->data.keys_primary, keys_primary);
		rkp->modified_keys_primary = TRUE;
	}
}

RyosKeysFunction *ryos_rkp_get_keys_function(RyosRkp const *rkp) {
	RyosKeysFunction *keys_function;
	keys_function = (RyosKeysFunction *)g_malloc(sizeof(RyosKeysFunction));
	ryos_keys_function_copy(keys_function, &rkp->data.keys_function);
	return keys_function;
}

void ryos_rkp_set_keys_function(RyosRkp *rkp, RyosKeysFunction const *keys_function) {
	if (!ryos_keys_function_equal(&rkp->data.keys_function, keys_function)) {
		ryos_keys_function_copy(&rkp->data.keys_function, keys_function);
		rkp->modified_keys_function = TRUE;
	}
}

RyosKeysMacro *ryos_rkp_get_keys_macro(RyosRkp const *rkp) {
	RyosKeysMacro *keys_macro;
	keys_macro = (RyosKeysMacro *)g_malloc(sizeof(RyosKeysMacro));
	ryos_keys_macro_copy(keys_macro, &rkp->data.keys_macro);
	return keys_macro;
}

void ryos_rkp_set_keys_macro(RyosRkp *rkp, RyosKeysMacro const *keys_macro) {
	if (!ryos_keys_macro_equal(&rkp->data.keys_macro, keys_macro)) {
		ryos_keys_macro_copy(&rkp->data.keys_macro, keys_macro);
		rkp->modified_keys_macro = TRUE;
	}
}

RyosKeysThumbster *ryos_rkp_get_keys_thumbster(RyosRkp const *rkp) {
	RyosKeysThumbster *keys_thumbster;
	keys_thumbster = (RyosKeysThumbster *)g_malloc(sizeof(RyosKeysThumbster));
	ryos_keys_thumbster_copy(keys_thumbster, &rkp->data.keys_thumbster);
	return keys_thumbster;
}

void ryos_rkp_set_keys_thumbster(RyosRkp *rkp, RyosKeysThumbster const *keys_thumbster) {
	if (!ryos_keys_thumbster_equal(&rkp->data.keys_thumbster, keys_thumbster)) {
		ryos_keys_thumbster_copy(&rkp->data.keys_thumbster, keys_thumbster);
		rkp->modified_keys_thumbster = TRUE;
	}
}

RyosKeysExtra *ryos_rkp_get_keys_extra(RyosRkp const *rkp) {
	RyosKeysExtra *keys_extra;
	keys_extra = (RyosKeysExtra *)g_malloc(sizeof(RyosKeysExtra));
	ryos_keys_extra_copy(keys_extra, &rkp->data.keys_extra);
	return keys_extra;
}

void ryos_rkp_set_keys_extra(RyosRkp *rkp, RyosKeysExtra const *keys_extra) {
	if (!ryos_keys_extra_equal(&rkp->data.keys_extra, keys_extra)) {
		ryos_keys_extra_copy(&rkp->data.keys_extra, keys_extra);
		rkp->modified_keys_extra = TRUE;
	}
}

RyosKeysEasyzone *ryos_rkp_get_keys_easyzone(RyosRkp const *rkp) {
	RyosKeysEasyzone *keys_easyzone;
	keys_easyzone = (RyosKeysEasyzone *)g_malloc(sizeof(RyosKeysEasyzone));
	ryos_keys_easyzone_copy(keys_easyzone, &rkp->data.keys_easyzone);
	return keys_easyzone;
}

void ryos_rkp_set_keys_easyzone(RyosRkp *rkp, RyosKeysEasyzone const *keys_easyzone) {
	if (!ryos_keys_easyzone_equal(&rkp->data.keys_easyzone, keys_easyzone)) {
		ryos_keys_easyzone_copy(&rkp->data.keys_easyzone, keys_easyzone);
		rkp->modified_keys_easyzone = TRUE;
	}
}

void ryos_rkp_set_key_to_macro_without_modified(RyosRkp *rkp, guint macro_index) {
	if (ryos_macro_index_is_keys_easyzone(macro_index)) {
		ryos_key_set_to_normal(&rkp->data.keys_easyzone.keys[ryos_macro_index_to_keys_easyzone_index(macro_index)], RYOS_KEY_TYPE_MACRO);
	} else if (ryos_macro_index_is_keys_macro(macro_index)) {
		ryos_key_set_to_normal(&rkp->data.keys_macro.keys[ryos_macro_index_to_keys_macro_index(macro_index)], RYOS_KEY_TYPE_MACRO);
	} else if (ryos_macro_index_is_keys_thumbster(macro_index)) {
		ryos_key_set_to_normal(&rkp->data.keys_thumbster.keys[ryos_macro_index_to_keys_thumbster_index(macro_index)], RYOS_KEY_TYPE_MACRO);
	} else {
		; // error
	}
}

void ryos_rkp_set_key_to_macro(RyosRkp *rkp, guint macro_index) {
	RyosKeysEasyzone *keys_easyzone;
	RyosKeysMacro *keys_macro;
	RyosKeysThumbster *keys_thumbster;

	if (ryos_macro_index_is_keys_easyzone(macro_index)) {
		keys_easyzone = ryos_rkp_get_keys_easyzone(rkp);
		ryos_key_set_to_normal(&keys_easyzone->keys[ryos_macro_index_to_keys_easyzone_index(macro_index)], RYOS_KEY_TYPE_MACRO);
		ryos_rkp_set_keys_easyzone(rkp, keys_easyzone);
		g_free(keys_easyzone);
	} else if (ryos_macro_index_is_keys_macro(macro_index)) {
		keys_macro = ryos_rkp_get_keys_macro(rkp);
		ryos_key_set_to_normal(&keys_macro->keys[ryos_macro_index_to_keys_macro_index(macro_index)], RYOS_KEY_TYPE_MACRO);
		ryos_rkp_set_keys_macro(rkp, keys_macro);
		g_free(keys_macro);
	} else if (ryos_macro_index_is_keys_thumbster(macro_index)) {
		keys_thumbster = ryos_rkp_get_keys_thumbster(rkp);
		ryos_key_set_to_normal(&keys_thumbster->keys[ryos_macro_index_to_keys_thumbster_index(macro_index)], RYOS_KEY_TYPE_MACRO);
		ryos_rkp_set_keys_thumbster(rkp, keys_thumbster);
		g_free(keys_thumbster);
	} else {
		; // error
	}
}

RyosKeyMask *ryos_rkp_get_key_mask(RyosRkp const *rkp) {
	RyosKeyMask *key_mask;
	key_mask = (RyosKeyMask *)g_malloc(sizeof(RyosKeyMask));
	ryos_key_mask_copy(key_mask, &rkp->data.key_mask);
	return key_mask;
}

void ryos_rkp_set_key_mask(RyosRkp *rkp, RyosKeyMask const *key_mask) {
	if (!ryos_key_mask_equal(&rkp->data.key_mask, key_mask)) {
		ryos_key_mask_copy(&rkp->data.key_mask, key_mask);
		rkp->modified_key_mask = TRUE;
	}
}

RyosLight *ryos_rkp_get_light(RyosRkp const *rkp) {
	RyosLight *light;
	light = (RyosLight *)g_malloc(sizeof(RyosLight));
	ryos_light_copy(light, &rkp->data.light);
	return light;
}

void ryos_rkp_set_light_brightness_without_modified(RyosRkp *rkp, guint8 new_brightness) {
		rkp->data.light.brightness = new_brightness;
}

void ryos_rkp_set_light_brightness(RyosRkp *rkp, guint8 new_brightness) {
	if (rkp->data.light.brightness != new_brightness) {
		ryos_rkp_set_light_brightness_without_modified(rkp, new_brightness);
		rkp->modified_light = TRUE;
	}
}

void ryos_rkp_set_light_dimness(RyosRkp *rkp, guint8 new_dimness) {
	if (rkp->data.light.dimness != new_dimness) {
		rkp->data.light.dimness = new_dimness;
		rkp->modified_light = TRUE;
	}
}

void ryos_rkp_set_light_dimness_type(RyosRkp *rkp, guint8 new_type) {
	if (rkp->data.light.dimness_type != new_type) {
		rkp->data.light.dimness_type = new_type;
		rkp->modified_light = TRUE;
	}
}

void ryos_rkp_set_light_timeout(RyosRkp *rkp, guint8 new_timeout) {
	if (rkp->data.light.timeout != new_timeout) {
		rkp->data.light.timeout = new_timeout;
		rkp->modified_light = TRUE;
	}
}

void ryos_rkp_set_light_macro_exec_feedback(RyosRkp *rkp, guint8 new_feedback) {
	if (rkp->data.light.macro_exec_feedback != new_feedback) {
		rkp->data.light.macro_exec_feedback = new_feedback;
		rkp->modified_light = TRUE;
	}
}

void ryos_rkp_set_light_illumination_mode(RyosRkp *rkp, guint8 new_illumination_mode) {
	if (rkp->data.light.illumination_mode != new_illumination_mode) {
		rkp->data.light.illumination_mode = new_illumination_mode;
		rkp->modified_light = TRUE;
	}
}

void ryos_rkp_set_light(RyosRkp *rkp, RyosLight const *light) {
	if (!ryos_light_equal(&rkp->data.light, light)) {
		ryos_light_copy(&rkp->data.light, light);
		rkp->modified_light = TRUE;
	}
}

RyosMacro *ryos_rkp_get_light_macro(RyosRkp const *rkp) {
	return (RyosMacro *)ryos_macro_dup(&rkp->data.light_macro);
}

void ryos_rkp_set_light_macro(RyosRkp *rkp, RyosMacro const *light_macro) {
	if (!ryos_macro_equal(&rkp->data.light_macro, light_macro)) {
		ryos_macro_copy(&rkp->data.light_macro, light_macro);
		rkp->modified_light_macro = TRUE;
	}
}

RyosStoredLights *ryos_rkp_get_lights_automatic(RyosRkp const *rkp) {
	RyosStoredLights *lights;
	lights = (RyosStoredLights *)g_malloc(sizeof(RyosStoredLights));
	ryos_stored_lights_copy(lights, &rkp->data.lights_automatic);
	return lights;
}

void ryos_rkp_set_lights_automatic(RyosRkp *rkp, RyosStoredLights *lights) {
	if (!ryos_stored_lights_equal(&rkp->data.lights_automatic, lights)) {
		ryos_stored_lights_copy(&rkp->data.lights_automatic, lights);
		rkp->modified_lights_automatic = TRUE;
	}
}

RyosStoredLights *ryos_rkp_get_lights_manual(RyosRkp const *rkp) {
	RyosStoredLights *lights;
	lights = (RyosStoredLights *)g_malloc(sizeof(RyosStoredLights));
	ryos_stored_lights_copy(lights, &rkp->data.lights_manual);
	return lights;
}

void ryos_rkp_set_lights_manual(RyosRkp *rkp, RyosStoredLights *lights) {
	if (!ryos_stored_lights_equal(&rkp->data.lights_manual, lights)) {
		ryos_stored_lights_copy(&rkp->data.lights_manual, lights);
		rkp->modified_lights_manual = TRUE;
	}
}

RyosMacro *ryos_rkp_get_macro(RyosRkp const *rkp, guint index) {
	return (RyosMacro *)g_memdup(&rkp->data.macros[index], sizeof(RyosMacro));
}

void ryos_rkp_set_macro_without_modified(RyosRkp *rkp, guint index, RyosMacro const *macro) {
	ryos_macro_copy(&rkp->data.macros[index], macro);
}

void ryos_rkp_set_macro(RyosRkp *rkp, guint index, RyosMacro const *macro) {
	if (!ryos_macro_equal(&rkp->data.macros[index], macro)) {
		ryos_rkp_set_macro_without_modified(rkp, index, macro);
		rkp->modified_macros[index] = TRUE;
	}
}

void ryos_rkp_set_led_macro(RyosRkp *rkp, guint index, RyosLedMacro const *led_macro) {
	if (!ryos_led_macro_equal(&rkp->data.led_macros[index], led_macro)) {
		ryos_led_macro_copy(&rkp->data.led_macros[index], led_macro);
		rkp->modified_rkp = TRUE;
	}
}

RyosRkpTimer *ryos_rkp_get_timer(RyosRkp const *rkp, guint index) {
	return ryos_rkp_timer_dup(&rkp->data.timers[index]);
}

void ryos_rkp_set_timer(RyosRkp *rkp, guint index, RyosRkpTimer const *timer) {
	if (!ryos_rkp_timer_equal(&rkp->data.timers[index], timer)) {
		ryos_rkp_timer_copy(&rkp->data.timers[index], timer);
		rkp->modified_rkp = TRUE;
	}
}

RyosRkpQuicklaunch *ryos_rkp_get_quicklaunch(RyosRkp const *rkp, guint index) {
	return ryos_rkp_quicklaunch_dup(&rkp->data.launchers[index]);
}

void ryos_rkp_set_quicklaunch(RyosRkp *rkp, guint index, RyosRkpQuicklaunch const *quicklaunch) {
	if (!ryos_rkp_quicklaunch_equal(&rkp->data.launchers[index], quicklaunch)) {
		ryos_rkp_quicklaunch_copy(&rkp->data.launchers[index], quicklaunch);
		rkp->modified_rkp = TRUE;
	}
}

RyosRkpTalk *ryos_rkp_get_talk(RyosRkp const *rkp, guint index) {
	return ryos_rkp_talk_dup(&rkp->data.talks[index]);
}

void ryos_rkp_set_talk(RyosRkp *rkp, guint index, RyosRkpTalk const *talk) {
	if (!ryos_rkp_talk_equal(&rkp->data.talks[index], talk)) {
		ryos_rkp_talk_copy(&rkp->data.talks[index], talk);
		rkp->modified_rkp = TRUE;
	}
}

void ryos_rkp_set_led_volume_indicator(RyosRkp *rkp, guint8 new_value) {
	if (rkp->data.led_volume_indicator != new_value) {
		rkp->data.led_volume_indicator = new_value;
		rkp->modified_rkp = TRUE;
	}
}
