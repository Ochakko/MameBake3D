#include "pch.h"


#include "DSManager.h"
#include "DS4Device.h"
#include "DSenseDevice.h"

//#include <rpc.h>
//#pragma comment ( lib, "Rpcrt4.lib" )


DSManager::DSManager()
{
	for (int i = 0; i < 4; i++) {
		dsDevice[i] = 0;
	}
	initflag = true;
}


//BOOL GuidFromString(GUID* pGuid, std::string oGuidString)
//{
//	// �������GUID�ɕϊ�����
//	if (RPC_S_OK == ::UuidFromString((RPC_CSTR)oGuidString.c_str(), (UUID*)pGuid)) {
//
//		// �ϊ��ł��܂����B
//		return(TRUE);
//	}
//	return(FALSE);
//}

bool DSManager::GetDevice()
{
	bool foundflag = false;

	GUID hidGuid;
	HDEVINFO hdevInfo;


	HidD_GetHidGuid(&hidGuid);

	//hidGuid = GUID("000011240000-10008-8000-00805F9B34FB");
	//std::string strguid = "000011240000-10008-8000-00805F9B34FB";//HID�Ή���bluetooth�f�o�C�X
	//GuidFromString(&hidGuid, strguid);

	hdevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE));
	SP_DEVICE_INTERFACE_DATA data;
	data.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
	for (int i = 0; SetupDiEnumDeviceInterfaces(hdevInfo, NULL, &hidGuid, i, &data); i++)
	{
		PSP_INTERFACE_DEVICE_DETAIL_DATA detail = 0;
		DWORD size = 0;

		//�f�o�C�X�C���^�t�F�[�X�ڍ׏��̃������T�C�Y���擾
		SetupDiGetDeviceInterfaceDetail(hdevInfo, &data, NULL, 0, &size, NULL);

		if (size > 0) {//size�����̏ꍇ��if���ǉ��@2021/03/25 add OchakkoLAB

			//�f�o�C�X�C���^�[�t�F�[�X�ڍ׏��̗̈���m�ۂ���
			//detail = new SP_INTERFACE_DEVICE_DETAIL_DATA[size];//�N���X�ł͂Ȃ��\���̂Ȃ̂�malloc�ɕύX
			detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA) * size);
			if (detail)
			{
				ZeroMemory(detail, sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA) * size);

				DWORD len = 0;
				memset(detail, 0, size);

				//�f�o�C�X�C���^�[�t�F�[�X�̏ڍ׏���Ǎ���
				detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
				if (SetupDiGetInterfaceDeviceDetail(hdevInfo, &data, detail, size, &len, NULL))
				{
					//Hid�f�o�C�X�̍쐬
					HidDevice device = device.Create(detail->DevicePath, 0);

					if (device.GetVendorID() == 0x54C && device.GetProductID() == 0xce6) {
						//	PS5�R���g���[���[
						for (int i = 0; i < 4; i++) {
							if (!dsDevice[i]) {
								dsDevice[i] = new DSenseDevice(device, i);
								foundflag = true;
								break;
							}
						}
					}
					else if (device.GetVendorID() == 0x54c && (device.GetProductID() == 0x5c4 || device.GetProductID() == 0x9CC))
					{
						for (int i = 0; i < 4; i++) {
							if (!dsDevice[i]) {
								//PS4�R���g���[���[�Ƃ��Đݒ�
								dsDevice[i] = new DS4Device(device, i);
								foundflag = true;
								break;
							}
						}
					}
					else
					{
						//�f�o�C�X�̔j��
						device.Destroy();
					}
				}

			}
			if (detail) {
				delete[] detail;
			}
		}
	}
	SetupDestroyDiskSpaceList(hdevInfo);
	return foundflag;
}

bool DSManager::IsDevice(int id)
{
	if (!dsDevice[id]) return false;
	bool device = dsDevice[id]->IsDSDevice();
	if (!device)
	{
		dsDevice[id]->Destroy();
		delete dsDevice[id];
		dsDevice[id] = nullptr;
	}
	return device;
}

void DSManager::ChangeColor(int id, const UCHAR r, const UCHAR g, const UCHAR b)
{
	if (!IsDevice(id)) return;
	dsDevice[id]->ChangeLedColor(LED(r, g, b));
}

void DSManager::ChangePlayerLight(int id, const UCHAR val)
{
	if (!IsDevice(id)) return;
	dsDevice[id]->ChangePlayerLight(val);
}

void DSManager::ChangeVibration(int id, UCHAR right, UCHAR left)
{
	if (!IsDevice(id)) return;
	dsDevice[id]->ChangeVibration(right, left);
}

void DSManager::ChangeTriggerLock(int id, UCHAR rMode, UCHAR right, UCHAR lMode, UCHAR left)
{
	if (!IsDevice(id)) return;
	dsDevice[id]->ChangeTriggerLock(rMode, right, lMode, left);
}

void DSManager::SendOutput(int id)
{
	if (!IsDevice(id)) return;
	dsDevice[id]->SendOutputReport();
}

void DSManager::InputUpdate()
{
	for (int i = 0; i < 4; i++)
	{
		if (dsDevice[i] == nullptr) continue;
		if (dsDevice[i]->IsDSDevice() == false) { continue; }
		dsDevice[i]->GetInputReport();
	}
}

bool DSManager::GetButton(int id, UCHAR key)
{
	if (IsDevice(id) == false) return false;
	return dsDevice[id]->GetButton(key);
}

bool DSManager::GetButtonDown(int id, UCHAR key)
{
	if (IsDevice(id) == false) return false;
	return dsDevice[id]->GetButtonDown(key);
}

bool DSManager::GetButtonUp(int id, UCHAR key)
{
	if (IsDevice(id) == false) return false;
	return dsDevice[id]->GetButtonUp(key);
}

float DSManager::GetAxis(int id, UCHAR axis)
{
	if (IsDevice(id) == false) return 0;
	return dsDevice[id]->GetAxis(axis);
}

void DSManager::Destroy()
{
	for (int i = 0; i < 4; i++)
	{
		if (dsDevice[i]) {
			dsDevice[i]->Destroy();
			delete dsDevice[i];//20210325 add
			dsDevice[i] = nullptr;
		}
	}
}
