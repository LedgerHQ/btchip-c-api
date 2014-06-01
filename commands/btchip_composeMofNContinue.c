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
	char pin[100];	
	unsigned char publicKey[100];
	int publicKeyLength;

	if (argc < 3) {
		fprintf(stderr, "Usage : %s [second factor ascii] [public key hex]\n", argv[0]);
		return 0;
	}
	if (strlen(argv[1]) > sizeof(pin) - 1) {
		fprintf(stderr, "Invalid second factor\n");
		return 0;
	}
	pin[sizeof(pin) - 1] = '\0';	
	strncpy(pin, argv[1], sizeof(pin) - 1);	
	publicKeyLength = hexToBin(argv[2], publicKey, sizeof(publicKey));
	if (publicKeyLength < 0) {
		fprintf(stderr, "Invalid public key\n");
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
	in[apduSize++] = BTCHIP_INS_COMPOSE_MOFN_ADDRESS;
	in[apduSize++] = 0x80;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = strlen(pin);
	memcpy(in + apduSize, pin, strlen(pin));
	apduSize += strlen(pin);
	memcpy(in + apduSize, publicKey, publicKeyLength);
	apduSize += publicKeyLength;
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
	printf("M of N address added, please powercycle to get the second factor then proceed with the next one or read the generated P2SH address\n");
	return 1;
}
