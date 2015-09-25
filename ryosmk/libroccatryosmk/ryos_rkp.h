#ifndef __ROCCAT_RYOS_RKP_H__
#define __ROCCAT_RYOS_RKP_H__

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

#include "ryos.h"
#include "ryos_keys_primary.h"
#include "ryos_keys_function.h"
#include "ryos_keys_macro.h"
#include "ryos_keys_thumbster.h"
#include "ryos_keys_extra.h"
#include "ryos_keys_easyzone.h"
#include "ryos_key_mask.h"
#include "ryos_light.h"
#include "ryos_macro.h"
#include "ryos_led_macro.h"
#include "ryos_rkp_talk.h"
#include "ryos_rkp_timer.h"
#include "ryos_rkp_quicklaunch.h"
#include "ryos_stored_lights.h"

G_BEGIN_DECLS

typedef struct _RyosRkpData RyosRkpData;
typedef struct _RyosRkp RyosRkp;

struct _RyosRkpData {
	RyosKeysPrimary keys_primary;
	guint8 unused1;
	RyosKeysFunction keys_function;
	guint8 unused2;
	RyosKeysMacro keys_macro;
	guint8 unused3;
	RyosKeysThumbster keys_thumbster;
	guint8 unused4;
	RyosKeysExtra keys_extra;
	guint8 unused5;
	RyosKeysEasyzone keys_easyzone;
	guint8 unused6;
	RyosKeyMask key_mask;
	guint8 unused7;
	RyosLight light;
	guint8 unused8;
	Ryos14 _14;
	guint8 unused9;
	RyosMacro macros[RYOS_RKP_KEYS_NUM];
	guint8 unused10;
	guint16 profile_name[RYOS_RKP_PROFILE_NAME_LENGTH];
	guint32 character_repeat_delay;
	guint32 character_repeat_rate;
	guint8 unknown1;
	guint8 led_volume_indicator;
	guint8 sound_feedback_mask;
	guint8 sound_feedback_volume;
	guint8 unknown2[5];
	RyosRkpTalk talks[RYOS_RKP_TALK_NUM];
	guint8 unused11;
	guint16 gamefile_names[RYOS_RKP_PROFILE_GAMEFILE_NUM][RYOS_RKP_PROFILE_GAMEFILE_LENGTH];
	guint8 unknown3[5];
	RyosRkpTimer timers[RYOS_RKP_KEYS_NUM];
	guint8 unknown4[1070];
	RyosRkpQuicklaunch launchers[RYOS_RKP_KEYS_NUM];


	// guint8 unknown5[0x5c15];
	/* Windows drivers still doesn't store light macro in rkp */
	guint8 unknown5[0x5443];
	RyosMacro light_macro; /* used in dimness settings */


	RyosStoredLights lights_automatic;
	guint8 unused12;
	RyosStoredLights lights_manual;
	guint8 unused13;
	RyosLedMacro led_macros[RYOS_RKP_KEYS_NUM];
	guint8 unknown6[0xc9];
} __attribute__ ((packed)); // size 0x852fa.

typedef enum {
	RYOS_RKP_DATA_LED_VOLUME_INDICATOR_ON = 1,
	RYOS_RKP_DATA_LED_VOLUME_INDICATOR_OFF = 0,
} RyosRkpDataLedVolumeIndicator;

struct _RyosRkp {
	RyosRkpData data;
	gboolean modified_rkp;
	gboolean modified_keys_primary;
	gboolean modified_keys_function;
	gboolean modified_keys_macro;
	gboolean modified_keys_thumbster;
	gboolean modified_keys_extra;
	gboolean modified_keys_easyzone;
	gboolean modified_key_mask;
	gboolean modified_light;
	gboolean modified_macros[RYOS_RKP_KEYS_NUM];
	gboolean modified_light_macro;
	gboolean modified_lights_automatic;
	gboolean modified_lights_manual;
};

RyosRkp *ryos_rkp_new(void);
void ryos_rkp_free(RyosRkp *rkp);
RyosRkp *ryos_rkp_dup(RyosRkp const *other);
gboolean ryos_rkp_update_from_device(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error);
gboolean ryos_rkp_get_modified(RyosRkp const *rkp);
void ryos_rkp_set_modified(RyosRkp *rkp);
void ryos_rkp_set_unmodified(RyosRkp *rkp);

gboolean ryos_rkp_write_with_path(gchar const *path, RyosRkp *rkp, GError **error);
gboolean ryos_rkp_save_actual(RyosRkp *rkp, guint profile_index, GError **error);
gboolean ryos_rkp_save(RoccatDevice *ryos, RyosRkp *rkp, guint profile_index, GError **error);
RyosRkp *ryos_rkp_load_save_after_reset(RoccatDevice *ryos, guint profile_index, GError **error);
RyosRkp *ryos_rkp_read_with_path(gchar const *path, GError **error);
RyosRkp *ryos_rkp_load_actual(guint profile_index);
RyosRkp *ryos_rkp_load(RoccatDevice *ryos, guint profile_index, GError **error);

G_END_DECLS

#endif
