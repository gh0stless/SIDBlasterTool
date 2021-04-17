// SIDBlastertool.cpp
// ©2021 by Andreas Schumm for crazy-midi.de
// 2021-04-17 v1.3

#include <iostream>
#include <cstring>
#include <string>
#include <regex>
#include <chrono>
#include <thread>


#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#endif

#include <inttypes.h>
#include <chrono>

#if defined(__linux) || defined(__APPLE__)
#include "hardsid.hpp"
#endif

using namespace std;

typedef unsigned char  Uint8;
typedef unsigned short Uint16;
typedef unsigned char  boolean;

enum SID_TYPE {
	SID_TYPE_NONE = 0, SID_TYPE_6581, SID_TYPE_8580
};

#if defined(_WIN64) || defined(_WIN32) 
typedef Uint8(CALLBACK* lpHardSID_Read)(Uint8 DeviceID, int Cycles, Uint8 SID_reg);
typedef Uint8(CALLBACK* lpReadFromHardSID)(Uint8 DeviceID, Uint8 SID_reg);
typedef int  (CALLBACK* lpHardSID_Version)(void);
typedef int  (CALLBACK* lpHardSID_Devices)(void);
typedef void (CALLBACK* lpHardSID_GetSerial)(char* output, int buffersize, Uint8 DeviceID);
typedef int  (CALLBACK* lpHardSID_SetSIDType)(Uint8 DeviceID, int sidtype);
typedef int  (CALLBACK* lpHardSID_GetSIDType)(Uint8 DeviceID);
typedef int  (CALLBACK* lpHardSID_SetSerial)(Uint8 DeviceID, const char *SerialNo);

lpHardSID_Read HardSID_Read = NULL;
lpReadFromHardSID HardSID_ReadFromHardSID = NULL;
lpHardSID_Version HardSID_Version = NULL;
lpHardSID_Devices HardSID_Devices = NULL;
lpHardSID_GetSerial HardSID_GetSerial = NULL;
lpHardSID_SetSIDType HardSID_SetSIDType = NULL;
lpHardSID_GetSIDType HardSID_GetSIDType = NULL;
lpHardSID_SetSerial HardSID_SetSerial = NULL;

HINSTANCE hardsiddll = 0;

#endif

boolean dll_initialized = false;

