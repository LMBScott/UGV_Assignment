#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <UGV_Module.h>

#include <SMObject.h>
#include <smstructs.h>

#include "VC.hpp"

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

constexpr char* VC_IP = "192.168.1.200";
constexpr int VC_PORT = 25000;

int main() {
	Console::WriteLine("Starting.");
	VehicleControl^ VC = gcnew VehicleControl;
	Console::WriteLine("Set up Vehicle Control Module instance.");

	VC->setupSharedMemory();

	Console::WriteLine("Set up Vehicle Control Module shared memory.");

	String^ IPString = gcnew String(VC_IP);
	VC->connect(IPString, VC_PORT);

	Console::WriteLine("Connected to VC Server.");

	__int64 Frequency, Counter, prevCounter;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
	prevCounter = Counter;

	long int PMDownCycles = 0;

	while (!_kbhit()) {
		prevCounter = Counter;
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		
		VC->sendSteeringData();

		if (VC->getHeartbeat()) {
			Console::WriteLine("Lost PM Heartbeat!");
			// Get process management down time in seconds
			long int PMLifeTime = PMDownCycles / (double)Frequency;

			if (PMLifeTime >= MAX_PM_WAIT) { // Check if proc. man. has been unresponsive for too long
				break;
			}

			PMDownCycles += Counter - prevCounter;
		}
		else {
			VC->setHeartbeat(true);
			PMDownCycles = 0;
		}

		if (VC->getShutdownFlag()) {
			break;
		}

		Thread::Sleep(50);
	}
	return 0;
}