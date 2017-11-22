/*
    Copyright (C) 2017  github.com/jackfagner

    This file is part of NfcKey.

    NfcKey is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NfcKey is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with NfcKey.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#ifndef ROTL
# define ROTL(x,n) (((uintmax_t)(x) << (n)) | ((uintmax_t)(x) >> ((sizeof(x) * 8) - (n))))
#endif

void transform(uint8_t* ru);
uint32_t getkey(uint8_t* uid);
uint16_t getpack(uint8_t* uid);
void parseuid(char* arg, uint8_t* uid);

int main(int argc, char *argv[]) {
	if (argc != 2 || strlen(argv[1]) != 14)
		return 1;
	
	uint8_t uid[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	parseuid(argv[1], uid);
	
	uint32_t key = getkey(uid);
	
	printf("KEY : %02X%02X%02X%02X\n", 
		key & 0xFF, 
		(key >> 8) & 0xFF,
		(key >> 16) & 0xFF,
		(key >> 24) & 0xFF);
	
	uint16_t pack = getpack(uid);
	
	printf("PACK: %02X%02X\n", 
		pack & 0xFF, 
		(pack >> 8) & 0xFF);

	return 0;
}

void parseuid(char* arg, uint8_t* uid)
{
	if (strlen(arg) != 14)
		return;
	unsigned int u[7];
	sscanf(arg, "%2x%2x%2x%2x%2x%2x%2x", &u[0], &u[1], &u[2], &u[3], &u[4], &u[5], &u[6]);
	uint8_t i;
	for (i = 0; i < 7; i++)
		uid[i] = u[i] & 0xFF;
}

const uint32_t c[] = { 
	0x6D835AFC, 0x7D15CD97, 0x0942B409, 0x32F9C923, 0xA811FB02, 0x64F121E8, 
	0xD1CC8B4E, 0xE8873E6F, 0x61399BBB, 0xF1B91926, 0xAC661520, 0xA21A31C9, 
	0xD424808D, 0xFE118E07, 0xD18E728D, 0xABAC9E17, 0x18066433, 0x00E18E79, 
	0x65A77305, 0x5AE9E297, 0x11FC628C, 0x7BB3431F, 0x942A8308, 0xB2F8FD20, 
	0x5728B869, 0x30726D5A
};

uint32_t getkey(uint8_t* uid)
{
	uint8_t i;
	//Rotate
	uint8_t r = (uid[1] + uid[3] + uid[5]) & 7; //Rotation offset
	uint8_t ru[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Rotated UID
	for (i = 0; i < 7; i++)
		ru[(i + r) & 7] = uid[i];
	
	//Transform
	transform(ru);
	
	//Calc key
	uint32_t k = 0; //Key as int
	r = (ru[0] + ru[2] + ru[4] + ru[6]) & 3; //Offset
	for (i = 0; i < 4; i++)
		k = ru[i + r] + (k << 8);

	return k;
}

uint16_t getpack(uint8_t* uid)
{
	uint8_t i;
	//Rotate
	uint8_t r = (uid[2] + uid[5]) & 7; //Rotation offset
	uint8_t ru[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Rotated UID
	for (i = 0; i < 7; i++)
		ru[(i + r) & 7] = uid[i];
	
	//Transform
	transform(ru);
	
	//Calc pack
	uint32_t p = 0;	
	for (i = 0; i < 8; i++)
		p += ru[i] * 13;

	return (p ^ 0x5555) & 0xFFFF;
}

void transform(uint8_t* ru)
{
	//Transform
	uint8_t i;
	uint8_t p = 0;
	uint32_t v1 = ((ru[3] << 24) | (ru[2] << 16) | (ru[1] << 8) | ru[0]) + c[p++];
	uint32_t v2 = ((ru[7] << 24) | (ru[6] << 16) | (ru[5] << 8) | ru[4]) + c[p++];
	for (i = 0; i < 12; i += 2)
	{
		uint32_t t1 = ROTL(v1 ^ v2, v2 & 0x1F) + c[p++];
		uint32_t t2 = ROTL(v2 ^ t1, t1 & 0x1F) + c[p++];
		v1 = ROTL(t1 ^ t2, t2 & 0x1F) + c[p++];
		v2 = ROTL(t2 ^ v1, v1 & 0x1F) + c[p++];
	}

	//Re-use ru
	ru[0] = v1 & 0xFF;
	ru[1] = (v1 >> 8) & 0xFF;
	ru[2] = (v1 >> 16) & 0xFF;
	ru[3] = (v1 >> 24) & 0xFF;
	ru[4] = v2 & 0xFF;
	ru[5] = (v2 >> 8) & 0xFF;
	ru[6] = (v2 >> 16) & 0xFF;
	ru[7] = (v2 >> 24) & 0xFF;
}

