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
#include "dongleCommHidHidapi.h"
#include "dongleCommWinUSB.h"
#ifdef DEBUG_COMM
#include "hexUtils.h"
#endif

typedef enum {
	TRANSPORT_NONE,
	TRANSPORT_HID,
	TRANSPORT_WINUSB,
	TRANSPORT_HID_HIDAPI
} dongleTransport;

typedef struct dongleHandleInternal {
	dongleTransport transport;
	unsigned char ledger;
	void* handle;
} dongleHandleInternal;

int initDongle(void) {
	int result = -1;
#ifdef HAVE_LIBUSB	
	result = initHid();
	if (result < 0) {
		return result;
	}
	result = initWinUSB();
	if (result < 0) {
		return result;
	}
#endif
#ifdef HAVE_HIDAPI
	result = initHidHidapi();
	if (result < 0) {
		return result;
	}
#endif	
	return result;
}

int exitDongle(void) {
	int result = -1;
#ifdef HAVE_LIBUSB	
	result = exitHid();
	if (result < 0) {
		return result;
	}
	result = exitWinUSB();
	if (result < 0) {
		return result;
	}
#endif
#ifdef HAVE_HIDAPI
	result = exitHidHidapi();
	if (result < 0)	 {
		return result;
	}
#endif	
	return result;
}

int sendApduDongle(dongleHandle handle, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw) 
{
	int result = -1;
#ifdef DEBUG_COMM
	printf("=> ");
	displayBinary((unsigned char*)apdu, apduLength);
#endif		
#ifdef HAVE_LIBUSB	
	if (handle->transport == TRANSPORT_HID) {
		result = sendApduHid((libusb_device_handle*)handle->handle, handle->ledger, apdu, apduLength, out, outLength, sw);
	}
	else
	if (handle->transport == TRANSPORT_WINUSB) {
		result = sendApduWinUSB((libusb_device_handle*)handle->handle, apdu, apduLength, out, outLength, sw);
	}
#endif
#ifdef HAVE_HIDAPI
	if (handle->transport == TRANSPORT_HID_HIDAPI) {
		result = sendApduHidHidapi((hid_device*)handle->handle, handle->ledger, apdu, apduLength, out, outLength, sw);
	}
#endif	
	if (result < 0) {
		return -1;
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
	dongleHandle result = (dongleHandle)malloc(sizeof(dongleHandleInternal));
	if (result == NULL) {
		return result;
	}
	result->ledger = 0;
#ifdef HAVE_LIBUSB		
	result->transport = TRANSPORT_HID;
	result->handle = getFirstDongleHid(&result->ledger);
	if (result->handle != NULL) {
		return result;
	}
	result->transport = TRANSPORT_WINUSB;
	result->handle = getFirstDongleWinUSB();
	if (result->handle != NULL) {
		return result;
	}
#endif
#ifdef HAVE_HIDAPI
	result->transport = TRANSPORT_HID_HIDAPI;
	result->handle = getFirstDongleHidHidapi(&result->ledger);
	if (result->handle != NULL) {
		return result;
	}
#endif	
	free(result);
	return NULL;
}

void closeDongle(dongleHandle handle) {
#ifdef HAVE_LIBUSB	
	if (handle->transport == TRANSPORT_HID) {
		closeDongleHid((libusb_device_handle*)handle->handle);
	}
	else
	if (handle->transport == TRANSPORT_WINUSB) {
		closeDongleWinUSB((libusb_device_handle*)handle->handle);
	}
#endif	
#ifdef HAVE_HIDAPI
	if (handle->transport == TRANSPORT_HID_HIDAPI) {
		closeDongleHidHidapi((hid_device*)handle->handle);
	}
#endif	
	handle->transport = TRANSPORT_NONE;
	free(handle);
}

