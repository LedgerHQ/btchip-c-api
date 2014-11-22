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

#define FORMAT_BASE58 0x01
#define FORMAT_SEED 0x02

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned char in[260];
	unsigned char out[260];
	unsigned char encodedKey[100];
	int encodedKeyLength;
	int result;
	int sw;
	int apduSize;		

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [encoded private key hex]\n", argv[0]);
		return 0;
	}
	encodedKeyLength = hexToBin(argv[1], encodedKey, sizeof(encodedKey));
	if (encodedKeyLength < 0) {
		fprintf(stderr, "Invalid encoded key\n");
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
	in[apduSize++] = BTCHIP_INS_GET_PUBLIC_KEY;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = encodedKeyLength;
	memcpy(in + apduSize, encodedKey, encodedKeyLength);
	apduSize += encodedKeyLength;
	in[OFFSET_CDATA] = (apduSize - 5);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	closeDongle(dongle);
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
	if (out[0] == 0x01) {
		printf("Public key : ");
		displayBinary(out + 2, out[1]);
	}
	else
	if (out[0] == 0x02) {
		printf("Public key : ");
		displayBinary(out + 2, out[1]);
		apduSize = 2 + out[1];
		printf("Chain code : ");
		displayBinary(out + apduSize, 32);
		apduSize += 32;
		printf("Depth : %d\n", out[apduSize++]);
		printf("Parent fingerprint : ");
		displayBinary(out + apduSize, 4);
		apduSize += 4;
		printf("Child number : ");
		displayBinary(out + apduSize, 4);
		apduSize += 4;
	}
	return 1;
}
