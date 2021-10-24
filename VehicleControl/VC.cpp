#include "VC.hpp"
#include <SMObject.h>
#include <smstructs.h>
#include <string.h>

#using <mscorlib.dll>

int VehicleControl::connect(String^ hostName, int portNumber) {
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

	return SUCCESS;
}

int VehicleControl::setupSharedMemory() {
	// Declaration and Initialisation
	SMObject* PMObj = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject* VCObj = new SMObject(TEXT("SM_VehicleControl"), sizeof(SM_VehicleControl));

	// SM creation and access
	PMObj->SMAccess();

	VCObj->SMCreate();
	VCObj->SMAccess();

	ProcessManagementData = (SMObject*)PMObj->pData;
	SensorData = (SMObject*)VCObj->pData;

	return SUCCESS;
}
int VehicleControl::getData() {
	return SUCCESS;
}

int VehicleControl::checkData() {
	return 0;
}

int VehicleControl::sendSteeringData() {
	ControlFlag = !ControlFlag; // Flip control flag value with each transmission

	SM_VehicleControl *VCData = (SM_VehicleControl*)SensorData;

	Console::WriteLine("Steering: {0, 5:F3}, Speed: {1, 5:F3}", VCData->Steering, VCData->Speed);

	char buff[64];
	sprintf(buff, "# %.2f %.2f %d #", VCData->Steering, VCData->Speed, ControlFlag);
	
	String^ ControlString = gcnew String(buff); // Convert char array to CLR String handle

	SendData = System::Text::Encoding::ASCII->GetBytes(ControlString); // Encode control string to binary

	Stream->Write(SendData, 0, SendData->Length);

	return SUCCESS;
}

int VehicleControl::sendDataToSharedMemory() {
	return SUCCESS;
}

bool VehicleControl::getShutdownFlag() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Shutdown.Flags.VehicleControl;
}

int VehicleControl::setHeartbeat(bool heartbeat) {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	PMData->Heartbeat.Flags.VehicleControl = heartbeat;

	return SUCCESS;
}

bool VehicleControl::getHeartbeat() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Heartbeat.Flags.VehicleControl;
}

VehicleControl::~VehicleControl() {
	//Stream->Close();
	//Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}