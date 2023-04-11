#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>

class ProcessMemoryManager {
public:
	ProcessMemoryManager();
	explicit ProcessMemoryManager(DWORD processId);
	explicit ProcessMemoryManager(const std::wstring& processName);
	explicit ProcessMemoryManager(const std::string& processName);
	~ProcessMemoryManager();

	bool IsProcessHandleValid() const;
	bool OpenProcessById(DWORD processId);
	bool OpenProcessByName(const std::wstring& processName);
	bool OpenProcessByName(const std::string& processName);

	template <typename T>
	T ReadValue(uintptr_t address) {
		T value;
		SIZE_T bytesRead;
		if (!ReadProcessMemory(processHandle_, (LPCVOID)address, &value, sizeof(T), &bytesRead)) {
			return T();
		}
		return value;
	}

	template <typename T>
	bool WriteValue(uintptr_t address, T newValue) {
		SIZE_T bytesWritten;
		return WriteProcessMemory(processHandle_, (LPVOID)address, &newValue, sizeof(T), &bytesWritten);
	}

	template <typename T>
	T ReadValueWithOffsets(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) {
		uintptr_t address = baseAddress;
		for (size_t i = 0; i < offsets.size() - 1; ++i) {
			address = ReadValue<uintptr_t>(address + offsets[i]);
		}
		return ReadValue<T>(address + offsets.back());
	}

	template <typename T>
	bool WriteValueWithOffsets(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets, T newValue) {
		uintptr_t address = baseAddress;
		for (size_t i = 0; i < offsets.size() - 1; ++i) {
			address = ReadValue<uintptr_t>(address + offsets[i]);
		}
		return WriteValue<T>(address + offsets.back(), newValue);
	}

	uintptr_t GetAddressWithOffsets(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) {
		uintptr_t address = baseAddress;
		for (size_t i = 0; i < offsets.size()-1; ++i) {
			address = ReadValue<uintptr_t>(address + offsets[i]);

		}
		return address + offsets.back();
	}

	uintptr_t GetModuleBaseAddress(const std::wstring& moduleName);
	uintptr_t GetModuleBaseAddress(const std::string& moduleName);

private:
	HANDLE processHandle_;

	DWORD FindProcessId(const std::wstring& processName);
	std::string WideStringToString(const std::wstring& wideStr);
};
