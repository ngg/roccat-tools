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

#include "ryos_eventhandler_channel.h"
#include "g_cclosure_roccat_marshaller.h"
#include "ryos.h"
#include "ryos_device.h"
#include "roccat_device_hidraw.h"
#include "roccat.h"
#include "roccat_helper.h"
#include "i18n-lib.h"

#define RYOS_EVENTHANDLER_CHANNEL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RYOS_EVENTHANDLER_CHANNEL_TYPE, RyosEventhandlerChannelClass))
#define IS_RYOS_EVENTHANDLER_CHANNEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RYOS_EVENTHANDLER_CHANNEL_TYPE))
#define RYOS_EVENTHANDLER_CHANNEL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), RYOS_EVENTHANDLER_CHANNEL_TYPE, RyosEventhandlerChannelPrivate))

typedef struct _RyosEventhandlerChannelClass RyosEventhandlerChannelClass;
typedef struct _RyosEventhandlerChannelPrivate RyosEventhandlerChannelPrivate;

struct _RyosEventhandlerChannel {
	GObject parent;
	RyosEventhandlerChannelPrivate *priv;
};

struct _RyosEventhandlerChannelClass {
	GObjectClass parent_class;
};

struct _RyosEventhandlerChannelPrivate {
	guint event_source_id;
	gboolean running;
};

