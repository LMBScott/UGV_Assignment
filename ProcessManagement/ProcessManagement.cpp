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
	ModuleList = gcnew array<String^> { "Laser", "VehicleControl", "GPS", "Display", "Camera" };
	MaxWait = gcnew array<int>(ModuleList->Length) { 3, 3, 3, 3, 5 };
	Critical = gcnew array<int>(ModuleList->Length) { 1, 1, 0, 1, 0 };
	ProcessList = gcnew array<Process^>(ModuleList->Length);

	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData;

	memset(PMData->LifeCounters, 0, sizeof(PMData->LifeCounters)); // Initialise LifeCounters to 0

	return SUCCESS;
}

int PM_Module::startProcesses() {
	for (int i = 0; i < ModuleList->Length; i++) {
		if (Process::GetProcessesByName(ModuleList[i])->Length == 0) { // If there are no current instances of process
			ProcessList[i] = gcnew Process;
			ProcessList[i]->StartInfo->WorkingDirectory = HOME_WD;
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

	for (int i = 1; i <= ModuleList->Length; i++) {
		if ((PMData->Heartbeat.Status >> i) & 1) {
			PMData->Heartbeat.Status &= ~(1 << i);
			PMData->LifeCounters[i - 1] = 0;
		}
		else {
			double downTime = PMData->LifeCounters[i - 1] / Frequency;

			if (downTime >= MaxWait[i - 1]) {
				if (Critical[i - 1]) {
					PMData->Shutdown.Status = 0xFF;
				}
				else if (ProcessList[i - 1]->HasExited) {
					//ProcessList[i-1]->Kill();
					ProcessList[i - 1]->Start();
					PMData->LifeCounters[i - 1] = 0;
				}
			}

			Console::Write(ModuleList[i-1] + " Module's heartbeat is lost! ");
			std::cout << "LifeCounter:" << PMData->LifeCounters[i - 1] << "Downtime: " << downTime << "s." << "Critical ? : " << (bool)Critical[i - 1] << std::endl;

			PMData->LifeCounters[i - 1] += Counter - prevCounter;
		}
	}

	return SUCCESS;
}

PM_Module::~PM_Module() {
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
}