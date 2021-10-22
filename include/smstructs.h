#pragma once

#ifndef SMSTRUCTS_H
#define SMSTRUCTS_H

#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


#define STANDARD_LASER_LENGTH 361
#define NUM_MODULES 5

struct SM_Laser
{
	int numPoints;
	double x[STANDARD_LASER_LENGTH];
	double y[STANDARD_LASER_LENGTH];
};

struct SM_VehicleControl
{
	double Speed;
	double Steering;
};

struct SM_GPS
{
	unsigned int Header; // 0xAA 0x44 0x12 0x1C
	unsigned char DiscardLot1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char DiscardLot2[40];
	unsigned int CRC;
};

struct UnitFlags
{
	unsigned char	ProcessManagement : 1,	//NONCRITICAL
					Laser : 1,				//NONCRITICAL
					VehicleControl : 1,		//NONCRITICAL
					GPS : 1,				//NONCRITICAL
					OpenGL : 1,				//NONCRITICAL
					Camera : 1,				//NONCRITICAL
					Garbage : 2;
};

union ExecFlags
{
	UnitFlags Flags;
	unsigned short Status;
};

struct ProcessManagement
{
	ExecFlags Heartbeat;
	ExecFlags Shutdown;
	long int LifeCounter;
	long int LifeCounters[NUM_MODULES];
};

#define NONCRITICALMASK 0xff	//0 011 0000
#define CRITICALMASK 0x0		//0 100 1111
#endif
