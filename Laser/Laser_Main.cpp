#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <UGV_Module.h>

#include <SMObject.h>
#include <smstructs.h>

#include "Laser.h"

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

	double TimeStamp;
	__int64 Frequency, Counter;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	while (!_kbhit()) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = ((double)Counter / (double)Frequency) * 1000;
		LM->getData();
		LM->sendDataToSharedMemory();
		Console::WriteLine("Laser time stamp: {0, 12:F3}, Shutdown: {1, 12:X2}", TimeStamp, LM->getShutdownFlag());
		Thread::Sleep(25);

		if (LM->getShutdownFlag()) {
			break;
		}
	}

	return 0;
}