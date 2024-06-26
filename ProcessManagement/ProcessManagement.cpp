#include "ProcessManagement.hpp"
#include <SMObject.h>
#include <smstructs.h>
#include <string.h>

#using <mscorlib.dll>

using namespace System::Diagnostics;

int PM_Module::connect(String^ hostName, int portNumber) {
	return SUCCESS;
}

int PM_Module::setupSharedMemory() {
	// Declaration and Initialisation
	SMObject* PMObj = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));

	// SM creation and access
	PMObj->SMCreate();
	PMObj->SMAccess();

	ProcessManagementData = (SMObject*)PMObj->pData;

	return SUCCESS;
}

int PM_Module::setupDataStructures() {
	ModuleList = gcnew array<String^> { "Laser_z5207471", "VehicleControl_z5207471", "GPS_z5207471", "Display_z5207471", "Camera_z5207471" };
	MaxWait = gcnew array<int>(ModuleList->Length) { 5, 5, 5, 5, 5 };
	Critical = gcnew array<int>(ModuleList->Length) { 0, 0, 0, 0, 0 };
	ProcessList = gcnew array<Process^>(ModuleList->Length);

	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;

	memset(PMData->LifeCounters, 0, sizeof(PMData->LifeCounters)); // Initialise LifeCounters to 0

	return SUCCESS;
}

int PM_Module::startProcesses() {
	for (int i = 0; i < ModuleList->Length; i++) {
		if (Process::GetProcessesByName(ModuleList[i])->Length == 0) { // If there are no current instances of process
			ProcessList[i] = gcnew Process;
			ProcessList[i]->StartInfo->WorkingDirectory = REMOTE_WD;
			ProcessList[i]->StartInfo->FileName = ModuleList[i];
			ProcessList[i]->Start();
			Console::WriteLine("Started process for module: " + ModuleList[i]);
		}
	}

	return SUCCESS;
}

int PM_Module::getData() {

	return SUCCESS;
}

int PM_Module::checkData() {

	return 0;
}

int PM_Module::sendDataToSharedMemory() {

	return SUCCESS;
}

bool PM_Module::getShutdownFlag() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Shutdown.Flags.ProcessManagement;
}

int PM_Module::setHeartbeat(bool heartbeat) {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	PMData->Heartbeat.Flags.ProcessManagement = heartbeat;

	return SUCCESS;
}

bool PM_Module::getHeartbeat() {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	return PMData->Heartbeat.Flags.ProcessManagement;
}

int PM_Module::checkHeartbeats(__int64 Counter, __int64 prevCounter, __int64 Frequency) {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	
	// Iterate through all modules and verify their responsiveness based on heartbeats
	for (int i = 1; i <= ModuleList->Length; i++) {
		// Module heartbeat should be 1 if responsive
		if ((PMData->Heartbeat.Status >> i) & 1) {
			// Module is responsive, reset heartbeat to 0
			PMData->Heartbeat.Status &= ~(1 << i);
			PMData->LifeCounters[i - 1] = 0; // Reset module lifecounter
		} else {
			// Calculate module down time in seconds
			double downTime = PMData->LifeCounters[i - 1] / Frequency;

			if (downTime >= MaxWait[i - 1]) {
				// Module has been unresponsive for too long
				if (Critical[i - 1]) {
					// If module was critical, shut down all modules
					PMData->Shutdown.Status = 0xFF;
				}
				else if (ProcessList[i-1] != nullptr && ProcessList[i - 1]->HasExited) {
					// If module was non-critical, attempt to restart it
					ProcessList[i - 1]->Start();
					PMData->LifeCounters[i - 1] = 0;
				}
			}

			Console::Write(ModuleList[i-1] + " Module's heartbeat is lost! ");
			std::cout << "LifeCounter:" << PMData->LifeCounters[i - 1] << "Downtime: " << downTime << "s." << "Critical ? : " << (bool)Critical[i - 1] << std::endl;
			
			// Increment module life counter
			PMData->LifeCounters[i - 1] += Counter - prevCounter;
		}
	}

	return SUCCESS;
}

void PM_Module::setShutdown(bool shutdown) {
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;
	PMData->Shutdown.Status = shutdown ? 0xFF : 0x00;
}

PM_Module::~PM_Module() {
	delete ProcessManagementData;
}