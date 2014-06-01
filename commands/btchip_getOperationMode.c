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
#include "btchipArgs.h"

int main(int argc, char **argv) {
	dongleHandle dongle;
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;	

	initDongle();
	dongle = getFirstDongle();
	if (dongle == NULL) {
		fprintf(stderr, "No dongle found\n");
		return 0;
	}
	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_GET_OPERATION_MODE;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x01;
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	closeDongle(dongle);
	exitDongle();
	if (result < 0) {
		fprintf(stderr, "I/O error\n");
		return 0;
	}
	if (sw != SW_OK) {
		fprintf(stderr, "Dongle application error : %.4x\n", sw);
		return 0;
	}
	printf("Current operation mode : ");
	switch(out[0]) {
		case MODE_WALLET:
			printf("Wallet\n");
			break;
		case MODE_RELAXED_WALLET:
			printf("Relaxed wallet\n");
			break;
		case MODE_SERVER:
			printf("Server\n");
			break;
		case MODE_DEVELOPER:
			printf("Developer\n");
			break;
		default:
			printf("UNKNOWN %.2x\n", out[0]);
			break;	
	}
	return 1;
}

