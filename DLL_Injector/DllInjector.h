#pragma once

#include <Windows.h>
#include <string>

class DLLInjector {
public:
    DLLInjector();
    bool Inject(const std::wstring& process_name, const std::wstring& dll_path);

private:
    DWORD FindProcessId(const std::wstring& process_name);
    bool InjectDLL(DWORD process_id, const std::wstring& dll_path);
};