#using <System.dll>
#include <conio.h>

#include <SMObject.h>
#include <smstructs.h>
#include <bitset>
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

#define HOME_WD "C:\\Users\\z5207471\\source\\repos\\UGV_Assignment\\Executables"
#define REMOTE_WD "C:\\Users\\Lachy\\Documents\\Uni\\MTRN3500\\UGV_Assignment\\Executables"

int main() {
    // Declaration and Initialisation
    SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
    array<String^>^ ModuleList = gcnew array<String^> { "Laser", "VehicleControl", "GPS", "Display", "Camera" };
    const array<int>^ MaxWait = gcnew array<int>(ModuleList->Length) { 3, 3, 3, 3, 5 };
    const array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 1, 1, 0, 1, 0 };
    array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);

    // SM creation and access
    PMObj.SMCreate();
    PMObj.SMAccess();

    ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

    memset(PMData->LifeCounters, 0, sizeof(PMData->LifeCounters)); // Initialise LifeCounters to 0

    for (int i = 0; i < ModuleList->Length; i++) {
        if (Process::GetProcessesByName(ModuleList[i])->Length == 0) { // If there are no current instances of process
            ProcessList[i] = gcnew Process;
            ProcessList[i]->StartInfo->WorkingDirectory = HOME_WD;
            ProcessList[i]->StartInfo->FileName = ModuleList[i];
            ProcessList[i]->Start();
            Console::WriteLine("Started process for module: " + ModuleList[i]);
        }
    }

    std::bitset<8> statusBits(PMData->Heartbeat.Status);

    __int64 Frequency, Counter, prevCounter;
    QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
    QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
    prevCounter = Counter;

    // Main Loop
    while (!_kbhit() && PMData->Shutdown.Status != 0xFF) {
        prevCounter = Counter;
        QueryPerformanceCounter((LARGE_INTEGER*)&Counter);

        for (int i = 1; i <= ModuleList->Length; i++) {
            if ((PMData->Heartbeat.Status >> i) & 1) {
                PMData->Heartbeat.Status &= ~(1 << i);
                PMData->LifeCounters[i-1] = 0;
            } else {
                double downTime = PMData->LifeCounters[i-1] / Frequency;

                if (downTime >= MaxWait[i-1]) {
                    if (Critical[i-1]) {
                        PMData->Shutdown.Status = 0xFF;
                    } else if (ProcessList[i-1]->HasExited) {
                        //ProcessList[i-1]->Kill();
                        ProcessList[i-1]->Start();
                        PMData->LifeCounters[i - 1] = 0;
                    }
                }

                std::cout << "Module " << (i - 1) << "'s heartbeat is lost! LifeCounter:" << PMData->LifeCounters[i-1] << "Downtime: " << downTime << "s." << "Critical ? : " << (bool)Critical[i - 1] << std::endl;

                PMData->LifeCounters[i-1] += Counter - prevCounter;
            }
        }
        Thread::Sleep(100);
    }

    return 0;
}