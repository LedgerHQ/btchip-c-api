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

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned char trustedInput[TRUSTED_INPUT_SIZE];
	int result;
	bitcoinTransaction *transaction;
	uint32_t index;

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [hex transaction:output index]\n", argv[0]);
		return 0;
	}

	transaction = parseTransactionStringWithIndex(argv[1], &index);
	if (transaction == NULL) {
		fprintf(stderr, "Invalid transaction\n");
		return 0;
	}
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}
	result = getTrustedInput(dongle, transaction, index, trustedInput, sizeof(trustedInput));
	exitDongle();
	freeTransaction(transaction);
	if (result < 0) {
		return 0;
	}
	printf("Trusted input : ");
	displayBinary(trustedInput, result);
	return 1;
}
