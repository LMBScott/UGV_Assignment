#pragma once

#include <UGV_module.h>
#include <smstructs.h>

ref class Camera : public UGV_module
{
public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int setHeartbeat(bool heartbeat) override;
	bool getHeartbeat() override;
	~Camera();
};
