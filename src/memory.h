// memory.h
#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& other) const { return { x + other.x, y + other.y }; }
    Vec2 operator-(const Vec2& other) const { return { x - other.x, y - other.y }; }
    Vec2 operator*(float scalar) const { return { x * scalar, y * scalar }; }
};

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& other) const { return { x + other.x, y + other.y, z + other.z }; }
    Vec3 operator-(const Vec3& other) const { return { x - other.x, y - other.y, z - other.z }; }
    Vec3 operator*(float scalar) const { return { x * scalar, y * scalar, z * scalar }; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float Distance(const Vec3& other) const { return (*this - other).Length(); }
};

struct Matrix4x4 {
    float m[4][4];
};

class MemoryManager {
private:
    HANDLE m_processHandle;
    DWORD m_processId;
    uintptr_t m_baseAddress;
    std::string m_processName;
    bool m_attached;

    DWORD FindProcessId(const std::string& processName);
    uintptr_t FindModuleBase(const std::string& moduleName);

public:
    MemoryManager();
    ~MemoryManager();

    bool Attach(const std::string& processName);
    void Detach();
    bool IsAttached() const { return m_attached; }
    
    uintptr_t GetBaseAddress() const { return m_baseAddress; }
    DWORD GetProcessId() const { return m_processId; }

    // Template read/write functions
    template<typename T>
    T Read(uintptr_t address) const {
        T value{};
        if (m_attached && m_processHandle) {
            ReadProcessMemory(m_processHandle, reinterpret_cast<LPCVOID>(address), 
                            &value, sizeof(T), nullptr);
        }
        return value;
    }

    template<typename T>
    bool Write(uintptr_t address, const T& value) const {
        if (!m_attached || !m_processHandle) return false;
        return WriteProcessMemory(m_processHandle, reinterpret_cast<LPVOID>(address), 
                                 &value, sizeof(T), nullptr);
    }

    // Read string
    std::string ReadString(uintptr_t address, size_t maxLength = 64) const;
    
    // Read buffer
    bool ReadBuffer(uintptr_t address, void* buffer, size_t size) const;

    // Pattern scanning
    uintptr_t PatternScan(const std::string& pattern, const std::string& mask, 
                          uintptr_t startAddress, size_t scanSize);

    // Pointer chain resolution
    uintptr_t ResolvePointerChain(uintptr_t base, const std::vector<uintptr_t>& offsets) const;

    // NOP instruction
    bool Nop(uintptr_t address, size_t size) const;
    
    // Patch bytes
    bool PatchBytes(uintptr_t address, const std::vector<BYTE>& bytes) const;

    // Get module info
    struct ModuleInfo {
        uintptr_t base;
        size_t size;
        std::string name;
    };
    std::vector<ModuleInfo> GetModules() const;
};
