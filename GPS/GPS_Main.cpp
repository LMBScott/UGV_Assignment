#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include "GPS.hpp"
#include <SMObject.h>
#include <smstructs.h>
#include <bitset>

constexpr char* GPS_IP = "192.168.1.200";
constexpr int GPS_PORT = 24000;

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {
	GPS^ GM = gcnew GPS;
	Console::WriteLine("Set up GPS Module instance.");

	GM->setupSharedMemory();

	Console::WriteLine("Set up GPS Module shared memory.");

	String^ IPString = gcnew String(GPS_IP);
	GM->connect(IPString, GPS_PORT);

	Console::WriteLine("Connected to GPS Server.");

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
		
		if (GM->getData() == SUCCESS) {
			// Only attempt to process data if data was received
			if (GM->checkData()) {
				// Verify that data is in valid format and expected quantity
				Console::WriteLine("Data received, CRC values matched, sending data to shared memory.");
				GM->sendDataToSharedMemory();
			}
			else {
				Console::WriteLine("CRC values did not match.");
			}
		} else {
			Console::WriteLine("Failed to get data from GPS module.");
		}

		if (GM->getHeartbeat()) { // Check if heartbeat bit has not been return to 0 by PM module
			// Get process management down time in seconds
			long int PMLifeTime = PMDownCycles / (double)Frequency;

			if (PMLifeTime >= MAX_PM_WAIT) { // Check if proc. man. has been unresponsive for too long
				break;
			}
			
			// Track PM module down time in performance counter ticks
			PMDownCycles += Counter - prevCounter;
		}
		else {
			// Process management is operating normally, set heartbeat bit to 1
			GM->setHeartbeat(true);
			PMDownCycles = 0; // Reset PM module down cycle counter
		}

		Thread::Sleep(25);

		if (GM->getShutdownFlag()) { // Check if PM module has instructed module to shut down
			break;
		}
	}

	return 0;
}