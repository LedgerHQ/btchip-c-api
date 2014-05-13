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
#include <string.h>
#include <inttypes.h>
#include "dongleComm.h"
#include "hexUtils.h"
#include "btchipApdu.h"
#include "btchipArgs.h"

int main(int argc, char **argv) {
	dongleHandle dongle;
	uint8_t operationModeFlag;
	uint8_t hexFeaturesFlag;
	uint8_t keyVersion;
	uint8_t keyVersionP2SH;
	unsigned char userPublicKey[100];
	int userPublicKeyLength;
	unsigned char passwordBlob[40];
	unsigned char userEntropy[32];
	unsigned char in[255];
	unsigned char out[255];
	int result;
	int sw;
	int apduSize;	
	char *arg;

	if (argc < 8) {
		fprintf(stderr, "Usage : %s with the following parameters\n", argv[0]);
		fprintf(stderr, "\tOperation mode flags combined with + (WALLET|RELAXED|SERVER|DEVELOPER)]\n");
		fprintf(stderr, "\tFeatures flag combined with + (UNCOMPRESSED_KEYS|RFC6979|FREE_SIGHASHTYPE)\n");
		fprintf(stderr, "\thex key version (1 byte or empty for bitcoin mainnet)\n");
		fprintf(stderr, "\thex key version P2SH (1 byte or empty for bitcoin mainnet)\n");
		fprintf(stderr, "\thex end user public key\n");
		fprintf(stderr, "\thex end user password blob\n");
		fprintf(stderr, "\tuser entropy (32 bytes)\n");
		return 0;
	}
	operationModeFlag = 0;
	arg = strtok(argv[1], "+");
	while (arg != NULL) {
		int flag;
		flag = convertMode(arg);
		if (flag < 0) {
			fprintf(stderr, "Invalid operation mode flag %s\n", arg);
			return 0;
		}
		operationModeFlag |= flag;
		arg = strtok(NULL, "+");
	}
	hexFeaturesFlag = 0;
	arg = strtok(argv[2], "+");
	while (arg != NULL) {
		int flag;
		flag = convertOption(arg);
		if (flag < 0) {
			fprintf(stderr, "Invalid feature flag %s\n", arg);
			return 0;
		}
		hexFeaturesFlag |= flag;
		arg = strtok(NULL, "+");
	}	
	if (strlen(argv[3]) == 0) {
		argv[3] = "00";
	}
	result = hexToBin(argv[3], &keyVersion, sizeof(keyVersion));
	if (result == 0) {
		fprintf(stderr, "Invalid key version\n");
		return 0;
	}		
	if (strlen(argv[4]) == 0) {
		argv[4] = "05";
	}
	result = hexToBin(argv[4], &keyVersionP2SH, sizeof(keyVersionP2SH));
	if (result == 0) {
		fprintf(stderr, "Invalid P2SH key version\n");
		return 0;
	}			
	userPublicKeyLength = hexToBin(argv[5], userPublicKey, sizeof(userPublicKey));
	if (userPublicKeyLength == 0) {
		fprintf(stderr, "Invalid user public key\n");
		return 0;
	}			
	result = hexToBin(argv[6], passwordBlob, sizeof(passwordBlob));
	if (result != sizeof(passwordBlob)) {
		fprintf(stderr, "Invalid password blob\n");
		return 0;
	}
	if (strlen(argv[7]) == 0) {
		memset(userEntropy, 0, sizeof(userEntropy));
	}
	else {
		result = hexToBin(argv[7], userEntropy, sizeof(userEntropy));
		if (result == 0) {
			fprintf(stderr, "Invalid user entropy\n");
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
	in[apduSize++] = BTCHIP_INS_SETUP;
	in[apduSize++] = 0x80;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = operationModeFlag;
	in[apduSize++] = hexFeaturesFlag;
	in[apduSize++] = keyVersion;
	in[apduSize++] = keyVersionP2SH;
	in[apduSize++] = userPublicKeyLength;
	memcpy(in + apduSize, userPublicKey, userPublicKeyLength);
	apduSize += userPublicKeyLength;
	memcpy(in + apduSize, passwordBlob, sizeof(passwordBlob));
	apduSize += sizeof(passwordBlob);
	in[apduSize++] = 0x00;
	memcpy(in + apduSize, userEntropy, sizeof(userEntropy));
	apduSize += sizeof(userEntropy);
	in[apduSize++] = 0x00;
	in[OFFSET_CDATA] = (apduSize - 5);
	printf("Setup in progress, please wait ...\n");
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
	printf("Setup completed, seed generated\n");
	apduSize = 0;
	printf("Trusted input key : ");
	displayBinary(out + apduSize, 16);
	apduSize += 16;
	printf("Developer key wrapping key : ");
	displayBinary(out + apduSize, 16);
	apduSize += 16;
	return 1;
}
