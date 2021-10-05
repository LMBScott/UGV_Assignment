#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <UGV_Module.h>

#include <SMObject.h>
#include <smstructs.h>

#include "Laser_module.h"

constexpr char* LASER_IP = "192.168.1.200";
constexpr int LASER_PORT = 23000;

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {
	Laser_module^ LM = gcnew Laser_module;

	String^ IPString = gcnew String(LASER_IP);
	LM->connect(IPString, LASER_PORT);

	double TimeStamp;
	__int64 Frequency, Counter;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	while (!_kbhit()) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = ((double)Counter / (double)Frequency) * 1000;
		Console::WriteLine("Laser time stamp: {0, 12:F3}, Shutdown: {1, 12:X2}", TimeStamp, LM->getShutdownFlag());
		Thread::Sleep(25);
	}

	return 0;
}