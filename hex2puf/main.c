#include <stdio.h>
#include <string.h>

#define BOOT_PAGE_WORD_SIZE 64
#define BOOT_PAGE_BYTE_SIZE (BOOT_PAGE_WORD_SIZE*2)
#define BOOT_PAGE_COUNT 256
#define BOOT_NRWW_PAGE_COUNT 32
#define BOOT_RWW_PAGE_COUNT (BOOT_PAGE_COUNT - BOOT_NRWW_PAGE_COUNT)

unsigned char s_pFirmware[BOOT_PAGE_BYTE_SIZE * BOOT_RWW_PAGE_COUNT];
int main(int lArgCount, char *pArgs[]) {
	FILE *pHexFile, *pPuf;
	char szDataHexes[34+1];
	unsigned short int uwLine, uwSize, uwByteAddr, uwType, i;
	char bWhiteSpace;
	unsigned short int *pFirmwareWords;
	unsigned char ubPage, ubRealPages;
	char szPufPath[255];
	
	pHexFile = fopen(pArgs[1], "r");
	if(!pHexFile) {
		printf("ERR: couldn't open hex file %s", pArgs[2]);
		return 1;
	}
	
	// Read .hex file
	uwLine = 1;
	fgetc(pHexFile); // Omit first char
	while(!feof(pHexFile)) {
		fscanf(pHexFile, "%2hx%4hx%2hx", &uwSize, &uwByteAddr, &uwType);
		fgets(szDataHexes, 34+1, pHexFile);
		if(uwType == 1) {
			// printf("Reached EOF record!\n");
			break;
		}
		else if(uwType) {
			printf("ERR: unknown .hex type: %02x at line %u\n", uwType, uwLine);
			fclose(pHexFile);
			return 1;
		}
		if(uwByteAddr+uwSize > BOOT_PAGE_BYTE_SIZE * BOOT_RWW_PAGE_COUNT) {
			printf("ERR: address out of range: %u, max: %d", uwByteAddr, BOOT_PAGE_BYTE_SIZE * BOOT_RWW_PAGE_COUNT);
			fclose(pHexFile);
			return 1;
		}
		
		// Put read data into firmware buffer
		for(i = 0; i != uwSize; ++i) {
			sscanf(&szDataHexes[2*i], "%2hhx", (char*)&s_pFirmware[uwByteAddr + i]);
		}

		// Skip newline chars
		do {
			bWhiteSpace = fgetc(pHexFile);
		} while(bWhiteSpace <= 32);
		++uwLine;
	}
	fclose(pHexFile);
	
	// Convert endianness
	pFirmwareWords = (unsigned short int *)s_pFirmware;
	for(i = 0; i != BOOT_PAGE_WORD_SIZE*BOOT_RWW_PAGE_COUNT; ++i)
		pFirmwareWords[i] = (pFirmwareWords[i] >> 8) | ((pFirmwareWords[i] & 0xFF) << 8);
	
	strcpy(szPufPath, pArgs[1]);
	memcpy(&szPufPath[strlen(szPufPath)-4], ".puf", 4);
	pPuf = fopen(szPufPath, "wb");
	fwrite(&ubRealPages, 1, 1, pPuf); // Make space for page count
	// Count non-empty pages
	ubRealPages = 0;
	for(ubPage = 0; ubPage != BOOT_RWW_PAGE_COUNT; ++ubPage) {
		for(i = 0; i != BOOT_PAGE_BYTE_SIZE; ++i) {
			if(s_pFirmware[ubPage*BOOT_PAGE_BYTE_SIZE + i]) {
				// Write only non-empty pages
				++ubRealPages;
				fwrite(&ubPage, 1, 1, pPuf);
				fwrite(&s_pFirmware[ubPage*BOOT_PAGE_BYTE_SIZE], BOOT_PAGE_BYTE_SIZE, 1, pPuf);
				break;
			}
		}
	}
	printf("Pages written: %u\n", ubRealPages);
	
	// Write page count
	fseek(pPuf, 0, SEEK_SET);
	fwrite(&ubRealPages, 1, 1, pPuf);
	
	// fwrite(s_pFirmware, BOOT_PAGE_BYTE_SIZE * BOOT_RWW_PAGE_COUNT, 1, pPuf);
	fclose(pPuf);
}