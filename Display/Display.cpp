#using <System.dll>
#include "Display.hpp"
#include <SMObject.h>
#include <smstructs.h>

int Display::connect(String^ hostName, int portNumber)
{
	return SUCCESS;
}

int Display::setupSharedMemory()
{
	return SUCCESS;
}

int Display::getData()
{
	return SUCCESS;
}

int Display::checkData()
{
	return 1;
}

int Display::sendDataToSharedMemory()
{
	return 1;
}

bool Display::getShutdownFlag()
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Shutdown.Flags.OpenGL;
}

int Display::setHeartbeat(bool heartbeat)
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	PMData->Heartbeat.Flags.OpenGL = heartbeat;

	return SUCCESS;
}

bool Display::getHeartbeat() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Heartbeat.Flags.OpenGL;
}

Display::~Display()
{
}
