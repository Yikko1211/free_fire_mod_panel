// memory.cpp
#include "memory.h"
#include <algorithm>

MemoryManager::MemoryManager() 
    : m_processHandle(nullptr), m_processId(0), 
      m_baseAddress(0), m_attached(false) {}

MemoryManager::~MemoryManager() {
    Detach();
}

DWORD MemoryManager::FindProcessId(const std::string& processName) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(entry);

    if (Process32First(snap, &entry)) {
        do {
            if (std::string(entry.szExeFile) == processName) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &entry));
    }

    CloseHandle(snap);
    return pid;
}

uintptr_t MemoryManager::FindModuleBase(const std::string& moduleName) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_processId);
    
    if (snap == INVALID_HANDLE_VALUE) return 0;

    MODULEENTRY32 entry;
    entry.dwSize = sizeof(entry);

    uintptr_t base = 0;
    if (Module32First(snap, &entry)) {
        do {
            if (std::string(entry.szModule) == moduleName) {
                base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
                break;
            }
        } while (Module32Next(snap, &entry));
    }

    CloseHandle(snap);
    return base;
}

bool MemoryManager::Attach(const std::string& processName) {
    m_processName = processName;
    m_processId = FindProcessId(processName);
    
    if (m_processId == 0) {
        std::cout << "[Memory] Process not found: " << processName << std::endl;
        return false;
    }

    m_processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_processId);
    if (!m_processHandle) {
        std::cout << "[Memory] Failed to open process. Error: " << GetLastError() << std::endl;
        return false;
    }

    m_baseAddress = FindModuleBase(processName);
    m_attached = true;
    
    std::cout << "[Memory] Attached to " << processName 
              << " | PID: " << m_processId 
              << " | Base: 0x" << std::hex << m_baseAddress << std::dec << std::endl;
    
    return true;
}

void MemoryManager::Detach() {
    if (m_processHandle) {
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
    m_attached = false;
    m_processId = 0;
    m_baseAddress = 0;
    std::cout << "[Memory] Detached." << std::endl;
}

std::string MemoryManager::ReadString(uintptr_t address, size_t maxLength) const {
    std::vector<char> buffer(maxLength + 1, 0);
    if (m_attached && m_processHandle) {
        ReadProcessMemory(m_processHandle, reinterpret_cast<LPCVOID>(address), 
                         buffer.data(), maxLength, nullptr);
    }
    return std::string(buffer.data());
}

bool MemoryManager::ReadBuffer(uintptr_t address, void* buffer, size_t size) const {
    if (!m_attached || !m_processHandle) return false;
    SIZE_T bytesRead;
    return ReadProcessMemory(m_processHandle, reinterpret_cast<LPCVOID>(address), 
                            buffer, size, &bytesRead) && bytesRead == size;
}

uintptr_t MemoryManager::PatternScan(const std::string& pattern, const std::string& mask,
                                      uintptr_t startAddress, size_t scanSize) {
    std::vector<BYTE> buffer(scanSize);
    if (!ReadBuffer(startAddress, buffer.data(), scanSize)) return 0;

    size_t patternLength = mask.length();
    
    for (size_t i = 0; i <= scanSize - patternLength; i++) {
        bool found = true;
        for (size_t j = 0; j < patternLength; j++) {
            if (mask[j] == 'x' && buffer[i + j] != static_cast<BYTE>(pattern[j])) {
                found = false;
                break;
            }
        }
        if (found) {
            return startAddress + i;
        }
    }
    return 0;
}

uintptr_t MemoryManager::ResolvePointerChain(uintptr_t base, 
                                               const std::vector<uintptr_t>& offsets) const {
    uintptr_t address = base;
    for (size_t i = 0; i < offsets.size(); i++) {
        address = Read<uintptr_t>(address);
        if (address == 0) return 0;
        address += offsets[i];
    }
    return address;
}

bool MemoryManager::Nop(uintptr_t address, size_t size) const {
    if (!m_attached || !m_processHandle) return false;
    
    DWORD oldProtect;
    VirtualProtectEx(m_processHandle, reinterpret_cast<LPVOID>(address), size, 
                     PAGE_EXECUTE_READWRITE, &oldProtect);
    
    std::vector<BYTE> nops(size, 0x90);
    bool result = WriteProcessMemory(m_processHandle, reinterpret_cast<LPVOID>(address), 
                                     nops.data(), size, nullptr);
    
    VirtualProtectEx(m_processHandle, reinterpret_cast<LPVOID>(address), size, 
                     oldProtect, &oldProtect);
    return result;
}

bool MemoryManager::PatchBytes(uintptr_t address, const std::vector<BYTE>& bytes) const {
    if (!m_attached || !m_processHandle) return false;
    
    DWORD oldProtect;
    VirtualProtectEx(m_processHandle, reinterpret_cast<LPVOID>(address), bytes.size(), 
                     PAGE_EXECUTE_READWRITE, &oldProtect);
    
    bool result = WriteProcessMemory(m_processHandle, reinterpret_cast<LPVOID>(address), 
                                     bytes.data(), bytes.size(), nullptr);
    
    VirtualProtectEx(m_processHandle, reinterpret_cast<LPVOID>(address), bytes.size(), 
                     oldProtect, &oldProtect);
    return result;
}

std::vector<MemoryManager::ModuleInfo> MemoryManager::GetModules() const {
    std::vector<ModuleInfo> modules;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_processId);
    
    if (snap == INVALID_HANDLE_VALUE) return modules;

    MODULEENTRY32 entry;
    entry.dwSize = sizeof(entry);

    if (Module32First(snap, &entry)) {
        do {
            ModuleInfo info;
            info.base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
            info.size = entry.modBaseSize;
            info.name = entry.szModule;
            modules.push_back(info);
        } while (Module32Next(snap, &entry));
    }

    CloseHandle(snap);
    return modules;
}
