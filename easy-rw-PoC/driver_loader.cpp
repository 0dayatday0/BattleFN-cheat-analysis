#include <Windows.h>
#include <string>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

extern "C" NTSTATUS RtlAdjustPrivilege
(
	ULONG    Privilege,
	BOOLEAN  Enable,
	BOOLEAN  CurrentThread,
	PBOOLEAN Enabled
);

extern "C" NTSTATUS ZwLoadDriver(
	PUNICODE_STRING DriverServiceName
);

#define SeLoadDriverPrivilege 10ull

bool acquire_privilege(DWORD privilege)
{
	BOOLEAN enabled = FALSE;
	return NT_SUCCESS(RtlAdjustPrivilege(privilege, TRUE, FALSE, &enabled)) || enabled;
}

bool add_service_registry_key(const wchar_t* driver_name, const wchar_t* driver_path)
{
	std::wstring registry_path = std::wstring(L"System\\CurrentControlSet\\Services\\") + driver_name;

	HKEY key;
	if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, registry_path.c_str(), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, 0) != ERROR_SUCCESS)
		return false;
	
	if (RegSetValueExW(key, L"ImagePath", 0, REG_EXPAND_SZ, (PBYTE)driver_path, (DWORD)(wcslen(driver_path) * sizeof(wchar_t))) != ERROR_SUCCESS)
	{
		RegCloseKey(key);
		return false;
	}

	DWORD data = 1;
	if (RegSetValueExW(key, L"Type", 0, REG_DWORD, (PBYTE)&data, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		RegCloseKey(key);
		return false;
	}

	if (RegSetValueExW(key, L"ErrorControl", 0, REG_DWORD, (PBYTE)&data, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		RegCloseKey(key);
		return false;
	}

	data = 3;
	if (RegSetValueExW(key, L"Start", 0, REG_DWORD, (PBYTE)&data, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		RegCloseKey(key);
		return false;
	}

	RegCloseKey(key);
	return true;
}

bool remove_service_registry_key(const wchar_t* driver_name)
{
	std::wstring service_path = std::wstring(L"System\\CurrentControlSet\\Services\\") + driver_name;

	LSTATUS status = RegDeleteKeyW(HKEY_LOCAL_MACHINE,
		service_path.c_str());

	if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND)
		return true;

	return false;
}


bool load_driver(const wchar_t* driver_name, const wchar_t* driver_path, NTSTATUS& load_status)
{
	static bool driver_privilege_acquired = false;

	if (!driver_privilege_acquired)
	{
		if (!acquire_privilege(SeLoadDriverPrivilege))
			return false;

		driver_privilege_acquired = true;
	}

	if (!remove_service_registry_key(driver_name))
		return false;

	if (!add_service_registry_key(driver_name, driver_path))
		return false;

	std::wstring service_registry_path = std::wstring(L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\") + driver_name;

	UNICODE_STRING service_registry_us;
	service_registry_us.Buffer = (wchar_t*)service_registry_path.c_str();
	service_registry_us.Length = (USHORT)(service_registry_path.size()) * 2;
	service_registry_us.MaximumLength = (USHORT)(service_registry_path.size() + 1) * 2;

	load_status = ZwLoadDriver(&service_registry_us);

	return NT_SUCCESS(load_status);
}