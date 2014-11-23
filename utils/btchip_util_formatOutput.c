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
#include "bitcoinAmount.h"
#include "bitcoinVarint.h"
#include "btchipUtils.h"

int main(int argc, char **argv) {
	int result;
	int i;
	int size = 1;
	int offset = 0;
	unsigned char *buffer;


	if (argc < 3) {
		fprintf(stderr, "Usage : %s ([amount (in BTC string)] [output script])*N\n", argv[0]);
		return 0;
	}
	if (((argc - 1) % 2) != 0) {
		fprintf(stderr, "Invalid number of parameters\n");
		return 0;
	}
	for (i=1; i<argc; i += 2) {
		int64_t amount;
		int scriptSize;

		if (parseStringAmount(argv[i], &amount) < 0) {
			fprintf(stderr, "Invalid amount %d\n", ((i / 2) + 1));
			return 0;
		}
		if ((strlen(argv[i + 1]) % 2) != 0) {
			fprintf(stderr, "Invalid output script %d\n", ((i / 2) + 1));
			return 0;
		}
		scriptSize = strlen(argv[i + 1]) / 2;
		size += 8 + getVarintSize(scriptSize) + scriptSize;
	}
	buffer = malloc(size);
	if (buffer == NULL) {
		fprintf(stderr, "Couldn't allocate output buffer\n");
		return 0;
	}
	buffer[offset++] = (argc / 2);
	for (i=1; i<argc; i += 2) {
		int64_t amount;		
		int scriptSize;
		parseStringAmount(argv[i], &amount);
		writeHexAmount(amount, buffer + offset);
		offset += 8;
		scriptSize = strlen(argv[i + 1]) / 2;
		writeVarint(scriptSize, (buffer + offset), (size - offset));
		offset += getVarintSize(scriptSize);
		result = hexToBin(argv[i + 1], (buffer + offset), scriptSize);
		if (result == 0) {
			free(buffer);
			fprintf(stderr, "Invalid output script %d\n", ((i / 2) + 1));
			return 0;			
		}
		offset += scriptSize;
	}
	printf("Output script : ");
	displayBinary(buffer, size);
	free(buffer);
	return 1;
}
