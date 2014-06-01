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
#include "btchipUtils.h"
#include "bitcoinTransaction.h"

#define DEFAULT_VERSION 1
#define DEFAULT_LOCKTIME 0
#define DEFAULT_SEQUENCE 0xFF

#define BUFFER_SIZE 4096

int main(int argc, char **argv) {
	bitcoinTransaction *transaction;
	int offset = 0;
	int result;
	int version = DEFAULT_VERSION;
	int lockTime = DEFAULT_LOCKTIME;
	int i;
	int currentIndex = 1;
	bitcoinInput *lastInput = NULL;
	unsigned char *buffer;


	if (argc < 6) {
		fprintf(stderr, "Usage : %s [version (or empty for default)] [locktime (or empty for default)] [dongle output data] [trusted input 1] [input script 1] ... [last trusted input] [last input script]\n", argv[0]);
		return 0;
	}
	if (((argc - 4) % 2) != 0) {
		fprintf(stderr, "Invalid number of trusted input / input script parameters\n");
		return 0;
	}
	if (strlen(argv[1]) != 0) {
		version = strtol(argv[1], NULL, 10);
		if (version < 0) {
			fprintf(stderr, "Invalid version\n");
			return 0;
		}
	}
	if (strlen(argv[2]) != 0) {
		lockTime = strtol(argv[2], NULL, 10);
		if (lockTime < 0) {
			fprintf(stderr, "Invalid lockTime\n");
			return 0;
		}
	}	
	transaction = (bitcoinTransaction*)malloc(sizeof(bitcoinTransaction));
	if (transaction == NULL) {
		fprintf(stderr, "Failed to allocate transaction\n");
		return 0;
	}
	memset(transaction, 0, sizeof(bitcoinTransaction));
	writeUint32LE(transaction->version, version);
	for (i=4; i<argc; i += 2) {
		unsigned char trustedInput[56];
		bitcoinInput *input;
		input = (bitcoinInput*)malloc(sizeof(bitcoinInput));
		if (input == NULL) {
			fprintf(stderr, "Failed to allocate input\n");
			freeTransaction(transaction);
			return 0;
		}
		memset(input, 0, sizeof(bitcoinInput));
		result = hexToBin(argv[i], trustedInput, sizeof(trustedInput));
		if (result <= 0) {
			fprintf(stderr, "Invalid trustedInput %d\n", currentIndex);
			freeTransaction(transaction);
			free(input);
			return 0;
		}
		if (lastInput == NULL) {
			transaction->inputs = input;
		}
		else {
			lastInput->next = input;
		}
		input->scriptLength = (strlen(argv[i + 1]) / 2);
		input->script = (unsigned char*)malloc(input->scriptLength);
		if (input->script == NULL) {
			fprintf(stderr, "Failed to allocate script\n");
			freeTransaction(transaction);
			free(input);			
			return 0;
		}
		result = hexToBin(argv[i + 1], input->script, input->scriptLength);
		if (result <= 0) {
			fprintf(stderr, "Invalid script %d\n", currentIndex);
			freeTransaction(transaction);
			free(input);			
			return 0;
		}
		memcpy(input->prevOut, trustedInput + 4, sizeof(input->prevOut));
		memset(input->sequence, DEFAULT_SEQUENCE, sizeof(input->sequence));
		lastInput = input;
		currentIndex++;
	}	
	buffer = (unsigned char*)malloc(BUFFER_SIZE);
	if (buffer == NULL) {
		fprintf(stderr, "Failed to allocate output buffer\n");
		freeTransaction(transaction);
	}
	offset = writeTransactionWithoutOutputLocktime(transaction, buffer, BUFFER_SIZE);
	freeTransaction(transaction);
	result = hexToBin(argv[3], buffer + offset, BUFFER_SIZE - offset);
	if (result <= 0) {
		fprintf(stderr, "Invalid output\n");
		freeTransaction(transaction);
		free(buffer);
	}
	offset += result;
	writeUint32LE(buffer + offset, lockTime);
	offset += 4;
	printf("Transaction : ");
	displayBinary(buffer, offset);
	free(buffer);
	return 1;
}
