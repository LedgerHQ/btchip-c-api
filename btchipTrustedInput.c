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

int getTrustedInput(dongleHandle dongle, bitcoinTransaction *transaction, uint32_t index, unsigned char *trustedInput, size_t trustedInputLength) {
	unsigned char in[255];
	unsigned char out[255];
	int result;
	int sw;
	int apduSize;	
	int numberInputs;
	int numberOutputs;
	int i;
	bitcoinInput *currentInput = transaction->inputs;
	bitcoinOutput *currentOutput = transaction->outputs;

	numberInputs = countTransactionInputs(transaction);
	numberOutputs = countTransactionOutputs(transaction);

	// Init, before input list
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_GET_TRUSTED_INPUT;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	writeUint32BE(in + apduSize, index);
	apduSize += sizeof(uint32_t);
	memcpy(in + apduSize, transaction->version, sizeof(transaction->version));
	apduSize += sizeof(transaction->version);
	apduSize += writeVarint(numberInputs, (in + apduSize), (sizeof(in) - apduSize));
	in[OFFSET_CDATA] = (apduSize - 5);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	if (result < 0) {
		fprintf(stderr, "I/O error\n");
		return -1;
	}
	if (sw != SW_OK) {
		fprintf(stderr, "Dongle application error : %.4x\n", sw);
		return -1;
	}
	// Each input
	for (i=0; i<numberInputs; i++) {
		apduSize = 0;
		in[apduSize++] = BTCHIP_CLA;
		in[apduSize++] = BTCHIP_INS_GET_TRUSTED_INPUT;
		in[apduSize++] = 0x80;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		memcpy(in + apduSize, currentInput->prevOut, sizeof(currentInput->prevOut));
		apduSize += sizeof(currentInput->prevOut);
		apduSize += writeVarint(currentInput->scriptLength, (in + apduSize), (sizeof(in) - apduSize));
		memcpy(in + apduSize, currentInput->script, currentInput->scriptLength);
		apduSize += currentInput->scriptLength;
		memcpy(in + apduSize, currentInput->sequence, sizeof(currentInput->sequence));
		apduSize += sizeof(currentInput->sequence);
		in[OFFSET_CDATA] = (apduSize - 5);
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
		if (result < 0) {
			fprintf(stderr, "I/O error\n");
			return -1;
		}
		if (sw != SW_OK) {
			fprintf(stderr, "Dongle application error : %.4x\n", sw);
			return -1;
		}
		currentInput = currentInput->next;
	}
	// Number of outputs
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_GET_TRUSTED_INPUT;
	in[apduSize++] = 0x80;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	apduSize += writeVarint(numberOutputs, (in + apduSize), (sizeof(in) - apduSize));
	in[OFFSET_CDATA] = (apduSize - 5);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	if (result < 0) {
		fprintf(stderr, "I/O error\n");
		return -1;
	}
	if (sw != SW_OK) {
		fprintf(stderr, "Dongle application error : %.4x\n", sw);
		return -1;
	}	
	// Each output
	for (i=0; i<numberOutputs; i++) {
		apduSize = 0;
		in[apduSize++] = BTCHIP_CLA;
		in[apduSize++] = BTCHIP_INS_GET_TRUSTED_INPUT;
		in[apduSize++] = 0x80;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		memcpy(in + apduSize, currentOutput->amount, sizeof(currentOutput->amount));
		apduSize += sizeof(currentOutput->amount);
		apduSize += writeVarint(currentOutput->scriptLength, (in + apduSize), (sizeof(in) - apduSize));
		in[OFFSET_CDATA] = (apduSize - 5);
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
		if (result < 0) {
			fprintf(stderr, "I/O error\n");
			return -1;
		}
		if (sw != SW_OK) {
			fprintf(stderr, "Dongle application error : %.4x\n", sw);
			return -1;
		}		
		apduSize = 0;
		in[apduSize++] = BTCHIP_CLA;
		in[apduSize++] = BTCHIP_INS_GET_TRUSTED_INPUT;
		in[apduSize++] = 0x80;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		memcpy(in + apduSize, currentOutput->script, currentOutput->scriptLength);
		apduSize += currentOutput->scriptLength;
		in[OFFSET_CDATA] = (apduSize - 5);
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
		if (result < 0) {
			fprintf(stderr, "I/O error\n");
			return -1;
		}
		if (sw != SW_OK) {
			fprintf(stderr, "Dongle application error : %.4x\n", sw);
			return -1;
		}		
		currentOutput = currentOutput->next;
	}
	// Locktime	
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_GET_TRUSTED_INPUT;
	in[apduSize++] = 0x80;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	memcpy(in + apduSize, transaction->lockTime, sizeof(transaction->lockTime));
	apduSize += sizeof(transaction->lockTime);
	in[OFFSET_CDATA] = (apduSize - 5);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	if (result < 0) {
		fprintf(stderr, "I/O error\n");
		return -1;
	}
	if (sw != SW_OK) {
		fprintf(stderr, "Dongle application error : %.4x\n", sw);
		return -1;
	}		
	if (trustedInputLength < result) {
		return -1;
	}
	memcpy(trustedInput, out, result);
	return result;
}
