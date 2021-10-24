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

	unsigned int Header = 0;
	int i = 0;
	do {
		unsigned char Data = ReadData[i++];
		Header = ((Header << 8) | Data);
	} while (Header != GPS_HEADER);

	dataStartIndex = i - 4;

	Console::WriteLine("GPS data header found at index {0, 3:N} (Value: {1, 12:N} / Expected: {2, 12:N})", dataStartIndex, Header, GPS_HEADER);

	return SUCCESS;
}

GPS_Data_Struct GPS::packData() {
	GPS_Data_Struct readStruct = { 0 };
	unsigned char* BytePtr = (unsigned char*)&readStruct;

	for (int i = 0; i < GPS_DATA_LENGTH; i++) {
		*(BytePtr + i) = ReadData[dataStartIndex + i];
	}

	return readStruct;
}

int GPS::checkData()
{
	// Verify that CRC checksum matches expected value

	// Get data block as char array
	unsigned char dataBlock[GPS_DATA_LENGTH] = {};
	
	// Pack data into unsigned char array (excluding CRC value)
	for (int i = 0; i < GPS_DATA_LENGTH - CRC_LENGTH; i++) {
		dataBlock[i] = (unsigned char)ReadData[dataStartIndex + i];
	}

	// Calculate expected CRC value from binary data
	unsigned long trueCRC = CalculateBlockCRC32(GPS_DATA_LENGTH - CRC_LENGTH, dataBlock);
	
	Console::WriteLine("True CRC: {0, 12:N}", trueCRC);

	// Extract actual CRC value from struct-packed data
	GPS_Data_Struct GPSDataStruct = packData();

	Console::WriteLine("Read CRC: {0, 12:N}", GPSDataStruct.CRC);
	
	// Data validity is determined by whether or not CRC values match
	return (GPSDataStruct.CRC == trueCRC);
}

int GPS::sendDataToSharedMemory()
{
	SM_GPS* GPSData = (SM_GPS*)SensorData;

	GPS_Data_Struct GPSDataStruct = packData();

	GPSData->Northing = GPSDataStruct.Northing;
	GPSData->Easting = GPSDataStruct.Easting;
	GPSData->Height = GPSDataStruct.Height;
	
	Console::WriteLine("Northing: {0, 5:F3}m, Easting: {1, 5:F3}m, Height: {2, 5:F3}m, CRC: {3}", GPSData->Northing, GPSData->Easting, GPSData->Height, GPSDataStruct.CRC);

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