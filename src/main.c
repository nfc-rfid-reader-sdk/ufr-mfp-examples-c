/*
 ============================================================================
 Project Name: project_name
 Name        : file_name.c
 Author      : d-logic (http://www.d-logic.net/nfc-rfid-reader-sdk/)
 Version     :
 Copyright   : 2017.
 Description : Project in C (Language standard: c99)
 Dependencies: uFR firmware - min. version x.y.z {define in ini.h}
               uFRCoder library - min. version x.y.z {define in ini.h}
 ============================================================================
 */

/* includes:
 * stdio.h & stdlib.h are included by default (for printf and LARGE_INTEGER.QuadPart (long long) use %lld or %016llx).
 * inttypes.h, stdbool.h & string.h included for various type support and utilities.
 * conio.h is included for windows(dos) console input functions.
 * windows.h is needed for various timer functions (GetTickCount(), QueryPerformanceFrequency(), QueryPerformanceCounter())
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#if __WIN32 || __WIN64
#	include <conio.h>
#	include <windows.h>
#elif linux || __linux__ || __APPLE__
#	define __USE_MISC
#	include <unistd.h>
#	include <termios.h>
#	undef __USE_MISC
#	include "conio_gnu.h"
#else
#	error "Unknown build platform."
#endif
#include <uFCoder.h>
#include "ini.h"
#include "uFR.h"
#include "utils.h"
//------------------------------------------------------------------------------
void usage(void);
void menu(char key);
UFR_STATUS NewCardInField(uint8_t sak, uint8_t *uid, uint8_t uid_size);
void PersoCard(void);
void AesAuthSL1(void);
void SwitchSL3(void);
void ChangeMasterKey(void);
void ChangeConfigKey(void);
void ChangeSectorKey(void);
void FieldConfigSet(void);
void GetUid(void);
void ChangeVcPollEncKey(void);
void ChangeVcPollMacKey(void);
void DataRead(void);
void DataWrite(void);
void ReaderKeys(void);
//------------------------------------------------------------------------------
int main(void)
{
	char key;
	bool card_in_field = false;
	uint8_t old_sak = 0, old_uid_size = 0, old_uid[10];
	uint8_t sak, uid_size, uid[10];
	UFR_STATUS status;

	usage();
	printf(" --------------------------------------------------\n");
	printf("     Please wait while opening uFR NFC reader.\n");
	printf(" --------------------------------------------------\n");

#ifdef __DEBUG
	status = ReaderOpenEx(1, PORT_NAME, 1, NULL);
#else
	status = ReaderOpen();
#endif
	if (status != UFR_OK)
	{
		printf("Error while opening device 1, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}
	/*
	status = ReaderReset();
	if (status != UFR_OK)
	{
		ReaderClose();
		printf("Error while opening device 2, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}
#if __WIN32 || __WIN64
	Sleep(500);
#else // if linux || __linux__ || __APPLE__
	usleep(500000);
#endif
	*/

	if (!CheckDependencies())
	{
		ReaderClose();
		getchar();
		return EXIT_FAILURE;
	}

	printf(" --------------------------------------------------\n");
	printf("        uFR NFC reader successfully opened.\n");
	printf(" --------------------------------------------------\n");

#if linux || __linux__ || __APPLE__
	_initTermios(0);
#endif
	do
	{
		while (!_kbhit())
		{
			status = GetCardIdEx(&sak, uid, &uid_size);
			switch (status)
			{
				case UFR_OK:
					if (card_in_field)
					{
						if (old_sak != sak || old_uid_size != uid_size || memcmp(old_uid, uid, uid_size))
						{
							old_sak = sak;
							old_uid_size = uid_size;
							memcpy(old_uid, uid, uid_size);
							NewCardInField(sak, uid, uid_size);
						}
					}
					else
					{
						old_sak = sak;
						old_uid_size = uid_size;
						memcpy(old_uid, uid, uid_size);
						NewCardInField(sak, uid, uid_size);
						card_in_field = true;
					}
					break;
				case UFR_NO_CARD:
					card_in_field = false;
					status = UFR_OK;
					break;
				default:
					ReaderClose();
					printf(" Fatal error while trying to read card, status is: 0x%08X\n", status);
					getchar();
#if linux || __linux__ || __APPLE__
					_resetTermios();
					tcflush(0, TCIFLUSH); // Clear stdin to prevent characters appearing on prompt
#endif
					return EXIT_FAILURE;
			}
#if __WIN32 || __WIN64
			Sleep(300);
#else // if linux || __linux__ || __APPLE__
			usleep(300000);
#endif
		}

		key = _getch();
		menu(key);
	}
	while (key != '\x1b');

	ReaderClose();
#if linux || __linux__ || __APPLE__
	_resetTermios();
	tcflush(0, TCIFLUSH); // Clear stdin to prevent characters appearing on prompt
#endif
	return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------
