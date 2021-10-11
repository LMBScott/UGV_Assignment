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

	String^ AskScan = gcnew String("sRN LMDscandata");
	SendData = Text::Encoding::ASCII->GetBytes(AskScan);

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

int VehicleControl::checkData() {
	SM_Laser* LData = (SM_Laser*)SensorData;

	if (LData != NULL) {
		return (int)(LData->numPoints == STANDARD_LASER_LENGTH);
	}

	return 0;
}

int VehicleControl::sendDataToSharedMemory() {
	int dataLength = ReadData->Length;

	array<int>^ ConvertedData = gcnew array<int>(ReadData->Length);

	array<wchar_t>^ Sep = { ' ' };
	array <String^>^ ResponseData = System::Text::Encoding::ASCII->GetString(ReadData)->Split(Sep, System::StringSplitOptions::None);

	SM_Laser* LData = (SM_Laser*)SensorData;

	double StartAngle = System::Convert::ToInt32(ResponseData[23], 16);
	double Resolution = System::Convert::ToInt32(ResponseData[24], 16);
	int NumPoints = System::Convert::ToInt32(ResponseData[25], 16);

	LData->numPoints = NumPoints;

	array<double>^ Range = gcnew array<double>(NumPoints);
	array<double>^ RangeX = gcnew array<double>(NumPoints);
	array<double>^ RangeY = gcnew array<double>(NumPoints);

	for (int i = 0; i < NumPoints; i++) {
		Range[i] = System::Convert::ToInt32(ResponseData[25 + i], 16);
		LData->x[i] = Range[i] * sin(i * Resolution);
		LData->y[i] = -Range[i] * cos(i * Resolution);
		Console::WriteLine("Point {0, 0:N}: x: {1, 12:F3}, y: {2, 12:F3}", i, LData->x[i], LData->y[i]);
	}

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
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}