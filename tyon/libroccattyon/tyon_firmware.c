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

#include "tyon_firmware.h"

RoccatFirmwareState *tyon_firmware_state_new(RoccatDevice *device) {
	RoccatFirmwareState *state;
	state = roccat_firmware_state_new(device, ROCCAT_FIRMWARE_SIZE_VARIABLE, ROCCAT_FIRMWARE_NUMBER_DEFAULT);
	roccat_firmware_state_set_wait_0(state, TYON_FIRMWARE_UPDATE_WAIT_0);
	roccat_firmware_state_set_wait_1(state, TYON_FIRMWARE_UPDATE_WAIT_1);
	roccat_firmware_state_set_wait_2(state, TYON_FIRMWARE_UPDATE_WAIT_2);
	roccat_firmware_state_set_wait_34f(state, TYON_FIRMWARE_UPDATE_WAIT_34F);
	return state;
}
