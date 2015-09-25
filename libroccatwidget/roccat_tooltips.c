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

#include "roccat_tooltips.h"
#include "i18n-lib.h"

void roccat_widget_set_tooltip_gamefile_frame(GtkWidget *widget) {
	gtk_widget_set_tooltip_text(widget,
			_("Select an executable or give a window name.\n"
			"Accepts Pearl compatible regular expressions."));
}

void roccat_widget_set_tooltip_default_profile_number(GtkWidget *widget) {
	gtk_widget_set_tooltip_text(widget,
			_("Number of profile that gets activated when no gamefile setting is applied."));
}
