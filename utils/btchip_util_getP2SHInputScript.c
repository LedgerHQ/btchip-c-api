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
#ifdef EXTRA_DEBUG	
#include <assert.h>
#endif
#include "hexUtils.h"

#define OP_1_BEFORE 0x50
#define OP_PUSHDATA1 0x4c
#define OP_PUSHDATA2 0x4d
#define N_MAX 3

#define BUFFER_SIZE 100

int main(int argc, char **argv) {
	unsigned char *redeemScript;
	unsigned char *inputScript;
	unsigned char signature[BUFFER_SIZE * N_MAX];
	int signatureLength[N_MAX];
	int offset = 0;
	int m;
	int i;
	int inputScriptSize = 0;
	int extraPushSize = 0;
	int redeemScriptSize;

	if (argc < 3) {
		fprintf(stderr, "Usage : %s [redeemScript] M*[signature + hashtype]\n", argv[0]);
		return 0;
	}
	redeemScriptSize = strlen(argv[1]) / 2;
	redeemScript = (unsigned char*)malloc(redeemScriptSize);
	if (redeemScript == NULL) {
		fprintf(stderr, "Failed to allocate redeem script\n");
		return 0;
	}
	redeemScriptSize = hexToBin(argv[1], redeemScript, redeemScriptSize);
	if (redeemScriptSize <= 0) {
		fprintf(stderr, "Invalid redeem script\n");
		free(redeemScript);
		return 0;
	}
	m = redeemScript[0] - OP_1_BEFORE;
	if (argc != (2 + m)) {
		fprintf(stderr, "Invalid number of arguments\n");
		free(redeemScript);
		return 0;
	}
	for (i=0; i<m; i++) {
		signatureLength[i] = hexToBin(argv[2 + i], &signature[BUFFER_SIZE * i], BUFFER_SIZE);
		if (signatureLength[i] <= 0) {
			fprintf(stderr, "Invalid signature %d\n", (i + 1));
			free(redeemScript);
			return 0;
		}	
		inputScriptSize += signatureLength[i] + 1;
	}
	if (redeemScriptSize >= OP_PUSHDATA1) {
		if (redeemScriptSize > 255) {
			extraPushSize = 2;
		}
		else {
			extraPushSize = 1;
		}
	}
	inputScript = (unsigned char*)malloc(1 + inputScriptSize + redeemScriptSize + extraPushSize); // OP_0
	if (inputScript == NULL) {
		fprintf(stderr, "Failed to allocate input script\n");
		free(redeemScript);
		return 0;
	}
	inputScript[offset++] = 0x00; // CHECKMULTISIG bug workaround
	for (i=0; i<m; i++) {
		inputScript[offset++] = signatureLength[i];
		memcpy(inputScript + offset, &signature[BUFFER_SIZE * i], signatureLength[i]);
		offset += signatureLength[i];
	}
	if (extraPushSize == 0) {
		inputScript[offset++] = redeemScriptSize;
	}
	if (extraPushSize == 1) {
		inputScript[offset++] = OP_PUSHDATA1;
		inputScript[offset++] = redeemScriptSize;
	}
	else
	if (extraPushSize == 2) {
		inputScript[offset++] = OP_PUSHDATA2;
		inputScript[offset++] = (redeemScriptSize & 0xff);
		inputScript[offset++] = ((redeemScriptSize >> 8) & 0xff);
	}
	memcpy(inputScript + offset, redeemScript, redeemScriptSize);
	free(redeemScript);
	offset += redeemScriptSize;
	printf("Input script : ");
	displayBinary(inputScript, offset);
	free(inputScript);

	return 1;
}
