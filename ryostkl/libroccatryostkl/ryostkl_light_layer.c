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

#include "ryostkl_light_layer.h"
#include "ryostkl_device.h"

enum {
	RYOSTKL_LIGHT_LAYER_KEY_BLINK_SPEED_MASK = 0x001c,
	RYOSTKL_LIGHT_LAYER_KEY_BLINK_SPEED_SHIFT = 2,
	RYOSTKL_LIGHT_LAYER_KEY_BREATH_SPEED_MASK = 0x01c0,
	RYOSTKL_LIGHT_LAYER_KEY_BREATH_SPEED_SHIFT = 6,
	RYOSTKL_LIGHT_LAYER_KEY_EFFECT_MASK = 0x3000,
	RYOSTKL_LIGHT_LAYER_KEY_EFFECT_SHIFT = 12,
	RYOSTKL_LIGHT_LAYER_KEY_BIT_STATE = 15,
};


static guint16 ryostkl_light_layer_calc_checksum(RyostklLightLayer const *light_layer) {
	return ROCCAT_BYTESUM_PARTIALLY(light_layer, RyostklLightLayer, report_id, checksum);
}

static void ryostkl_light_layer_set_checksum(RyostklLightLayer *light_layer, guint16 new_value) {
	light_layer->checksum = GUINT16_TO_LE(new_value);
}

static void ryostkl_light_layer_finalize(RyostklLightLayer *light_layer, guint profile_index, RyosIlluminationMode mode, RyosStoredLightsLayers layer) {
	light_layer->report_id = RYOS_REPORT_ID_STORED_LIGHTS;
	light_layer->size = sizeof(RyostklLightLayer);
	light_layer->profile_index = profile_index;
	light_layer->illumination_mode = mode;
	light_layer->layer = layer;
	ryostkl_light_layer_set_checksum(light_layer, ryostkl_light_layer_calc_checksum(light_layer));
}

gboolean ryostkl_light_layer_write(RoccatDevice *device, guint profile_index, RyosIlluminationMode mode, RyosStoredLightsLayers layer, RyostklLightLayer *light_layer, GError **error) {
	g_assert(profile_index < RYOS_PROFILE_NUM);
	ryostkl_light_layer_finalize(light_layer, profile_index, mode, layer);
	return ryos_device_write(device, (gchar const *)light_layer, sizeof(RyostklLightLayer), error);
}

RyostklLightLayer *ryostkl_light_layer_read(RoccatDevice *device, guint profile_index, RyosIlluminationMode mode, RyosStoredLightsLayers layer, GError **error) {
	RyostklLightLayer *light_layer;
	guint request;

	g_assert(profile_index < RYOS_PROFILE_NUM);

	gaminggear_device_lock(GAMINGGEAR_DEVICE(device));

	request = (mode == RYOS_ILLUMINATION_MODE_AUTOMATIC) ? RYOSTKL_CONTROL_REQUEST_STORED_LIGHTS_AUTOMATIC_BASE : RYOSTKL_CONTROL_REQUEST_STORED_LIGHTS_MANUAL_BASE;
	request |= layer;

	if (!ryos_select(device, profile_index, request, error)) {
		gaminggear_device_unlock(GAMINGGEAR_DEVICE(device));
		return NULL;
	}

	light_layer = (RyostklLightLayer *)ryos_device_read(device, RYOS_REPORT_ID_STORED_LIGHTS, sizeof(RyostklLightLayer), error);

	gaminggear_device_unlock(GAMINGGEAR_DEVICE(device));

	return light_layer;
}

gboolean ryostkl_light_layer_equal(RyostklLightLayer const *left, RyostklLightLayer const *right) {
	gboolean equal;
	equal = ROCCAT_MEMCMP_PARTIALLY(left, right, RyostklLightLayer, effect, checksum);
	return equal ? FALSE : TRUE;
}

void ryostkl_light_layer_copy(RyostklLightLayer *destination, RyostklLightLayer const *source) {
	memcpy(destination, source, sizeof(RyostklLightLayer));
}

RyostklLightLayer *ryostkl_light_layer_dup(RyostklLightLayer const *source) {
	return (RyostklLightLayer *)g_memdup(source, sizeof(RyostklLightLayer));
}

