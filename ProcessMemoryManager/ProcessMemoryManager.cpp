#include "ProcessMemoryManager.h"


ProcessMemoryManager::ProcessMemoryManager() : processHandle_(nullptr) {}


ProcessMemoryManager::ProcessMemoryManager(DWORD processId) : ProcessMemoryManager() {
	OpenProcessById(processId);
}

ProcessMemoryManager::ProcessMemoryManager(const std::wstring& processName) : ProcessMemoryManager() {
	OpenProcessByName(processName);
}

ProcessMemoryManager::ProcessMemoryManager(const std::string& processName) : ProcessMemoryManager() {
	OpenProcessByName(processName);
}

ProcessMemoryManager::~ProcessMemoryManager() {
	if (processHandle_ != nullptr) {
		CloseHandle(processHandle_);
	}
}

bool ProcessMemoryManager::IsProcessHandleValid() const {
	return processHandle_ != nullptr && processHandle_ != INVALID_HANDLE_VALUE;
}

bool ProcessMemoryManager::OpenProcessById(DWORD processId) {
	processHandle_ = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (!IsProcessHandleValid()) {
		throw std::runtime_error("Failed to open process with ID: " + std::to_string(processId));
	}
	return true;
}

bool ProcessMemoryManager::OpenProcessByName(const std::wstring& processName) {
	DWORD processId = FindProcessId(processName);
	if (processId == 0) {
		throw std::runtime_error("Not found process named: " + WideStringToString(processName));
	}
	return OpenProcessById(processId);
}

bool ProcessMemoryManager::OpenProcessByName(const std::string& processName) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wideProcessName = converter.from_bytes(processName);
	return OpenProcessByName(wideProcessName);
}

uintptr_t ProcessMemoryManager::GetModuleBaseAddress(const std::wstring& moduleName) {
	uintptr_t moduleBaseAddress = 0;
	DWORD processId = GetProcessId(processHandle_);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
	if (snapshot != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 moduleEntry;
		moduleEntry.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(snapshot, &moduleEntry)) {
			do {
				if (moduleEntry.szModule == moduleName) {
					moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(snapshot, &moduleEntry));
		}
	}
	CloseHandle(snapshot);

	if (moduleBaseAddress == 0) {
		throw std::runtime_error("Failed to find module: " + WideStringToString(moduleName));
	}
	return moduleBaseAddress;
}

uintptr_t ProcessMemoryManager::GetModuleBaseAddress(const std::string& processName) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wideProcessName = converter.from_bytes(processName);
	return GetModuleBaseAddress(wideProcessName);
}

DWORD ProcessMemoryManager::FindProcessId(const std::wstring& processName) {
	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}

	if (Process32First(snapshot, &processEntry)) {
		do {
			if (std::wstring(processEntry.szExeFile) == processName) {
				CloseHandle(snapshot);
				return processEntry.th32ProcessID;
			}
		} while (Process32Next(snapshot, &processEntry));
	}

	CloseHandle(snapshot);
	return 0;
}

std::string ProcessMemoryManager::WideStringToString(const std::wstring& wideStr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.to_bytes(wideStr);
}