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
#include "btchipUtils.h"
#include "hexUtils.h"

unsigned char* writeUint32BE(unsigned char *buffer, uint32_t value) {
	*buffer = ((value >> 24) & 0xff);
	*(buffer + 1) = ((value >> 16) & 0xff);
	*(buffer + 2) = ((value >> 8) & 0xff);
	*(buffer + 3) = (value & 0xff);
	return (buffer + 4);
}

bitcoinTransaction* parseTransactionStringWithIndex(char *transactionString, uint32_t *index) {
	char *rawTransaction;
	char *accountIndex;
	unsigned char *buffer;
	int result;
	bitcoinTransaction *transaction;
	int transactionLength;

	rawTransaction = strtok(transactionString, ":");
	if (rawTransaction == NULL) {
		return NULL;
	}
	accountIndex = strtok(NULL, ":");
	if (accountIndex == NULL) {
		return NULL;
	}
	result = strtol(accountIndex, NULL, 10);
	if (result < 0) {
		return NULL;
	}
	*index = result;
	transactionLength = (strlen(rawTransaction) / 2);
	buffer = (unsigned char*)malloc(transactionLength);
	if (buffer == NULL) {
		return NULL;		
	}
	result = hexToBin(rawTransaction, buffer, transactionLength);
	if (result == 0) {
		free(buffer);
		return NULL;
	}
	transaction = parseTransaction(buffer, result);
	if (transaction == NULL) {
		free(buffer);
		return 0;
	}
	free(buffer);
	return transaction;
}

