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
#include <errno.h>
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
	uint32_t index;
	int result;
	int sw;
	int apduSize;		

	if (argc < 3) {
		fprintf(stderr, "Usage : %s [encoded private key hex] [index (base 16)]\n", argv[0]);
		return 0;
	}
	encodedKeyLength = hexToBin(argv[1], encodedKey, sizeof(encodedKey));
	if (encodedKeyLength < 0) {
		fprintf(stderr, "Invalid encoded key\n");
		return 0;
	}
	errno = 0;
	index = strtoll(argv[2], NULL, 16);
	if (errno != 0) {
		fprintf(stderr, "Invalid index\n");
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
	in[apduSize++] = BTCHIP_INS_DERIVE_BIP32_KEY;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = encodedKeyLength;
	memcpy(in + apduSize, encodedKey, encodedKeyLength);
	apduSize += encodedKeyLength;
	writeUint32BE(in + apduSize, index);
	apduSize += sizeof(index);
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
	printf("Encoded private key : ");
	displayBinary(out, result);	
	return 1;
}
