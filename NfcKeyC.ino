#include <SPI.h>
#include <MFRC522.h>

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#ifndef ROTL
# define ROTL(x,n) (((uintmax_t)(x) << (n)) | ((uintmax_t)(x) >> ((sizeof(x) * 8) - (n))))
#endif

constexpr uint8_t RST_PIN = 9;
constexpr uint8_t SS_PIN = 10;

MFRC522 mfrc522(SS_PIN, RST_PIN);

uint32_t c[] = {
	0x6D835AFC, 0x7D15CD97, 0x0942B409, 0x32F9C923, 0xA811FB02, 0x64F121E8,
	0xD1CC8B4E, 0xE8873E6F, 0x61399BBB, 0xF1B91926, 0xAC661520, 0xA21A31C9,
	0xD424808D, 0xFE118E07, 0xD18E728D, 0xABAC9E17, 0x18066433, 0x00E18E79,
	0x65A77305, 0x5AE9E297, 0x11FC628C, 0x7BB3431F, 0x942A8308, 0xB2F8FD20,
	0x5728B869, 0x30726D5A
};

void setup() {
	Serial.begin(115200);
	while (!Serial);
	SPI.begin();
	mfrc522.PCD_Init();

	pinMode(2, OUTPUT);
	digitalWrite(2, HIGH);

	Serial.println("Hold the cartridge tag over the reader..");
	while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial());
	uint32_t k = getkey(mfrc522.uid.uidByte);
	uint16_t p = getpack(mfrc522.uid.uidByte);
	Serial.print("UID:"); printHex(mfrc522.uid.uidByte, 7);
	Serial.print("KEY:"); Serial.println(k, HEX);
	Serial.print("PACK:"); Serial.println(p, HEX);
}

void loop() {}

void printHex(uint8_t array[], unsigned int len) {
	char buffer[3];
	buffer[2] = NULL;
	for (int j = 0; j < len; j++) {
		sprintf(&buffer[0], "%02X", array[j]);
		Serial.print(buffer);
	} Serial.println();
}

void transform(uint8_t* ru)
{
	//Transform
	uint8_t i;
	uint8_t p = 0;
	uint32_t v1 = (((uint32_t)ru[3] << 24) | ((uint32_t)ru[2] << 16) | ((uint32_t)ru[1] << 8) | (uint32_t)ru[0]) + c[p++];
	uint32_t v2 = (((uint32_t)ru[7] << 24) | ((uint32_t)ru[6] << 16) | ((uint32_t)ru[5] << 8) | (uint32_t)ru[4]) + c[p++];

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

uint32_t getkey(uint8_t* uid)
{
	int i;
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
	for (i = 4; i >= 0; i--) 
		k = ru[i + r] + (k << 8);

	return k;
}

uint16_t getpack(uint8_t* uid)
{
	int i;
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
	
	p = (p ^ 0x5555) & 0xFFFF;
	return (p & 0xFF00) >> 8 | (p & 0x00FF) << 8;
}