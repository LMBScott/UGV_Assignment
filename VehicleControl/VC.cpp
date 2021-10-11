#include "VC.hpp"
#include <SMObject.h>
#include <smstructs.h>
#include <string.h>

#using <mscorlib.dll>

int VehicleControl::connect(String^ hostName, int portNumber) {
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