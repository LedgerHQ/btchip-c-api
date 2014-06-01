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
	unsigned char publicKey[100];
	int result;

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [public key]\n", argv[0]);
		return 0;
	}
	result = hexToBin(argv[1], publicKey, sizeof(publicKey));
	if (result == 0) {
		fprintf(stderr, "Invalid public key\n");
		return 0;
	}
	if (publicKey[0] != 0x04) {
		fprintf(stderr, "Invalid public key format\n");
		return 0;
	}
	publicKey[0] = ((publicKey[64] & 1) ? 0x03 : 0x02);
	printf("Compressed public key : ");
	displayBinary(publicKey, 33);
	return 1;
}
