#using <System.dll>
#include "Camera.hpp"
#include <SMObject.h>

int Camera::connect(String^ hostName, int portNumber)
{
	return SUCCESS;
}

int Camera::setupSharedMemory()
{
	// Declaration and Initialisation
	SMObject* PMObj = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));

	// SM creation and access
	PMObj->SMAccess();

	ProcessManagementData = (SMObject*)PMObj->pData;

	return SUCCESS;
}

int Camera::getData()
{
	return SUCCESS;
}

int Camera::checkData()
{
	return 1;
}

int Camera::sendDataToSharedMemory()
{
	return SUCCESS;
}

bool Camera::getShutdownFlag()
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Shutdown.Flags.Camera;
}

int Camera::setHeartbeat(bool heartbeat)
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	PMData->Heartbeat.Flags.Camera = heartbeat;

	return SUCCESS;
}

bool Camera::getHeartbeat() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Heartbeat.Flags.Camera;
}

Camera::~Camera()
{
	delete ProcessManagementData;
}
