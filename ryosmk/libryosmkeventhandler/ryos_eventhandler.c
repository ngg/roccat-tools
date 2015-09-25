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

#include <gaminggear/hid_uid.h>
#include <gaminggear/macro.h>
#include "ryos_eventhandler_channel.h"
#include "ryos_dbus_server.h"
#include "roccat_eventhandler_plugin.h"
#include "roccat_process_helper.h"
#include "roccat_notification_profile.h"
#include "roccat_notification_live_recording.h"
#include "roccat_notification_timer.h"
#include "roccat_talk.h"
#include "ryos_device_state.h"
#include "ryos_talk.h"
#include "ryos_ripple_c.h"
#include "ryos_ripple_lua.h"
#include "ryos_config.h"
#include "ryos_sdk.h"
#include "ryos_rkp.h"
#include "ryos_rkp_accessors.h"
#include "ryos_profile.h"
#include "ryos_device.h"
#include "ryos_led_macro_thread.h"
#include "config.h"
#include "i18n-lib.h"
#include "g_roccat_helper.h"
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

GType ryos_eventhandler_get_type(void);

#define RYOS_EVENTHANDLER_TYPE (ryos_eventhandler_get_type())
#define RYOS_EVENTHANDLER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RYOS_EVENTHANDLER_TYPE, RyosEventhandler))
#define IS_RYOS_EVENTHANDLER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RYOS_EVENTHANDLER_TYPE))
#define RYOS_EVENTHANDLER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOS_EVENTHANDLER_TYPE, RyosEventhandlerClass))
#define IS_RYOS_EVENTHANDLER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOS_EVENTHANDLER_TYPE))
#define RYOS_EVENTHANDLER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOS_EVENTHANDLER_TYPE, RyosEventhandlerPrivate))

typedef struct _RyosEventhandler RyosEventhandler;
typedef struct _RyosEventhandlerClass RyosEventhandlerClass;
typedef struct _RyosEventhandlerPrivate RyosEventhandlerPrivate;

struct _RyosEventhandler {
	GObject parent;
	RyosEventhandlerPrivate *priv;
};

struct _RyosEventhandlerClass {
	GObjectClass parent_class;
};

struct _RyosEventhandlerPrivate {
	RoccatEventhandlerHost *host;
	RyosDBusServer *dbus_server;
	RoccatDeviceScanner *device_scanner;
	RoccatDevice *device;
	gboolean device_set_up;
	RoccatKeyFile *config;
	guint actual_profile_index;
	RyosRkp *rkp[RYOS_PROFILE_NUM];
	RyosEventhandlerChannel *channel;
	gulong active_window_changed_handler;

	RoccatNotificationProfile *profile_note;
	RoccatNotificationTimer *timer_note;
	RoccatNotificationLiveRecording *live_recording_note;

	RyosRipple *ripple;

	RyosRkpTalk active_talk;
	RyosLedMacroThread *led_macro_thread;
};

static void profile_changed(RyosEventhandler *eventhandler, guint profile_index) {
	RyosEventhandlerPrivate *priv = eventhandler->priv;

	priv->actual_profile_index = profile_index;
	roccat_notification_profile_update(priv->profile_note, ryos_configuration_get_profile_notification_type(priv->config),
			ryos_configuration_get_notification_volume(priv->config),
			profile_index + 1, ryos_rkp_get_profile_name(priv->rkp[profile_index]));

	ryos_dbus_server_emit_profile_changed(priv->dbus_server, profile_index);
}

static gboolean set_profile(RyosEventhandler *eventhandler, guint profile_number) {
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	guint profile_index;
	GError *local_error = NULL;

	if (profile_number == 0)
		return TRUE;

	profile_index = profile_number - 1;

	if (priv->actual_profile_index != profile_index) {
		if (!ryos_profile_write(priv->device, profile_index, &local_error)) {
			g_warning(_("Could not activate profile %i: %s"), profile_number, local_error->message);
			g_clear_error(&local_error);
			return FALSE;
		}
		profile_changed(eventhandler, profile_index);
	}
	return TRUE;
}

