#ifndef __ROCCAT_SAVUCONFIG_CONFIGURATION_EDITOR_H__
#define __ROCCAT_SAVUCONFIG_CONFIGURATION_EDITOR_H__

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

#include "roccat_key_file.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SAVUCONFIG_CONFIGURATION_DIALOG_TYPE (savuconfig_configuration_dialog_get_type())
#define SAVUCONFIG_CONFIGURATION_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SAVUCONFIG_CONFIGURATION_DIALOG_TYPE, SavuconfigConfigurationDialog))
#define IS_SAVUCONFIG_CONFIGURATION_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SAVUCONFIG_CONFIGURATION_DIALOG_TYPE))

typedef struct _SavuconfigConfigurationDialog SavuconfigConfigurationDialog;
typedef struct _SavuconfigConfigurationDialogPrivate SavuconfigConfigurationDialogPrivate;

struct _SavuconfigConfigurationDialog {
	GtkDialog dialog;
	SavuconfigConfigurationDialogPrivate *priv;
};

GType savuconfig_configuration_dialog_get_type(void);
gboolean savuconfig_configuration_dialog(GtkWindow *parent, RoccatKeyFile *config);
GtkWidget *savuconfig_configuration_dialog_new(GtkWindow *parent);

void savuconfig_configuration_dialog_get_configuration(SavuconfigConfigurationDialog *savuconfig_configuration_dialog, RoccatKeyFile *config);
void savuconfig_configuration_dialog_set_configuration(SavuconfigConfigurationDialog *savuconfig_configuration_dialog, RoccatKeyFile *config);

gboolean savuconfig_configuration_save(GtkWindow *parent, RoccatKeyFile *config);

G_END_DECLS

#endif
