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

#ifndef __BTCHIP_ARGS_H__

#define __BTCHIP_ARGS_H__

#define CHAIN_EXTERNAL 0x01
#define CHAIN_INTERNAL 0x02

#define MODE_WALLET 0x01
#define MODE_RELAXED_WALLET 0x02
#define MODE_SERVER 0x04
#define MODE_DEVELOPER 0x08

#define POS_SEEDKEY 0x01
#define POS_ENCRYPTEDSEED 0x02

#define MAX_BIP32_PATH 10

int convertMode(char *mode);
int convertOption(char *option);
int convertChain(char *chain);
int convertPos(char *pos);
int convertPath(char *path, unsigned int *pathBinary);

#endif
