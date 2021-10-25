#using <System.dll>
#include <conio.h>

#include <SMObject.h>
#include <smstructs.h>
#include <bitset>
#include "ProcessManagement.hpp"

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {
    PM_Module^ PM = gcnew PM_Module;

    PM->setupSharedMemory();
    PM->setupDataStructures();
    PM->startProcesses();
    
    // Set up timing parameters
    __int64 Frequency, Counter, prevCounter;
    QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
    QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
    prevCounter = Counter;

    // Main Loop
    while (!_kbhit() && !PM->getShutdownFlag()) {
        // Keep track of previous iteration's counter value to enable calculation of the time delta
        prevCounter = Counter;
        QueryPerformanceCounter((LARGE_INTEGER*)&Counter);

        PM->checkHeartbeats(Counter, prevCounter, Frequency);
        
        Thread::Sleep(100);
    }
    
    // PM process ended, shutdown all other modules
    PM->setShutdown(true);

    return 0;
}