#pragma once

#include<Windows.h>
#include<SetupAPI.h>
#include<bluetoothapis.h>
#include<iostream>

extern "C"
{
#include <hidsdi.h>
}

#pragma comment(lib,"hid.lib")
#pragma comment(lib,"setupapi.lib")

using namespace std;

class HidDevice
{
public:
	HidDevice Create(char* path, int id);
	HIDP_CAPS GetCapabilities() ;
	void Destroy() ;
	BOOL isDevice;
	inline HANDLE GetHandle() 
	{
		return deviceHandle;
	};
	inline USHORT GetProductID() 
	{
		return productID;
	};
	inline USHORT GetVendorID() 
	{
		return vendorID;
	};
	inline char* GetDevicePath() 
	{
		return devicePath;
	};

private:
	USHORT productID;
	USHORT vendorID;
	HANDLE deviceHandle;
	HIDP_CAPS capabilities;
	bool isCapabilities;
	char* devicePath;

};