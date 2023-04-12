#include "DLLInjector.h"
#include <TlHelp32.h>


DLLInjector::DLLInjector() {}

// Injects the DLL into the specified process
bool DLLInjector::Inject(const std::wstring& process_name, const std::wstring& dll_path) {
	DWORD process_id = FindProcessId(process_name);
	if (process_id == 0) {
		return false;
	}

	return InjectDLL(process_id, dll_path);
}


// Finds the process ID of the specified process
DWORD DLLInjector::FindProcessId(const std::wstring& process_name) {
	DWORD process_id = 0;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	// Create a snapshot of all processes
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	// Iterate through processes to find the target process
	if (Process32First(snapshot, &entry)) {
		do {
			if (_wcsicmp(entry.szExeFile, process_name.c_str()) == 0) {
				process_id = entry.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &entry));
	}

	// Close the snapshot handle and return the process ID
	CloseHandle(snapshot);
	return process_id;
}

// Injects the DLL into the process with the specified process ID
bool DLLInjector::InjectDLL(DWORD process_id, const std::wstring& dll_path) {
	// Open the target process with all access
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
	if (!process) {
		return false;
	}

	// Allocate memory in the target process for the DLL path
	LPVOID allocated_memory = VirtualAllocEx(process, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!allocated_memory) {
		CloseHandle(process);
		return false;
	}

	// Write the DLL path into the allocated memory
	if (!WriteProcessMemory(process, allocated_memory, dll_path.c_str(), dll_path.size() * sizeof(wchar_t) + 1, nullptr)) {
		VirtualFreeEx(process, allocated_memory, 0, MEM_RELEASE);
		CloseHandle(process);
		return false;
	}

	// Get the address of LoadLibraryW function
	LPVOID load_library_address = (LPVOID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
	if (!load_library_address) {
		VirtualFreeEx(process, allocated_memory, 0, MEM_RELEASE);
		CloseHandle(process);
		return false;
	}

	// Create a remote thread to load the DLL in the target process
	HANDLE remote_thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_address, allocated_memory, 0, NULL);
	if (!remote_thread) {
		VirtualFreeEx(process, allocated_memory, 0, MEM_RELEASE);
		CloseHandle(process);
		return false;
	}

	// Wait for the remote thread to finish
	WaitForSingleObject(remote_thread, INFINITE);
	// Cleanup handles and memory
	CloseHandle(remote_thread);
	VirtualFreeEx(process, allocated_memory, 0, MEM_RELEASE);
	CloseHandle(process);
	return true;
}