static gboolean talk_easyshift(RyosEventhandler *eventhandler, gboolean state) {
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	GError *error = NULL;
	gboolean retval = FALSE;

	if (priv->device_set_up) {
		retval = ryos_talk_easyshift(priv->device,
				(state != FALSE) ? RYOS_TALK_EASYSHIFT_ON : RYOS_TALK_EASYSHIFT_OFF,
				&error);
		if (error) {
			g_warning(_("Could not activate easyshift: %s"), error->message);
			g_clear_error(&error);
		}
	}

	return retval;
}

static gboolean talk_easyshift_lock(RyosEventhandler *eventhandler, gboolean state) {
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	GError *error = NULL;
	gboolean retval = FALSE;

	if (priv->device_set_up) {
		retval = ryos_talk_easyshift_lock(priv->device,
				(state != FALSE) ? RYOS_TALK_EASYSHIFT_ON : RYOS_TALK_EASYSHIFT_OFF,
				&error);
		if (error) {
			g_warning(_("Could not activate easyshift lock: %s"), error->message);
			g_clear_error(&error);
		}
	}

	return retval;
}

static guint ryos_eventhandler_get_talk_device(RoccatEventhandlerPlugin *self) {
	RyosEventhandlerPrivate *priv = RYOS_EVENTHANDLER(self)->priv;

	if (!priv->device_set_up)
		return 0;

	return gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(priv->device));
}

static void ryos_eventhandler_talk_easyshift(RoccatEventhandlerPlugin *self, gboolean state) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	talk_easyshift(eventhandler, state);
}

static void ryos_eventhandler_talk_easyshift_lock(RoccatEventhandlerPlugin *self, gboolean state) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	talk_easyshift_lock(eventhandler, state);
}

static void ryosmk_eventhandler_talkfx_ryos_set_sdk_mode(RoccatEventhandlerPlugin *self, gboolean state) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	ryos_sdk_set_kb_sdk_mode(eventhandler->priv->device, state, NULL);
}

static void ryosmk_eventhandler_talkfx_ryos_set_all_leds(RoccatEventhandlerPlugin *self, GArray const *data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	ryos_sdk_set_all_leds(eventhandler->priv->device, data, NULL);
}

static void ryosmk_eventhandler_talkfx_ryos_turn_on_all_leds(RoccatEventhandlerPlugin *self) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	ryos_sdk_turn_on_all_leds(eventhandler->priv->device, NULL);
}

static void ryosmk_eventhandler_talkfx_ryos_turn_off_all_leds(RoccatEventhandlerPlugin *self) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	ryos_sdk_turn_off_all_leds(eventhandler->priv->device, NULL);
}

static void ryosmk_eventhandler_talkfx_ryos_set_led_on(RoccatEventhandlerPlugin *self, guchar index) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	ryos_sdk_set_led_on(eventhandler->priv->device, index, NULL);
}

static void ryosmk_eventhandler_talkfx_ryos_set_led_off(RoccatEventhandlerPlugin *self, guchar index) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	ryos_sdk_set_led_off(eventhandler->priv->device, index, NULL);
}

static void ryosmk_eventhandler_talkfx_ryos_all_key_blinking(RoccatEventhandlerPlugin *self, guint interval, guint count) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	ryos_sdk_all_key_blinking(eventhandler->priv->device, interval, count, NULL);
}

static void talk_easyshift_cb(RyosDBusServer *server, guchar state, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	talk_easyshift(eventhandler, state);
}

static void talk_easyshift_lock_cb(RyosDBusServer *server, guchar state, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	talk_easyshift_lock(eventhandler, state);
}

static void open_gui_cb(gpointer source, gpointer user_data) {
	gchar *path;
	path = g_build_path("/", INSTALL_PREFIX, "bin", "roccatryosmkconfig", (gchar const *)NULL);
	roccat_double_fork(path);
	g_free(path);
}

static void profile_data_changed_outside_cb(RyosDBusServer *server, guchar profile_index, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;

	ryos_rkp_free(priv->rkp[profile_index]);
	priv->rkp[profile_index] = ryos_rkp_load_actual(profile_index);
}

