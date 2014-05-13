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
	unsigned char in[255];
	unsigned char out[255];
	unsigned char seed[65];
	int seedLength;
	int result;
	int sw;
	int apduSize;		
	int importFormat;

	if (argc < 3) {
		fprintf(stderr, "Usage : %s [imported data format (BASE58 or BIP32SEED)] [private key encoded ascii or hex encoded seed]\n", argv[0]);
		return 0;
	}
	if (strcasecmp(argv[1], "base58") == 0) {
		importFormat = FORMAT_BASE58;
	}
	else
	if (strcasecmp(argv[1], "bip32seed") == 0) {
		importFormat = FORMAT_SEED;
	}
	else {
		fprintf(stderr, "Invalid import format %s\n", argv[1]);
		return 0;
	}
	if (importFormat == FORMAT_BASE58) {
		if ((strlen(argv[2]) == 0) || (strlen(argv[2]) > 255)) {
			fprintf(stderr, "Invalid private key\n");
			return 0; 		
		}
	}
	else {
		seedLength = hexToBin(argv[2], seed, sizeof(seed));
		if ((seedLength < 0) || (seedLength == 65)) {
			fprintf(stderr, "Invalid seed\n");
			return 0;
		}
	}
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}	
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_IMPORT_PRIVATE_KEY;
	in[apduSize++] = importFormat;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	if (importFormat == FORMAT_BASE58) {
		memcpy(in + apduSize, argv[2], strlen(argv[2]));
		apduSize += strlen(argv[2]);
	}
	else {
		memcpy(in + apduSize, seed, seedLength);
		apduSize += seedLength;
	}
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
	printf("Encoded private key : ");
	displayBinary(out, result);
	return 1;
}
