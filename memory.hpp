#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <cstdint>

class Memory {
public:
    HANDLE hProcess = nullptr;
    uintptr_t clientBase = 0;
    uintptr_t engineBase = 0;
    DWORD pid = 0;

    bool Attach(const wchar_t* processName) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32W pe{ sizeof(pe) };
        if (Process32FirstW(snap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, processName) == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);

        if (!pid) return false;

        hProcess = OpenProcess(
            PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
            FALSE, pid);
        return hProcess != nullptr;
    }

    uintptr_t GetModuleBase(const wchar_t* moduleName) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap == INVALID_HANDLE_VALUE) return 0;

        MODULEENTRY32W me{ sizeof(me) };
        uintptr_t base = 0;
        if (Module32FirstW(snap, &me)) {
            do {
                if (_wcsicmp(me.szModule, moduleName) == 0) {
                    base = reinterpret_cast<uintptr_t>(me.modBaseAddr);
                    break;
                }
            } while (Module32NextW(snap, &me));
        }
        CloseHandle(snap);
        return base;
    }

    template<typename T>
    T Read(uintptr_t address) {
        T value{};
        if (address)
            ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
        return value;
    }

    template<typename T>
    bool Write(uintptr_t address, const T& value) {
        if (!address) return false;
        return WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
    }

    bool ReadRaw(uintptr_t address, void* buffer, size_t size) {
        if (!address || !buffer) return false;
        SIZE_T bytes = 0;
        return ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes) && bytes == size;
    }

    std::string ReadString(uintptr_t address, size_t maxLen = 64) {
        char buffer[256]{};
        if (address && maxLen < sizeof(buffer))
            ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer, maxLen, nullptr);
        return std::string(buffer);
    }

    // Resolve entity handle (CS2 style - 112 bytes / 0x70 stride)
    uintptr_t ResolveHandle(uintptr_t entityList, uint32_t handle) {
        if (!handle || handle == 0xFFFFFFFF) return 0;
        uintptr_t listEntry = Read<uintptr_t>(entityList + 0x8 * ((handle & 0x7FFF) >> 9) + 16);
        if (!listEntry) return 0;
        return Read<uintptr_t>(listEntry + 112 * (handle & 0x1FF));
    }

    void Close() {
        if (hProcess) {
            CloseHandle(hProcess);
            hProcess = nullptr;
        }
    }

    ~Memory() { Close(); }
};

inline Memory mem;
