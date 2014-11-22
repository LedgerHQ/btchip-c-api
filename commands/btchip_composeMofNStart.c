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
	uint32_t m;
	uint32_t n;	
	unsigned char publicKey[100];
	int publicKeyLength;

	if (argc < 4) {
		fprintf(stderr, "Usage : %s [M (must be smaller than N)] [N (must be smaller than 3, included)] [public key hex]\n", argv[0]);
		return 0;
	}
	result = strtol(argv[1], NULL, 10);
	if (result < 0) {
		fprintf(stderr, "Invalid m\n");
		return 0;
	}
	m = result;
	result = strtol(argv[2], NULL, 10);
	if (result < 0) {
		fprintf(stderr, "Invalid n\n");
		return 0;
	}
	n = result;
	if (n > 3) {
		fprintf(stderr, "Invalid n (must be smaller or equal to 3)\n");
		return 0;		
	}
	if (m > n) {
		fprintf(stderr, "Invalid m (must be smaller than n)\n");
		return 0;				
	}
	publicKeyLength = hexToBin(argv[3], publicKey, sizeof(publicKey));
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
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = (unsigned char)m;
	in[apduSize++] = (unsigned char)n;
	memcpy(in + apduSize, publicKey, publicKeyLength);
	apduSize += publicKeyLength;
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
	printf("M of N address started, please powercycle to get the second factor then proceed with the next one\n");
	return 1;
}
