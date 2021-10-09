#using <System.dll>
#include <conio.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {
    // Declaration and Initialisation
    SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
    array<String^>^ ModuleList = gcnew array<String^> { "Laser", "Display", "VehicleControl", "GPS", "Camera" };
    array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 1, 1, 1, 0, 0 };
    array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);

    // SM creation and access
    PMObj.SMCreate();
    PMObj.SMAccess();

    ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

    for (int i = 0; i < ModuleList->Length; i++) {
        if (Process::GetProcessesByName(ModuleList[i])->Length == 0) { // If there are no current instances of process
            ProcessList[i] = gcnew Process;
            ProcessList[i]->StartInfo->WorkingDirectory = "C:\\Users\\z5207471\\source\\repos\\UGV_Assignment\\Executables";
            ProcessList[i]->StartInfo->FileName = ModuleList[i];
            ProcessList[i]->Start();
            Console::WriteLine("Started process for module: " + ModuleList[i]);
        }
    }

    // Main Loop
    while (!_kbhit()) {
        // Check for heartbeats
            // Iterate through processes
                // Is the heartbeat bit of process[i] == 1?
                    // True -> flip bit to 0
                    // False -> increment heartbeat lost duration counter
                        // Has the counter exceeded the pre-defined limit for the process?
                            // True -> Is process[i] critical?
                                // True -> Shutdown all processes
                                // False -> Has process[i] exited? (HasExited())
                                    // True -> Start()
                                    // False -> Kill(), Start()
        for (int i = 2; i < 8; i++) { // Iterate through bits of heartbeat ExecFlags
            unsigned short beat = (PMData->Heartbeat.Status >> i) & 1U; // Get ith bit of heartbeat int
            if (beat) { // If heartbeat is 1
                PMData->Heartbeat.Status |= 0UL << i; // Set heartbeat bit to 0
            }
            else {
                // False -> increment heartbeat lost duration counter
                        // Has the counter exceeded the pre-defined limit for the process?
                            // True -> Is process[i] critical?
                                // True -> Shutdown all processes
                                // False -> Has process[i] exited? (HasExited())
                                    // True -> Start()
                                    // False -> Kill(), Start()
            }
        }
        Thread::Sleep(1000);
    }

    PMData->Shutdown.Status = 0xFF;
    
    // Clearing and Shutdown
    Console::ReadKey();

    return 0;
}