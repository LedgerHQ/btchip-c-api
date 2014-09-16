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
#include "dongleComm.h"
#include "hexUtils.h"
#include "btchipApdu.h"
#include "btchipArgs.h"
#include "btchipUtils.h"

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned int keyPath[10];
	int keyPathLength;
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;	
	int i;
	char address[40];

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [key path in a/b/c format using n' for hardened nodes]\n", argv[0]);
		return 0;
	}
	keyPathLength = convertPath(argv[1], keyPath);
	if (keyPathLength < 0) {
		fprintf(stderr, "Invalid key path\n");
		return 0;
	}
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_GET_WALLET_PUBLIC_KEY;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x01 + (4 * keyPathLength);
	in[apduSize++] = keyPathLength;
	for (i=0; i<keyPathLength; i++) {
		writeUint32BE(in + apduSize, keyPath[i]);
		apduSize += 4;
	}
	printf("Computing public key, please wait ...\n");
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
	printf("Uncompressed public key : ");
	displayBinary(out + 1, out[0]);
	apduSize += out[0] + 1;
	memcpy((unsigned char*)address, out + apduSize + 1, out[apduSize]);
	address[out[apduSize]] = '\0';
	apduSize += out[apduSize] + 1;
	printf("Address : %s\n", address);
	printf("Chaincode : ");
	displayBinary(out + apduSize, 32);
	return 1;
}