static void configuration_reload(RyosEventhandler *eventhandler) {
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	if (priv->config)
		roccat_configuration_free(priv->config);
	priv->config = ryos_configuration_load();
}

static void configuration_changed_outside_cb(RyosDBusServer *server, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	configuration_reload(eventhandler);
}

static void talkfx_ryos_set_sdk_mode_cb(RyosDBusServer *server, gboolean state, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	ryos_sdk_set_kb_sdk_mode(eventhandler->priv->device, state, NULL);
}

static void talkfx_ryos_set_all_leds_cb(RyosDBusServer *server, GArray const *data, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	ryos_sdk_set_all_leds(eventhandler->priv->device, data, NULL);
}

static void talkfx_ryos_turn_on_all_leds_cb(RyosDBusServer *server, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	ryos_sdk_turn_on_all_leds(eventhandler->priv->device, NULL);
}

static void talkfx_ryos_turn_off_all_leds_cb(RyosDBusServer *server, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	ryos_sdk_turn_off_all_leds(eventhandler->priv->device, NULL);
}

static void talkfx_ryos_set_led_on_cb(RyosDBusServer *server, guchar index, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	ryos_sdk_set_led_on(eventhandler->priv->device, index, NULL);
}

static void talkfx_ryos_set_led_off_cb(RyosDBusServer *server, guchar index, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	ryos_sdk_set_led_off(eventhandler->priv->device, index, NULL);
}

static void talkfx_ryos_all_key_blinking_cb(RyosDBusServer *server, guint interval, guint count, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	ryos_sdk_all_key_blinking(eventhandler->priv->device, interval, count, NULL);
}

static void profile_changed_cb(gpointer source, guchar profile_index, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	profile_changed(eventhandler, profile_index);
}

static void quicklaunch_cb(RyosEventhandlerChannel *channel, guchar key_index, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	RyosRkpQuicklaunch *quicklaunch;
	gchar *path;

	quicklaunch = ryos_rkp_get_quicklaunch(priv->rkp[priv->actual_profile_index], key_index);
	path = ryos_rkp_quicklaunch_get_path(quicklaunch);

	roccat_double_fork(path);

	g_free(path);
	g_free(quicklaunch);
}

static void ripple_cb(RyosEventhandlerChannel *channel, guchar hid, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;

	hid = ryos_keys_primary_correct_remapped(&priv->rkp[priv->actual_profile_index]->data.keys_primary, hid);

	ryos_ripple_start(priv->ripple, priv->device, hid);
}