void menu(char key)
{

	switch (key)
	{
		case '1':
			PersoCard();
			break;

		case '2':
			AesAuthSL1();
			break;

		case '3':
			SwitchSL3();
			break;

		case '4':
			ChangeMasterKey();
			break;

		case '5':
			ChangeConfigKey();
			break;

		case '6':
			ChangeSectorKey();
			break;

		case '7':
			FieldConfigSet();
			break;

		case '8':
			GetUid();
			break;

		case '9':
			ChangeVcPollEncKey();
			break;

		case 'a':
		case 'A':
			ChangeVcPollMacKey();
			break;

		case 'b':
		case 'B':
			DataRead();
			break;

		case 'c':
		case 'C':
			DataWrite();
			break;

		case 'd':
		case 'D':
			ReaderKeys();
			break;

		case '\x1b':
			break;

		default:
			usage();
			break;
	}
}
//------------------------------------------------------------------------------
void usage(void)
{
		printf(" +------------------------------------------------+\n"
			   " |              MIFARE PLUS EXAMPLE               |\n"
			   " |                 version "APP_VERSION"                    |\n"
			   " +------------------------------------------------+\n"
			   "                              For exit, hit escape.\n");
		printf(" --------------------------------------------------\n");
		printf("  (1) - Personalization Card\n"
			   "  (2) - AES authentication on SL1\n"
			   "  (3) - Switch to SL3\n"
			   "  (4) - Change master key\n"
			   "  (5) - Change configuration key\n"
			   "  (6) - Change sectors keys\n"
			   "  (7) - Field configuration set\n"
			   "  (8) - Get card UID\n"
			   "  (9) - Change VC polling ENC key\n"
			   "  (a) - Change VC polling MAC key\n"
			   "  (b) - Data read\n"
			   "  (c) - Data write\n"
			   "  (d) - Writing keys into reader\n"
				);
}
//------------------------------------------------------------------------------
UFR_STATUS NewCardInField(uint8_t sak, uint8_t *uid, uint8_t uid_size)
{
	UFR_STATUS status;
	uint8_t dl_card_type;

	status = GetDlogicCardType(&dl_card_type);
	if (status != UFR_OK)
		return status;

	printf(" \a-------------------------------------------------------------------\n");
	printf(" Card type: %s, sak = 0x%02X, uid[%d] = ", GetDlTypeName(dl_card_type), sak, uid_size);
	print_hex_ln(uid, uid_size, ":");
	printf(" -------------------------------------------------------------------\n");

	return UFR_OK;
}
//------------------------------------------------------------------------------
bool EnterAesKeyOrData(uint8_t *aes_key)
{
	char str[100];
	size_t str_len;

	scanf("%[^\n]%*c", str);
	str_len = hex2bin(aes_key, str);
	if(str_len != 16)
	{
		printf("\nYou need to enter 16 hexadecimal numbers with or without spaces or with : as delimiter\n");
		scanf("%[^\n]%*c", str);
		str_len = hex2bin(aes_key, str);
		if(str_len != 16)
			return false;
	}

	return true;
}
//------------------------------------------------------------------------------
bool EnterCrypto1Key(uint8_t *crypto_1_key)
{
	char str[100];
	size_t str_len;
	memset(str, 0 , 100);

	scanf("%[^\n]%*c", str);
	str_len = hex2bin(crypto_1_key, str);
	if(str_len != 6)
	{
		printf("\nYou need to enter 6 hexadecimal numbers with or without spaces or with : as delimiter\n");
		scanf("%[^\n]%*c", str);
		str_len = hex2bin(crypto_1_key, str);
		if(str_len != 6)
			return false;
	}

	return true;
}
//------------------------------------------------------------------------------
bool EnterLinearData(uint8_t *linear_data, uint16_t *linear_len)
{
	char str[3440];
	size_t str_len;
	char key;

	*linear_len = 0;

	printf(" (1) - ASCI\n"
		   " (2) - HEX\n");

	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("Enter ASCI text\n");
		scanf("%[^\n]%*c", str);
		str_len = strlen(str);
		*linear_len = str_len;
		memcpy(linear_data, str, *linear_len);
		return true;
	}
	else if(key == '2')
	{
		printf("Enter hexadecimal bytes\n");
		scanf("%[^\n]%*c", str);
		str_len = hex2bin(linear_data, str);
		*linear_len = str_len;
		return true;
	}
	else
		return false;
}

