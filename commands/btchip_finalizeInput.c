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
#include "bitcoinTransaction.h"
#include "bitcoinVarint.h"
#include "bitcoinAmount.h"
#include "btchipUtils.h"
#include "btchipTrustedInput.h"

#define PREVOUT_SIZE 36

const unsigned char DEFAULT_VERSION[] = { 0x01, 0x00, 0x00, 0x00 };
const unsigned char DEFAULT_SEQUENCE[] = { 0xFF, 0xFF, 0xFF, 0xFF };

typedef struct prevout {
	unsigned char prevout[TRUSTED_INPUT_SIZE];
	unsigned char isTrusted;
	uint32_t outputIndex;
} prevout;

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned char in[255];
	unsigned char out[255];
	int result;
	int sw;
	int apduSize;		
	char address[100];
	int64_t amount;
	int64_t fees;
	uint32_t changeAccount;
	uint32_t changeIndex;

	if (argc < 6) {
		fprintf(stderr, "Usage : %s [output address] [amount (in BTC string)] [fees (in BTC string)] [account number for change] [index for change]\n", argv[0]);
		return 0;
	}
	address[sizeof(address) - 1] = '\0';
	strncpy(address, argv[1], sizeof(address) - 1);
	if (parseStringAmount(argv[2], &amount) < 0) {
		fprintf(stderr, "Invalid amount\n");
		return 0;
	}
	if (parseStringAmount(argv[3], &fees) < 0) {
		fprintf(stderr, "Invalid fees\n");
		return 0;
	}
	result = strtol(argv[4], NULL, 10);
	if (result < 0) {
		fprintf(stderr, "Invalid change account number\n");
		return 0;
	}
	changeAccount = result;
	result = strtol(argv[5], NULL, 10);
	if (result < 0) {
		fprintf(stderr, "Invalid change index\n");
		return 0;
	}
	changeIndex = result;
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}	
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_HASH_INPUT_FINALIZE;
	in[apduSize++] = 0x02;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = strlen(address);	
	memcpy(in + apduSize, address, strlen(address));
	apduSize += strlen(address);
	writeHexAmountBE(amount, in + apduSize);
	apduSize += sizeof(amount);
	writeHexAmountBE(fees, in + apduSize);
	apduSize += sizeof(fees);
	writeUint32BE(in + apduSize, changeAccount);
	apduSize += sizeof(changeAccount);
	writeUint32BE(in + apduSize, changeIndex);
	apduSize += sizeof(changeIndex);
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
	printf("Output script : ");
	displayBinary(out + 1, out[0]);
	apduSize = 1 + out[0];
	if (out[apduSize] == 0x00) {
		printf("Input finalized, proceed with signing\n");
	}
	else
	if (out[apduSize] == 0x01) {
		printf("Input finalized, please powercycle to get the second factor then proceed with signing\n");
	}
	else {
		fprintf(stderr, "Invalid transaction state %.2x\n", out[apduSize]);
		return 0;
	}	
	return 1;
}