static void talk_cb(RyosEventhandlerChannel *channel, guchar hid, guchar event, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	static gboolean state = FALSE;
	gint talk_index;

	if (event == RYOS_SPECIAL_ACTION_PRESS) {
		talk_index = ryos_hid_to_rkp_talk_index(hid);
		if (talk_index == -1) {
			g_warning(_("Could not find talk info for hid 0x%02x"), hid);
			return;
		}
		priv->active_talk = priv->rkp[priv->actual_profile_index]->data.talks[talk_index];
	}

	switch(priv->active_talk.type) {
	case RYOS_KEY_TYPE_TALK_EASYSHIFT:
		roccat_eventhandler_plugin_emit_talk_easyshift(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler),
				ryos_rkp_talk_get_product_id(&priv->active_talk),
				(event == RYOS_SPECIAL_ACTION_PRESS) ? TRUE : FALSE);
		break;
	case RYOS_KEY_TYPE_TALK_EASYSHIFT_ALL:
		roccat_eventhandler_plugin_emit_talk_easyshift(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler),
				ROCCAT_TALK_DEVICE_MOUSE,
				(event == RYOS_SPECIAL_ACTION_PRESS) ? TRUE : FALSE);
		break;
	case RYOS_KEY_TYPE_TALK_EASYSHIFT_LOCK:
		if (event == RYOS_SPECIAL_ACTION_PRESS) {
			state = !state;
			roccat_eventhandler_plugin_emit_talk_easyshift_lock(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler),
					ryos_rkp_talk_get_product_id(&priv->active_talk), state);
		}
		break;
	case RYOS_KEY_TYPE_TALK_EASYAIM_1:
		roccat_eventhandler_plugin_emit_talk_easyaim(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler),
				ryos_rkp_talk_get_product_id(&priv->active_talk),
				(event == RYOS_SPECIAL_ACTION_PRESS) ? ROCCAT_TALK_EASYAIM_1 : ROCCAT_TALK_EASYAIM_OFF);
		break;
	case RYOS_KEY_TYPE_TALK_EASYAIM_2:
		roccat_eventhandler_plugin_emit_talk_easyaim(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler),
				ryos_rkp_talk_get_product_id(&priv->active_talk),
				(event == RYOS_SPECIAL_ACTION_PRESS) ? ROCCAT_TALK_EASYAIM_2 : ROCCAT_TALK_EASYAIM_OFF);
		break;
	case RYOS_KEY_TYPE_TALK_EASYAIM_3:
		roccat_eventhandler_plugin_emit_talk_easyaim(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler),
				ryos_rkp_talk_get_product_id(&priv->active_talk),
				(event == RYOS_SPECIAL_ACTION_PRESS) ? ROCCAT_TALK_EASYAIM_3 : ROCCAT_TALK_EASYAIM_OFF);
		break;
	case RYOS_KEY_TYPE_TALK_EASYAIM_4:
		roccat_eventhandler_plugin_emit_talk_easyaim(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler),
				ryos_rkp_talk_get_product_id(&priv->active_talk),
				(event == RYOS_SPECIAL_ACTION_PRESS) ? ROCCAT_TALK_EASYAIM_4 : ROCCAT_TALK_EASYAIM_OFF);
		break;
	case RYOS_KEY_TYPE_TALK_EASYAIM_5:
		roccat_eventhandler_plugin_emit_talk_easyaim(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler),
				ryos_rkp_talk_get_product_id(&priv->active_talk),
				(event == RYOS_SPECIAL_ACTION_PRESS) ? ROCCAT_TALK_EASYAIM_5 : ROCCAT_TALK_EASYAIM_OFF);
		break;
	default:
		g_message(_("Got unknown Talk type 0x%02x"), priv->active_talk.type);
		break;
	}

	if (event == RYOS_SPECIAL_ACTION_RELEASE)
		priv->active_talk.type = RYOS_KEY_TYPE_DISABLED;
}

static void light_changed_cb(RyosEventhandlerChannel *channel, guchar brightness_index, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	guint profile_index;
	GError *error = NULL;

	profile_index = priv->actual_profile_index;

	ryos_rkp_set_light_brightness(priv->rkp[profile_index], brightness_index);
	ryos_rkp_save_actual(priv->rkp[profile_index], profile_index, &error);
	if (error) {
		g_clear_error(&error);
		return;
	}
	ryos_rkp_set_unmodified(priv->rkp[profile_index]);

	ryos_dbus_server_emit_brightness_changed(priv->dbus_server, profile_index, brightness_index);
}

static gboolean recording_event_store_roccat_macro(RyosMacro const *macro, GError **error) {
	GaminggearMacro *gaminggear_macro;
	gboolean retval;

	gaminggear_macro = ryos_macro_to_gaminggear_macro(macro);

	/*
	 * Using device specific macroset name instead of default "MLR"
	 * because macro file is shared between devices.
	 * Macroset name stored in device is still "MLR" to reduce writes.
	 */
	gaminggear_macro_set_macroset_name(gaminggear_macro, RYOS_DEVICE_NAME " MLR");

	retval = gaminggear_macros_store_macro(gaminggear_macro, error);

	gaminggear_macro_free(gaminggear_macro);

	return retval;
}

static RyosMacro *recording_event_get_data(RoccatDevice *device, guint profile_index, guint key_index, GError **error) {
	return ryos_macro_read(device, profile_index, key_index, error);
}

