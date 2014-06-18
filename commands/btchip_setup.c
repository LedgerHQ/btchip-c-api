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

const char * QWERTY_KEYMAP = "000000000000000000000000760f00d4ffffffc7000000782c1e3420212224342627252e362d3738271e1f202122232425263333362e37381f0405060708090a0b0c0d0e0f101112131415161718191a1b1c1d2f3130232d350405060708090a0b0c0d0e0f101112131415161718191a1b1c1d2f313035";
const char * AZERTY_KEYMAP = "08000000010000200100007820c8ffc3feffff07000000002c38202030341e21222d352e102e3637271e1f202122232425263736362e37101f1405060708090a0b0c0d0e0f331112130415161718191d1b1c1a2f64302f2d351405060708090a0b0c0d0e0f331112130415161718191d1b1c1a2f643035";

int main(int argc, char **argv) {
	dongleHandle dongle;
	uint8_t operationModeFlag;
	uint8_t hexFeaturesFlag;
	uint8_t keyVersion;
	uint8_t keyVersionP2SH;
	unsigned char pin[0x20];
	int pinLength;	
	unsigned char wipePin[0x04];
	int wipePinLength;
	unsigned char keymapEncoding[119];
	unsigned char seed[32];
	int seedLength;
	unsigned char userEntropy[32];
	unsigned char developerKey[16];
	int developerKeyLength;
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;	
	char *arg;

	if (argc < 11) {
		fprintf(stderr, "Usage : %s with the following parameters\n", argv[0]);
		fprintf(stderr, "\tOperation mode flags combined with + (WALLET|RELAXED|SERVER|DEVELOPER)]\n");
		fprintf(stderr, "\tFeatures flag combined with + (UNCOMPRESSED_KEYS|RFC6979|FREE_SIGHASHTYPE)\n");
		fprintf(stderr, "\thex key version (1 byte or empty for bitcoin mainnet)\n");
		fprintf(stderr, "\thex key version P2SH (1 byte or empty for bitcoin mainnet)\n");
		fprintf(stderr, "\thex user pin (min 4 bytes)\n");
		fprintf(stderr, "\thex wipe pin (or empty)\n");
		fprintf(stderr, "\tkeymap encoding (QWERTY or AZERTY or 119 bytes)\n");
		fprintf(stderr, "\tseed (32 bytes or empty to generate a new one)\n");
		fprintf(stderr, "\tuser entropy (32 bytes or empty, must be empty if restoring a seed)\n");
		fprintf(stderr, "\tdeveloper key (16 bytes or empty)\n");
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
	pinLength = hexToBin(argv[5], pin, sizeof(pin));
	if (pinLength == 0) {
		fprintf(stderr, "Invalid PIN\n");
		return 0;
	}			
	if (pinLength < 4) {
		fprintf(stderr, "Minimum PIN length is 4 bytes\n");
		return 0;		
	}
	if (strlen(argv[6]) == 0) {
		wipePinLength = 0;
	}
	else {
		wipePinLength = hexToBin(argv[6], wipePin, sizeof(wipePin));
		if (wipePinLength == 0) {
			fprintf(stderr, "Invalid wipe PIN\n");
			return 0;
		}			
	}
	if (strcasecmp(argv[7], "qwerty") == 0) {
		argv[7] = (char*)QWERTY_KEYMAP;
	}
	else
	if (strcasecmp(argv[7], "azerty") == 0) {
		argv[7] = (char*)AZERTY_KEYMAP;
	}	
	result = hexToBin(argv[7], keymapEncoding, sizeof(keymapEncoding));
	if (result == 0) {
		fprintf(stderr, "Invalid keymap encoding\n");
		return 0;
	}		
	if (result != sizeof(keymapEncoding)) {
		fprintf(stderr, "Invalid keymap encoding length\n");
		return 0;
	}	
	if (strlen(argv[8]) == 0) {
		seedLength = 0;
	}
	else {
		result = hexToBin(argv[8], seed, sizeof(seed));
		if (result == 0) {
			fprintf(stderr, "Invalid seed\n");
			return 0;
		}
		if (result != sizeof(seed)) {
			fprintf(stderr, "Invalid seed length\n");
			return 0;
		}
		seedLength = result;
	}
	if (strlen(argv[9]) == 0) {
		memset(userEntropy, 0, sizeof(userEntropy));
	}
	else {
		if (seedLength != 0) {
			fprintf(stderr, "Cannot provide user entropy when restoring a seed\n");
			return 0;
		}
		result = hexToBin(argv[9], userEntropy, sizeof(userEntropy));
		if (result == 0) {
			fprintf(stderr, "Invalid user entropy\n");
			return 0;
		}
	}
	if (strlen(argv[10]) == 0) {
		developerKeyLength = 0;
	}
	else {
		developerKeyLength = hexToBin(argv[10], developerKey, sizeof(developerKey));
		if (developerKeyLength == 0) {
			fprintf(stderr, "Invalid developer key\n");
			return 0;
		}
		if (developerKeyLength != sizeof(developerKey)) {
			fprintf(stderr, "Invalid developer key length\n");
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
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = operationModeFlag;
	in[apduSize++] = hexFeaturesFlag;
	in[apduSize++] = keyVersion;
	in[apduSize++] = keyVersionP2SH;
	in[apduSize++] = pinLength;
	memcpy(in + apduSize, pin, pinLength);
	apduSize += pinLength;
	in[apduSize++] = wipePinLength;
	memcpy(in + apduSize, wipePin, wipePinLength);
	apduSize += wipePinLength;
	memcpy(in + apduSize, keymapEncoding, sizeof(keymapEncoding));
	apduSize += sizeof(keymapEncoding);
	in[apduSize++] = (seedLength == 0 ? 0x00 : 0x01);
	if (seedLength != 0) {
		memcpy(in + apduSize, seed, sizeof(seed));
		apduSize += sizeof(seed);
	}
	else {
		memcpy(in + apduSize, userEntropy, sizeof(userEntropy));
		apduSize += sizeof(userEntropy);
	}	
	in[apduSize++] = developerKeyLength;
	if (developerKeyLength != 0) {
		memcpy(in + apduSize, developerKey, sizeof(developerKey));
		apduSize += sizeof(developerKey);
	}
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
	if (seedLength == 0) {
		printf("Setup completed, seed generated\n");
	}
	else {
		printf("Setup completed, seed restored\n");
	}
	apduSize = 0;
	printf("Trusted input key : ");
	displayBinary(out + apduSize, 16);
	apduSize += 16;
	printf("Developer key wrapping key : ");
	displayBinary(out + apduSize, 16);
	apduSize += 16;
	if (seedLength == 0) {
		printf("Powercycle to read the generated seed\n");
	}
	return 1;
}
