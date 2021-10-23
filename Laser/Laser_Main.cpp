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

	__int64 Frequency, Counter, prevCounter;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
	prevCounter = Counter;

	long int PMDownCycles = 0;

	while (!_kbhit()) {
		prevCounter = Counter;
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);

		LM->getData();
		Console::WriteLine("Got Laser Data");
		LM->sendDataToSharedMemory();

		if (LM->getHeartbeat()) {
			// Get process management down time in seconds
			long int PMLifeTime = PMDownCycles / (double)Frequency;

			if (PMLifeTime >= MAX_PM_WAIT) { // Check if proc. man. has been unresponsive for too long
				break;
			}

			PMDownCycles += Counter - prevCounter;
		} else {
			LM->setHeartbeat(true);
			PMDownCycles = 0;
		}

		Thread::Sleep(25);

		if (LM->getShutdownFlag()) {
			break;
		}
	}

	return 0;
}