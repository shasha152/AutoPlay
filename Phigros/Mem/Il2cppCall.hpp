#pragma once

#include "Reader.hpp"
#include <mutex>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <dlfcn.h>
#include <sys/mman.h>

namespace Mem {

namespace Il2cpp {

namespace Call {

class call_main {
    pid_t pid = ::Mem::pid;
    struct user_regs_struct regs, original_regs;
    struct iovec iov;
  public:
    call_main(const std::vector<long>& args, long& result, uintptr_t call_addr) { 
        __attach();
        call(args, result, call_addr);
    }
    explicit call_main(pid_t _pid, const std::vector<long>& args, long& result, uintptr_t call_addr) : pid(_pid) { 
        __attach(); 
        call(args, result, call_addr);
    }
    
    ~call_main() {
        if(ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
            //throw std::logic_error("call main : detach");
        }
    }
    
  private:
    void __attach() {
        if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
            throw std::runtime_error("call main : attach");
        }
        
        waitpid(pid, NULL, 0);
    }
    
    void call(const std::vector<long> &args, long& result, uintptr_t call_addr) {
        
        iov.iov_base = &original_regs;
        iov.iov_len = sizeof(original_regs);
        if(ptrace(PTRACE_GETREGSET, pid, (void*)NT_PRSTATUS, &iov) == -1) {
            throw std::logic_error("call : get regs");
        }
        
        regs = original_regs;
        regs.pc = call_addr;
        regs.regs[30] = 0;
        for(int i = 0; i < args.size(); i++) {
            regs.regs[i] = args[i];
        }
        
        iov.iov_base = &regs;
        if(ptrace(PTRACE_SETREGSET, pid, (void*)NT_PRSTATUS, &iov) == -1) {
            throw std::logic_error("call : set regs(regs)");
        }
        if(ptrace(PTRACE_CONT, pid, NULL, NULL) == -1) {
            throw std::logic_error("call : cont");
        }
        waitpid(pid, NULL, 0);
        if(ptrace(PTRACE_GETREGSET, pid, (void*)NT_PRSTATUS, &iov) == -1) {
            throw std::logic_error("call : get regs");
        }
        result = regs.regs[0];
        
        iov.iov_base = &original_regs;
        if(ptrace(PTRACE_SETREGSET, pid, (void*)NT_PRSTATUS, &iov) == -1) {
            throw std::logic_error("call : set regs(old)");
        }
        
        
    }
};

inline long PtraceCall(uintptr_t call_addr, const std::vector<long>& v) {
    static std::mutex mtx;
    mtx.lock();
    long result = 0;
    call_main(v, result, call_addr);
    mtx.unlock();
    return result;
}

template<const char *function_str>
inline uintptr_t get_offset_function() {
    static uintptr_t offset = 0;
    static auto base = get_module_base("libc.so", 1);
    if(offset == 0) {
        
        void *handle = dlopen("libc.so", RTLD_NOW);
        void *addr = (void *)dlsym(handle, function_str);
        if(addr == nullptr) {
            throw std::logic_error(std::string("get_offset_function : 获取函数地址失败 arg : ") + function_str);
        } else {
            Dl_info info;
            dladdr(addr, &info);
            offset = reinterpret_cast<uintptr_t>(addr) - reinterpret_cast<uintptr_t>(info.dli_fbase);
        }
        dlclose(handle);
    }
    
    return base + offset;
}

inline uintptr_t newPage(int prot = PROT_READ | PROT_WRITE | PROT_EXEC, size_t size = 0x1000) {
    static constexpr char __mmap_str[] = "mmap";
    auto __mmap = get_offset_function<__mmap_str>();
    long result = PtraceCall(__mmap, {0, 
        static_cast<long>(size), 
        static_cast<long>(prot), 
        static_cast<long>(MAP_PRIVATE | MAP_ANONYMOUS), -1, 0});
    if(result <= 0) {
        throw std::runtime_error("newPage : 调用mmap出现异常");
    }
    return static_cast<uintptr_t>(result);
}

inline uintptr_t CreateString(const std::u16string& str, uintptr_t unity_str_class) {
    auto addr = newPage(PROT_READ | PROT_WRITE, 0x12 + str.size() * 2);
#pragma pack(push, 1)
    struct __unity_string {
        uintptr_t vclass;
        void *null;
        int len;
    } __str{unity_str_class, nullptr, static_cast<int>(str.size())};
#pragma pack(pop)
    vNormalRw rw;
    rw.write(addr, &__str, sizeof(__str));
    for(int i = 0; i < str.size(); i++) {
        rw.write(addr + sizeof(__str) + i * 2, (std::u16string::value_type *)&str[i], 2);
    }
    
    return addr;
}

template<typename T>
inline Il2ObjectArray<T> FindObjectsOfType(uintptr_t unity_string_addr, uintptr_t unity_gettype_function, uintptr_t unity_findobjectsoftype_function) {
    uintptr_t type_addr = PtraceCall(unity_gettype_function, {(static_cast<long>(unity_string_addr))});
    if(type_addr <= 0x8000) {
        throw std::runtime_error("FindObject : type 获取失败");
    }
    uintptr_t addr = PtraceCall(unity_findobjectsoftype_function, {static_cast<long>(type_addr)});
    if(addr <= 0x8000) {
        throw std::runtime_error("FindObject : object 获取失败");
    }
    return Il2ObjectArray<T>(addr);
}


};

};

};