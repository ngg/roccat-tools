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

#include "nyth_tcu_calibration_assistant.h"
#include "nyth_control_unit.h"
#include "nyth_device.h"
#include "g_roccat_helper.h"
#include "roccat_helper.h"
#include "roccat_device_hidraw.h"
#include "roccat_warning_dialog.h"
#include <gaminggear/gaminggear_timeout_dialog.h>
#include "i18n.h"
#include "../libroccatnyth/nyth_special.h"

#define NYTH_TCU_CALIBRATION_ASSISTANT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), NYTH_TCU_CALIBRATION_ASSISTANT_TYPE, NythTcuCalibrationAssistantClass))
#define IS_NYTH_TCU_CALIBRATION_ASSISTANT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), NYTH_TCU_CALIBRATION_ASSISTANT_TYPE))
#define NYTH_TCU_CALIBRATION_ASSISTANT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), NYTH_TCU_CALIBRATION_ASSISTANT_TYPE, NythTcuCalibrationAssistantPrivate))

typedef struct _NythTcuCalibrationAssistantClass NythTcuCalibrationAssistantClass;
typedef struct _NythTcuCalibrationAssistantPrivate NythTcuCalibrationAssistantPrivate;

struct _NythTcuCalibrationAssistant {
	GtkAssistant parent;
	NythTcuCalibrationAssistantPrivate *priv;
};

struct _NythTcuCalibrationAssistantClass {
	GtkAssistantClass parent_class;
};

struct _NythTcuCalibrationAssistantPrivate {
	RoccatDevice *device;
	GtkWidget *test_page;
	GtkWidget *finish_page;
	gint finish_page_number;
	GtkProgressBar *progress_bar;
	GtkWindow *parent;
	guint tcu_value;
	guint timeout_event_source_id;
};

enum {
	COMPLETED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(NythTcuCalibrationAssistant, nyth_tcu_calibration_assistant, GTK_TYPE_ASSISTANT);

static void append_finish_page(NythTcuCalibrationAssistant *assistant) {
	NythTcuCalibrationAssistantPrivate *priv = assistant->priv;
	GtkWidget *label;

	label = gtk_label_new(_("Finished!"));

	priv->finish_page_number = gtk_assistant_append_page(GTK_ASSISTANT(assistant), label);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), label, _("Finish"));
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), label, GTK_ASSISTANT_PAGE_CONFIRM);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), label, TRUE);
}

static void append_test_page(NythTcuCalibrationAssistant *assistant) {
	NythTcuCalibrationAssistantPrivate *priv = assistant->priv;

	priv->test_page = gtk_hbox_new(FALSE, 0);
	priv->progress_bar = GTK_PROGRESS_BAR(gtk_progress_bar_new());

	gtk_box_pack_start(GTK_BOX(priv->test_page), GTK_WIDGET(priv->progress_bar), TRUE, FALSE, 0);

	gtk_assistant_append_page(GTK_ASSISTANT(assistant), priv->test_page);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), priv->test_page, _("Surface test"));
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), priv->test_page, GTK_ASSISTANT_PAGE_PROGRESS);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), priv->test_page, FALSE);
}

static void append_intro_page(NythTcuCalibrationAssistant *assistant) {
	GtkWidget *label;

	label = gtk_label_new(_("Please don't move your mouse after pressing next"));

	gtk_assistant_append_page(GTK_ASSISTANT(assistant), label);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), label, _("Start calibration"));
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), label, GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), label, TRUE);
}

static gboolean io_cb(GIOChannel *channel, GIOCondition condition, gpointer user_data) {
	NythTcuCalibrationAssistant *cal_assistant = NYTH_TCU_CALIBRATION_ASSISTANT(user_data);
	NythTcuCalibrationAssistantPrivate *priv = cal_assistant->priv;
	NythSpecial event;
	GIOStatus status;
	GError *error = NULL;
	gsize length;

	if (condition & G_IO_HUP)
		return FALSE;

	status = g_io_channel_read_chars(channel, (gchar *)&event, sizeof(NythSpecial), &length, &error);

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

	if (length < sizeof(NythSpecial))
		return TRUE;

	if (event.report_id != NYTH_REPORT_ID_SPECIAL)
		return TRUE;

	if (event.type != NYTH_SPECIAL_TYPE_TCU)
		return TRUE;

	g_debug(_("TCU calibration ended with: %02x %02x"), event.data1, event.data2);

	gtk_assistant_set_page_complete(GTK_ASSISTANT(cal_assistant), priv->test_page, TRUE);
	gtk_assistant_set_current_page(GTK_ASSISTANT(cal_assistant), priv->finish_page_number);
	g_source_remove(priv->timeout_event_source_id);
	return FALSE;
}

static gboolean timeout_cb(gpointer user_data) {
	gtk_progress_bar_pulse(((NythTcuCalibrationAssistant *)user_data)->priv->progress_bar);
	return TRUE;
}

