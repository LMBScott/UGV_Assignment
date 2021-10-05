#using <System.dll>
#include <Windows.h>
#include <conio.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {
	// Declaration and Initialisation
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));

	double TimeStamp;
	__int64 Frequency, Counter;
	int Shutdown = 0x00;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	// SM creation and access
	PMObj.SMCreate();
	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	while (1) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = ((double)Counter / (double)Frequency) * 1000;
		Console::WriteLine("VC time stamp: {0, 12:F3}, Shutdown: {1, 12:X2}", TimeStamp, Shutdown);
		Thread::Sleep(25);

		if (PMData->Shutdown.Status || _kbhit()) {
			break;
		}
	}

	return 0;
}