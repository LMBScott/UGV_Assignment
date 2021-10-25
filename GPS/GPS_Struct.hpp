#pragma once
#pragma pack(1)

// Struct to enable simple translation of GPS data from raw binary to C++ data types
struct GPS_Data_Struct
{
	unsigned int Header;			// GPS data block header, expected value: 0xAA 0x44 0x12 0x1C
	unsigned char DiscardLot1[40];  // Not needed for assignment
	double Northing;                // Northing value in metres, UTM coordinates
	double Easting;                 // Easting value in metres, UTM coordinates
	double Height;                  // Height value in metres
	unsigned char DiscardLot2[40];  // Not needed for assignment
	unsigned int CRC;				// CRC checksum
};
