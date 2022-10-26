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

#ifndef __AMOUNT_H__

#define __AMOUNT_H__

#include <inttypes.h>

extern inline int64_t atoi64(const char* psz);

int parseStringAmount(char *amount, int64_t* result);
void writeHexAmount(int64_t amount, unsigned char *buffer);
void writeHexAmountBE(int64_t amount, unsigned char *buffer);
int64_t parseHexAmount(unsigned char *buffer);
void formatAmount(int64_t amount, char *result, size_t length);

#endif
