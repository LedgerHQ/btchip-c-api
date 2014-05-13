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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dongleComm.h"
#include "dongleCommHid.h"
#include "dongleCommWinUSB.h"
#ifdef DEBUG_COMM
#include "hexUtils.h"
#endif

static dongleTransport transport = TRANSPORT_WINUSB;

void setTransport(dongleTransport transportParam) {
	transport = transportParam;	
}

int initDongle(void) {
	if (transport == TRANSPORT_HID) {
		return initHid();
	}
	else {
		return initWinUSB();
	}
}

int exitDongle(void) {
	if (transport == TRANSPORT_HID) {
		return exitHid();
	}
	else {
		return exitWinUSB();
	}
}

int sendApduDongle(dongleHandle handle, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw) 
{
	int result;
#ifdef DEBUG_COMM
	printf("=> ");
	displayBinary((unsigned char*)apdu, apduLength);
#endif	
	if (transport == TRANSPORT_HID) {
		result = sendApduHid((hid_device*)handle, apdu, apduLength, out, outLength, sw);
	}
	else {
		result = sendApduWinUSB((libusb_device_handle*)handle, apdu, apduLength, out, outLength, sw);
	}
#ifdef DEBUG_COMM
	if (result > 0) {
		printf("<= ");
		displayBinary(out, result);
	}
#endif		
	return result;
}

dongleHandle getFirstDongle() {
	if (transport == TRANSPORT_HID) {
		return (dongleHandle)getFirstDongleHid();
	}
	else {
		return (dongleHandle)getFirstDongleWinUSB();
	}
}

void closeDongle(dongleHandle handle) {
	if (transport == TRANSPORT_HID) {
		closeDongleHid((hid_device*)handle);
	}
	else {
		closeDongleWinUSB((libusb_device_handle*)handle);
	}
}

