/*
*******************************************************************************    
*   BTChip Bitcoin Hardware Wallet C test interface
*   (c) 2017 BTChip - 1BTChip7VfTnrPra5jqci7ejnMguuHogTn
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

/* 
Sample configuration file 

rawTx = "0100000002ba0eb35fa910ccd759ff46b5233663e96017e8dfaedd315407dc5be45d8c260f000000001976a9146ce472b3cfced15a7d50b6b0cd75a3b042554e8e88acfdffffff69c84956a9cc0ec5986091e1ab229e1a7ea6f4813beb367c01c8ccc708e160cc000000001976a9146ce472b3cfced15a7d50b6b0cd75a3b042554e8e88acfdffffff01a17c0100000000001976a914efd0919fc05311850a8382b9c7e80abcd347343288ac00000000"
keyPaths = [ "44'/1'/0'/0/0", "44'/1'/0'/0/0" ]
rawPrevTxs = [ "010000000324c6fae955eae55c27639e5537d00e6ef11559c26f9c36c6770030b38702b19b0d0000006b483045022100c369493b6caa7016efd537eedce8d9e44fe14c345cd5edbb8bdca5545daf4cbe022053ac076f1c04f2f10f107f2890d5d95513547690b9a27d647d1c1ea68f6f3512012102f812962645e606a97728876d93324f030c1fe944d58466960104d810e8dc4945ffffffff24c6fae955eae55c27639e5537d00e6ef11559c26f9c36c6770030b38702b19b0a0000006b48304502210094f901df086a6499f24f678eef305e81eed11d730696cfa23cf1a9e2208ab98302205e628d259e2450d71d67ad54a58b0f58d6b643b70957c8a72e8df1293b2eb9be012102f812962645e606a97728876d93324f030c1fe944d58466960104d810e8dc4945ffffffff24c6fae955eae55c27639e5537d00e6ef11559c26f9c36c6770030b38702b19b0c0000006a47304402205c59502f9075f764dad17d59da9eb5429e969e2608ab579e3185f639dfda2eee0220614d2101e2c17612dc59a247f6f5cbdefcd7ea8f74654caa08b11f42873e586201210268a925507fd7e84295e172b3eea2f056c166ddc874fcda45864d872725094225ffffffff0150c30000000000001976a9146ce472b3cfced15a7d50b6b0cd75a3b042554e8e88ac00000000", "0100000001df5401686b5608195037e8978f6775db0c59d6cee8bb82aa25f4d8635481f56f010000006a47304402201d43a31c9d0f23f2bf2d39ae6d03ff217cb8bf7ddc7c5b1725f6f2f98d855b0c0220459426150782b01ca75958428e34f5e345e85ccae4333025eeb9baef85b3f9fc0121024bb68261bac7e49c99ad1e52fb5e91f09973d45f5d24715c9e64582a24856cc3ffffffff0260ea0000000000001976a9146ce472b3cfced15a7d50b6b0cd75a3b042554e8e88ac91351900000000001976a914efd0919fc05311850a8382b9c7e80abcd347343288ac00000000" ]
*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include <libconfig.h>
#include "dongleComm.h"
#include "hexUtils.h"
#include "btchipArgs.h"
#include "btchipApdu.h"
#include "bitcoinTransaction.h"
#include "bitcoinVarint.h"
#include "btchipUtils.h"
#include "btchipTrustedInput.h"

#define MAX_KEY_PATH 10

#define SIGHASH_ALL 0x01

#define RAW_TX "rawTx"
#define KEY_PATHS "keyPaths"
#define TX_PREV "rawPrevTxs"
#define CHANGE "change"

typedef struct signData_t {
	unsigned char prevout[TRUSTED_INPUT_SIZE];
	uint32_t outputIndex;
	unsigned int keyPath[MAX_KEY_PATH];
	int keyPathLength;
} signData_t;

uint32_t readUint32LE(unsigned char *buffer) {
	return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
}

bitcoinTransaction* parseTransactionString(const char *transactionString) {
	unsigned char *buffer;
	size_t bufferLength;
	bitcoinTransaction *result = NULL;	
	buffer = malloc(strlen(transactionString) / 2);
	if (!buffer) {
		fprintf(stderr, "Couldn't allocate transaction buffer\n");
		return NULL;
	}
	bufferLength = hexToBin(transactionString, buffer, strlen(transactionString));
	if (!bufferLength) {
		fprintf(stderr, "Invalid transaction\n");
	}
	else {
		result = parseTransaction(buffer, bufferLength);
		free(buffer);
	}
	return result;
}

int main(int argc, char **argv) {
	FILE *outFile = NULL;
	dongleHandle dongle = NULL;
	config_t cfg;
	config_setting_t *keyPaths;
	config_setting_t *rawPrevTxs;
	const char *rawTxString;	
	const char *change;
	bitcoinTransaction *rawTx = NULL;
		bitcoinTransaction **prevTx = NULL;
	signData_t *signData = NULL;
	unsigned int changeKeyPath[MAX_KEY_PATH];
	int changeKeyPathLength;	
	size_t txInputs, txOutputs;
	size_t i, j;

	if (argc < 3) {
		fprintf(stderr, "Usage : %s [configuration file] [output file]\n", argv[0]);
		return(EXIT_FAILURE);
	}

	initDongle();

	// Check configuration file

	config_init(&cfg);
	if (!config_read_file(&cfg, argv[1])) {
		fprintf(stderr, "Error reading configuration file %s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		goto error;
	}

	if (!config_lookup_string(&cfg, RAW_TX, &rawTxString)) {
		fprintf(stderr, "Missing %s\n", RAW_TX);
		goto error;
	}
	rawTx = parseTransactionString(rawTxString);
	if (!rawTx) {
		fprintf(stderr, "Invalid raw transaction\n");
		goto error;
	}
	txInputs = countTransactionInputs(rawTx);
	txOutputs = countTransactionOutputs(rawTx);

	debugTransaction(rawTx);

	prevTx = malloc(txInputs * sizeof(bitcoinTransaction*));
	if (prevTx == NULL) {
		fprintf(stderr, "Couldn't allocate prevTxs\n");
		goto error;
	}
	for (i=0; i<txInputs; i++) {
		prevTx[i] = NULL;
	}

	signData = malloc(txInputs * sizeof(signData_t));
	if (signData == NULL) {
		fprintf(stderr, "Couldn't allocate signData\n");
		goto error;
	}

	keyPaths = config_lookup(&cfg, KEY_PATHS);
	if (!keyPaths) {
		fprintf(stderr, "Missing %s\n", KEY_PATHS);
		goto error;
	}
	if (!config_setting_is_array(keyPaths)) {
		fprintf(stderr, "Invalid type for %s - expecting array\n", KEY_PATHS);
		goto error;
	}
	if (config_setting_length(keyPaths) != txInputs) {
		fprintf(stderr, "Invalid length for %s - expected %ld\n", KEY_PATHS, txInputs);
		goto error;
	}
	for (i=0; i<txInputs; i++) {
		const char *data = config_setting_get_string_elem(keyPaths, i);
		signData[i].keyPathLength = 0;
		if (data != NULL) {
			signData[i].keyPathLength = convertPath((char*)data, signData[i].keyPath);
		}
		if (signData[i].keyPathLength <= 0) {
			fprintf(stderr, "Invalid key path %s\n", data);
			goto error;
		}
	}

	rawPrevTxs = config_lookup(&cfg, TX_PREV);
	if (!rawPrevTxs) {
		fprintf(stderr, "Missing %s\n", TX_PREV);
		goto error;
	}
	if (!config_setting_is_array(rawPrevTxs)) {
		fprintf(stderr, "Invalid type for %s - expecting array\n", TX_PREV);
		goto error;
	}
	if (config_setting_length(rawPrevTxs) != txInputs) {
		fprintf(stderr, "Invalid length for %s - expected %ld\n", TX_PREV, txInputs);
		goto error;
	}
	for (i=0; i<txInputs; i++) {
		const char *data = config_setting_get_string_elem(rawPrevTxs, i);
		if (data == NULL) {
			fprintf(stderr, "Invalid raw TX %s\n", data);
			goto error;
		}
		prevTx[i] = parseTransactionString(data);
		if (!prevTx[i]) {
			goto error;
		}
	}

	if (config_lookup_string(&cfg, CHANGE, &change)) {		
		changeKeyPathLength = convertPath((char*)change, changeKeyPath);
		if (changeKeyPathLength <= 0) {
			fprintf(stderr, "Invalid change key path %s\n", change);
			goto error;
		}

	}
	else {
		change = NULL;
		changeKeyPathLength = 0;
	}

	// Open the device
	
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		goto error;
	}	

	outFile = fopen(argv[2], "w");
	if (outFile == NULL) {
		fprintf(stderr, "Failed to open output file %s\n", argv[2]);
		goto error;
	}

	// Compute trusted inputs 

	for (i=0; i<txInputs; i++) {
		bitcoinInput *currentInput = rawTx->inputs;
		uint32_t index = readUint32LE(currentInput->prevOut + 32);
		int result;
		result = getTrustedInput(dongle, prevTx[i], index, signData[i].prevout, sizeof(signData[i].prevout));
		if (result < 0) {
			fprintf(stderr, "Error getting trusted input %ld\n", i + 1);
			goto error;
		}
		signData[i].outputIndex = index;
		currentInput = currentInput->next;
	}

	// Start signing

	for (j=0; j<txInputs; j++) {
		unsigned char in[260];
		unsigned char out[260];
		unsigned int lockTime;
		int sw;
		int apduSize;		
		int result;
		bitcoinInput *currentInput = rawTx->inputs;

		// Initialize the signature request

		apduSize = 0;
		in[apduSize++] = BTCHIP_CLA;
		in[apduSize++] = BTCHIP_INS_HASH_INPUT_START;
		in[apduSize++] = 0x00;
		in[apduSize++] = ((j == 0) ? 0x00 : 0x80);
		in[apduSize++] = 0x00;
		memcpy(in + apduSize, rawTx->version, sizeof(rawTx->version));
		apduSize += sizeof(rawTx->version);
		apduSize += writeVarint(txInputs, (in + apduSize), (sizeof(in) - apduSize));
		in[OFFSET_CDATA] = (apduSize - 5);
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
		if (result < 0) {
			fprintf(stderr, "I/O error\n");
			goto error;
		}
		if (sw != SW_OK) {
			fprintf(stderr, "Dongle application error : %.4x\n", sw);
			goto error;
		}

		// Process each input, scriptSig containing the redeem script

		for (i=0; i<txInputs; i++) {			
			int scriptLength;
			int scriptOffset = 0;
			apduSize = 0;
			in[apduSize++] = BTCHIP_CLA;
			in[apduSize++] = BTCHIP_INS_HASH_INPUT_START;
			in[apduSize++] = 0x80;
			in[apduSize++] = 0x00;
			in[apduSize++] = 0x00;
			// Add trusted input			
			in[apduSize++] = 0x01; 
			in[apduSize++] = sizeof(signData[i].prevout);
			memcpy(in + apduSize, signData[i].prevout, sizeof(signData[i].prevout));
			apduSize += sizeof(signData[i].prevout);
			if (i == j) {
				scriptLength = currentInput->scriptLength;
			}
			else {
				scriptLength = 0;
			}
			apduSize += writeVarint(scriptLength, (in + apduSize), (sizeof(in) - apduSize));			
			if (scriptLength == 0) {
				// Add the sequence
				memcpy(in + apduSize, currentInput->sequence, sizeof(currentInput->sequence));
				apduSize += sizeof(currentInput->sequence);
			}
			in[OFFSET_CDATA] = (apduSize - 5);
			result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
			if (result < 0) {
				fprintf(stderr, "I/O error\n");
				goto error;
			}
			if (sw != SW_OK) {
				fprintf(stderr, "Dongle application error : %.4x\n", sw);
				goto error;
			}
			// Pass the redeem script
			while (scriptOffset < scriptLength) {
				int blockLength;
				if ((scriptOffset + 255) < scriptLength) {
					blockLength = 255;
				}
				else {
					blockLength = scriptLength - scriptOffset;
				}
				apduSize = 0;
				in[apduSize++] = BTCHIP_CLA;
				in[apduSize++] = BTCHIP_INS_HASH_INPUT_START;
				in[apduSize++] = 0x80;
				in[apduSize++] = 0x00;
				in[apduSize++] = 0x00;
				memcpy(in + apduSize, currentInput->script + scriptOffset, blockLength);
				apduSize += blockLength;
				// Add the script sequence on the last block
				if ((scriptOffset + blockLength) == scriptLength) {
					memcpy(in + apduSize, currentInput->sequence, sizeof(currentInput->sequence));
					apduSize += sizeof(currentInput->sequence);
				}

				in[OFFSET_CDATA] = (apduSize - 5);
				result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
				if (result < 0) {
					fprintf(stderr, "I/O error\n");
					goto error;
				}
				if (sw != SW_OK) {
					fprintf(stderr, "Dongle application error : %.4x\n", sw);
					goto error;
				}
				scriptOffset += blockLength;
			}
			currentInput = currentInput->next;
		}

		// Process the outputs

		if (change != NULL) {	
			// Provide a hint about the change
			apduSize = 0;
			in[apduSize++] = BTCHIP_CLA;
			in[apduSize++] = BTCHIP_INS_HASH_INPUT_FINALIZE_FULL;
			in[apduSize++] = 0xFF;
			in[apduSize++] = 0x00;
			in[apduSize++] = 0x01 + (4 * changeKeyPathLength);
			for (i=0; i<changeKeyPathLength; i++) {
				writeUint32BE(in + apduSize, changeKeyPath[i]);
				apduSize += 4;
			}
			result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
			if (result < 0) {
				fprintf(stderr, "I/O error\n");
				goto error;
			}
			if (sw != SW_OK) {
				fprintf(stderr, "Dongle application error : %.4x\n", sw);
				goto error;
			}
		}

		// Start with the number of outputs

		apduSize = 0;
		in[apduSize++] = BTCHIP_CLA;
		in[apduSize++] = BTCHIP_INS_HASH_INPUT_FINALIZE_FULL;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		apduSize += writeVarint(txOutputs, (in + apduSize), (sizeof(in) - apduSize));
		in[OFFSET_CDATA] = (apduSize - 5);
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
		if (result < 0) {
			fprintf(stderr, "I/O error\n");
			goto error;
		}
		if (sw != SW_OK) {
			fprintf(stderr, "Dongle application error : %.4x\n", sw);
			goto error;
		}

		// Write each output

		for (i=0; i<txOutputs; i++) {
			bitcoinOutput *currentOutput = rawTx->outputs;
			int scriptOffset = 0;
			// Write the output amount and script size
			apduSize = 0;
			in[apduSize++] = BTCHIP_CLA;
			in[apduSize++] = BTCHIP_INS_HASH_INPUT_FINALIZE_FULL;
			in[apduSize++] = 0x00;
			in[apduSize++] = 0x00;
			in[apduSize++] = 0x00;
			memcpy(in + apduSize, currentOutput->amount, sizeof(currentOutput->amount));
			apduSize += sizeof(currentOutput->amount);
			apduSize += writeVarint(currentOutput->scriptLength, (in + apduSize), (sizeof(in) - apduSize));
			in[OFFSET_CDATA] = (apduSize - 5);
			result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
			if (result < 0) {
				fprintf(stderr, "I/O error\n");
				goto error;
			}
			if (sw != SW_OK) {
				fprintf(stderr, "Dongle application error : %.4x\n", sw);
				goto error;
			}
			// Pass the output script
			while (scriptOffset < currentOutput->scriptLength) {
				int blockLength;
				if ((scriptOffset + 50) < currentOutput->scriptLength) {
					blockLength = 50;
				}
				else {
					blockLength = currentOutput->scriptLength - scriptOffset;
				}
				apduSize = 0;
				in[apduSize++] = BTCHIP_CLA;
				in[apduSize++] = BTCHIP_INS_HASH_INPUT_FINALIZE_FULL;
				in[apduSize++] = ((scriptOffset + blockLength) == currentOutput->scriptLength ? 0x80 : 0x00);
				in[apduSize++] = 0x00;
				in[apduSize++] = 0x00;
				memcpy(in + apduSize, currentOutput->script + scriptOffset, blockLength);
				apduSize += blockLength;
				in[OFFSET_CDATA] = (apduSize - 5);
				result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
				if (result < 0) {
					fprintf(stderr, "I/O error\n");
					goto error;
				}
				if (sw != SW_OK) {
					fprintf(stderr, "Dongle application error : %.4x\n", sw);
					goto error;
				}
				scriptOffset += blockLength;
			}			
			currentOutput = currentOutput->next;
		}

		// Get the signature

		apduSize = 0;
		in[apduSize++] = BTCHIP_CLA;
		in[apduSize++] = BTCHIP_INS_HASH_SIGN;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		in[apduSize++] = 0x00;
		in[apduSize++] = signData[j].keyPathLength;
		for (i=0; i<signData[j].keyPathLength; i++) {
			writeUint32BE(in + apduSize, signData[j].keyPath[i]);
			apduSize += 4;
		}
		in[apduSize++] = 0x00; // no 2FA on new platform
		lockTime = readUint32LE(rawTx->lockTime);
		writeUint32BE(in + apduSize, lockTime);
		apduSize += 4;
		in[apduSize++] = SIGHASH_ALL;
		in[OFFSET_CDATA] = (apduSize - 5);
		result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
		if (result < 0) {
			fprintf(stderr, "I/O error\n");
			goto error;
		}
		if (sw != SW_OK) {
			fprintf(stderr, "Dongle application error : %.4x\n", sw);
			goto error;
		}

		out[0] = 0x30; // clear parity information
		for (i=0; i<result; i++) {
			fprintf(outFile, "%.2x", out[i]);
		}
		fprintf(outFile, "\n");
	}

	// Cleanup

	closeDongle(dongle);
	exitDongle();
	for (i=0; i<txInputs; i++) {
		freeTransaction(prevTx[i]);
	}
	free(prevTx);
	freeTransaction(rawTx);
	free(signData);
	config_destroy(&cfg);
	if (outFile != NULL) {
		fclose(outFile);
	}	
	return(EXIT_SUCCESS);

error:
	if (dongle != NULL) {
		closeDongle(dongle);
	}
	exitDongle();
	if (rawTx != NULL) {
		freeTransaction(rawTx);
	}	
	if (prevTx != NULL) {
		for (i=0; i<txInputs; i++) {
			if (prevTx[i] != NULL) {
				freeTransaction(prevTx[i]);
			}
		}
		free(prevTx);
	}
	if (signData != NULL) {
		free(signData);
	}
	config_destroy(&cfg);
	if (outFile != NULL) {
		fclose(outFile);
		unlink(argv[2]);
	}		
	return(EXIT_FAILURE);
}