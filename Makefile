CC     = gcc
CFLAGS = -Wall -Werror -Os -DEXTRA_DEBUG -DDEBUG_COMM -I.
OBJS   = hexUtils.o btchipUtils.o bitcoinVarint.o bitcoinTransaction.o bitcoinAmount.o btchipArgs.o
OBJS_COMM = $(OBJS) dongleCommHid.o dongleCommWinUSB.o dongleComm.o

all: btchip_setup btchip_setup_forward btchip_setup_143 btchip_verifyPin btchip_getWalletPublicKey btchip_getOperationMode btchip_setOperationMode btchip_getTrustedInput btchip_startUntrustedTransaction btchip_finalizeInput btchip_finalizeInputFull btchip_untrustedHashSign btchip_signMessagePrepare btchip_signMessageSign btchip_composeMofNStart btchip_composeMofNContinue btchip_importPrivateKey btchip_getPublicKey btchip_deriveBip32Key btchip_signImmediate btchip_verifyImmediate btchip_getRandom btchip_getFirmwareVersion btchip_util_runScript btchip_util_parseRawTransaction btchip_util_compressPublicKey btchip_util_getRegularInputScript btchip_util_getP2SHRedeemScript btchip_util_getP2SHInputScript btchip_util_formatTransaction

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

btchip_setup: commands/btchip_setup.o $(OBJS_COMM)
	gcc commands/btchip_setup.o $(OBJS_COMM) -o bin/btchip_setup -l hidapi-hidraw -l usb-1.0

btchip_setup_forward: commands/btchip_setup_forward.o $(OBJS_COMM)
	gcc commands/btchip_setup_forward.o $(OBJS_COMM) -o bin/btchip_setup_forward -l hidapi-hidraw -l usb-1.0

btchip_setup_143: commands/btchip_setup_143.o $(OBJS_COMM)
	gcc commands/btchip_setup_143.o $(OBJS_COMM) -o bin/btchip_setup_143 -l hidapi-hidraw -l usb-1.0

btchip_verifyPin: commands/btchip_verifyPin.o $(OBJS_COMM)
	gcc commands/btchip_verifyPin.o $(OBJS_COMM) -o bin/btchip_verifyPin -l hidapi-hidraw -l usb-1.0

btchip_getOperationMode: commands/btchip_getOperationMode.o $(OBJS_COMM)
	gcc commands/btchip_getOperationMode.o $(OBJS_COMM) -o bin/btchip_getOperationMode -l hidapi-hidraw -l usb-1.0

btchip_setOperationMode: commands/btchip_setOperationMode.o $(OBJS_COMM)
	gcc commands/btchip_setOperationMode.o $(OBJS_COMM) -o bin/btchip_setOperationMode -l hidapi-hidraw -l usb-1.0

btchip_getWalletPublicKey: commands/btchip_getWalletPublicKey.o $(OBJS_COMM)
	gcc commands/btchip_getWalletPublicKey.o $(OBJS_COMM) -o bin/btchip_getWalletPublicKey -l hidapi-hidraw -l usb-1.0

btchip_getTrustedInput: commands/btchip_getTrustedInput.o  btchipTrustedInput.o $(OBJS_COMM)
	gcc commands/btchip_getTrustedInput.o btchipTrustedInput.o $(OBJS_COMM) -o bin/btchip_getTrustedInput -l hidapi-hidraw -l usb-1.0

btchip_startUntrustedTransaction: commands/btchip_startUntrustedTransaction.o  btchipTrustedInput.o $(OBJS_COMM)
	gcc commands/btchip_startUntrustedTransaction.o btchipTrustedInput.o $(OBJS_COMM) -o bin/btchip_startUntrustedTransaction -l hidapi-hidraw -l usb-1.0

btchip_finalizeInput: commands/btchip_finalizeInput.o $(OBJS_COMM)
	gcc commands/btchip_finalizeInput.o $(OBJS_COMM) -o bin/btchip_finalizeInput -l hidapi-hidraw -l usb-1.0

btchip_finalizeInputFull: commands/btchip_finalizeInputFull.o $(OBJS_COMM)
	gcc commands/btchip_finalizeInputFull.o $(OBJS_COMM) -o bin/btchip_finalizeInputFull -l hidapi-hidraw -l usb-1.0

btchip_untrustedHashSign: commands/btchip_untrustedHashSign.o $(OBJS_COMM)
	gcc commands/btchip_untrustedHashSign.o $(OBJS_COMM) -o bin/btchip_untrustedHashSign -l hidapi-hidraw -l usb-1.0

