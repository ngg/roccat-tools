#ifndef __ROCCAT_ISKUCONFIG_CONFIGURATION_EDITOR_H__
#define __ROCCAT_ISKUCONFIG_CONFIGURATION_EDITOR_H__

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

#include "roccat_config.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ISKUCONFIG_CONFIGURATION_DIALOG_TYPE (iskuconfig_configuration_dialog_get_type())
#define ISKUCONFIG_CONFIGURATION_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ISKUCONFIG_CONFIGURATION_DIALOG_TYPE, IskuconfigConfigurationDialog))
#define IS_ISKUCONFIG_CONFIGURATION_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ISKUCONFIG_CONFIGURATION_DIALOG_TYPE))

typedef struct _IskuconfigConfigurationDialog IskuconfigConfigurationDialog;
typedef struct _IskuconfigConfigurationDialogPrivate IskuconfigConfigurationDialogPrivate;

struct _IskuconfigConfigurationDialog {
	GtkDialog dialog;
	IskuconfigConfigurationDialogPrivate *priv;
};

GType iskuconfig_configuration_dialog_get_type(void);
gboolean iskuconfig_configuration_dialog(GtkWindow *parent, RoccatKeyFile *config);
GtkWidget *iskuconfig_configuration_dialog_new(GtkWindow *parent);

void iskuconfig_configuration_dialog_get_configuration(IskuconfigConfigurationDialog *iskuconfig_configuration_dialog, RoccatKeyFile *config);
void iskuconfig_configuration_dialog_set_configuration(IskuconfigConfigurationDialog *iskuconfig_configuration_dialog, RoccatKeyFile *config);

gboolean iskuconfig_configuration_save(GtkWindow *parent, RoccatKeyFile *config);

G_END_DECLS

#endif
