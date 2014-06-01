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
#include "btchipUtils.h"
#include "btchipTrustedInput.h"

#define PREVOUT_SIZE 36

const unsigned char DEFAULT_VERSION[] = { 0x01, 0x00, 0x00, 0x00 };
const unsigned char DEFAULT_SEQUENCE[] = { 0xFF, 0xFF, 0xFF, 0xFF };

typedef struct prevout {
	unsigned char prevout[TRUSTED_INPUT_SIZE];
	unsigned char isTrusted;
	uint32_t outputIndex;
} prevout;

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;		
	uint32_t signingIndex;
	unsigned char newTransaction;
	bitcoinTransaction **transactions = NULL;
	prevout *prevouts = NULL;
	int transactionsNumber = 0;
	int status = 0;
	int i;

	initDongle();
	if (argc < 5) {
		fprintf(stderr, "Usage : %s [NEW for a new transaction|CONTINUE to keep on signing inputs in a previous transaction] [index of input to sign] [redeem script to use or empty to use the default one] [list of transactions output to use in this transaction]\n", argv[0]);
		fprintf(stderr, "Transaction outputs are coded as [hex transaction:output index] to generate a trusted input, or [-hex transaction:output index] to use the prevout directly for an output you don't own (relaxed wallet mode)\n");		
		goto cleanup;
	}
	if (strcasecmp(argv[1], "new") == 0) {
		newTransaction = 0x01;
	}
	else
	if (strcasecmp(argv[1], "continue") == 0) {
		newTransaction = 0x00;
	}
	else {
		fprintf(stderr, "Invalid transaction usage %s\n", argv[1]);
		goto cleanup;
	}
	result = strtol(argv[2], NULL, 10);
	if (result < 0) {
		fprintf(stderr, "Invalid input to sign index\n");
		goto cleanup;
	}
	signingIndex = result;
	transactionsNumber = argc - 1 - 3;
	transactions = (bitcoinTransaction**)malloc(sizeof(bitcoinTransaction*) * transactionsNumber);
	if (transactions == NULL) {
		fprintf(stderr, "Couldn't allocate transactions list\n");
		goto cleanup;
	}
	for (i=0; i<transactionsNumber; i++) {
		transactions[i] = NULL;
	}
	prevouts = (prevout*)malloc(sizeof(prevout) * transactionsNumber);
	if (prevouts == NULL) {
		fprintf(stderr, "Couldn't allocate prevouts list\n");
		goto cleanup;
	}
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}	
	// Parse each provided transaction, get the associated trusted input when necessary
	for (i=0; i<transactionsNumber; i++) {
		uint32_t index;
		unsigned char untrusted;
		untrusted = (argv[4 + i][0] == '-');
		transactions[i] = parseTransactionStringWithIndex(argv[4 + i] + (untrusted ? 1 : 0), &index);
		if (transactions[i] == NULL) {
			fprintf(stderr, "Invalid transaction %d\n", i + 1);
			goto cleanup;
		}
		if (untrusted) {
			fprintf(stderr, "Untrusted mode not supported\n");
			goto cleanup;
		}
		else {
			result = getTrustedInput(dongle, transactions[i], index, prevouts[i].prevout, sizeof(prevouts[i].prevout));
			if (result < 0) {
				fprintf(stderr, "Error getting trusted input %d\n", i + 1);
				goto cleanup;
			}
			prevouts[i].isTrusted = 1;
			printf("Trusted input #%d\n", (i + 1));
			displayBinary(prevouts[i].prevout, result);
		}
		prevouts[i].outputIndex = index;
	}
	// Then start building a fake transaction with the inputs we want
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_HASH_INPUT_START;
	in[apduSize++] = 0x00;
	in[apduSize++] = (newTransaction ? 0x00 : 0x80);
	in[apduSize++] = 0x00;
	memcpy(in + apduSize, DEFAULT_VERSION, sizeof(DEFAULT_VERSION));
	apduSize += sizeof(DEFAULT_VERSION);
	apduSize += writeVarint(transactionsNumber, (in + apduSize), (sizeof(in) - apduSize));
	in[OFFSET_CDATA] = (apduSize - 5);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	if (result < 0) {
		fprintf(stderr, "I/O error\n");
		return 0;
	}
	if (sw != SW_OK) {
		fprintf(stderr, "Dongle application error : %.4x\n", sw);
		return 0;
	}
	// Each input
	for (i=0; i<transactionsNumber; i++) {
		int scriptLength;
		unsigned char *script;			
		apduSize = 0;
		in[apduSize++] = BTCHIP_CLA;
		in[apduSize++] = BTCHIP_INS_HASH_INPUT_START;
		in[apduSize++] = 0x80;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		if (prevouts[i].isTrusted) {
			in[apduSize++] = 0x01;
			in[apduSize++] = sizeof(prevouts[i].prevout);
			memcpy(in + apduSize, prevouts[i].prevout, sizeof(prevouts[i].prevout));
			apduSize += sizeof(prevouts[i].prevout);
		}
		else {
			in[apduSize++] = 0x00;
			in[apduSize++] = PREVOUT_SIZE;
			memcpy(in + apduSize, prevouts[i].prevout, PREVOUT_SIZE);
			apduSize += PREVOUT_SIZE;
		}
		// Get the script length - use either the output script if signing the current index
		// Or a null script
		if (i == signingIndex) {
			if (strlen(argv[3]) != 0) {
				scriptLength = strlen(argv[3]) / 2;
				script = (unsigned char*)malloc(scriptLength);
				if (script == NULL) {
					fprintf(stderr, "Failed to allocate script\n");
					goto cleanup;
				}
				scriptLength = hexToBin(argv[3], script, scriptLength);
				if (scriptLength <= 0) {
					free(script);
					fprintf(stderr, "Invalid redeem script\n");
					goto cleanup;
				}
			}
			else {
				int j;
				bitcoinOutput *output = transactions[i]->outputs;
				for (j=0; j<prevouts[i].outputIndex; j++) {
					output = output->next;
				}
				scriptLength = output->scriptLength;
				script = output->script;
			}
		}
		else {
			scriptLength = 0;
			script = NULL;
		}
		apduSize += writeVarint(scriptLength, (in + apduSize), (sizeof(in) - apduSize));
		if (scriptLength != 0) {
			memcpy(in + apduSize, script, scriptLength);
			apduSize += scriptLength;
		}
		if (strlen(argv[3]) != 0) {
			free(script);
		}
		memcpy(in + apduSize, DEFAULT_SEQUENCE, sizeof(DEFAULT_SEQUENCE));
		apduSize += sizeof(DEFAULT_SEQUENCE);
		in[OFFSET_CDATA] = (apduSize - 5);
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
		if (result < 0) {
			fprintf(stderr, "I/O error\n");
			return 0;
		}
		if (sw != SW_OK) {
			fprintf(stderr, "Dongle application error : %.4x\n", sw);
			return 0;
		}
	}
	printf("Transaction submitted, waiting to be finalized\n");
	status = 1;

cleanup:
	exitDongle();

	if (transactions != NULL) {
		for (i = 0; i < transactionsNumber; i++) {
			if (transactions[i] != NULL) {
				freeTransaction(transactions[i]);
			}
		}
	}
	if (prevouts != NULL) {
		free(prevouts);
	}

	return status;
}
