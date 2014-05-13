/*
*******************************************************************************    
*   BTChip Bitcoin Hardware Wallet C test interface
*   (c) 2014 BTChip - 1BTChip7VfTnrPra5jqci7ejnMguuHogTn
*   
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*   limitations under the License.
********************************************************************************/

#include "dongleCommHid.h"

#define BTCHIP_VID 0x2581
#define BTCHIP_HID_PID 0x2b7c
#define PAGE_HIDGEN 65440

#define TIMEOUT 10000
#define SW1_DATA 0x61

int initHid() {
	return 0;
}

int exitHid() {
	return 0;
}

// TODO : support longer APDUs
int sendApduHid(hid_device *handle, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw) {
	unsigned char buffer[65];
	int result;
	int length;
	int swOffset;

	if (apduLength > 65) {
		return -1;
	}
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = 0x00;
	memcpy(buffer + 1, apdu, apduLength);
	result = hid_write(handle, buffer, sizeof(buffer));
	if (result < 0) {
		return result;
	}
	result = hid_read_timeout(handle, buffer, sizeof(buffer), TIMEOUT);
	if (result <= 0) {
		return result;
	}
	if (buffer[0] == SW1_DATA) {
		length = buffer[1];
		if (outLength < length) {
			return -1;
		}
		memcpy(out, buffer + 2, length);
		swOffset = 2 + length;
	}	
	else {
		length = 0;
		swOffset = 0;
	}
	if (sw != NULL) {
		*sw = (buffer[swOffset] << 8) | buffer[swOffset + 1];
	}
	return length;
}

hid_device* getFirstDongleHid() {
	struct hid_device_info *devs, *cur_dev;
	hid_device *dongle = NULL;
	devs = hid_enumerate(BTCHIP_VID, BTCHIP_HID_PID);
	cur_dev = devs;
	while(cur_dev) {
		if ((cur_dev->interface_number == 1) || (cur_dev->usage_page == PAGE_HIDGEN)) {
			dongle = hid_open_path(cur_dev->path);
			break;
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
	return dongle;
}

void closeDongleHid(hid_device *handle) {
	hid_close(handle);
}

