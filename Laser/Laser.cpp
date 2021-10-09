#include "Laser.h"
#include <SMObject.h>
#include <smstructs.h>
#include <string.h>

#using <mscorlib.dll>

int Laser::connect(String^ hostName, int portNumber) {
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

	return SUCCESS;
}
int Laser::setupSharedMemory() {
	// Declaration and Initialisation
	SMObject *PMObj = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject *LaserObj = new SMObject(TEXT("SM_Laser"), sizeof(SM_Laser));

	// SM creation and access
	PMObj->SMCreate();
	PMObj->SMAccess();

	LaserObj->SMCreate();
	LaserObj->SMAccess();

	ProcessManagementData = (SMObject*)PMObj->pData;
	SensorData = (SMObject*)LaserObj->pData;

	return SUCCESS;
}
int Laser::getData() {
	array<unsigned char>^ SendData;
	SendData = gcnew array<unsigned char>(16);
	String^ AskScan = gcnew String("sRN LMDscandata");
	SendData = Text::Encoding::ASCII->GetBytes(AskScan);

	// Write command asking for data
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);

	System::Threading::Thread::Sleep(10);

	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);

	Console::WriteLine(ReadData);

	return SUCCESS;
}

int Laser::checkData() {
	// Check device command
	

	// Check data string length
	int expectedLength = 87 + 2 * NUM_DATA_POINTS; // Expected length of data string in bytes
	
	try {
		if (ReadData->Length != expectedLength) {
			throw ReadData->Length;
		}
	}
	catch (int length) {
		std::cerr << "Error: Laser data string was of unexpected length (Expected " << expectedLength << " bytes, got " << length << ")." << std::endl;
	}
	

	return SUCCESS;
}

int Laser::sendDataToSharedMemory() {
	int dataLength = ReadData->Length;

	array<int>^ ConvertedData = gcnew array<int>(ReadData->Length);

	for (int i = 0; i < ReadData->Length; i++) {
		ConvertedData[i] = System::Convert::ToInt32(ReadData[i]);
	}
	
	SensorData->pData = &ConvertedData;

	return SUCCESS;
}

bool Laser::getShutdownFlag() {
	return ((ProcessManagement*)ProcessManagementData)->Shutdown.Status;
}

int Laser::setHeartbeat(bool heartbeat) {
	((ProcessManagement*)ProcessManagementData)->Heartbeat.Flags.Laser |= 1UL;
	return SUCCESS;
}

Laser::~Laser() {
	delete ProcessManagementData;
	delete SensorData;
}