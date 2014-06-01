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

#ifndef __TRANSACTION_H__

#define __TRANSACTION_H__

struct bitcoinInput;
typedef struct bitcoinInput {
	unsigned char prevOut[36];
	unsigned char *script;
	size_t scriptLength;
	unsigned char sequence[4];	
	struct bitcoinInput *next;
} bitcoinInput;

struct bitcoinOutput;
typedef struct bitcoinOutput {
	unsigned char amount[8];
	unsigned char *script;
	size_t scriptLength;
	struct bitcoinOutput *next;
} bitcoinOutput;

typedef struct bitcoinTransaction {
	unsigned char version[4];
	bitcoinInput *inputs;
	bitcoinOutput *outputs;
	unsigned char lockTime[4];
} bitcoinTransaction;

bitcoinTransaction* parseTransaction(unsigned char *transaction, size_t transactionLength);
int computeTransactionBufferSize(bitcoinTransaction* transaction);
size_t writeTransaction(bitcoinTransaction* transaction, unsigned char *buffer, size_t bufferSize);
size_t writeTransactionWithoutOutputLocktime(bitcoinTransaction* transaction, unsigned char *buffer, size_t bufferSize);
size_t countTransactionInputs(bitcoinTransaction *transaction);
size_t countTransactionOutputs(bitcoinTransaction *transaction);
void debugTransaction(bitcoinTransaction* transaction);
void freeTransaction(bitcoinTransaction* transaction);

#endif
