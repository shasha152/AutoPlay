#pragma once

#include "Mem/All.hpp"
#include <unordered_map>

namespace Phigros {

class PhigrosBase {
  public:
    static inline Mem::Il2cpp::Domain::Il2Assembly<Mem::Il2cpp::PhigrosOffset> assembly;
  private:
    static inline uintptr_t FindObjectsAddr;
    static inline uintptr_t GetTypeAddr;
    static inline uintptr_t StringAddr;
    
    static inline std::unordered_map<std::u16string, uintptr_t> UnityString;
  protected:
    uintptr_t m_addr{0};
  public:
    static void init(uintptr_t array_addr);
    
    static uintptr_t GetUString(const std::u16string& str);
    template<typename T>
    static Mem::Il2cpp::Il2ObjectArray<T> FindObjects(const std::u16string& str);
    
    virtual void update() = 0;
    void set_addr(uintptr_t addr) noexcept;
};

void Init(const char *package);
void Run();

template<typename T>
inline Mem::Il2cpp::Il2ObjectArray<T>
PhigrosBase::FindObjects(const std::u16string& str) {
    return Mem::Il2cpp::Call::FindObjectsOfType<T>(GetUString(str), GetTypeAddr, FindObjectsAddr);
}

};



