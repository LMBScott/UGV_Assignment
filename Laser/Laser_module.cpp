#include "Laser_module.h"
#include <SMObject.h>
#include <smstructs.h>

int Laser_module::connect(String^ hostName, int portNumber) {
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
int Laser_module::setupSharedMemory() {
	// Declaration and Initialisation
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject LaserObj(TEXT("SM_Laser"), sizeof(SM_Laser));

	// SM creation and access
	PMObj.SMCreate();
	PMObj.SMAccess();

	LaserObj.SMCreate();
	LaserObj.SMAccess();

	ProcessManagementData = (SMObject*)PMObj.pData;
	SensorData = (SMObject*)LaserObj.pData;

	return SUCCESS;
}
int Laser_module::getData() {
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

	return SUCCESS;
}

int Laser_module::checkData() {
	return SUCCESS;
}

int Laser_module::sendDataToSharedMemory() {
	return SUCCESS;
}

bool Laser_module::getShutdownFlag() {
	return ((ProcessManagement*)ProcessManagementData)->Shutdown.Flags.Laser;
}

int Laser_module::setHeartbeat(bool heartbeat) {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	PMData->Heartbeat.Flags.Laser |= 1UL;
	return SUCCESS;
}