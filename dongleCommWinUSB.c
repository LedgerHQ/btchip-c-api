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

#include "dongleCommWinUSB.h"

#define BTCHIP_VID 0x2581
#define BTCHIP_WINUSB_PID 0x1b7c
#define BTCHIP_WINUSB_BOOTLOADER_PID 0x1808

#define TIMEOUT 10000
#define SW1_DATA 0x61

#ifdef HAVE_LIBUSB

int initWinUSB() {
	return libusb_init(NULL);
}

int exitWinUSB() {
	libusb_exit(NULL);
	return 0;
}

int sendApduWinUSB(libusb_device_handle *handle, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw) {
	unsigned char buffer[260];
	int result;
	int length;
	int swOffset;

	result = libusb_bulk_transfer(handle, 0x02, (unsigned char*)apdu, apduLength, &length, TIMEOUT);
	if (result < 0) {
		return result;
	}
	result = libusb_bulk_transfer(handle, 0x82, buffer, sizeof(buffer), &length, TIMEOUT);
	if (result < 0) {
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

libusb_device_handle* getFirstDongleWinUSB() {
	libusb_device_handle *result = libusb_open_device_with_vid_pid(NULL, BTCHIP_VID, BTCHIP_WINUSB_PID);
	if (result == NULL) {
		result = libusb_open_device_with_vid_pid(NULL, BTCHIP_VID, BTCHIP_WINUSB_BOOTLOADER_PID);
		if (result == NULL) {		
			return NULL;
		}
	}

	libusb_claim_interface(result, 0);
	return result;
}

void closeDongleWinUSB(libusb_device_handle *handle) {
	libusb_release_interface(handle, 0);
	libusb_close(handle);
}

#endif