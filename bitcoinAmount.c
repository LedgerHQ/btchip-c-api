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
#include <ctype.h>
#include "bitcoinAmount.h"

// Shamelessly borrowed from Bitcoin Core to avoid rounding issues

static const int64_t CENT = 1000000;
static const int64_t COIN = 100000000;

inline int64_t atoi64(const char* psz)
{
#ifdef _MSC_VER
    return _atoi64(psz);
#else
    return strtoll(psz, NULL, 10);
#endif
}

int parseStringAmount(char *amount, int64_t* result) {
	int strOffset = 0;
    char strWhole[11];
    int64_t nUnits = 0;
    int64_t nWhole;
    int64_t nValue;
    const char* p = amount;
    while (isspace(*p))
        p++;
    for (; *p; p++)
    {
        if (*p == '.')
        {
            p++;
            int64_t nMult = CENT*10;
            while (isdigit(*p) && (nMult > 0))
            {
                nUnits += nMult * (*p++ - '0');
                nMult /= 10;
            }
            break;
        }
        if (isspace(*p))
            break;
        if (!isdigit(*p))
            return -1;
        strWhole[strOffset] = *p;
        strOffset++;
        if (strOffset > 10) {
        	return -1;
        }
    }
    strWhole[strOffset] = '\0';
    for (; *p; p++)
        if (!isspace(*p))
            return -1;
    if (nUnits < 0 || nUnits > COIN)
        return -1;
    nWhole = atoi64(strWhole);
    nValue = nWhole*COIN + nUnits;

    *result = nValue;
    return 1;	
}

void writeHexAmount(int64_t amount, unsigned char *buffer) {
	*(buffer) = (amount & 0xff);
	*(buffer + 1) = ((amount >> 8) & 0xff);
	*(buffer + 2) = ((amount >> 16) & 0xff);
	*(buffer + 3) = ((amount >> 24) & 0xff);
	*(buffer + 4) = ((amount >> 32) & 0xff);
	*(buffer + 5) = ((amount >> 40) & 0xff);
	*(buffer + 6) = ((amount >> 48) & 0xff);
	*(buffer + 7) = ((amount >> 56) & 0xff);
}

void writeHexAmountBE(int64_t amount, unsigned char *buffer) {
	*(buffer + 7) = (amount & 0xff);
	*(buffer + 6) = ((amount >> 8) & 0xff);
	*(buffer + 5) = ((amount >> 16) & 0xff);
	*(buffer + 4) = ((amount >> 24) & 0xff);
	*(buffer + 3) = ((amount >> 32) & 0xff);
	*(buffer + 2) = ((amount >> 40) & 0xff);
	*(buffer + 1) = ((amount >> 48) & 0xff);
	*(buffer) = ((amount >> 56) & 0xff);
}

int64_t parseHexAmount(unsigned char *buffer) {
	int64_t result = 0;
	result += ((int64_t)*buffer);
	result += ((int64_t)*(buffer + 1)) << 8;
	result += ((int64_t)*(buffer + 2)) << 16;
	result += ((int64_t)*(buffer + 3)) << 24;
	result += ((int64_t)*(buffer + 4)) << 32;
	result += ((int64_t)*(buffer + 5)) << 40;
	result += ((int64_t)*(buffer + 6)) << 48;
	result += ((int64_t)*(buffer + 7)) << 56;
	return result;
}

void formatAmount(int64_t amount, char *result, size_t length) {
	char tmp[20];
	int i;
    int64_t n_abs = (amount > 0 ? amount : -amount);
    int64_t quotient = n_abs/COIN;
    int64_t remainder = n_abs%COIN;
    tmp[19] = '\0';
    snprintf(tmp, sizeof(tmp) - 1, "%lld.%08lld", quotient, remainder);

    // Right-trim excess zeros before the decimal point:
    int nTrim = 0;
    for (i = strlen(tmp)-1; (tmp[i] == '0' && isdigit(tmp[i-2])); --i)
        ++nTrim;
    if (nTrim)
        tmp[strlen(tmp) - nTrim] = '\0';
    strncpy(result, tmp, length);
}
