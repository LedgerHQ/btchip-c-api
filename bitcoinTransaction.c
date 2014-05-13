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
#include "hexUtils.h"
#include "bitcoinVarint.h"
#include "bitcoinTransaction.h"
#include "bitcoinAmount.h"

size_t countTransactionInputs(bitcoinTransaction *transaction) {
	size_t numberInputs = 0;	
	bitcoinInput *currentInput = transaction->inputs;
	while (currentInput != NULL) {
		numberInputs++;
		currentInput = currentInput->next;
	}
	return numberInputs;
}

size_t countTransactionOutputs(bitcoinTransaction *transaction) {
	int numberOutputs = 0;	
	bitcoinOutput *currentOutput = transaction->outputs;
	while (currentOutput != NULL) {
		numberOutputs++;
		currentOutput = currentOutput->next;
	}
	return numberOutputs;
}


int computeTransactionBufferSize(bitcoinTransaction* transaction) {
	size_t size = 0;
	int result;
	int numberInputs;
	int numberOutputs;
	int i;
	bitcoinInput *currentInput = transaction->inputs;
	bitcoinOutput *currentOutput = transaction->outputs;
	size += sizeof(transaction->version);
	numberInputs = countTransactionInputs(transaction);
	numberOutputs = countTransactionOutputs(transaction);
	result = getVarintSize(numberInputs);
	if (result < 0) {
		return -1;
	}
	size += result;
	currentInput = transaction->inputs;
	for (i=0; i<numberInputs; i++) {
		size += sizeof(currentInput->prevOut);
		size += sizeof(currentInput->sequence);
		result = getVarintSize(currentInput->scriptLength);
		if (result < 0) {
			return -1;
		}
		size += result;
		size += currentInput->scriptLength;
		currentInput = currentInput->next;
	}
	result = getVarintSize(numberOutputs);
	if (result < 0) {
		return -1;
	}
	size += result;	
	currentOutput = transaction->outputs;
	for (i=0; i<numberOutputs; i++) {
		size += sizeof(currentOutput->amount);
		result = getVarintSize(currentOutput->scriptLength);
		if (result < 0) {
			return -1;
		}
		size += result;
		size += currentOutput->scriptLength;
		currentOutput = currentOutput->next;
	}	
	size += sizeof(transaction->lockTime);
	return size;
}
 
void debugTransaction(bitcoinTransaction* transaction) {
	int counter;
	bitcoinInput *currentInput = transaction->inputs;
	bitcoinOutput *currentOutput = transaction->outputs;
	printf("Version : ");
	displayBinary(transaction->version, sizeof(transaction->version));
	counter = 1;
	while (currentInput != NULL) {
		printf("\tInput #%d :\n", counter);
		printf("\t\tPrevout : ");
		displayBinary(currentInput->prevOut, sizeof(currentInput->prevOut));
		printf("\t\tSequence : ");
		displayBinary(currentInput->sequence, sizeof(currentInput->sequence));
		printf("\t\tScript : ");
		displayBinary(currentInput->script, currentInput->scriptLength);
		currentInput = currentInput->next;
		counter++;
	}
	counter = 1;
	while (currentOutput != NULL) {
		int64_t amount;
		char amountString[20];		
		printf("\tOutput #%d :\n", counter);
		printf("\t\tAmount : ");
		amount = parseHexAmount(currentOutput->amount);
		formatAmount(amount, amountString, sizeof(amountString) - 1);
		printf("%s BTC - ", amountString);
		displayBinary(currentOutput->amount, sizeof(currentOutput->amount));
		printf("\t\tScript : ");
		displayBinary(currentOutput->script, currentOutput->scriptLength);
		currentOutput = currentOutput->next;
		counter++;
	}
	printf("Locktime : ");
	displayBinary(transaction->lockTime, sizeof(transaction->lockTime));
}