void list_devices(int No_Of_Dev) {
	cout << endl;
	
	for (Uint8 i = 0; i < No_Of_Dev; i++) {
		char serial[9];
        HardSID_GetSerial(serial, 9,(Uint8)i);
		cout << "Device No. " << (int)i << " Serial: " << serial;
		cout << "  SIDType: " << HardSID_GetSIDType(i);
		switch (HardSID_GetSIDType(i)){
			case 0: cout << " none" << endl;
				break;
			case 1: cout << " 6581" << endl;
				break;
			case 2: cout << " 8580" << endl;
				break;
		}
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
    int error_code = 0;
	
	list_devices(No_Of_Dev);
	
	cout << endl;
	cout << "which device? (No.)" << endl;
	cin >> Dev_To_Prog;
	cout << "which type? (0 - none; 1 - 6581; 2 - 8580" << endl;
	cin >> Prog_Type;
	cout << "sure? (y/n)" << endl;
	cin >> sure;
	
	if (sure == 'y' && (Prog_Type <= 2 && Prog_Type >= 0) && (Dev_To_Prog< No_Of_Dev)) {
		error_code = HardSID_SetSIDType(Dev_To_Prog, SID_TYPE(Prog_Type));
        
        if (error_code){
            cout << "function reports error!" << endl;
        }
        else{
            cout << "well, done!" << endl;
        }
		cout << endl;
		cout << "*************************************************" << endl;
		cout << " done! exit tool and reconnect SIDBlaster!!!!!! *" << endl;
		cout << "*************************************************" << endl;
        
	}
	else {
		cout << "cancel..." << endl;
	}
}

void set_the_serial(int No_Of_Dev) {
    int Dev_To_Prog = 0;
    char new_serial[9];
    char sure;
    int error_code = 0;
	string s_new_serial;

    list_devices(No_Of_Dev);
    
    cout << endl;
    cout << "which device? (No.)" << endl;
    cin >> Dev_To_Prog;
    cout << "enter serial (8 digits, capital letters or numbers):" << endl;
    cin >> s_new_serial;
	
	std::string regExprStr("([A-Z]|\\d){8}"); // regular expression
	std::regex rgx(regExprStr); // regular expression holder
	std::smatch smatch; // search result holder
	if (std::regex_match(s_new_serial, smatch, rgx)) {
		#pragma warning(suppress : 4996)
		std::strcpy(new_serial, s_new_serial.c_str());

		cout << "sure? (y/n)" << endl;
		cin >> sure;

		if (sure == 'y' && (Dev_To_Prog < No_Of_Dev)) {
			error_code = HardSID_SetSerial(Dev_To_Prog, new_serial);

			if (error_code) {
				cout << "function reports error!" << endl;
			}
			else {
				cout << "well, done!" << endl;
			}
			cout << endl;
			cout << "*************************************************" << endl;
			cout << " done! exit tool and reconnect SIDBlaster!!!!!! *" << endl;
			cout << "*************************************************" << endl;
		}
		else {
			cout << "cancel..." << endl;
		}
	}
	else {
		cout << "invalid serial" << endl;
	}

}

void windows_fix(int No_Of_Dev) {
	int Dev_To_Prog = 0;
	int Prog_Type = 0;
	char sure;
	int error_code = 0;

	list_devices(No_Of_Dev);

	cout << endl;
	cout << "only necessary if the serial number or chip type was written under linux or macos" << endl;
	cout << "and the sidblaster is to be used under windows again." << endl;
	
	cout << "sure? (y/n)" << endl;
	cin >> sure;
	if (sure != 'y') return;

	cout << "which device? (No.)" << endl;
	cin >> Dev_To_Prog;
	
	cout << "sure? (y/n)" << endl;
	cin >> sure;

	if ((sure == 'y') && (Dev_To_Prog < No_Of_Dev)) {
		
		
		Prog_Type = HardSID_GetSIDType(Dev_To_Prog);

		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		
		error_code = HardSID_SetSIDType(Dev_To_Prog, SID_TYPE(Prog_Type));

		if (error_code) {
			cout << "function reports error!" << endl;
		}
		else {
			cout << "well, done!" << endl;
		}
		cout << endl;
		cout << "*************************************************" << endl;
		cout << " done! exit tool and reconnect SIDBlaster!!!!!! *" << endl;
		cout << "*************************************************" << endl;

	}
	else {
		cout << "cancel..." << endl;
	}

}

int show_menue(void) {
	int choice = 0;
	cout << endl;
	cout << "enter:" << endl;
	cout << "1 list sidblasters" << endl;
	cout << "2 read test" << endl;
	cout << "3 set sid type" << endl;
    cout << "4 set serial" << endl;
	cout << "5 windows driver fix" << endl;
    cout << "9 exit" << endl;
	cin >> choice;
	if (!(choice >= 1 && choice <= 9)) choice = 9;
	return choice;
}



int main(int argc, const char * argv[]) {

	cout << "*** SIDBlasterTool 1.3 by A. Schumm for crazy-midi.de" << endl;
	cout << endl;
	
#if defined(_WIN64) || defined(_WIN32) 
	hardsiddll = LoadLibrary("hardsid.dll");
	
	if (hardsiddll != 0) {
		cout << "hardsid library loaded!" << endl;
	}
	else {
		cout << "hardsid library failed to load!" << endl;
		return 10;
	}

	HardSID_Read = (lpHardSID_Read)GetProcAddress(hardsiddll, "HardSID_Read");
	HardSID_ReadFromHardSID = (lpReadFromHardSID)GetProcAddress(hardsiddll, "HardSID_ReadFromHardSID");
	HardSID_Version = (lpHardSID_Version)GetProcAddress(hardsiddll, "HardSID_Version");
	HardSID_Devices = (lpHardSID_Devices)GetProcAddress(hardsiddll, "HardSID_Devices");
	HardSID_GetSerial = (lpHardSID_GetSerial)GetProcAddress(hardsiddll, "HardSID_GetSerial");
	HardSID_SetSIDType = (lpHardSID_SetSIDType)GetProcAddress(hardsiddll, "HardSID_SetSIDType");
	HardSID_GetSIDType = (lpHardSID_GetSIDType)GetProcAddress(hardsiddll, "HardSID_GetSIDType");
    HardSID_SetSerial = (lpHardSID_SetSerial)GetProcAddress(hardsiddll, "HardSID_SetSerial");
#endif
	
	// check version & device count
	int DLL_Version = (int)HardSID_Version();
	cout << "hardsid library version: " << DLL_Version << endl;
#if defined(_WIN64) || defined(_WIN32) 
	if (DLL_Version < 0x0203) {
		cout << "to old hardsid library Version" << endl;
		cout << "version 0x203 (515) at least!" << endl;
		if (hardsiddll != 0) FreeLibrary(hardsiddll);
		return 9;
	}
#endif
	
	int Number_Of_Devices = (int)HardSID_Devices();
	cout << "Number of devices: " << Number_Of_Devices << endl;
	if ((DLL_Version >= 0x0203) && (Number_Of_Devices > 0)) {
		
		for (;;) {
			int choice = 0;
			choice = show_menue();
			if ((choice == 9) || (choice > 5)) break;
			switch (choice) {
			case 1: list_devices(Number_Of_Devices);
				break;
			case 2:	read_test(Number_Of_Devices);
				break;
			case 3:	set_the_type(Number_Of_Devices);
				break;
            case 4: set_the_serial(Number_Of_Devices);
                    break;
			case 5: windows_fix(Number_Of_Devices);
			default:
				break;
			}
		}
		cout << "exit..." << endl;
	}
	else {
		cout << "no Sidblaster(s), or not library version 0x203 (515) at least!" << endl;
#if defined(_WIN64) || defined(_WIN32) 
		if (hardsiddll != 0) FreeLibrary(hardsiddll);
#endif
		return 8;
	}
#if defined(_WIN64) || defined(_WIN32) 
	if (hardsiddll != 0) FreeLibrary(hardsiddll);
#else
    HardSID_Uninitialize();
#endif
	return 0;
}
