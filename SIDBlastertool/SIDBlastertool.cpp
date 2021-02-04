// SIDBlastertool.cpp
// ©2021 by Andreas Schumm for crazy-midi.de

#include <iostream>
//#include "stdafx.h"
#include <windows.h>
#include <inttypes.h>
#include <chrono>

using namespace std;

typedef unsigned char  Uint8;
typedef unsigned short Uint16;
typedef unsigned char  boolean;

typedef Uint8(CALLBACK* lpHardSID_Read)(Uint8 DeviceID, int Cycles, Uint8 SID_reg);
typedef Uint8(CALLBACK* lpReadFromHardSID)(Uint8 DeviceID, Uint8 SID_reg);
typedef int  (CALLBACK* lpHardSID_Version)(void);
typedef int  (CALLBACK* lpHardSID_Devices)(void);
typedef void (CALLBACK* lpHardSID_GetSerial)(Uint8 DevieID, char* output);
typedef void (CALLBACK* lpHardSID_SetSIDInfo)(Uint8 DeviceID, int sidtype);
typedef int (CALLBACK* lpHardSID_GetSIDInfo)(Uint8 DeviceID);

lpHardSID_Read HardSID_Read = NULL;
lpReadFromHardSID HardSID_ReadFromHardSID = NULL;
lpHardSID_Version HardSID_Version = NULL;
lpHardSID_Devices HardSID_Devices = NULL;
lpHardSID_GetSerial HardSID_GetSerial = NULL;
lpHardSID_SetSIDInfo HardSID_SetSIDInfo = NULL;
lpHardSID_GetSIDInfo HardSID_GetSIDInfo = NULL;

HINSTANCE hardsiddll = 0;
BOOL dll_initialized = FALSE;

void list_devices(int No_Of_Dev) {
	cout << endl;
	
	for (Uint8 i = 0; i < No_Of_Dev; i++) {
		char serial[9];
		HardSID_GetSerial((Uint8)i, serial);
		cout << "Device No. " << (int)i << " Serial: " << serial;
		cout << "  SIDType: " << HardSID_GetSIDInfo(i) << endl;
	}
}

void read_test(int No_Of_Dev) {
	cout << endl;
	cout << "SIDBlaster-USB HardSID_Read Test" << endl;
	
	for (Uint8 i = 0; i < No_Of_Dev; i++) {
		for (Uint8 j = 25; j <= 28; j++) {
			Uint8 result = HardSID_Read(i, 0, j);
			cout << "SIDBlaster No. " << (int)i << " Register: " << (int)j << " returns: " << (int)result << endl;
		}
	}
}

void set_the_type(int No_Of_Dev) {
	int Dev_To_Prog = 0;
	int Prog_Type = 0;
	char sure;
	
	list_devices(No_Of_Dev);
	
	cout << endl;
	cout << "which device? (No.)" << endl;
	cin >> Dev_To_Prog;
	cout << "which type? (0 - none; 1 - 6581; 2 - 8580" << endl;
	cin >> Prog_Type;
	cout << "sure? (y/n)" << endl;
	cin >> sure;
	
	if (sure == 'y' && (Prog_Type <= 2 && Prog_Type >= 0) && (Dev_To_Prog < No_Of_Dev)) {
		HardSID_SetSIDInfo(Dev_To_Prog, Prog_Type);
		cout << "*************************************************" << endl;
		cout << " done! exit tool and reconnect SIDBlaster!!!!!! *" << endl;
		cout << "*************************************************" << endl;
	}
	else {
		cout << "ERROR!" << endl;
	}
}

int show_menue(void) {
	int choice = 0;
	cout << endl;
	cout << "Enter:" << endl;
	cout << "1 list sidblasters" << endl;
	cout << "2 read test" << endl;
	cout << "3 set sid type" << endl;
	cout << "9 exit" << endl;
	cin >> choice;
	return choice;
}

int main()
{
	cout << "*** SIDBlastertool 1.0 by A. Schumm for crazy-midi.de" << endl;
	cout << endl;
	hardsiddll = LoadLibrary("hardsid.dll");
	
	if (hardsiddll != 0) {
		cout << "hardsid.dll library loaded!" << endl;
	}
	else {
		cout << "hardsid.dll library failed to load!" << endl;
		return 10;
	}

	HardSID_Read = (lpHardSID_Read)GetProcAddress(hardsiddll, "HardSID_Read");
	HardSID_ReadFromHardSID = (lpReadFromHardSID)GetProcAddress(hardsiddll, "HardSID_ReadFromHardSID");
	HardSID_Version = (lpHardSID_Version)GetProcAddress(hardsiddll, "HardSID_Version");
	HardSID_Devices = (lpHardSID_Devices)GetProcAddress(hardsiddll, "HardSID_Devices");
	HardSID_GetSerial = (lpHardSID_GetSerial)GetProcAddress(hardsiddll, "HardSID_GetSerial");
	HardSID_SetSIDInfo = (lpHardSID_SetSIDInfo)GetProcAddress(hardsiddll, "HardSID_SetSIDInfo");
	HardSID_GetSIDInfo = (lpHardSID_GetSIDInfo)GetProcAddress(hardsiddll, "HardSID_GetSIDInfo");
	
	// check version & device count
	int DLL_Version = (int)HardSID_Version();
	cout << "hardsid.dll version: " << DLL_Version << endl;
	if (DLL_Version < 0x0202) {
		cout << "to old hardsid.dll Version" << endl;
		if (hardsiddll != 0) FreeLibrary(hardsiddll);
		return 9;
	}
	int Number_Of_Devices = (int)HardSID_Devices();
	cout << "Number of devices: " << Number_Of_Devices << endl;
	if ((DLL_Version >= 0x0202) && (Number_Of_Devices > 0)) {
		int choice = 0;
		for (;;) {
			choice = show_menue();
			if (choice == 9) break;
			switch (choice) {
			case 1: list_devices(Number_Of_Devices);
				break;
			case 2:	read_test(Number_Of_Devices);
				break;
			case 3:	set_the_type(Number_Of_Devices);
				break;
			case 9: 
				break;
			}
		}
	}
	else {
		cout << "no Sidblaster(s), or not dll version 0x202 (514) at least!" << endl;
		if (hardsiddll != 0) FreeLibrary(hardsiddll);
		return 8;
	}
	if (hardsiddll != 0) FreeLibrary(hardsiddll);
	return 0;
}