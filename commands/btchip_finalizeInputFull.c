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

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;		
	unsigned char* outputData;
	int outputDataLength;
	int offset = 0;
	int scriptBlockLength = 50;

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [output data] [block length (default 50, use 255 for older firmwares)]\n", argv[0]);
		return 0;
	}
	outputDataLength = strlen(argv[1]) / 2;
	outputData = (unsigned char*)malloc(outputDataLength);
	result = hexToBin(argv[1], outputData, outputDataLength);
	if (result < 0) {
		free(outputData);
		fprintf(stderr, "Invalid output data\n");
		return 0;		
	}
	if (argc > 2) {
		scriptBlockLength = atoi(argv[2]);
		if (scriptBlockLength < 50) {
			free(outputData);
			fprintf(stderr, "Invaid block length\n");
			return 0;
		}
	}
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}	
	while (offset < outputDataLength) {
		int blockLength = scriptBlockLength;
		int dataLength;
		unsigned char p1;
		if (offset + blockLength < outputDataLength) {
			dataLength = blockLength;
			p1 = 0x00;
		}
		else {
			dataLength = outputDataLength - offset;
			p1 = 0x80;
		}
		apduSize = 0;
		in[apduSize++] = BTCHIP_CLA;
		in[apduSize++] = BTCHIP_INS_HASH_INPUT_FINALIZE_FULL;
		in[apduSize++] = p1;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		memcpy(in + apduSize, outputData + offset, dataLength);
		apduSize += dataLength;
		offset += dataLength;		
		in[OFFSET_CDATA] = (apduSize - 5);
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
		if (result < 0) {
			closeDongle(dongle);
			exitDongle();
			free(outputData);
			fprintf(stderr, "I/O error\n");
			return 0;
		}
		if (sw != SW_OK) {
			closeDongle(dongle);
			exitDongle();
			free(outputData);			
			fprintf(stderr, "Dongle application error : %.4x\n", sw);
			return 0;
		}			
	}
	closeDongle(dongle);
	exitDongle();
	apduSize = 0;
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
