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
#include "bitcoinVarint.h"

int readVarint(unsigned char *buffer, size_t *varintSize, size_t remainingSize) {
	if (remainingSize == 0) {
		return -1;
	}
	if (*buffer < 0xfd) {
		*varintSize = 1;
		return *buffer;
	}
	else
	if (*buffer == 0xfd) {
		if (remainingSize < 3) {
			return -1;
		}
		*varintSize = 3;
		return (*(buffer + 2) << 8) | (*(buffer + 1));
	}
	else
	if (*buffer == 0xfe) {
		if (remainingSize < 5) {
			return -1;
		}
		*varintSize = 5;
		return (*(buffer + 4) << 24) | (*(buffer + 3) << 16) | (*(buffer + 2) << 8) | (*(buffer + 1));		
	}
	else {
		return -1;
	}	
}

int writeVarint(size_t size, unsigned char *buffer, size_t remainingSize) {
	int varintSize = -1;
	if (remainingSize == 0) {
		return -1;
	}	
	if (size < 0xfd) {
		varintSize = 1;
		*buffer = size;
	}
	else
	if (size <= 0xffff) {
		if (remainingSize < 3) {
			return -1;
		}		
		varintSize = 3;
		*(buffer++) = 0xfd;
		*(buffer++) = (size & 0xff);
		*(buffer++) = ((size >> 8) & 0xff);
	}
	else
	if (size <= 0xffffffff) {
		if (remainingSize < 5) {
			return -1;
		}
		varintSize = 5;
		*(buffer++) = 0xfe;
		*(buffer++) = (size & 0xff);
		*(buffer++) = ((size >> 8) & 0xff);
		*(buffer++) = ((size >> 16) & 0xff);
		*(buffer++) = ((size >> 24) & 0xff);
	}
	else {
		return -1;
	}
	return varintSize;
}

int getVarintSize(size_t size) {
	if (size < 0xfd) {
		return 1;
	}
	else
	if (size <= 0xffff) {
		return 3;
	}
	else
	if (size <= 0xffffffff) {
		return 5;
	}
	else {
		return -1;
	}
}
