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
#include "bitcoinTransaction.h"

int main(int argc, char **argv) {
	unsigned char *buffer;
	unsigned char *bufferCheckSerialize;
	bitcoinTransaction *transaction;	
	int result;
	int transactionLength;
	int transactionLengthSerialized;

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [raw transaction]\n", argv[0]);
		return 0;
	}
	transactionLength = (strlen(argv[1]) / 2);
	buffer = (unsigned char*)malloc(transactionLength);
	if (buffer == NULL) {
		fprintf(stderr, "Couldn't allocate transaction buffer\n");
		return 0;
	}
	result = hexToBin(argv[1], buffer, transactionLength);
	if (result == 0) {
		free(buffer);
		fprintf(stderr, "Invalid raw transaction\n");
		return 0;
	}
	transaction = parseTransaction(buffer, result);
	if (transaction == NULL) {
		free(buffer);
		fprintf(stderr, "Failed to parse raw transaction\n");
		return 0;
	}
	debugTransaction(transaction);
#ifdef EXTRA_DEBUG	
	transactionLengthSerialized = computeTransactionBufferSize(transaction);	
	assert(transactionLengthSerialized == transactionLength);
	bufferCheckSerialize = (unsigned char*)malloc(transactionLengthSerialized);
	assert(bufferCheckSerialize != NULL);
	assert(writeTransaction(transaction, bufferCheckSerialize, transactionLengthSerialized) == transactionLengthSerialized);
	assert(memcmp(bufferCheckSerialize, buffer, transactionLengthSerialized) == 0);	
	free(bufferCheckSerialize);
#endif	
	freeTransaction(transaction);
	free(buffer);
	return 1;
}
