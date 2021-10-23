#pragma once
#pragma pack(1)

struct GPS_Data_Struct
{
	unsigned int Header; // 0xAA 0x44 0x12 0x1C
	unsigned char DiscardLot1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char DiscardLot2[40];
	unsigned int CRC;
};