//------------------------------------------------------------------------------
void PersoCard(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                     CARD PERSONALIZATON                            \n");
	printf(" 			MIFARE PLUS CARD MUST BE IN SL0 MODE					\n");
	printf("      ALL AES SECTOR KEYS WILL BE FACTORY DEFAULT 16 x 0xFF         \n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t master_key[16], config_key[16], l2_sw_key[16], l3_sw_key[16], l1_auth_key[16];
	uint8_t sel_vc_key[16], prox_chk_key[16],  vc_poll_enc_key[16], vc_poll_mac_key[16];
	uint8_t dl_card_type;

	status = GetDlogicCardType(&dl_card_type);
	if(status)
	{
		printf("\nCommunication with card failed \n");
		return;
	}

	if(!(dl_card_type >= DL_MIFARE_PLUS_S_2K_SL0 && dl_card_type <= DL_MIFARE_PLUS_X_4K_SL0))
	{
		printf("\nCard is not in security level 0 mode\n");
		return;
	}

	//check if card type S in SL0
	if(dl_card_type == DL_MIFARE_PLUS_S_2K_SL0 || dl_card_type == DL_MIFARE_PLUS_S_4K_SL0)
	{
		//S type card
		//For this type of card there are no following keys
		//Switch to SL2, Select VC, Proximity Check.

		memset(l2_sw_key, 0, 16);
		memset(sel_vc_key, 0, 16);
		memset(prox_chk_key, 0, 16);
	}

	printf("\nEnter card master key\n");
	if(!EnterAesKeyOrData(master_key))
	{
		printf("\nError while key entry\n");
		return;
	}

	printf("\nEnter card configuration key\n");
	if(!EnterAesKeyOrData(config_key))
	{
		printf("\nError while key entry\n");
		return;
	}

	if(dl_card_type == DL_MIFARE_PLUS_X_2K_SL0 || dl_card_type == DL_MIFARE_PLUS_X_4K_SL0)
	{
		printf("\nEnter level 2 switch key\n");
		if(!EnterAesKeyOrData(l2_sw_key))
		{
			printf("\nError while key entry\n");
			return;
		}
	}

	printf("\nEnter level 3 switch key\n");
	if(!EnterAesKeyOrData(l3_sw_key))
	{
		printf("\nError while key entry\n");
		return;
	}

	printf("\nEnter SL1 card authentication key\n");
	if(!EnterAesKeyOrData(l1_auth_key))
	{
		printf("\nError while key entry\n");
		return;
	}

	if(dl_card_type == DL_MIFARE_PLUS_X_2K_SL0 || dl_card_type == DL_MIFARE_PLUS_X_4K_SL0)
	{
		printf("\nEnter select VC key\n");
		if(!EnterAesKeyOrData(sel_vc_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter proximity check key\n");
		if(!EnterAesKeyOrData(prox_chk_key))
		{
			printf("\nError while key entry\n");
			return;
		}
	}

	printf("\nEnter VC polling ENC key\n");
	if(!EnterAesKeyOrData(vc_poll_enc_key))
	{
		printf("\nError while key entry\n");
		return;
	}

	printf("\nEnter VC polling MAC key\n");
	if(!EnterAesKeyOrData(vc_poll_mac_key))
	{
		printf("\nError while key entry\n");
		return;
	}

	status = MFP_PersonalizationMinimal(master_key, config_key, l2_sw_key, l3_sw_key, l1_auth_key,
										sel_vc_key, prox_chk_key, vc_poll_enc_key, vc_poll_mac_key);
	if(status)
	{
		printf("\nCard personalization failed\n");
		printf("Error code = %02X\n", status);
	}
	else
		printf("\nCard personalization successful\n");
}
//------------------------------------------------------------------------------
void AesAuthSL1(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                   AES AUTHENTICATION ON SL1                    	\n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL1 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t sl1_auth_key[16], key_index;
	char key;
	int key_index_int;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter SL1 card authentication key\n");
		if(!EnterAesKeyOrData(sl1_auth_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_AesAuthSecurityLevel1_PK(sl1_auth_key);
		if(status)
			printf("\nSL1 AES authentication failed\n");
		else
			printf("\nSL1 AES authentication successful\n");
	}
	else if(key == '2')
	{
		printf("\nEnter reader key index for SL1 card authentication key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;
		status = MFP_AesAuthSecurityLevel1(key_index);
		if(status)
		{
			printf("\nSL1 AES authentication failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nSL1 AES authentication successful\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void SwitchSL3(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                           SWITCH TO SL3                            \n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL1 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t sl3_sw_key[16], key_index;
	char key;
	int key_index_int;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter level 3 switch key\n");
		if(!EnterAesKeyOrData(sl3_sw_key))
		{
			printf("\nError while key entry\n");
			return;
		}
		status = MFP_SwitchToSecurityLevel3_PK(sl3_sw_key);
		if(status)
			printf("\nSwitch to security level 3 failed\n");
		else
			printf("\nSwitch to security level 3 successful\n");
	}
	else if(key == '2')
	{
		printf("\nEnter reader key index for level 3 switch key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;
		status = MFP_SwitchToSecurityLevel3(key_index);
		if(status)
		{
			printf("\nSwitch to security level 3 failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nSwitch to security level 3 successful\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void ChangeMasterKey(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                      CHANGE CARD MASTER KEY                        \n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL3 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t old_master_key[16], new_master_key[16], key_index;
	char key;
	int key_index_int;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter old card master key\n");
		if(!EnterAesKeyOrData(old_master_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter new card master key\n");
		if(!EnterAesKeyOrData(new_master_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeMasterKey_PK(old_master_key, new_master_key);
		if(status)
		{
			printf("\nMaster key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of master key was successful\n");
	}
	else if(key == '2')
	{
		printf("\nEnter reader key index for old card master key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;

		printf("\nEnter new card master key\n");
		if(!EnterAesKeyOrData(new_master_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeMasterKey(key_index, new_master_key);
		if(status)
		{
			printf("\nMaster key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of master key was successful\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void ChangeConfigKey(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                    CHANGE CARD CONFIGURATION KEY                   \n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL3 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t old_config_key[16], new_config_key[16], key_index;
	char key;
	int key_index_int;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter old card configuration key\n");
		if(!EnterAesKeyOrData(old_config_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter new card configuration key\n");
		if(!EnterAesKeyOrData(new_config_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeConfigurationKey_PK(old_config_key, new_config_key);
		if(status)
		{
			printf("\nConfiguration key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of configuration key was successful\n");
	}
	else if(key == '2')
	{
		printf("\nEnter reader key index for old card configuration key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;

		printf("\nEnter new card configuration key\n");
		if(!EnterAesKeyOrData(new_config_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeConfigurationKey(key_index, new_config_key);
		if(status)
		{
			printf("\nConfiguration key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of configuration key was successful\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void ChangeSectorKey(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                       CHANGE SECTOR AES KEY                        \n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL3 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t old_sector_key[16], new_sector_key[16], key_index;
	uint8_t sector_nr, auth_mode;
	char key;
	int key_index_int, sector_nr_int, auth_mode_int;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter sector number (0 - 31 for 2K card) (0 - 39 for 4K card)\n");
		scanf("%d%*c", &sector_nr_int);
		sector_nr = sector_nr_int;

		printf("\nEnter code for authentication mode\n"
			   " (1) - AES KEY A\n"
			   " (2) - AES KEY B\n");
		scanf("%d%*c", &auth_mode_int);
		if(auth_mode_int == 1)
			auth_mode =  MIFARE_PLUS_AES_AUTHENT1A;
		else if(auth_mode_int == 2)
			auth_mode = MIFARE_PLUS_AES_AUTHENT1B;
		else
		{
			printf("\nWrong choice\n");
			return;
		}

		printf("\nEnter old AES sector key\n");
		if(!EnterAesKeyOrData(old_sector_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter new AES sector key\n");
		if(!EnterAesKeyOrData(new_sector_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeSectorKey_PK(sector_nr, auth_mode, old_sector_key, new_sector_key);
		if(status)
		{
			printf("\nAES sector key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of AES sector key was successful\n");
	}
	else if(key == '2')
	{
		printf("\nEnter sector number (0 - 31 for 2K card) (0 - 39 for 4K card)\n");
		scanf("%d%*c", &sector_nr_int);
		sector_nr = sector_nr_int;

		printf("\nEnter code for authentication mode\n"
			   " (1) - AES KEY A\n"
			   " (2) - AES KEY B\n");
		scanf("%d%*c", &auth_mode_int);
		if(auth_mode_int == 1)
			auth_mode =  MIFARE_AUTHENT1A;
		else if(auth_mode_int == 2)
			auth_mode = MIFARE_AUTHENT1B;
		else
		{
			printf("\nWrong choice\n");
			return;
		}

		printf("\nEnter reader key index for old AES sector key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;

		printf("\nEnter new card configuration key\n");
		if(!EnterAesKeyOrData(new_sector_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeSectorKey(sector_nr, auth_mode, key_index, new_sector_key);
		if(status)
		{
			printf("\nAES sector key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of AES sector key was successful\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void FieldConfigSet(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                       FIELD CONFIGURATION SETTING                  \n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL3 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t config_key[16], key_index;
	uint8_t rid_use, prox_check_use;
	char key;
	int key_index_int, rid_int;

	//Proximity check for X and EV1 card is not implemented yet
	prox_check_use = 0;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter random ID option\n"
				" (1) - use random ID\n"
				" (2) - use UID\n");
		scanf("%d%*c", &rid_int);
		if(rid_int == 1)
			rid_use = 1;
		else if(rid_int == 2)
			rid_use = 0;
		else
		{
			printf("\nWrong choice\n");
			return;
		}

		printf("\nEnter configuration key\n");
		if(!EnterAesKeyOrData(config_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_FieldConfigurationSet_PK(config_key, rid_use, prox_check_use);
		if(status)
			printf("\nField configuration block change has failed\n");
		else
			printf("\nChange of field configuration block was successful\n");
	}
	else if(key == '2')
	{
		printf("\nEnter random ID option\n"
				" (1) - use random ID\n"
				" (2) - use UID\n");
		scanf("%d%*c", &rid_int);
		if(rid_int == 1)
			rid_use = 1;
		else if(rid_int == 2)
			rid_use = 0;
		else
		{
			printf("\nWrong choice\n");
			return;
		}

		printf("\nEnter reader key index for old AES sector key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;

		status = MFP_FieldConfigurationSet(key_index, rid_use, prox_check_use);
		if(status)
		{
			printf("\nField configuration block change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of field configuration block was successful\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void GetUid(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                              GET CARD UID                          \n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL3 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t vc_enc_key[16], vc_mac_key[16], key_index_enc, key_index_mac;
	char key;
	int key_index_int;
	uint8_t uid[10], uid_len;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter VC polling ENC key\n");
		if(!EnterAesKeyOrData(vc_enc_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter VC polling MAC key\n");
		if(!EnterAesKeyOrData(vc_mac_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_GetUid_PK(vc_enc_key, vc_mac_key, uid, &uid_len);
		if(status)
		{
			printf("\nReading card UID has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
		{
			printf("\nCard UID =");
			print_hex_ln(uid, uid_len, " ");
		}
	}
	else if(key == '2')
	{
		printf("\nEnter reader key index for VC polling ENC key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index_enc = key_index_int;

		printf("\nEnter reader key index for VC polling MAC key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index_mac = key_index_int;

		status = MFP_GetUid(key_index_enc, key_index_mac, uid, &uid_len);
		if(status)
		{
			printf("\nReading card UID has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
		{
			printf("\nCard UID =");
			print_hex_ln(uid, uid_len, " ");
		}
	}
	else
		printf("\nWrong choice\n");

	return;

}
//------------------------------------------------------------------------------
void ChangeVcPollEncKey(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                       CHANGE VC POLLING ENC KEY                    \n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL3 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t config_key[16], new_vc_enc_key[16], key_index;
	char key;
	int key_index_int;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter card configuration key\n");
		if(!EnterAesKeyOrData(config_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter new VC polling ENC key\n");
		if(!EnterAesKeyOrData(new_vc_enc_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeVcPollingEncKey_PK(config_key, new_vc_enc_key);
		if(status)
		{
			printf("\nVC polling ENC key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of VC polling ENC key was successful\n");
	}
	else if(key == '2')
	{
		printf("\nEnter reader key index for configuration key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;

		printf("\nEnter new card VC polling ENC key\n");
		if(!EnterAesKeyOrData(new_vc_enc_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeVcPollingEncKey(key_index, new_vc_enc_key);
		if(status)
		{
			printf("\nVC polling ENC key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of VC polling ENC key was successful\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void ChangeVcPollMacKey(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                       CHANGE VC POLLING MAC KEY                    \n");
	printf(" 		       MIFARE PLUS CARD MUST BE IN SL3 MODE	    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t config_key[16], new_vc_mac_key[16], key_index;
	char key;
	int key_index_int;

	printf(" (1) - Provided AES key \n"
		   " (2) - Reader AES key \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter card configuration key\n");
		if(!EnterAesKeyOrData(config_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter new VC polling MAC key\n");
		if(!EnterAesKeyOrData(new_vc_mac_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeVcPollingMacKey_PK(config_key, new_vc_mac_key);
		if(status)
		{
			printf("\nVC polling MAC key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of VC polling MAC key was successful\n");
	}
	else if(key == '2')
	{
		printf("\nEnter reader key index for configuration key (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;

		printf("\nEnter new card VC polling MAC key\n");
		if(!EnterAesKeyOrData(new_vc_mac_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		status = MFP_ChangeVcPollingMacKey(key_index, new_vc_mac_key);
		if(status)
		{
			printf("\nVC polling MAC key change has failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nChange of VC polling MAC key was successful\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void DataRead(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                       READ DATA FROM CARD	                        \n");
	printf(" 		 MIFARE PLUS CARD MUST BE IN SL1 OR SL3 MODE    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t crypto_1_sector_key[6], aes_sector_key[16];
	uint8_t block_nr, sector_nr, auth_mode, key_index;
	int key_index_int, block_nr_int, sector_nr_int;
	uint8_t dl_card_type;
	char key;
	uint8_t block_data[16];
	uint8_t linear_data[3440];
	uint16_t lin_addr, lin_len, ret_bytes;
	int lin_addr_int, lin_len_int;

	status = GetDlogicCardType(&dl_card_type);
	if(status)
	{
		printf("\nCommunication with card failed \n");
		return;
	}

	if(!((dl_card_type >= DL_MIFARE_PLUS_S_2K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL1)
			|| (dl_card_type >= DL_MIFARE_PLUS_S_4K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL1)
			|| (dl_card_type >= DL_MIFARE_PLUS_S_2K_SL3 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL3)
			|| (dl_card_type >= DL_MIFARE_PLUS_S_2K_SL3 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL3)))
	{
		printf("\nCard is not in security level 1 or 3 mode\n");
		return;
	}

	printf(" (1) - Block read\n"
		   " (2) - Block in sector read\n"
		   " (3) - Linear read\n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nBlock read select key mode\n");

		if((dl_card_type >= DL_MIFARE_PLUS_S_2K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL1)
			|| (dl_card_type >= DL_MIFARE_PLUS_S_4K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL1))
		{
			printf(" (1) - Provided CRYPTO 1 key\n"
				   " (2) - Reader CRYPTO 1 key\n"
				   " (3) - AKM1 CRYPTO 1 key\n"
				   " (4) - AKM2 CRYPTO 1 key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter CRYPTO 1 key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter CRYPTO 1 key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterCrypto1Key(crypto_1_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				status = BlockRead_PK(block_data, block_nr, auth_mode, crypto_1_sector_key);
				if(status)
					printf("\nBlock read failed\n");
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '2':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();

				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader CRYPTO 1 key A index (0 -31)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader CRYPTO 1 key A index (0 -31)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				status = BlockRead(block_data, block_nr, auth_mode, key_index);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '3':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();

				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = BlockRead_AKM1(block_data, block_nr, auth_mode);
				if(status)
					printf("\nBlock read failed\n");
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '4':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();

				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = BlockRead_AKM2(block_data, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
		else
		{
			printf(" (1) - Provided AES key\n"
				   " (2) - Reader AES key\n"
				   " (3) - AKM1 AES key\n"
				   " (4) - AKM2 AES key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_PLUS_AES_AUTHENT1A;
					printf("\nEnter AES key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_PLUS_AES_AUTHENT1B;
					printf("\nEnter AES key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterAesKeyOrData(aes_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				status = BlockRead_PK(block_data, block_nr, auth_mode, aes_sector_key);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '2':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader AES key A index (0 - 16)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader AES key B index (0 - 16)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				status = BlockRead(block_data, block_nr, auth_mode, key_index);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '3':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = BlockRead_AKM1(block_data, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '4':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = BlockRead_AKM2(block_data, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
	}
	else if(key == '2')
	{
		printf("\nBlock in sector read select key mode\n");

		if((dl_card_type >= DL_MIFARE_PLUS_S_2K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL1)
					|| (dl_card_type >= DL_MIFARE_PLUS_S_4K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL1))
		{
			printf(" (1) - Provided CRYPTO 1 key\n"
				   " (2) - Reader CRYPTO 1 key\n"
				   " (3) - AKM1 CRYPTO 1 key\n"
				   " (4) - AKM2 CRYPTO 1 key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter CRYPTO 1 key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter CRYPTO 1 key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterCrypto1Key(crypto_1_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				status = BlockInSectorRead_PK(block_data, sector_nr, block_nr, auth_mode, crypto_1_sector_key);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '2':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader CRYPTO 1 key A index (0 -31)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader CRYPTO 1 key B index (0 -31)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				status = BlockInSectorRead(block_data, sector_nr, block_nr, auth_mode, key_index);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '3':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = BlockInSectorRead_AKM1(block_data, sector_nr, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '4':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = BlockInSectorRead_AKM2(block_data, sector_nr, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
		else
		{

			printf(" (1) - Provided AES key\n"
				   " (2) - Reader AES key\n"
				   " (3) - AKM1 AES key\n"
				   " (4) - AKM2 AES key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_PLUS_AES_AUTHENT1A;
					printf("\nEnter AES key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_PLUS_AES_AUTHENT1B;
					printf("\nEnter AES key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterAesKeyOrData(aes_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				status = BlockInSectorRead_PK(block_data, sector_nr, block_nr, auth_mode, aes_sector_key);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '2':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader AES key A index (0 - 16)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader AES key A index (0 - 16)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				status = BlockInSectorRead(block_data, sector_nr, block_nr, auth_mode, key_index);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '3':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = BlockInSectorRead_AKM1(block_data, sector_nr, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			case '4':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = BlockInSectorRead_AKM2(block_data, sector_nr, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nBlock read successful\n");
					printf("Data = ");
					print_hex_ln(block_data, 16, " ");
					printf("\n");
				}
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
	}
	else if(key == '3')
	{
		printf("\nLinear read select key mode\n");

		if((dl_card_type >= DL_MIFARE_PLUS_S_2K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL1)
							|| (dl_card_type >= DL_MIFARE_PLUS_S_4K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL1))
		{
			printf(" (1) - Provided CRYPTO 1 key\n"
				   " (2) - Reader CRYPTO 1 key\n"
				   " (3) - AKM1 CRYPTO 1 key\n"
				   " (4) - AKM2 CRYPTO 1 key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter number of bytes for read\n");
				scanf("%d%*c", &lin_len_int);
				lin_len = lin_len_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter CRYPTO 1 key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter CRYPTO 1 key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterCrypto1Key(crypto_1_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				status = LinearRead_PK(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode, crypto_1_sector_key);
				if(status)
				{
					printf("\nLinear read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nLinear read successful\n");
					printf("Data = ");
					print_hex_ln(linear_data, ret_bytes, " ");
					printf("ASCI = %s\n", linear_data);
				}
				break;
			case '2':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter number of bytes for read\n");
				scanf("%d%*c", &lin_len_int);
				lin_len = lin_len_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader CRYPTO 1 key A index (0 -31)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader CRYPTO 1 key B index (0 -31)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				status = LinearRead(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode, key_index);
				if(status)
				{
					printf("\nLinear read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nLinear read successful\n");
					printf("Data = ");
					print_hex_ln(linear_data, ret_bytes, " ");
					printf("ASCI = %s\n", linear_data);
				}
				break;
			case '3':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter number of bytes for read\n");
				scanf("%d%*c", &lin_len_int);
				lin_len = lin_len_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = LinearRead_AKM1(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode);
				if(status)
				{
					printf("\nLinear read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nLinear read successful\n");
					printf("Data = ");
					print_hex_ln(linear_data, ret_bytes, " ");
					printf("ASCI = %s\n", linear_data);
				}
				break;
			case '4':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter number of bytes for read\n");
				scanf("%d%*c", &lin_len_int);
				lin_len = lin_len_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				status = LinearRead_AKM2(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode);
				if(status)
				{
					printf("\nLinear read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nLinear read successful\n");
					printf("Data = ");
					print_hex_ln(linear_data, ret_bytes, " ");
					printf("ASCI = %s\n", linear_data);
				}
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
		else
		{
			printf(" (1) - Provided AES key\n"
				   " (2) - Reader AES key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter number of bytes for read\n");
				scanf("%d%*c", &lin_len_int);
				lin_len = lin_len_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_PLUS_AES_AUTHENT1A;
					printf("\nEnter AES key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_PLUS_AES_AUTHENT1B;
					printf("\nEnter AES key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterAesKeyOrData(aes_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				status = LinearRead_PK(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode, aes_sector_key);
				if(status)
				{
					printf("\nLinear read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nLinear read successful\n");
					printf("Data = ");
					print_hex_ln(linear_data, ret_bytes, " ");
					printf("ASCI = %s\n", linear_data);
				}
				break;
			case '2':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter number of bytes for read\n");
				scanf("%d%*c", &lin_len_int);
				lin_len = lin_len_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
						;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader AES key A index (0 - 16)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader AES key B index (0 - 16)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				status = LinearRead(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode, key_index);
				if(status)
				{
					printf("\nLinear read failed\n");
					printf("Error code = %02X\n", status);
				}
				else
				{
					printf("\nLinear read successful\n");
					printf("Data = ");
					print_hex_ln(linear_data, ret_bytes, " ");
					printf("ASCI = %s\n", linear_data);
				}
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void DataWrite(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                       WRITE DATA TO CARD	                        \n");
	printf(" 		 MIFARE PLUS CARD MUST BE IN SL1 OR SL3 MODE    			\n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t crypto_1_sector_key[6], aes_sector_key[16];
	uint8_t block_nr, sector_nr, auth_mode, key_index;
	int key_index_int, block_nr_int, sector_nr_int;
	uint8_t dl_card_type;
	char key;
	uint8_t block_data[16];
	uint8_t linear_data[3440];
	uint16_t lin_addr, lin_len, ret_bytes;
	int lin_addr_int, lin_len_int;

	status = GetDlogicCardType(&dl_card_type);
	if(status)
	{
		printf("\nCommunication with card failed \n");
		return;
	}

	if(!((dl_card_type >= DL_MIFARE_PLUS_S_2K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL1)
			|| (dl_card_type >= DL_MIFARE_PLUS_S_4K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL1)
			|| (dl_card_type >= DL_MIFARE_PLUS_S_2K_SL3 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL3)
			|| (dl_card_type >= DL_MIFARE_PLUS_S_2K_SL3 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL3)))
	{
		printf("\nCard is not in security level 1 or 3 mode\n");
		return;
	}

	printf(" (1) - Block write\n"
		   " (2) - Block in sector write\n"
		   " (3) - Linear write\n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nBlock write select key mode\n");

		if((dl_card_type >= DL_MIFARE_PLUS_S_2K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL1)
			|| (dl_card_type >= DL_MIFARE_PLUS_S_4K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL1))
		{
			printf(" (1) - Provided CRYPTO 1 key\n"
				   " (2) - Reader CRYPTO 1 key\n"
				   " (3) - AKM1 CRYPTO 1 key\n"
				   " (4) - AKM2 CRYPTO 1 key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter CRYPTO 1 key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter CRYPTO 1 key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterCrypto1Key(crypto_1_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockWrite_PK(block_data, block_nr, auth_mode, crypto_1_sector_key);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '2':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader CRYPTO 1 key A index (0 -31)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader CRYPTO 1 key B index (0 -31)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockWrite(block_data, block_nr, auth_mode, key_index);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock read successful\n");
				break;
			case '3':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockWrite_AKM1(block_data, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '4':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockWrite_AKM2(block_data, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
		else
		{
			printf(" (1) - Provided AES key\n"
				   " (2) - Reader AES key\n"
				   " (3) - AKM1 AES key\n"
				   " (4) - AKM2 AES key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_PLUS_AES_AUTHENT1A;
					printf("\nEnter AES key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_PLUS_AES_AUTHENT1B;
					printf("\nEnter AES key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterAesKeyOrData(aes_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockWrite_PK(block_data, block_nr, auth_mode, aes_sector_key);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '2':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader AES key A index (0 - 16)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader AES key B index (0 - 16)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockWrite(block_data, block_nr, auth_mode, key_index);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '3':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
					printf("\nWrong choice\n");

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockWrite_AKM1(block_data, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '4':
				printf("\nEnter block number (0 - 128 2K card) (0 - 255 4K card)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockWrite_AKM2(block_data, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
	}
	else if(key == '2')
	{
		printf("\nBlock in sector write select key mode\n");

		if((dl_card_type >= DL_MIFARE_PLUS_S_2K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL1)
					|| (dl_card_type >= DL_MIFARE_PLUS_S_4K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL1))
		{
			printf(" (1) - Provided CRYPTO 1 key\n"
				   " (2) - Reader CRYPTO 1 key\n"
				   " (3) - AKM1 CRYPTO 1 key\n"
				   " (4) - AKM2 CRYPTO 1 key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter CRYPTO 1 key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter CRYPTO 1 key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterCrypto1Key(crypto_1_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockInSectorWrite_PK(block_data, sector_nr, block_nr, auth_mode, crypto_1_sector_key);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '2':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader CRYPTO 1 key A index (0 -31)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader CRYPTO 1 key B index (0 -31)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockInSectorWrite(block_data, sector_nr, block_nr, auth_mode, key_index);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '3':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockInSectorWrite_AKM1(block_data, sector_nr, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '4':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockInSectorWrite_AKM2(block_data, sector_nr, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
		else
		{
			printf(" (1) - Provided AES key\n"
				   " (2) - Reader AES key\n"
				   " (3) - AKM1 AES key\n"
				   " (4) - AKM2 AES key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_PLUS_AES_AUTHENT1A;
					printf("\nEnter AES key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_PLUS_AES_AUTHENT1B;
					printf("\nEnter AES key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterAesKeyOrData(aes_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockInSectorWrite_PK(block_data, sector_nr, block_nr, auth_mode, aes_sector_key);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '2':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader AES key A index (0 - 16)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader AES key B index (0 - 16)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockInSectorWrite(block_data, sector_nr, block_nr, auth_mode, key_index);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '3':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockInSectorWrite_AKM1(block_data, sector_nr, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			case '4':
				printf("\nEnter sector number (0 - 31 2K card) (0 - 39 4K card)\n");
				scanf("%d%*c", &sector_nr_int);
				sector_nr = sector_nr_int;

				printf("\nEnter block in sector number (0 - 3 for sectors 0 - 31) (0 - 15 for sectors 32 - 39)\n");
				scanf("%d%*c", &block_nr_int);
				block_nr = block_nr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				printf("\nEnter block data\n");
				if(!EnterAesKeyOrData(block_data))
				{
					printf("\nError while block data entry\n");
					return;
				}

				status = BlockInSectorWrite_AKM2(block_data, sector_nr, block_nr, auth_mode);
				if(status)
				{
					printf("\nBlock write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nBlock write successful\n");
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
	}
	else if(key == '3')
	{
		printf("\nLinear write select key mode\n");

		if((dl_card_type >= DL_MIFARE_PLUS_S_2K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_2K_SL1)
							|| (dl_card_type >= DL_MIFARE_PLUS_S_4K_SL1 && dl_card_type <= DL_MIFARE_PLUS_EV1_4K_SL1))
		{
			printf(" (1) - Provided CRYPTO 1 key\n"
				   " (2) - Reader CRYPTO 1 key\n"
				   " (3) - AKM1 CRYPTO 1 key\n"
				   " (4) - AKM2 CRYPTO 1 key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter CRYPTO 1 key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter CRYPTO 1 key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterCrypto1Key(crypto_1_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				printf("\nEnter data\n");
				if(!EnterLinearData(linear_data, &lin_len))
				{
					printf("\nError while data entry\n");
					return;
				}

				status = LinearWrite_PK(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode, crypto_1_sector_key);
				if(status)
				{
					printf("\nLinear write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nLinear write successful\n");
				break;
			case '2':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader CRYPTO 1 key A index (0 -31)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader CRYPTO 1 key B index (0 -31)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}


				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				printf("\nEnter data\n");
				if(!EnterLinearData(linear_data, &lin_len))
				{
					printf("\nError while data entry\n");
					return;
				}

				status = LinearWrite(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode, key_index);
				if(status)
				{
					printf("\nLinear write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nLinear write successful\n");
				break;
			case '3':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
					printf("\nWrong choice\n");

				printf("\nEnter data\n");
				if(!EnterLinearData(linear_data, &lin_len))
				{
					printf("\nError while data entry\n");
					return;
				}

				status = LinearWrite_AKM1(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode);
				if(status)
				{
					printf("\nLinear write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nLinear write successful\n");
				break;
			case '4':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter number of bytes for read\n");
				scanf("%d%*c", &lin_len_int);
				lin_len = lin_len_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - CRYPTO 1 KEY A\n"
					   " (2) - CRYPTO 1 KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
					auth_mode =  MIFARE_AUTHENT1A;
				else if(key == '2')
					auth_mode = MIFARE_AUTHENT1B;
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				printf("\nEnter data\n");
				if(!EnterLinearData(linear_data, &lin_len))
				{
					printf("\nError while data entry\n");
					return;
				}

				status = LinearRead_AKM2(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode);
				if(status)
				{
					printf("\nLinear write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nLinear write successful\n");
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
		else
		{
			printf(" (1) - Provided AES key\n"
				   " (2) - Reader AES key\n");
			while (!_kbhit())
					;
			key = _getch();

			switch(key)
			{
			case '1':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_PLUS_AES_AUTHENT1A;
					printf("\nEnter AES key A\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_PLUS_AES_AUTHENT1B;
					printf("\nEnter AES key B\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				if(!EnterAesKeyOrData(aes_sector_key))
				{
					printf("\nError while key entry\n");
					return;
				}

				printf("\nEnter data\n");
				if(!EnterLinearData(linear_data, &lin_len))
				{
					printf("\nError while data entry\n");
					return;
				}

				status = LinearWrite_PK(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode, aes_sector_key);
				if(status)
				{
					printf("\nLinear write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nLinear write successful\n");
				break;
			case '2':
				printf("\nEnter linear address (0 - 1519 2K cards) (0 - 3439 4K cards)\n");
				scanf("%d%*c", &lin_addr_int);
				lin_addr = lin_addr_int;

				printf("\nEnter code for authentication mode\n"
					   " (1) - AES KEY A\n"
					   " (2) - AES KEY B\n");
				while (!_kbhit())
					;
				key = _getch();
				if(key == '1')
				{
					auth_mode =  MIFARE_AUTHENT1A;
					printf("\nEnter reader AES key A index (0 - 16)\n");
				}
				else if(key == '2')
				{
					auth_mode = MIFARE_AUTHENT1B;
					printf("\nEnter reader AES key B index (0 - 16)\n");
				}
				else
				{
					printf("\nWrong choice\n");
					return;
				}

				scanf("%d%*c", &key_index_int);
				key_index = key_index_int;

				printf("\nEnter data\n");
				if(!EnterLinearData(linear_data, &lin_len))
				{
					printf("\nError while data entry\n");
					return;
				}

				status = LinearWrite(linear_data, lin_addr, lin_len, &ret_bytes, auth_mode, key_index);
				if(status)
				{
					printf("\nLinear write failed\n");
					printf("Error code = %02X\n", status);
				}
				else
					printf("\nLinear write successful\n");
				break;
			default:
				printf("\nWrong choice\n");
				break;
			}
		}
	}
	else
		printf("\nWrong choice\n");

	return;
}
//------------------------------------------------------------------------------
void ReaderKeys(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                	WRITING KEYS INTO READER                        \n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t crypto_1_key[6], aes_key[16], password[16], key_index;
	uint16_t pass_len;
	char key;
	int key_index_int;

	printf(" (1) - CRIPTO 1 keys \n"
		   " (2) - AES keys \n"
		   " (3) - Unlock reader \n"
		   " (4) - Lock reader \n");
	while (!_kbhit())
		;
	key = _getch();

	if(key == '1')
	{
		printf("\nEnter CRYPTO 1 key\n");
		if(!EnterCrypto1Key(crypto_1_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter key index (0 - 31)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;

		status = ReaderKeyWrite(crypto_1_key, key_index);
		if(status)
		{
			printf("\nWriting key into reader failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nKey written into reader\n");
	}
	else if(key == '2')
	{
		printf("\nEnter AES key\n");
		if(!EnterAesKeyOrData(aes_key))
		{
			printf("\nError while key entry\n");
			return;
		}

		printf("\nEnter key index (0 - 15)\n");
		scanf("%d%*c", &key_index_int);
		key_index = key_index_int;

		status = uFR_int_DesfireWriteAesKey(key_index, aes_key);
		if(status)
		{
			printf("\nWriting key into reader failed\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nKey written into reader\n");
	}
	else if(key == '3')
	{
		printf("\nEnter password of 8 bytes\n");

		if(!EnterLinearData(password, &pass_len))
		{
			printf("\nError while password entry\n");
			return;
		}
		if(pass_len != 8)
		{
			printf("\nPassword length is wrong\n");
			return;
		}

		status = ReaderKeysUnlock(password);
		if(status)
		{
			printf("\nUnlock keys error\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nReader keys are unlocked\n");
	}
	else if(key == '4')
	{
		printf("\nEnter password of 8 bytes\n");

		if(!EnterLinearData(password, &pass_len))
		{
			printf("\nError while password entry\n");
			return;
		}
		if(pass_len != 8)
		{
			printf("\nPassword length is wrong\n");
			return;
		}

		status = ReaderKeysLock(password);
		if(status)
		{
			printf("\nUnlock keys error\n");
			printf("Error code = %02X\n", status);
		}
		else
			printf("\nReader keys are locked\n");
	}
	else
		printf("\nWrong choice\n");

	return;
}