bitcoinTransaction* parseTransaction(unsigned char *transaction, size_t transactionLength) {
	size_t offset = 0;
	int numberInputs;
	int numberOutputs;
	size_t varintSize;
	int i;
	bitcoinTransaction *result = (bitcoinTransaction*)malloc(sizeof(bitcoinTransaction));
	bitcoinInput *lastInput = NULL;
	bitcoinOutput *lastOutput = NULL;
	if (result == NULL) {
		return NULL;
	}
	if ((transactionLength - offset) < sizeof(result->version)) {
		freeTransaction(result);
		return NULL;
	}
	memcpy(result->version, (transaction + offset), sizeof(result->version));
	offset += sizeof(result->version);
	numberInputs = readVarint((transaction + offset), &varintSize, (transactionLength - offset));
	if (numberInputs < 0) {
		freeTransaction(result);
		return NULL;		
	}
	offset += varintSize;
	for (i=0; i<numberInputs; i++) {		
		int scriptLength;
		bitcoinInput *input = (bitcoinInput*)malloc(sizeof(bitcoinInput));
		if (input == NULL) {
			freeTransaction(result);
			return NULL;
		}
		input->next = NULL;
		if ((transactionLength - offset) < sizeof(input->prevOut)) {
			free(input);
			freeTransaction(result);
			return NULL;			
		}
		memcpy(input->prevOut, (transaction + offset), sizeof(input->prevOut));
		offset += sizeof(input->prevOut);
		scriptLength = readVarint((transaction + offset), &varintSize, (transactionLength - offset));
		if (scriptLength < 0) {
			free(input);			
			freeTransaction(result);
			return NULL;		
		}
		input->scriptLength = scriptLength;
		input->script = (unsigned char*)malloc(scriptLength);
		if (input->script == NULL) {
			free(input);
			freeTransaction(result);
			return NULL;						
		}
		offset += varintSize;
		if ((transactionLength - offset) < scriptLength) {
			free(input->script);
			input->script = NULL;
			free(input);
			freeTransaction(result);
			return NULL;			
		}		
		memcpy(input->script, (transaction + offset), scriptLength);
		offset += scriptLength;
		if ((transactionLength - offset) < sizeof(input->sequence)) {
			free(input->script);
			input->script = NULL;
			free(input);
			freeTransaction(result);
			return NULL;			
		}
		memcpy(input->sequence, (transaction + offset), sizeof(input->sequence));
		offset += sizeof(input->sequence);
		if (lastInput == NULL) {
			result->inputs = input;
		}
		else {
			lastInput->next = input;
		}
		lastInput = input;
	}
	numberOutputs = readVarint((transaction + offset), &varintSize, (transactionLength - offset));
	if (numberOutputs < 0) {
		freeTransaction(result);
		return NULL;		
	}
	offset += varintSize;
	for (i=0; i<numberOutputs; i++) {		
		int scriptLength;
		bitcoinOutput *output = (bitcoinOutput*)malloc(sizeof(bitcoinOutput));
		if (output == NULL) {
			freeTransaction(result);
			return NULL;
		}
		output->next = NULL;
		if ((transactionLength - offset) < sizeof(output->amount)) {
			free(output);
			freeTransaction(result);
			return NULL;			
		}
		memcpy(output->amount, (transaction + offset), sizeof(output->amount));
		offset += sizeof(output->amount);
		scriptLength = readVarint((transaction + offset), &varintSize, (transactionLength - offset));
		if (scriptLength < 0) {
			free(output);			
			freeTransaction(result);
			return NULL;		
		}
		output->scriptLength = scriptLength;
		output->script = (unsigned char*)malloc(scriptLength);
		if (output->script == NULL) {
			free(output);
			freeTransaction(result);
			return NULL;						
		}
		offset += varintSize;
		if ((transactionLength - offset) < scriptLength) {
			free(output->script);
			output->script = NULL;
			free(output);
			freeTransaction(result);
			return NULL;			
		}		
		memcpy(output->script, (transaction + offset), scriptLength);
		offset += scriptLength;
		if (lastOutput == NULL) {
			result->outputs = output;
		}
		else {
			lastOutput->next = output;
		}
		lastOutput = output;
	}
	if ((transactionLength - offset) < sizeof(result->lockTime)) {
		freeTransaction(result);
		return NULL;			
	}	
	memcpy(result->lockTime, (transaction + offset), sizeof(result->lockTime));
	offset += sizeof(result->lockTime);
	if (offset != transactionLength) {
		freeTransaction(result);
		return NULL;
	}
	return result;	
}

size_t writeTransaction(bitcoinTransaction* transaction, unsigned char *buffer, size_t bufferSize) {
	int offset = 0;
	bitcoinInput *currentInput = transaction->inputs;
	bitcoinOutput *currentOutput = transaction->outputs;
	int requiredBufferSize = computeTransactionBufferSize(transaction);
	if (bufferSize < requiredBufferSize) {
		return 0;
	}	
	memcpy((buffer + offset), transaction->version, sizeof(transaction->version));
	offset += sizeof(transaction->version);
	offset += writeVarint(countTransactionInputs(transaction), (buffer + offset), (bufferSize - offset));
	while(currentInput != NULL) {
		memcpy((buffer + offset), currentInput->prevOut, sizeof(currentInput->prevOut));
		offset += sizeof(currentInput->prevOut);
		offset += writeVarint(currentInput->scriptLength, (buffer + offset), (bufferSize - offset));
		memcpy((buffer + offset), currentInput->script, currentInput->scriptLength);
		offset += currentInput->scriptLength;
		memcpy((buffer + offset), currentInput->sequence, sizeof(currentInput->sequence));
		offset += sizeof(currentInput->sequence);		
		currentInput = currentInput->next;
	}
	offset += writeVarint(countTransactionOutputs(transaction), (buffer + offset), (bufferSize - offset));
	while(currentOutput != NULL) {
		memcpy((buffer + offset), currentOutput->amount, sizeof(currentOutput->amount));
		offset += sizeof(currentOutput->amount);
		offset += writeVarint(currentOutput->scriptLength, (buffer + offset), (bufferSize - offset));
		memcpy((buffer + offset), currentOutput->script, currentOutput->scriptLength);
		offset += currentOutput->scriptLength;
		currentOutput = currentOutput->next;
	}
	memcpy((buffer + offset), transaction->lockTime, sizeof(transaction->lockTime));
	offset += sizeof(transaction->lockTime);
	return offset;
}

void freeTransaction(bitcoinTransaction* transaction) {
	bitcoinInput *input = transaction->inputs;
	bitcoinOutput *output = transaction->outputs;
	while (input != NULL) {
		bitcoinInput *previousInput = input;
		input = input->next;
		if (previousInput->script != NULL) {
			free(previousInput->script);
			previousInput->script = NULL;
		}
		free(previousInput);		
	}
	while (output != NULL) {
		bitcoinOutput *previousOutput = output;
		output = output->next;
		if (previousOutput->script != NULL) {
			free(previousOutput->script);
			previousOutput->script = NULL;
		}
		free(previousOutput);		
	}
	transaction->inputs = NULL;
	transaction->outputs = NULL;
	free(transaction);
}
