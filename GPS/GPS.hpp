#pragma once

#include <UGV_module.h>
#include <smstructs.h>
#include "GPS_Struct.hpp"

#define CRC32_POLYNOMIAL 0xEDB88320L
#define GPS_HEADER 0xAA44121C // Header of a GPS data stream
#define GPS_DATA_LENGTH 112   // Length in bytes of a GPS data packet
#define CRC_LENGTH 4		  // Length in bytes of a GPS CRC checksum

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

ref class GPS : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	GPS_Data_Struct packData();
	bool getShutdownFlag() override;
	int setHeartbeat(bool heartbeat) override;
	bool getHeartbeat() override;
	~GPS();

protected:
	int dataStartIndex;
};
