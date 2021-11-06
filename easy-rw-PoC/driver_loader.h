#pragma once

#include <Windows.h>

bool load_driver(const wchar_t* driver_name, const wchar_t* driver_path, NTSTATUS& load_status);
