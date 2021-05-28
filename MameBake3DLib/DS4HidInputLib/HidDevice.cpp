#include "pch.h"


#include "HidDevice.h"


#pragma comment(lib,"hid.lib")
#pragma comment(lib,"setupapi.lib")

using namespace std;

HidDevice::HidDevice()
{
	isDevice = FALSE;
	productID = 0;
	vendorID = 0;
	deviceHandle = INVALID_HANDLE_VALUE;
	ZeroMemory(&capabilities, sizeof(HIDP_CAPS));
	isCapabilities = false;
	devicePath = 0;
}

HidDevice::~HidDevice()
{
	Destroy();
}

HidDevice HidDevice::Create(char * path, int id)
{
	if (devicePath) {
		free(devicePath);
		devicePath = 0;
	}

	if (!path) {
		_ASSERT(0);
		isDevice = FALSE;
		return *this;//!!!!!!!!
	}

	//パスのコピー
	size_t size = 1;
	//for (UINT i = 0; path[i] != '\0'; i++)
	//{
	//	size += 1;
	//}
	size = strlen(path);
	if ((size <= 0) || (size > 2048)) {
		_ASSERT(0);
		isDevice = FALSE;
		return *this;//!!!!!!!!
	}

	//devicePath = new char[size];
	devicePath = (char*)malloc(sizeof(char) * (size + 1));
	if (!devicePath) {
		_ASSERT(0);
		isDevice = FALSE;
		return *this;//!!!!!!!!
	}
	if (devicePath) {
		ZeroMemory(devicePath, sizeof(char) * (size + 1));
	}
	//for (UINT i = 0; i < size; i++)
	//{
	//	devicePath[i] = path[i];
	//}
	strcpy_s(devicePath, (size + 1), path);

	//デバイスハンドルの作成
	deviceHandle = CreateFile(
		devicePath,
		(GENERIC_READ | GENERIC_WRITE),
		(FILE_SHARE_READ | FILE_SHARE_WRITE),
		NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	HIDD_ATTRIBUTES attributes;
	if (HidD_GetAttributes(deviceHandle, &attributes))
	{
		productID = attributes.ProductID;
		vendorID = attributes.VendorID;
	}

	isCapabilities = false;
	isDevice = 1;

	return *this;
}

HIDP_CAPS HidDevice::GetCapabilities() 
{
	if (isCapabilities == true)
	{
		return capabilities;
	}

	PHIDP_PREPARSED_DATA preparsedData;
	if (HidD_GetPreparsedData(deviceHandle, &preparsedData))
	{
		//基本情報の取得
		if (HIDP_STATUS_SUCCESS == HidP_GetCaps(preparsedData, &capabilities))
		{
			isCapabilities = true;
		}
	}

	HidD_FreePreparsedData(preparsedData);

	return capabilities;
}

void HidDevice::Destroy() 
{
	isDevice = 0;
	if (deviceHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(deviceHandle);
	}
	deviceHandle = INVALID_HANDLE_VALUE;

	//delete[] devicePath;
	if (devicePath) {
		free(devicePath);
		devicePath = 0;
	}
}
