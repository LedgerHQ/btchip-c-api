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
#include "btchipArgs.h"

int convertMode(char *mode) {
	if (strcasecmp(mode, "wallet") == 0) {
		return 0x01;
	}
	else
	if (strcasecmp(mode, "relaxed") == 0) {
		return 0x02;
	}
	else
	if (strcasecmp(mode, "server") == 0) {
		return 0x04;
	}
	else
	if (strcasecmp(mode, "developer") == 0) {
		return 0x08;
	}
	else {
		return -1;
	}
}

int convertOption(char *option) {
	if (strcasecmp(option, "UNCOMPRESSED_KEYS") == 0) {
		return 0x01;
	}
	else
	if (strcasecmp(option, "RFC6979") == 0) {
		return 0x02;
	}
	else
	if (strcasecmp(option, "FREE_SIGHASHTYPE") == 0) {
		return 0x04;
	}
	else {
		return -1;
	}
}

int convertChain(char *chain) {
	if (strcasecmp(chain, "external") == 0) {
		return CHAIN_EXTERNAL;
	}
	else
	if (strcasecmp(chain, "internal") == 0) {
		return CHAIN_INTERNAL;
	}
	else {
		return -1;
	}
}

int convertPos(char *pos) {
	if (strcasecmp(pos, "seedkey") == 0) {
		return POS_SEEDKEY;
	}
	else
	if (strcasecmp(pos, "encryptedseed") == 0) {
		return POS_ENCRYPTEDSEED;
	}
	else {
		return -1;
	}
}

