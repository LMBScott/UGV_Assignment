#pragma once

#include <SMObject.h>
#include <smstructs.h>
#include <UGV_module.h>

ref class Laser : public UGV_module {
public:
	int connect(String^ hostName, int portNumber) override;	// Establish TCP connection
	int setupSharedMemory() override;						// Create and access shared memory objects
	int getData() override;									// Get data from sensor (GPS / Laser)
	int checkData() override;								// Check Data is correct (eg headers)
	int sendDataToSharedMemory() override;					// Save Data in shared memory structures
	bool getShutdownFlag() override;						// Get Shutdown signal for module, from Process Management SM
	int setHeartbeat(bool heartbeat) override;				// Update heartbeat signal for module
	bool getHeartbeat() override;							// Get heartbeat signal for module
	~Laser();
protected:
	int numPointsRead = 0;
};