static gboolean handle_recording_event(RyosEventhandler *eventhandler, guint profile_index, guint key_index) {
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	RyosMacro *macro;
	GError *error = NULL;

	macro = recording_event_get_data(priv->device, profile_index, key_index, &error);
	if (error) {
		g_clear_error(&error);
		return FALSE;
	}

	ryos_rkp_set_key_to_macro(priv->rkp[profile_index], key_index);
	ryos_rkp_set_macro(priv->rkp[profile_index], key_index, macro);

	ryos_rkp_save_actual(priv->rkp[profile_index], profile_index, &error);
	if (error) {
		g_clear_error(&error);
		g_free(macro);
		return FALSE;
	}
	ryos_rkp_set_unmodified(priv->rkp[profile_index]);

	recording_event_store_roccat_macro(macro, &error);
	if (error) {
		g_clear_error(&error);
		g_free(macro);
		return FALSE;
	}

	/* rkp has to be stored before this */
	ryos_dbus_server_emit_macro_changed(priv->dbus_server, profile_index, key_index, macro);

	g_free(macro);
	return TRUE;
}

static void live_recording_cb(RyosEventhandlerChannel *channel, guchar key_index, guchar event, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;

	switch(event) {
	case RYOS_SPECIAL_LIVE_RECORDING_ACTION_START:
		roccat_notification_live_recording_select_macro_key(priv->live_recording_note,
				ryos_configuration_get_live_recording_notification_type(priv->config),
				ryos_configuration_get_notification_volume(priv->config));
		break;
	case RYOS_SPECIAL_LIVE_RECORDING_ACTION_MACRO_KEY_SELECTED:
		roccat_notification_live_recording_record_macro(priv->live_recording_note,
				ryos_configuration_get_live_recording_notification_type(priv->config),
				ryos_configuration_get_notification_volume(priv->config));
		break;
	case RYOS_SPECIAL_LIVE_RECORDING_ACTION_END_SUCCESS:
		roccat_notification_live_recording_end_success(priv->live_recording_note,
				ryos_configuration_get_live_recording_notification_type(priv->config),
				ryos_configuration_get_notification_volume(priv->config));
		handle_recording_event(eventhandler, priv->actual_profile_index, key_index);
		break;
	case RYOS_SPECIAL_LIVE_RECORDING_ACTION_END_ABORT:
		roccat_notification_live_recording_end_abort(priv->live_recording_note,
				ryos_configuration_get_live_recording_notification_type(priv->config),
				ryos_configuration_get_notification_volume(priv->config));
		break;
	case RYOS_SPECIAL_LIVE_RECORDING_ACTION_INVALID_KEY:
		roccat_notification_live_recording_invalid_key(priv->live_recording_note,
				ryos_configuration_get_live_recording_notification_type(priv->config),
				ryos_configuration_get_notification_volume(priv->config));
		break;
	default:
		g_debug(_("Got unknown live recording event 0x%02x"), event);
	}
}

static void led_macro_cb(RyosEventhandlerChannel *channel, guchar key_index, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	
	if (priv->led_macro_thread && ryos_led_macro_thread_get_running(priv->led_macro_thread)) {
		ryos_led_macro_thread_set_cancelled(priv->led_macro_thread);
		g_clear_object(&priv->led_macro_thread);
		return;
	}
	
	if (priv->led_macro_thread) {
		g_clear_object(&priv->led_macro_thread);
	}
	
	priv->led_macro_thread = ryos_led_macro_thread_new(priv->device, &priv->rkp[priv->actual_profile_index]->data.led_macros[key_index]);
}

static void timer_start_cb(RyosEventhandlerChannel *channel, guchar key_index, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	RyosRkpTimer *timer;

	timer = ryos_rkp_get_timer(priv->rkp[priv->actual_profile_index], key_index);

	roccat_notification_timer_start(priv->timer_note,
			ryos_configuration_get_timer_notification_type(priv->config),
			ryos_configuration_get_notification_volume(priv->config),
			(gchar const *)timer->name, timer->duration);
	g_free(timer);
}

static void timer_stop_cb(RyosEventhandlerChannel *channel, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;

	roccat_notification_timer_stop(priv->timer_note);
}

