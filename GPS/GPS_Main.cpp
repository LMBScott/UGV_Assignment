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

	//String^ IPString = gcnew String(GPS_IP);
	//GM->connect(IPString, GPS_PORT);

	//Console::WriteLine("Connected to GPS Server.");

	double TimeStamp;
	__int64 Frequency, Counter, prevCounter;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
	prevCounter = Counter;

	long int PMDownCycles = 0;

	while (!_kbhit()) {
		prevCounter = Counter;
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);

		TimeStamp = ((double)Counter / (double)Frequency) * 1000;

		//GM->getData();
		//if (GM->checkData()) {
		//	GM->sendDataToSharedMemory();
		//}

		if (GM->getHeartbeat()) {
			// Get process management down time in seconds
			long int PMLifeTime = PMDownCycles / (double)Frequency;

			if (PMLifeTime >= MAX_PM_WAIT) { // Check if proc. man. has been unresponsive for too long
				break;
			}

			PMDownCycles += Counter - prevCounter;
		}
		else {
			GM->setHeartbeat(true);
			PMDownCycles = 0;
		}
		Console::WriteLine("GPS time stamp: {0, 12:F3}, Shutdown: {1, 12:X2}", TimeStamp, GM->getShutdownFlag());
		Thread::Sleep(25);

		if (GM->getShutdownFlag()) {
			break;
		}
	}

	return 0;
}