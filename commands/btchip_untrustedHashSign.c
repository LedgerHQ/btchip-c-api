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
	int chain;
	uint32_t account;
	uint32_t chainIndex;	
	char pin[100];
	uint32_t lockTime;
	unsigned char sigHashType;

	if (argc < 7) {
		fprintf(stderr, "Usage : %s [chain (INTERNAL or EXTERNAL)] [private key account number] [private key chain index] [second factor ascii, or empty string] [locktime or empty for default] [sighashType or empty for SIGHASH_ALL]\n", argv[0]);
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
	if (strlen(argv[4]) > sizeof(pin) - 1) {
		fprintf(stderr, "Invalid second factor\n");
		return 0;
	}
	pin[sizeof(pin) - 1] = '\0';	
	strncpy(pin, argv[4], sizeof(pin) - 1);
	if (strlen(argv[5]) == 0) {
		lockTime = 0;
	}
	else {
		result = strtol(argv[5], NULL, 10);
		if (result < 0) {
			fprintf(stderr, "Invalid chain index\n");
			return 0;
		}
		lockTime = result;
	}
	if (strlen(argv[6]) == 0) {
		sigHashType = 0x01;
	}
	else {
		result = hexToBin(argv[6], &sigHashType, sizeof(sigHashType));
		if (result < 0) {
			fprintf(stderr, "Invalid sigHashType\n");
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
	in[apduSize++] = BTCHIP_INS_HASH_SIGN;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	writeUint32BE(in + apduSize, account);
	apduSize += sizeof(account);
	writeUint32BE(in + apduSize, chainIndex);
	apduSize += sizeof(chainIndex);
	in[apduSize++] = chain;
	in[apduSize++] = strlen(pin);
	memcpy(in + apduSize, pin, strlen(pin));
	apduSize += strlen(pin);
	writeHexAmountBE(lockTime, in + apduSize);
	apduSize += sizeof(lockTime);
	in[apduSize++] = sigHashType;
	in[OFFSET_CDATA] = (apduSize - 5);
	printf("Singing, please wait ...\n");
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
	printf("Signature + hashtype : ");
	displayBinary(out, result);
	return 1;
}