guint8 ryostkl_light_layer_key_to_thumb(guint16 const *key) {
	guint8 thumb = 0;
	ryostkl_light_layer_thumb_set_state(&thumb, ryostkl_light_layer_key_get_state(key));
	return thumb;
}

guint16 ryostkl_light_layer_thumb_to_key(guint8 const *thumb) {
	guint16 key = 0;
	ryostkl_light_layer_key_set_state(&key, ryostkl_light_layer_thumb_get_state(thumb));
	return key;
}

guint ryostkl_light_layer_key_get_blink_speed(guint16 const *key) {
	return (GUINT16_FROM_BE(*key) & RYOSTKL_LIGHT_LAYER_KEY_BLINK_SPEED_MASK) >> RYOSTKL_LIGHT_LAYER_KEY_BLINK_SPEED_SHIFT;
}

void ryostkl_light_layer_key_set_blink_speed(guint16 *key, guint speed) {
	guint16 tmp = GUINT16_FROM_BE(*key);
	tmp &= ~RYOSTKL_LIGHT_LAYER_KEY_BLINK_SPEED_MASK;
	tmp |= speed << RYOSTKL_LIGHT_LAYER_KEY_BLINK_SPEED_SHIFT;
	*key = GUINT16_TO_BE(tmp);
}

guint ryostkl_light_layer_key_get_breath_speed(guint16 const *key) {
	return (GUINT16_FROM_BE(*key) & RYOSTKL_LIGHT_LAYER_KEY_BREATH_SPEED_MASK) >> RYOSTKL_LIGHT_LAYER_KEY_BREATH_SPEED_SHIFT;
}

void ryostkl_light_layer_key_set_breath_speed(guint16 *key, guint speed) {
	guint16 tmp = GUINT16_FROM_BE(*key);
	tmp &= ~RYOSTKL_LIGHT_LAYER_KEY_BREATH_SPEED_MASK;
	tmp |= speed << RYOSTKL_LIGHT_LAYER_KEY_BREATH_SPEED_SHIFT;
	*key = GUINT16_TO_BE(tmp);
}

guint ryostkl_light_layer_key_get_effect(guint16 const *key) {
	return (GUINT16_FROM_BE(*key) & RYOSTKL_LIGHT_LAYER_KEY_EFFECT_MASK) >> RYOSTKL_LIGHT_LAYER_KEY_EFFECT_SHIFT;
}

void ryostkl_light_layer_key_set_effect(guint16 *key, guint effect) {
	guint16 tmp = GUINT16_FROM_BE(*key);
	tmp &= ~RYOSTKL_LIGHT_LAYER_KEY_EFFECT_MASK;
	tmp |= effect << RYOSTKL_LIGHT_LAYER_KEY_EFFECT_SHIFT;
	*key = GUINT16_TO_BE(tmp);
}

gboolean ryostkl_light_layer_key_get_state(guint16 const *key) {
	return roccat_get_bit16(GUINT16_FROM_BE(*key), RYOSTKL_LIGHT_LAYER_KEY_BIT_STATE);
}

void ryostkl_light_layer_key_set_state(guint16 *key, gboolean state) {
	guint16 tmp = GUINT16_FROM_BE(*key);
	roccat_set_bit16(&tmp, RYOSTKL_LIGHT_LAYER_KEY_BIT_STATE, state);
	*key = GUINT16_TO_BE(tmp);
}

void ryostkl_light_layer_to_custom_lights(RyostklLightLayer const *light_layer, RyosCustomLights *custom_lights) {
	guint i;

	for (i = 0; i < RYOS_KEYS_PRIMARY_NUM; ++i)
		roccat_bitfield_set_bit(&custom_lights->data[0], i, ryostkl_light_layer_key_get_state(&light_layer->keys[i]));
}

guint16 ryostkl_light_layer_key_create(gboolean state, guint effect, guint blink_speed, guint breath_speed) {
	guint16 key = 0;
	ryostkl_light_layer_key_set_state(&key, state);
	if (state) {
		ryostkl_light_layer_key_set_effect(&key, effect);
		if (effect == RYOSTKL_LIGHT_LAYER_KEY_EFFECT_BLINK)
			ryostkl_light_layer_key_set_blink_speed(&key, blink_speed);
		else if (effect == RYOSTKL_LIGHT_LAYER_KEY_EFFECT_BREATH)
			ryostkl_light_layer_key_set_breath_speed(&key, breath_speed);
	}
	return key;
}
