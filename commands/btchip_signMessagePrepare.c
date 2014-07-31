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
#include <inttypes.h>
#include <string.h>
#include "dongleComm.h"
#include "hexUtils.h"
#include "btchipApdu.h"
#include "bitcoinTransaction.h"
#include "bitcoinVarint.h"
#include "bitcoinAmount.h"
#include "btchipUtils.h"
#include "btchipArgs.h"
#include "btchipTrustedInput.h"

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;		
	char message[140];
	unsigned int keyPath[10];
	int keyPathLength;	
	int i;

	if (argc < 3) {
		fprintf(stderr, "Usage : %s [key path in a/b/c format using n' for hardened nodes] [message]\n", argv[0]);
		return 0;
	}
	keyPathLength = convertPath(argv[1], keyPath);
	if (keyPathLength < 0) {
		fprintf(stderr, "Invalid key path\n");
		return 0;
	}
	if (strlen(argv[2]) > sizeof(message) - 1) {
		fprintf(stderr, "Invalid message\n");
		return 0;
	}
	message[sizeof(message) - 1] = '\0';	
	strncpy(message, argv[2], sizeof(message) - 1);
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}	
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_SIGN_MESSAGE;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = keyPathLength;
	for (i=0; i<keyPathLength; i++) {
		writeUint32BE(in + apduSize, keyPath[i]);
		apduSize += 4;
	}		
	in[apduSize++] = strlen(message);
	memcpy(in + apduSize, message, strlen(message));
	apduSize += strlen(message);
	in[OFFSET_CDATA] = (apduSize - 5);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	exitDongle();
	if (result < 0) {
		fprintf(stderr, "I/O error\n");
		return 0;
	}
	if (sw != SW_OK) {
		fprintf(stderr, "Dongle application error : %.4x\n", sw);
		return 0;
	}	
	apduSize = 0;
	if (out[apduSize] == 0x00) {
		printf("Message signature prepared, proceed with signing\n");
	}
	else
	if (out[apduSize] == 0x01) {
		printf("Message signature prepared, please powercycle to get the second factor then proceed with signing\n");
	}
	else {
		fprintf(stderr, "Invalid transaction state %.2x\n", out[apduSize]);
		return 0;
	}		
	return 1;
}