enum {
	LIGHT_CHANGED, /* brightness_index */
	LIVE_RECORDING, /* key_index, event */
	OPEN_DRIVER,
	PROFILE_CHANGED, /* profile_index */
	QUICKLAUNCH, /* key_index */
	RIPPLE, /* hid */
	TALK, /* key_type, event */
	TIMER_START, /* key_index */
	TIMER_STOP,
	LED_MACRO, /* key_index */
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(RyosEventhandlerChannel, ryos_eventhandler_channel, G_TYPE_OBJECT);

static void debug_unknown_event(RyosSpecial const *event) {
	gchar *string = roccat_data_to_string((guchar const *)event, sizeof(RyosSpecial));
	g_debug(_("Got unknown event: %s"), string);
	g_free(string);
}

static void process_chrdev_event(RyosEventhandlerChannel *eventhandler_channel, RyosSpecial const *event) {
	switch (event->special) {
	case RYOS_SPECIAL_SPECIAL_RIPPLE:
		if (event->action == RYOS_SPECIAL_FUNCTION_ACTIVATE)
			g_signal_emit((gpointer)eventhandler_channel, signals[RIPPLE], 0, event->data);
		break;
	case RYOS_SPECIAL_SPECIAL_LAYER:
	case RYOS_SPECIAL_SPECIAL_EASYSHIFT:
	case RYOS_SPECIAL_SPECIAL_EASYSHIFT_SELF:
	case RYOS_SPECIAL_SPECIAL_PROFILE_START:
	case RYOS_SPECIAL_SPECIAL_MULTIMEDIA:
		/* do nothing */
		break;
	case RYOS_SPECIAL_SPECIAL_LED_MACRO:
		if (event->action == RYOS_SPECIAL_ACTION_PRESS)
			g_signal_emit((gpointer)eventhandler_channel, signals[LED_MACRO], 0, event->data);
		break;
	case RYOS_SPECIAL_SPECIAL_BACKLIGHT:
		g_signal_emit((gpointer)eventhandler_channel, signals[LIGHT_CHANGED], 0, event->data);
		break;
	case RYOS_SPECIAL_SPECIAL_PROFILE:
		g_signal_emit((gpointer)eventhandler_channel, signals[PROFILE_CHANGED], 0, event->data);
		break;
	case RYOS_SPECIAL_SPECIAL_LIVE_RECORDING:
		g_signal_emit((gpointer)eventhandler_channel, signals[LIVE_RECORDING], 0, event->data, event->action);
		break;
	case RYOS_SPECIAL_SPECIAL_QUICKLAUNCH:
		if (event->action == RYOS_SPECIAL_ACTION_PRESS)
			g_signal_emit((gpointer)eventhandler_channel, signals[QUICKLAUNCH], 0, event->data);
		break;
	case RYOS_SPECIAL_SPECIAL_OPEN_DRIVER:
		if (event->data == RYOS_SPECIAL_FUNCTION_ACTIVATE)
			g_signal_emit((gpointer)eventhandler_channel, signals[OPEN_DRIVER], 0);
		break;
	case RYOS_SPECIAL_SPECIAL_TIMER_START:
		if (event->action == RYOS_SPECIAL_ACTION_PRESS)
			g_signal_emit((gpointer)eventhandler_channel, signals[TIMER_START], 0, event->data);
		break;
	case RYOS_SPECIAL_SPECIAL_TIMER_STOP:
		if (event->action == RYOS_SPECIAL_ACTION_PRESS)
			g_signal_emit((gpointer)eventhandler_channel, signals[TIMER_STOP], 0);
		break;
	case RYOS_SPECIAL_SPECIAL_TALK:
		g_signal_emit((gpointer)eventhandler_channel, signals[TALK], 0, event->data, event->action);
		break;
	default:
		debug_unknown_event(event);
		break;
	}
}

static gboolean io_cb(GIOChannel *channel, GIOCondition condition, gpointer user_data) {
	RyosEventhandlerChannel *eventhandler_channel = RYOS_EVENTHANDLER_CHANNEL(user_data);
	GIOStatus status;
	GError *error = NULL;
	gsize length;
	RyosSpecial event;

	if (condition & G_IO_HUP)
		return FALSE;

	status = g_io_channel_read_chars(channel, (gchar *)&event, sizeof(RyosSpecial), &length, &error);

	if (status == G_IO_STATUS_AGAIN)
		return TRUE;

	if (status == G_IO_STATUS_ERROR) {
		g_critical(_("Could not read io_channel: %s"), error->message);
		return FALSE;
	}

	if (status == G_IO_STATUS_EOF) {
		g_warning(_("Could not read io_channel: %s"), _("End of file"));
		return FALSE;
	}

	if (length < sizeof(RyosSpecial))
		return TRUE;

	switch (event.report_id) {
	case RYOS_REPORT_ID_SPECIAL:
		switch (event.type) {
		case RYOS_SPECIAL_TYPE_SPECIAL:
			process_chrdev_event(eventhandler_channel, &event);
			break;
		case RYOS_SPECIAL_TYPE_RAD_W:
		case RYOS_SPECIAL_TYPE_RAD_A:
		case RYOS_SPECIAL_TYPE_RAD_S:
		case RYOS_SPECIAL_TYPE_RAD_D:
		case RYOS_SPECIAL_TYPE_RAD_THUMBSTER1:
		case RYOS_SPECIAL_TYPE_RAD_THUMBSTER2:
		case RYOS_SPECIAL_TYPE_RAD_THUMBSTER3:
		case RYOS_SPECIAL_TYPE_RAD_EASYSHIFT:
		case RYOS_SPECIAL_TYPE_RAD_MULTIMEDIA:
		case RYOS_SPECIAL_TYPE_RAD_M1:
		case RYOS_SPECIAL_TYPE_RAD_M2:
		case RYOS_SPECIAL_TYPE_RAD_M3:
		case RYOS_SPECIAL_TYPE_RAD_M4:
		case RYOS_SPECIAL_TYPE_RAD_M5:
		case RYOS_SPECIAL_TYPE_RAD_MACRO_SHORTCUT:
		case RYOS_SPECIAL_TYPE_RAD_TALK:
		case RYOS_SPECIAL_TYPE_RAD_MACRO_LIFE_REC:
		case RYOS_SPECIAL_TYPE_RAD_BACKLIGHT:
		case RYOS_SPECIAL_TYPE_RAD_TOTAL:
			// TODO maybe handle rad events
			break;
		default:
			debug_unknown_event(&event);
			break;
		}
		break;
	default:
		break;
	}

	return TRUE; // keep on going
}

RyosEventhandlerChannel *ryos_eventhandler_channel_new(void) {
	return RYOS_EVENTHANDLER_CHANNEL(g_object_new(RYOS_EVENTHANDLER_CHANNEL_TYPE, NULL));
}

static void ryos_eventhandler_channel_init(RyosEventhandlerChannel *channel) {
	RyosEventhandlerChannelPrivate *priv = RYOS_EVENTHANDLER_CHANNEL_GET_PRIVATE(channel);
	channel->priv = priv;
	priv->running = FALSE;
}

gboolean ryos_eventhandler_channel_start(RyosEventhandlerChannel *eventhandler_channel, RoccatDevice *device, GError **error) {
	GError *local_error = NULL;
	RyosEventhandlerChannelPrivate *priv = eventhandler_channel->priv;
	priv->event_source_id = roccat_device_hidraw_add_event_watch(device, RYOS_INTERFACE_MOUSE, io_cb, eventhandler_channel, &local_error);
	if (local_error) {
		g_propagate_error(error, local_error);
		priv->running = FALSE;
		return FALSE;
	} else {
		priv->running = TRUE;
		return TRUE;
	}
}

void ryos_eventhandler_channel_stop(RyosEventhandlerChannel *eventhandler_channel) {
	RyosEventhandlerChannelPrivate *priv = eventhandler_channel->priv;
	if (priv->running) {
		g_source_remove(priv->event_source_id);
		priv->running = FALSE;
	}
}

static void finalize(GObject *object) {
	RyosEventhandlerChannel *eventhandler_channel = RYOS_EVENTHANDLER_CHANNEL(object);
	ryos_eventhandler_channel_stop(eventhandler_channel);
	G_OBJECT_CLASS(ryos_eventhandler_channel_parent_class)->finalize(object);
}

static void ryos_eventhandler_channel_class_init(RyosEventhandlerChannelClass *klass) {
	GObjectClass *gobject_class;

	gobject_class = (GObjectClass *)klass;

	gobject_class->finalize = finalize;

	g_type_class_add_private(klass, sizeof(RyosEventhandlerChannelPrivate));

	signals[LED_MACRO] = g_signal_new("led-macro", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__UCHAR, G_TYPE_NONE,
			1, G_TYPE_UCHAR);

	signals[LIGHT_CHANGED] = g_signal_new("light-changed", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__UCHAR, G_TYPE_NONE,
			1, G_TYPE_UCHAR);

	signals[LIVE_RECORDING] = g_signal_new("live-recording", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_roccat_marshal_VOID__UCHAR_UCHAR, G_TYPE_NONE,
			2, G_TYPE_UCHAR, G_TYPE_UCHAR);

	signals[OPEN_DRIVER] = g_signal_new("open-driver", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE,
			0);

	signals[PROFILE_CHANGED] = g_signal_new("profile-changed", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__UCHAR, G_TYPE_NONE,
			1, G_TYPE_UCHAR);

	signals[QUICKLAUNCH] = g_signal_new("quicklaunch", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__UCHAR, G_TYPE_NONE,
			1, G_TYPE_UCHAR);

	signals[RIPPLE] = g_signal_new("ripple", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__UCHAR, G_TYPE_NONE,
			1, G_TYPE_UCHAR);

	signals[TALK] = g_signal_new("talk", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_roccat_marshal_VOID__UCHAR_UCHAR, G_TYPE_NONE,
			2, G_TYPE_UCHAR, G_TYPE_UCHAR);

	signals[TIMER_START] = g_signal_new("timer-start", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__UCHAR, G_TYPE_NONE,
			1, G_TYPE_UCHAR);

	signals[TIMER_STOP] = g_signal_new("timer-stop", RYOS_EVENTHANDLER_CHANNEL_TYPE,
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE,
			0);
}
