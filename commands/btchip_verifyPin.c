/*
 * Copyright (c) 2014 UBINITY SAS All rights reserved.
 * This source code is the property of UBINITY SAS. 
 * Redistribution and use in source (source code) or binary (object code)
 * forms with or without modification, for commercial, educational or
 * research purposes is not permitted without the prior written consent
 * of UBINITY SAS. 
 */

#include "dongleComm.h"
#include "hexUtils.h"
#include "btchipApdu.h"

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned char pin[8];
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;	

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [hex PIN]\n", argv[0]);
		return 0;
	}
	result = hexToBin(argv[1], pin, sizeof(pin));
	if (result == 0) {
		fprintf(stderr, "Invalid PIN\n");
		return 0;
	}
	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_VERIFY_PIN;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = result;
	memcpy(in + apduSize, pin, result);
	apduSize += result;
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	exitDongle();
	if (result < 0) {
		fprintf(stderr, "I/O error\n");
		return 0;
	}
	if (sw != SW_OK) {
		fprintf(stderr, "Dongle application error : %.4x\n", sw);
		return 0;
	}
	printf("PIN verified\n");
	if ((out[0] & 0x01) != 0) {
		printf("Powercycle to read the generated seed\n");
	}
	return 1;
}
