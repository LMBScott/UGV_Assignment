#using <System.dll>
#include "GPS.hpp"
#include <SMObject.h>
#include <smstructs.h>

using namespace System::Threading;

int GPS::connect(String^ hostName, int portNumber)
{
	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient(hostName, portNumber);

	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 1000;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 2048;
	Client->SendBufferSize = 1024;

	// Get the network stream object associated with client so we 
	// can use it to read and write
	Stream = Client->GetStream();

	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	return SUCCESS;
}

int GPS::setupSharedMemory()
{
	// Declaration and Initialisation
	SMObject* PMObj = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject* GPSObj = new SMObject(TEXT("SM_GPS"), sizeof(SM_GPS));

	// SM creation and access
	PMObj->SMAccess();

	GPSObj->SMCreate();
	GPSObj->SMAccess();

	ProcessManagementData = (SMObject*)PMObj->pData;
	SensorData = (SMObject*)GPSObj->pData;

	return SUCCESS;
}

int GPS::getData()
{
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);

	Thread::Sleep(20);

	unsigned int Header = 0;
	int i = 0;
	do {
		unsigned char Data = ReadData[i++];
		Header = ((Header << 8) | Data);
	} while (Header != GPS_HEADER);

	dataStartIndex = i - 4;

	return SUCCESS;
}

int GPS::checkData()
{
	// Verify that CRC checksum matches expected value

	// Get data block as char array
	unsigned char dataBlock[GPS_DATA_LENGTH] = {};

	for (int i = 0; i < GPS_DATA_LENGTH; i++) {
		dataBlock[i] = ReadData[dataStartIndex + i];
	}

	// Calculate expected CRC value
	unsigned long trueCRC = CalculateBlockCRC32(GPS_DATA_LENGTH, dataBlock);
	
	Console::WriteLine("True CRC: {0:12N}", trueCRC);

	// Extract actual CRC value
	unsigned long blockCRC = 0;

	unsigned long* blockCRCPtr = (unsigned long*)&blockCRC;

	for (int i = 0; i < sizeof(unsigned long); i++) {
		*(blockCRCPtr + i) = dataBlock[GPS_DATA_LENGTH - 4 + i];
	}

	return (blockCRC == trueCRC);
}

int GPS::sendDataToSharedMemory()
{
	SM_GPS* GPSData = (SM_GPS*)SensorData;
	unsigned char* BytePtr = (unsigned char*)GPSData;

	for (int i = 0; i < GPS_DATA_LENGTH; i++) {
		*(BytePtr + i) = ReadData[dataStartIndex + i];
	}
	
	Console::WriteLine("Northing: {0, 5:F3}, Easting: {1, 5:F3}, Height: {2, 5:F3}m", GPSData->Northing, GPSData->Easting, GPSData->Height);

	return 1;
}

bool GPS::getShutdownFlag()
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Shutdown.Flags.GPS;
}

int GPS::setHeartbeat(bool heartbeat)
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	PMData->Heartbeat.Flags.GPS = heartbeat;

	return SUCCESS;
}

bool GPS::getHeartbeat() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Heartbeat.Flags.GPS;
}

GPS::~GPS()
{
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}

unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}