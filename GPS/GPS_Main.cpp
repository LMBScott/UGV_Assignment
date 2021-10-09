#using <System.dll>
#include <zmq.hpp>
#include <Windows.h>
#include <conio.h>
#include "GPS.h"
#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {
	// Declaration and Initialisation
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject GPSObj(TEXT("SM_GPS"), sizeof(SM_GPS));

	double TimeStamp;
	__int64 Frequency, Counter;
	int Shutdown = 0x00;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	// SM creation and access
	PMObj.SMCreate();
	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	SM_GPS* GPSData = (SM_GPS*)GPSObj.pData;

	zmq::context_t context(1);
	zmq::socket_t subscriber(context, ZMQ_SUB);

	//Socket to talk to server
	subscriber.connect("tcp://192.168.1.200:24000");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	while (1) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = ((double)Counter / (double)Frequency) * 1000;
		Console::WriteLine("GPS time stamp: {0, 12:F3}, Shutdown: {1, 12:X2}", TimeStamp, Shutdown);
		Thread::Sleep(25);

		if (PMData->Shutdown.Status || _kbhit()) {
			break;
		}
	}

	return 0;
}