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

	SendData = gcnew array<unsigned char>(1024);

	String^ zID = gcnew String("5207471");
	SendData = System::Text::Encoding::ASCII->GetBytes(zID);
	Stream->Write(SendData, 0, SendData->Length);

	System::Threading::Thread::Sleep(20);

	String^ AskScan = gcnew String("sRN LMDscandata");
	SendData = Text::Encoding::ASCII->GetBytes(AskScan);

	ReadData = gcnew array<unsigned char>(1024);

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

	System::Threading::Thread::Sleep(20);

	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);

	String^ ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);

	Console::WriteLine(ResponseData);

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

	array<wchar_t>^ Sep = { ' ' };
	array <String^>^ ResponseData = System::Text::Encoding::ASCII->GetString(ReadData)->Split(Sep, System::StringSplitOptions::None);

	double StartAngle = System::Convert::ToInt32(ResponseData[23], 16);
	double Resolution = System::Convert::ToInt32(ResponseData[24], 16);
	int NumRanges = System::Convert::ToInt32(ResponseData[25], 16);

	array<double>^ Range = gcnew array<double>(NumRanges);
	array<double>^ RangeX = gcnew array<double>(NumRanges);
	array<double>^ RangeY = gcnew array<double>(NumRanges);

	for (int i = 0; i < NumRanges; i++) {
		Range[i] = System::Convert::ToInt32(ResponseData[25 + i], 16);
		RangeX[i] = Range[i] * sin(i * Resolution);
		RangeY[i] = Range[i] * cos(i * Resolution);
	}

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
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}