static gboolean profile_data_load(RyosEventhandler *eventhandler, GError **error) {
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	GError *local_error = NULL;
	guint i;

	for (i = 0; i < RYOS_PROFILE_NUM; ++i) {
		priv->rkp[i] = ryos_rkp_load(priv->device, i, &local_error);
		if (local_error) {
			g_propagate_error(error, local_error);
			return FALSE;
		}
	}
	return TRUE;
}

static void window_changed_cb(RoccatEventhandlerHost *host, gchar const *title, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	guint profile_index;
	guint gamefile_index;
	gchar *pattern;

	for (profile_index = 0; profile_index < RYOS_PROFILE_NUM; ++profile_index) {

		if (priv->rkp[profile_index] == NULL)
			continue;

		for (gamefile_index = 0; gamefile_index < RYOS_RKP_PROFILE_GAMEFILE_NUM; ++gamefile_index) {
			pattern = ryos_rkp_get_gamefile_name(priv->rkp[profile_index], gamefile_index);
			if (strcmp(pattern, "") == 0) {
				g_free(pattern);
				continue;
			}

			if (g_regex_match_simple(pattern, title, 0, 0)) {
				set_profile(eventhandler, profile_index + 1);
				g_free(pattern);
				return;
			}
			g_free(pattern);
		}
	}

	set_profile(eventhandler, ryos_configuration_get_default_profile_number(priv->config));
	return;
}

