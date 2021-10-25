#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <UGV_Module.h>

#include <SMObject.h>
#include <smstructs.h>

#include "Laser.hpp"

constexpr char* LASER_IP = "192.168.1.200";
constexpr int LASER_PORT = 23000;

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {
	Laser^ LM = gcnew Laser;
	Console::WriteLine("Set up Laser Module instance.");

	LM->setupSharedMemory();

	Console::WriteLine("Set up Laser Module shared memory.");

	String^ IPString = gcnew String(LASER_IP);
	LM->connect(IPString, LASER_PORT);

	Console::WriteLine("Connected to Laser Server.");

	// Set up timing parameters
	__int64 Frequency, Counter, prevCounter;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
	prevCounter = Counter;

	long int PMDownCycles = 0;

	while (!_kbhit()) {
		// Keep track of previous iteration's counter value to enable calculation of the time delta
		prevCounter = Counter;
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		
		// Only attempt to process data if data was received
		if (LM->getData() == SUCCESS) {
			LM->sendDataToSharedMemory();
		} else {
			Console::WriteLine("Failed to get data from Laser module.");
		}

		if (LM->getHeartbeat()) { // Check if heartbeat bit has not been return to 0 by PM module
			// Get process management down time in seconds
			long int PMLifeTime = PMDownCycles / (double)Frequency;

			if (PMLifeTime >= MAX_PM_WAIT) { // Check if proc. man. has been unresponsive for too long
				break;
			}
			
			// Track PM module down time in performance counter ticks
			PMDownCycles += Counter - prevCounter;
		} else {
			// Process management is operating normally, set heartbeat bit to 1
			LM->setHeartbeat(true);
			PMDownCycles = 0; // Reset PM module down cycle counter
		}

		Thread::Sleep(25);

		if (LM->getShutdownFlag()) { // Check if PM module has instructed module to shut down
			break;
		}
	}

	return 0;
}