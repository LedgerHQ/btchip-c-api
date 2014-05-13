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
	unsigned char in[255];
	unsigned char out[255];
	int result;
	int sw;
	int apduSize;		
	int chain;
	uint32_t account;
	uint32_t chainIndex;	
	char message[140];

	if (argc < 5) {
		fprintf(stderr, "Usage : %s [chain (INTERNAL or EXTERNAL)] [private key account number] [private key chain index] [message]\n", argv[0]);
		return 0;
	}
	chain = convertChain(argv[1]);
	if (chain < 0) {
		fprintf(stderr, "Invalid chain\n");
		return 0;
	}
	result = strtol(argv[2], NULL, 10);
	if (result < 0) {
		fprintf(stderr, "Invalid account number\n");
		return 0;
	}
	account = result;
	result = strtol(argv[3], NULL, 10);
	if (result < 0) {
		fprintf(stderr, "Invalid chain index\n");
		return 0;
	}
	chainIndex = result;
	if (strlen(argv[4]) > sizeof(message) - 1) {
		fprintf(stderr, "Invalid message\n");
		return 0;
	}
	message[sizeof(message) - 1] = '\0';	
	strncpy(message, argv[4], sizeof(message) - 1);
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
	writeUint32BE(in + apduSize, account);
	apduSize += sizeof(account);
	writeUint32BE(in + apduSize, chainIndex);
	apduSize += sizeof(chainIndex);
	in[apduSize++] = chain;
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
