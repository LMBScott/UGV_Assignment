#pragma once

#include <UGV_module.h>

#define HOME_WD "C:\\Users\\Lachy\\Documents\\Uni\\MTRN3500\\UGV_Assignment\\Executables"
#define REMOTE_WD "C:\\Users\\z5207471\\source\\repos\\UGV_Assignment\\Executables"

ref class PM_Module : public UGV_module {
public:
	int connect(String^ hostName, int portNumber) override;	// Establish TCP connection
	int setupSharedMemory() override;						// Create and access shared memory objects
	int setupDataStructures();
	int startProcesses();
	int getData() override;									// Get data from sensor (GPS / Laser)
	int checkData() override;								// Check Data is correct (eg headers)
	int sendDataToSharedMemory() override;					// Save Data in shared memory structures
	bool getShutdownFlag() override;						// Get Shutdown signal for module, from Process Management SM
	int setHeartbeat(bool heartbeat) override;				// Update heartbeat signal for module
	bool getHeartbeat() override;							// Get heartbeat signal for module
	int checkHeartbeats(__int64 Counter, __int64 prevCounter, __int64 Frequency);
	~PM_Module();
protected:
	array<String^>^ ModuleList;
	array<int>^ MaxWait;
	array<int>^ Critical;
	array<System::Diagnostics::Process^>^ ProcessList;
};
