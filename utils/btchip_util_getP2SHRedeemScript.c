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
#define OP_CHECKMULTISIG 0xAE
#define N_MAX 3

#define BUFFER_SIZE 100

int main(int argc, char **argv) {
	unsigned char *redeemScript;
	unsigned char publicKey[BUFFER_SIZE * N_MAX];
	int publicKeyLength[N_MAX];
	int offset = 0;
	int m;
	int n;
	int i;
	int inputScriptSize = 0;

	if (argc < 3) {
		fprintf(stderr, "Usage : %s [M] [N] N*[public key]\n", argv[0]);
		return 0;
	}
	m = atoi(argv[1]);
	n = atoi(argv[2]);
	if ((m == 0) || (n == 0) || (n > N_MAX) || (m > n)) {
		fprintf(stderr, "Invalid M / N\n");
		return 0;
	}
	if (argc != (3 + n)) {
		fprintf(stderr, "Invalid number of arguments\n");
		return 0;
	}
	for (i=0; i<n; i++) {
		publicKeyLength[i] = hexToBin(argv[3 + i], &publicKey[BUFFER_SIZE * i], BUFFER_SIZE);
		if (publicKeyLength[i] <= 0) {
			fprintf(stderr, "Invalid public key %d\n", (i + 1));
			return 0;
		}
		inputScriptSize += publicKeyLength[i] + 1;
	}
	redeemScript = (unsigned char*)malloc(inputScriptSize + 3); // M - N - CHECKMULTISIG
	if (redeemScript == NULL) {
		fprintf(stderr, "Failed to allocate redeem script\n");
		return 0;
	}
	redeemScript[offset++] = OP_1_BEFORE + m;
	for (i=0; i<n; i++) {
		redeemScript[offset++] = publicKeyLength[i];
		memcpy(redeemScript + offset, &publicKey[BUFFER_SIZE * i], publicKeyLength[i]);
		offset += publicKeyLength[i];
	}
	redeemScript[offset++] = OP_1_BEFORE + n;
	redeemScript[offset++] = OP_CHECKMULTISIG;	
	printf("Redeem script : ");
	displayBinary(redeemScript, offset);
	free(redeemScript);
	return 1;
}
