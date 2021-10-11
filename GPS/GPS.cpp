#using <System.dll>
#include "GPS.hpp"
#include <SMObject.h>
#include <smstructs.h>

int GPS::connect(String^ hostName, int portNumber)
{
	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient(hostName, portNumber);

	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// Get the network stream object associated with client so we 
	// can use it to read and write
	Stream = Client->GetStream();

	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	// Authenticate user
	String^ zID = gcnew String("z5207471\n");
	SendData = System::Text::Encoding::ASCII->GetBytes(zID);
	Stream->Write(SendData, 0, SendData->Length);

	System::Threading::Thread::Sleep(10);

	Stream->Read(ReadData, 0, ReadData->Length);

	String^ ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	Console::WriteLine(ResponseData);

	String^ AskScan = gcnew String("sRN LMDscandata");
	SendData = Text::Encoding::ASCII->GetBytes(AskScan);

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
	// Write command asking for data
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);

	System::Threading::Thread::Sleep(10);

	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);

	String^ ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);

	return SUCCESS;
}

int GPS::checkData()
{
	// YOUR CODE HERE
	return 1;
}

int GPS::sendDataToSharedMemory()
{
	SM_GPS GPSStruct;
	unsigned char* BytePtr = (unsigned char*)&GPSStruct;

	for (int i = 0; i < sizeof(SM_GPS); i++) {
		*(BytePtr + i) = ReadData[i];
	}
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