btchip_signMessagePrepare: commands/btchip_signMessagePrepare.o $(OBJS_COMM)
	gcc commands/btchip_signMessagePrepare.o $(OBJS_COMM) -o bin/btchip_signMessagePrepare -l hidapi-hidraw -l usb-1.0

btchip_signMessageSign: commands/btchip_signMessageSign.o $(OBJS_COMM)
	gcc commands/btchip_signMessageSign.o $(OBJS_COMM) -o bin/btchip_signMessageSign -l hidapi-hidraw -l usb-1.0

btchip_composeMofNStart: commands/btchip_composeMofNStart.o $(OBJS_COMM)
	gcc commands/btchip_composeMofNStart.o $(OBJS_COMM) -o bin/btchip_composeMofNStart -l hidapi-hidraw -l usb-1.0

btchip_composeMofNContinue: commands/btchip_composeMofNContinue.o $(OBJS_COMM)
	gcc commands/btchip_composeMofNContinue.o $(OBJS_COMM) -o bin/btchip_composeMofNContinue -l hidapi-hidraw -l usb-1.0

btchip_importPrivateKey: commands/btchip_importPrivateKey.o $(OBJS_COMM)
	gcc commands/btchip_importPrivateKey.o $(OBJS_COMM) -o bin/btchip_importPrivateKey -l hidapi-hidraw -l usb-1.0

btchip_getPublicKey: commands/btchip_getPublicKey.o $(OBJS_COMM)
	gcc commands/btchip_getPublicKey.o $(OBJS_COMM) -o bin/btchip_getPublicKey -l hidapi-hidraw -l usb-1.0

btchip_deriveBip32Key: commands/btchip_deriveBip32Key.o $(OBJS_COMM)
	gcc commands/btchip_deriveBip32Key.o $(OBJS_COMM) -o bin/btchip_deriveBip32Key -l hidapi-hidraw -l usb-1.0

btchip_signImmediate: commands/btchip_signImmediate.o $(OBJS_COMM)
	gcc commands/btchip_signImmediate.o $(OBJS_COMM) -o bin/btchip_signImmediate -l hidapi-hidraw -l usb-1.0

btchip_verifyImmediate: commands/btchip_verifyImmediate.o $(OBJS_COMM)
	gcc commands/btchip_verifyImmediate.o $(OBJS_COMM) -o bin/btchip_verifyImmediate -l hidapi-hidraw -l usb-1.0

btchip_getRandom: commands/btchip_getRandom.o $(OBJS_COMM)
	gcc commands/btchip_getRandom.o $(OBJS_COMM) -o bin/btchip_getRandom -l hidapi-hidraw -l usb-1.0

btchip_getFirmwareVersion: commands/btchip_getFirmwareVersion.o $(OBJS_COMM)
	gcc commands/btchip_getFirmwareVersion.o $(OBJS_COMM) -o bin/btchip_getFirmwareVersion -l hidapi-hidraw -l usb-1.0

btchip_util_runScript: utils/btchip_util_runScript.o $(OBJS_COMM)
	gcc utils/btchip_util_runScript.o $(OBJS_COMM) -o bin/btchip_util_runScript -l hidapi-hidraw -l usb-1.0

btchip_util_parseRawTransaction: utils/btchip_util_parseRawTransaction.o 
	gcc utils/btchip_util_parseRawTransaction.o $(OBJS) -o bin/btchip_util_parseRawTransaction

btchip_util_compressPublicKey: utils/btchip_util_compressPublicKey.o 
	gcc utils/btchip_util_compressPublicKey.o $(OBJS) -o bin/btchip_util_compressPublicKey

btchip_util_getRegularInputScript: utils/btchip_util_getRegularInputScript.o 
	gcc utils/btchip_util_getRegularInputScript.o $(OBJS) -o bin/btchip_util_getRegularInputScript

btchip_util_getP2SHRedeemScript: utils/btchip_util_getP2SHRedeemScript.o 
	gcc utils/btchip_util_getP2SHRedeemScript.o $(OBJS) -o bin/btchip_util_getP2SHRedeemScript

btchip_util_getP2SHInputScript: utils/btchip_util_getP2SHInputScript.o 
	gcc utils/btchip_util_getP2SHInputScript.o $(OBJS) -o bin/btchip_util_getP2SHInputScript

btchip_util_formatTransaction: utils/btchip_util_formatTransaction.o 
	gcc utils/btchip_util_formatTransaction.o $(OBJS) -o bin/btchip_util_formatTransaction

clean:
	rm -f *.o bin/* commands/*.o utils/*.o

