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

int main(int argc, char **argv) {
	unsigned char *inputScript;
	unsigned char signature[100];
	unsigned char publicKey[100];
	int signatureLength;
	int publicKeyLength;
	int offset = 0;

	if (argc < 3) {
		fprintf(stderr, "Usage : %s [signature + hashtype] [public key]\n", argv[0]);
		return 0;
	}
	signatureLength = hexToBin(argv[1], signature, sizeof(signature));
	if (signatureLength <= 0) {
		fprintf(stderr, "Invalid signature\n");
		return 0;
	}	
	publicKeyLength = hexToBin(argv[2], publicKey, sizeof(publicKey));
	if (publicKeyLength <= 0) {
		fprintf(stderr, "Invalid public key\n");
		return 0;
	}
	inputScript = (unsigned char*)malloc(signatureLength + publicKeyLength + 2);
	if (inputScript == NULL) {
		fprintf(stderr, "Failed to allocate input script\n");
		return 0;
	}
	inputScript[offset++] = signatureLength;
	memcpy(inputScript + offset, signature, signatureLength);
	offset += signatureLength;
	inputScript[offset++] = publicKeyLength;
	memcpy(inputScript + offset, publicKey, publicKeyLength);
	offset += publicKeyLength;
	printf("Input script : ");
	displayBinary(inputScript, offset);
	free(inputScript);
	return 1;
}
