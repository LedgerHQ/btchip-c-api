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

#include "dongleComm.h"
#include "hexUtils.h"
#include "btchipApdu.h"
#include "btchipArgs.h"

int main(int argc, char **argv) {
	dongleHandle dongle;
	char data[520];
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;	
	FILE *file;

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [script to run]\n", argv[0]);
		return 0;
	}
	file = fopen(argv[1], "r");
	if (file == NULL) {
		fprintf(stderr, "Invalid file %s\n", argv[1]);
		return 0;
	}
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fclose(file);
		fprintf(stderr, "No dongle found\n");
		return 0;
	}
	for (;;) {
		int offset = 0;
		char *apduLine = fgets(data, sizeof(data), file);
		if (apduLine == NULL) {
			break;
		}
		if ((apduLine[0] == '\0') || (apduLine[0] == '#')) {
			continue;
		}
		apduLine[strlen(apduLine) - 1] = '\0';
		if (apduLine[strlen(apduLine) - 1] == '\r') {
			apduLine[strlen(apduLine) - 1] = '\0';
		}
		if (apduLine[0] == '!') {
			offset = 1;
		}
		apduSize = hexToBin(apduLine + offset, in, sizeof(in));
		if (apduSize == 0) {
			fclose(file);
			fprintf(stderr, "Invalid APDU %s\n", apduLine);
			return 0;
		}
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);	
		if (offset == 0) {
			if (result < 0) {
				fclose(file);
				fprintf(stderr, "I/O error\n");
				return 0;
			}
			if (sw != SW_OK) {
				fclose(file);
				fprintf(stderr, "Dongle application error : %.4x\n", sw);
				return 0;
			}
		}
	}
	fclose(file);
	closeDongle(dongle);
	exitDongle();
	printf("Script executed\n");
	return 1;
}
