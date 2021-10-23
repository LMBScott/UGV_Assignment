#include "Laser.hpp"
#include <SMObject.h>
#include <smstructs.h>
#include <string.h>

#using <mscorlib.dll>

#define START_ANGLE_INDEX 23
#define RESOLUTION_INDEX 24
#define NUM_POINTS_INDEX 25
#define DATA_START_INDEX 26

#define ANGLE_DIVISION 10000
#define PI 3.1415926536

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
int Laser::setupSharedMemory() {
	// Declaration and Initialisation
	SMObject *PMObj = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject *LaserObj = new SMObject(TEXT("SM_Laser"), sizeof(SM_Laser));

	// SM creation and access
	PMObj->SMAccess();

	LaserObj->SMCreate();
	LaserObj->SMAccess();

	ProcessManagementData = (SMObject*)PMObj->pData;
	SensorData = (SMObject*)LaserObj->pData;

	return SUCCESS;
}
int Laser::getData() {
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

int Laser::checkData() {
	SM_Laser* LData = (SM_Laser*)SensorData;

	if (LData != NULL) {
		return (int)(numPointsRead == STANDARD_LASER_LENGTH);
	}

	return 0;
}

int Laser::sendDataToSharedMemory() {
	array<wchar_t>^ Sep = { ' ' };
	array <String^>^ ResponseData = System::Text::Encoding::ASCII->GetString(ReadData)->Split(Sep, System::StringSplitOptions::None);

	SM_Laser* LData = (SM_Laser*)SensorData;

	double StartAngle = System::Convert::ToInt32(ResponseData[START_ANGLE_INDEX], 16) / (double)ANGLE_DIVISION;
	//Console::WriteLine("Start angle:" + StartAngle);
	double Resolution = System::Convert::ToInt32(ResponseData[RESOLUTION_INDEX], 16) / (double)ANGLE_DIVISION;
	//Console::WriteLine("Resolution:" + Resolution);
	int NumPoints = System::Convert::ToInt32(ResponseData[NUM_POINTS_INDEX], 16);

	numPointsRead = NumPoints;

	Console::WriteLine("Laser data points received:" + LData->numPoints);

	if (checkData()) {
		LData->numPoints = NumPoints;
		array<double>^ Range = gcnew array<double>(NumPoints);

		for (int i = 0; i < NumPoints; i++) {
			Range[i] = System::Convert::ToInt32(ResponseData[DATA_START_INDEX + i], 16);
			double angle = (StartAngle + i * Resolution) * PI / 180; // Get point angle in radians
			LData->x[i] = Range[i] * cos(angle);
			LData->y[i] = Range[i] * sin(angle);
			//Console::WriteLine("Point {0, 3:N}: x: {1, 12:F3}mm, y: {2, 12:F3}mm", i, LData->x[i], LData->y[i]);
		}
	}

	return SUCCESS;
}

bool Laser::getShutdownFlag() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Shutdown.Flags.Laser;
}

int Laser::setHeartbeat(bool heartbeat) {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	PMData->Heartbeat.Flags.Laser = heartbeat;

	return SUCCESS;
}

bool Laser::getHeartbeat() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Heartbeat.Flags.Laser;
}

Laser::~Laser() {
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}