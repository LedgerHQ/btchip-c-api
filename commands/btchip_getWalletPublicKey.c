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
	int chain;
	uint32_t account;
	uint32_t chainIndex;
	unsigned char in[255];
	unsigned char out[255];
	int result;
	int sw;
	int apduSize;	

	if (argc < 4) {
		fprintf(stderr, "Usage : %s [chain (INTERNAL or EXTERNAL)] [account number] [chain index]\n", argv[0]);
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
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_GET_WALLET_PUBLIC_KEY;
	in[apduSize++] = chain;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x08;
	writeUint32BE(in + apduSize, account);
	apduSize += sizeof(account);
	writeUint32BE(in + apduSize, chainIndex);
	apduSize += sizeof(chainIndex);
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
	out[apduSize + 1 + out[apduSize]] = '\0';
	printf("Address : %s\n", out + apduSize + 1);
	return 1;
}
