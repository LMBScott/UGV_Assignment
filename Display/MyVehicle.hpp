#ifndef __MYVEHICLE_HPP__
#define __MYVEHICLE_HPP__


#include "Vehicle.hpp"
#include "smstructs.h"

class MyVehicle : public Vehicle
{
public:
	MyVehicle(SM_Laser* LMData, SM_GPS* GPSData);
	virtual void draw();
protected:
	void drawLaserScans();
	SM_Laser* LMData;
	SM_GPS* GPSData;
};

#endif