static void device_add_cb(RoccatDeviceScannerInterface *interface, RoccatDevice *device, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	GError *local_error = NULL;
	guint driver_state;

	// FIXME test equality for no error
	if (priv->device) {
		g_critical(_("Multiple devices present."));
		return;
	}

	priv->device = device;
	g_object_ref(G_OBJECT(device));
	roccat_device_debug(device);

	configuration_reload(eventhandler);

	driver_state = roccat_eventhandler_host_should_set_driver_state(priv->host);
	if (driver_state == ROCCATEVENTHANDLER_CONFIGURATION_DRIVER_STATE_ON || driver_state == ROCCATEVENTHANDLER_CONFIGURATION_DRIVER_STATE_OFF) {
		if (!ryos_device_state_write(priv->device, (driver_state == ROCCATEVENTHANDLER_CONFIGURATION_DRIVER_STATE_ON) ? RYOS_DEVICE_STATE_STATE_ON : RYOS_DEVICE_STATE_STATE_OFF, &local_error)) {
			g_warning(_("Could not correct driver state: %s"), local_error->message);
			g_clear_error(&local_error);
		}
	}

	/* Notificators gets initialized only once if a device is found.
	 * Get deinited once in finalizer.
	 */
	if (!priv->profile_note)
		priv->profile_note = roccat_notification_profile_new(roccat_eventhandler_host_get_notificator(priv->host), RYOS_DEVICE_NAME);
	if (!priv->timer_note)
		priv->timer_note = roccat_notification_timer_new(roccat_eventhandler_host_get_notificator(priv->host), RYOS_DEVICE_NAME);
	if (!priv->live_recording_note)
		priv->live_recording_note = roccat_notification_live_recording_new(roccat_eventhandler_host_get_notificator(priv->host), RYOS_DEVICE_NAME);

	priv->actual_profile_index = ryos_profile_read(priv->device, &local_error);
	if (local_error) {
		g_warning(_("Could not read actual profile: %s"), local_error->message);
		g_clear_error(&local_error);
	}

	profile_data_load(eventhandler, &local_error);
	if (local_error) {
		g_warning(_("Could not read rkp: %s"), local_error->message);
		g_clear_error(&local_error);
	}

	priv->dbus_server = ryos_dbus_server_new();
	g_signal_connect(G_OBJECT(priv->dbus_server), "talk-easyshift", G_CALLBACK(talk_easyshift_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "talk-easyshift-lock", G_CALLBACK(talk_easyshift_lock_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "open-gui", G_CALLBACK(open_gui_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "profile-changed-outside", G_CALLBACK(profile_changed_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "profile-data-changed-outside", G_CALLBACK(profile_data_changed_outside_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "configuration-changed-outside", G_CALLBACK(configuration_changed_outside_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "talkfx-ryos-set-sdk-mode", G_CALLBACK(talkfx_ryos_set_sdk_mode_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "talkfx-ryos-set-all-leds", G_CALLBACK(talkfx_ryos_set_all_leds_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "talkfx-ryos-turn-on-all-leds", G_CALLBACK(talkfx_ryos_turn_on_all_leds_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "talkfx-ryos-turn-off-all-leds", G_CALLBACK(talkfx_ryos_turn_off_all_leds_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "talkfx-ryos-set-led-on", G_CALLBACK(talkfx_ryos_set_led_on_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "talkfx-ryos-set-led-off", G_CALLBACK(talkfx_ryos_set_led_off_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->dbus_server), "talkfx-ryos-all-key-blinking", G_CALLBACK(talkfx_ryos_all_key_blinking_cb), eventhandler);
	ryos_dbus_server_connect(priv->dbus_server);

	priv->active_window_changed_handler = g_signal_connect(G_OBJECT(priv->host), "active-window-changed", G_CALLBACK(window_changed_cb), eventhandler);

	ryos_eventhandler_channel_start(priv->channel, priv->device, &local_error);
	if (local_error) {
		g_warning(_("Could not start chardev eventhandler: %s"), local_error->message);
		g_clear_error(&local_error);
	}

	priv->active_talk.type = RYOS_KEY_TYPE_DISABLED;
	priv->device_set_up = TRUE;

	roccat_eventhandler_plugin_emit_device_added(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler), gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(priv->device)));
}

static void stop(RyosEventhandler *eventhandler) {
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	guint i;

	roccat_eventhandler_plugin_emit_device_removed(ROCCAT_EVENTHANDLER_PLUGIN(eventhandler), gaminggear_device_get_product_id(GAMINGGEAR_DEVICE(priv->device)));

	priv->device_set_up = FALSE;

	g_clear_object(&priv->dbus_server);
	ryos_eventhandler_channel_stop(priv->channel);
	g_signal_handler_disconnect(G_OBJECT(priv->host), priv->active_window_changed_handler);
	g_clear_object(&priv->device);
	g_clear_pointer(&priv->config, roccat_configuration_free);

	for (i = 0; i < RYOS_PROFILE_NUM; ++i)
		g_clear_pointer(&priv->rkp[i], ryos_rkp_free);
}

static void device_remove_cb(RoccatDeviceScanner *scanner, RoccatDevice *device, gpointer user_data) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(user_data);
	RyosEventhandlerPrivate *priv = eventhandler->priv;
	if (priv->device && gaminggear_device_equal(GAMINGGEAR_DEVICE(device), GAMINGGEAR_DEVICE(priv->device)))
		stop(eventhandler);
}

static void ryos_eventhandler_start(RoccatEventhandlerPlugin *self, RoccatEventhandlerHost *host) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	RyosEventhandlerPrivate *priv = eventhandler->priv;

	priv->host = host;
	g_object_ref(priv->host);

#ifdef LUA_FOUND
	RoccatKeyFile *config;
	gchar *script;
	config = ryos_configuration_load();
	if (ryos_configuration_get_use_lua(config)) {
		script = ryos_configuration_get_ripple_module(config);
		priv->ripple = RYOS_RIPPLE(ryos_ripple_lua_new(script));
		g_free(script);
	} else {
		priv->ripple = RYOS_RIPPLE(ryos_ripple_c_new());
	}
	roccat_configuration_free(config);
#else
	priv->ripple = RYOS_RIPPLE(ryos_ripple_c_new());
#endif

	priv->channel = ryos_eventhandler_channel_new();
	g_signal_connect(G_OBJECT(priv->channel), "led-macro", G_CALLBACK(led_macro_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "open-driver", G_CALLBACK(open_gui_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "profile-changed", G_CALLBACK(profile_changed_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "quicklaunch", G_CALLBACK(quicklaunch_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "ripple", G_CALLBACK(ripple_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "talk", G_CALLBACK(talk_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "timer-start", G_CALLBACK(timer_start_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "timer-stop", G_CALLBACK(timer_stop_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "light-changed", G_CALLBACK(light_changed_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->channel), "live-recording", G_CALLBACK(live_recording_cb), eventhandler);

	priv->device_scanner = ryos_device_scanner_new();
	g_signal_connect(G_OBJECT(priv->device_scanner), "device-added", G_CALLBACK(device_add_cb), eventhandler);
	g_signal_connect(G_OBJECT(priv->device_scanner), "device-removed", G_CALLBACK(device_remove_cb), eventhandler);
	roccat_device_scanner_interface_start(ROCCAT_DEVICE_SCANNER_INTERFACE(priv->device_scanner));
}

static void ryos_eventhandler_stop(RoccatEventhandlerPlugin *self) {
	RyosEventhandler *eventhandler = RYOS_EVENTHANDLER(self);
	RyosEventhandlerPrivate *priv = eventhandler->priv;

	stop(eventhandler);

	g_clear_object(&priv->device_scanner);
	g_clear_object(&priv->channel);
	g_clear_object(&priv->ripple);
	g_clear_object(&priv->host);
}

static void ryos_roccat_eventhandler_interface_init(RoccatEventhandlerPluginInterface *iface) {
	iface->start = ryos_eventhandler_start;
	iface->stop = ryos_eventhandler_stop;
	iface->get_talk_device = ryos_eventhandler_get_talk_device;
	iface->talk_easyshift = ryos_eventhandler_talk_easyshift;
	iface->talk_easyshift_lock = ryos_eventhandler_talk_easyshift_lock;
	iface->talk_easyaim = NULL;
	iface->talkfx_set_led_rgb = NULL;
	iface->talkfx_restore_led_rgb = NULL;
	iface->talkfx_ryos_set_sdk_mode = ryosmk_eventhandler_talkfx_ryos_set_sdk_mode;
	iface->talkfx_ryos_set_all_leds = ryosmk_eventhandler_talkfx_ryos_set_all_leds;
	iface->talkfx_ryos_turn_on_all_leds = ryosmk_eventhandler_talkfx_ryos_turn_on_all_leds;
	iface->talkfx_ryos_turn_off_all_leds = ryosmk_eventhandler_talkfx_ryos_turn_off_all_leds;
	iface->talkfx_ryos_set_led_on = ryosmk_eventhandler_talkfx_ryos_set_led_on;
	iface->talkfx_ryos_set_led_off = ryosmk_eventhandler_talkfx_ryos_set_led_off;
	iface->talkfx_ryos_all_key_blinking = ryosmk_eventhandler_talkfx_ryos_all_key_blinking;
}

G_DEFINE_TYPE_WITH_CODE(RyosEventhandler, ryos_eventhandler, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(ROCCAT_EVENTHANDLER_PLUGIN_TYPE, ryos_roccat_eventhandler_interface_init));

static void ryos_eventhandler_init(RyosEventhandler *eventhandler) {
	eventhandler->priv = RYOS_EVENTHANDLER_GET_PRIVATE(eventhandler);
}

static void dispose(GObject *object) {
	ryos_eventhandler_stop(ROCCAT_EVENTHANDLER_PLUGIN(object));

	G_OBJECT_CLASS(ryos_eventhandler_parent_class)->dispose(object);
}

static void finalize(GObject *object) {
	RyosEventhandlerPrivate *priv = RYOS_EVENTHANDLER(object)->priv;

	g_clear_pointer(&priv->live_recording_note, roccat_notification_live_recording_free);
	g_clear_pointer(&priv->timer_note, roccat_notification_timer_free);
	g_clear_pointer(&priv->profile_note, roccat_notification_profile_free);

	G_OBJECT_CLASS(ryos_eventhandler_parent_class)->finalize(object);
}

static void ryos_eventhandler_class_init(RyosEventhandlerClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;

	g_type_class_add_private(klass, sizeof(RyosEventhandlerPrivate));
}

ROCCAT_EVENTHANDLER_PLUGIN_API RoccatEventhandlerPlugin *roccat_eventhandler_plugin_new(void) {
	return ROCCAT_EVENTHANDLER_PLUGIN(g_object_new(RYOS_EVENTHANDLER_TYPE, NULL));
}
