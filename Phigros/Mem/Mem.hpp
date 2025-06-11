#pragma once

#include <sys/syscall.h>
#include <sys/uio.h>
#include <string>
#include <dirent.h>
#include <fstream>
#include <elf.h>
#include <unistd.h>
#include <fcntl.h>

namespace Mem {
    inline pid_t pid = 0;
    
    inline pid_t init(const char *packname) {
        DIR* dir = opendir("/proc");
        if(dir == nullptr) std::runtime_error("/proc打开失败");
        dirent *energy = nullptr;
        while((energy = readdir(dir))) {
            std::string pach = "/proc/" + std::string(energy->d_name) + "/cmdline";
            std::ifstream file(pach.c_str());
            if(file.is_open()) {
                std::getline(file, pach);
                file.close();
                if(strcmp(pach.c_str(), packname) == 0) {
                    pid = std::stoi(energy->d_name);
                    break;
                }
            }
        }
        closedir(dir);
        return pid;
    }
    
    inline uintptr_t get_module_base(const char *module, size_t count) {
        std::string path = std::string("/proc/") + std::to_string(pid) + "/maps";
        std::ifstream file(path);
        if(file.is_open()) {
            std::string line;
            size_t m_count = 0;
            while(getline(file, line)) {
                if(strstr(line.c_str(), module)) {
                    if(count == m_count) {
                        uintptr_t result;
                        sscanf(line.c_str(), "%lx-%*lx", &result);
                        file.close();
                        return result;
                    }
                    m_count++;
                }
            }
            file.close();
            throw std::runtime_error((std::string("未找到模块 : ") + module).c_str());
        }
        else {
            throw std::runtime_error("maps文件打开失败");
        }
        return 0;
    }
    
    class memory_base {
      public:
        virtual void read(uintptr_t addr, void *data, size_t size) {}
        virtual void write(uintptr_t addr, void *data, size_t size) {}
        
        uintptr_t loadchain(uintptr_t start, uintptr_t* offset, size_t size) {
            uintptr_t result = start;
            for(int i = 0; i < size; i++)
                read(result + offset[i], &result, sizeof(long));
            return result;
        }
        
        template<typename T>
        T read(uintptr_t addr) {
            T value;
            read(addr, &value, sizeof(T));
            return value;
        }
    };
    
    
    class vm_memory : public memory_base {
        iovec local_iov;
        iovec remote_iov;
      public:
        using memory_base::read;
        using memory_base::write;
      public:
        
        void read(uintptr_t addr, void *data, size_t size) override {
            local_iov.iov_base = data;
            local_iov.iov_len = size;
            remote_iov.iov_base = (void*)addr;
            remote_iov.iov_len = size;
            syscall(SYS_process_vm_readv, pid, &local_iov, 1, &remote_iov, 1, 0);
        }
        
        void write(uintptr_t addr, void *data, size_t size) override {
            local_iov.iov_base = data;
            local_iov.iov_len = size;
            remote_iov.iov_base = (void*)addr;
            remote_iov.iov_len = size;
            syscall(SYS_process_vm_writev, pid, &local_iov, 1, &remote_iov, 1, 0);
        }
        
        virtual ~vm_memory() = default;
    };
    
    class p_memory : public memory_base {
        int mem_fd;
      public:
        using memory_base::read;
        using memory_base::write;
      public:
        p_memory() {
            std::string path = std::string("/proc/") + std::to_string(pid) + "/mem";
            mem_fd = open(path.c_str(), O_RDWR);
            if(mem_fd == -1) throw std::runtime_error(path + "打开失败");
        }
        p_memory(const p_memory& other) {
            std::string path = std::string("/proc/") + std::to_string(pid) + "/mem";
            mem_fd = open(path.c_str(), O_RDWR);
            if(mem_fd == -1) throw std::runtime_error(path + "打开失败");
        }
        p_memory(p_memory &&other) noexcept {
            mem_fd = other.mem_fd;
        }
        virtual ~p_memory() {
            close(mem_fd);
        }
        
        void read(uintptr_t addr, void *data, size_t size) override {
            pread64(mem_fd, data, size, addr);
        }
        
        void write(uintptr_t addr, void *data, size_t size) override {
            pwrite64(mem_fd, data, size, addr);
        }
        
    };
    
    
};