static void prepare_cb(GtkAssistant *assistant, GtkWidget *page, gpointer user_data) {
	NythTcuCalibrationAssistant *cal_assistant = NYTH_TCU_CALIBRATION_ASSISTANT(assistant);
	NythTcuCalibrationAssistantPrivate *priv = cal_assistant->priv;
	GError *local_error = NULL;

	if (page == priv->test_page) {
		nyth_tracking_control_unit_start(priv->device, &local_error);
		roccat_device_hidraw_add_event_watch(priv->device, NYTH_INTERFACE_MOUSE, io_cb, cal_assistant, &local_error);
		// FIXME error handling
		priv->timeout_event_source_id = g_timeout_add(0.5 * G_ROCCAT_MSEC_PER_SEC, timeout_cb, cal_assistant);
	}
}

static void close_cb(GtkAssistant *assistant, gpointer user_data) {
	NythTcuCalibrationAssistant *cal_assistant = NYTH_TCU_CALIBRATION_ASSISTANT(assistant);

	g_debug(_("TCU calibration closed"));

	g_signal_emit((gpointer)cal_assistant, signals[COMPLETED], 0);
}

static void cancel_cb(GtkAssistant *assistant, gpointer user_data) {
	NythTcuCalibrationAssistant *cal_assistant = NYTH_TCU_CALIBRATION_ASSISTANT(assistant);
	NythTcuCalibrationAssistantPrivate *priv = cal_assistant->priv;
	GError *local_error = NULL;

	g_debug(_("TCU calibration cancelled"));

	nyth_tracking_control_unit_cancel(priv->device, &local_error);
	priv->tcu_value = NYTH_TRACKING_CONTROL_UNIT_OFF;

	roccat_handle_error_dialog(priv->parent, _("Could not set TCU data"), &local_error);

	/* close is not issued on cancel */
	g_signal_emit((gpointer)assistant, signals[COMPLETED], 0);
}

static void apply_cb(GtkAssistant *assistant, gpointer user_data) {
	NythTcuCalibrationAssistantPrivate *priv = NYTH_TCU_CALIBRATION_ASSISTANT(assistant)->priv;
	GError *local_error = NULL;
	gboolean answer;

	answer = gaminggear_timeout_dialog(priv->parent,
			_("Keep settings?"),
			_("Do you want to keep this settings?"), 20);

	if (answer) {
		nyth_tracking_control_unit_accept(priv->device, &local_error);
		priv->tcu_value = NYTH_TRACKING_CONTROL_UNIT_ON;
	} else {
		nyth_tracking_control_unit_cancel(priv->device, &local_error);
		priv->tcu_value = NYTH_TRACKING_CONTROL_UNIT_OFF;
	}
	if (!roccat_handle_error_dialog(priv->parent, _("Could not set TCU data"), &local_error))
		return;

	/* close gets issued after this */
}

guint nyth_tcu_calibration_assistant_get_tcu_value(NythTcuCalibrationAssistant *cal_assistant) {
	return cal_assistant->priv->tcu_value;
}

GtkWidget *nyth_tcu_calibration_assistant_new(GtkWindow *parent, RoccatDevice *device) {
	NythTcuCalibrationAssistant *assistant;
	NythTcuCalibrationAssistantPrivate *priv;

	assistant = NYTH_TCU_CALIBRATION_ASSISTANT(g_object_new(NYTH_TCU_CALIBRATION_ASSISTANT_TYPE,
			NULL));

	priv = assistant->priv;
	priv->device = device;
	priv->parent = parent;

	gtk_window_set_transient_for(GTK_WINDOW(assistant), parent);
	gtk_window_set_modal(GTK_WINDOW(assistant), TRUE);
	gtk_window_set_title(GTK_WINDOW(assistant), _("TCU calibration"));

	return GTK_WIDGET(assistant);
}

static void nyth_tcu_calibration_assistant_init(NythTcuCalibrationAssistant *assistant) {
	NythTcuCalibrationAssistantPrivate *priv = NYTH_TCU_CALIBRATION_ASSISTANT_GET_PRIVATE(assistant);
	assistant->priv = priv;

	append_intro_page(assistant);
	append_test_page(assistant);
	append_finish_page(assistant);

	g_signal_connect(G_OBJECT(assistant), "prepare", G_CALLBACK(prepare_cb), NULL);
	g_signal_connect(G_OBJECT(assistant), "close", G_CALLBACK(close_cb), NULL);
	g_signal_connect(G_OBJECT(assistant), "cancel", G_CALLBACK(cancel_cb), NULL);
	g_signal_connect(G_OBJECT(assistant), "apply", G_CALLBACK(apply_cb), NULL);
}

static void nyth_tcu_calibration_assistant_class_init(NythTcuCalibrationAssistantClass *klass) {
	g_type_class_add_private(klass, sizeof(NythTcuCalibrationAssistantPrivate));

	signals[COMPLETED] = g_signal_new("completed",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0,
			NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}
