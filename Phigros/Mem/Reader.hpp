#pragma once

#include "Mem.hpp"
#include <memory>
#include <unordered_map>

namespace Mem {

template<typename T>
class OffsetValue {
    std::unique_ptr<T> __ptr;
    size_t __type_size = sizeof(T);
    uint16_t __offset;
  public:
    using value_pointer = T*;
    using value_ref = T&;
    using const_value_ref = const T&;
  public:
    OffsetValue() : __ptr(std::make_unique<T>()),
        __offset(0) {}
    explicit OffsetValue(uint16_t offset) :
        __ptr(std::make_unique<T>()),
        __offset(offset) {}
    OffsetValue& operator=(const OffsetValue& other) {
        __ptr = std::make_unique<T>(other());
        __type_size = other.get_size();
        __offset = other.get_offset();
        return *this;
    }
    
    OffsetValue& operator=(OffsetValue&& other) noexcept {
        __ptr = std::move(other.__ptr);
        __type_size = other.get_size();
        __offset = other.get_offset();
        return *this;
    }
    
    OffsetValue(const OffsetValue& other) {
        *this = other;
    }
    OffsetValue(OffsetValue&& other) noexcept {
        *this = std::move(other);
    }
    
    OffsetValue& set_offset(uint16_t offset) noexcept {
        __offset = offset;
        return *this;
    }
    
    size_t get_size() const noexcept { return __type_size; }
    uint16_t get_offset() const noexcept { return __offset; }
    
    value_pointer get_data() noexcept { return __ptr.get(); }
    const value_pointer get_data() const noexcept { return __ptr.get(); }
    
    value_ref operator()() noexcept { return *get_data(); }
    const_value_ref operator()() const noexcept { return *get_data(); }
    
};



enum emReaderType : uint8_t {
    Reader_Nomal = 0,
    Reader_OffsetValue = 1,
    Reader_OffsetValue_Struct = 2
};

template<typename T, class MemRw = vm_memory, emReaderType Type = Reader_OffsetValue>
class ReadWrite;

template<typename T, class MemRw>
class ReadWrite<T, MemRw, Reader_OffsetValue> : public MemRw {
    static_assert(std::is_base_of<memory_base, std::decay_t<MemRw>>::value, "读写错误");

  public:
    using MemRw::read;
    using MemRw::write;
    
    void read(uintptr_t __addr, T& out) {
        read(__addr + out.get_offset(), out.get_data(), out.get_size());
    }
    
    void write(uintptr_t __addr, T& in) {
        write(__addr + in.get_offset(), in.get_data(), in.get_size());
    }
    
    
};

namespace detail {
  template<typename T, typename = void>
  struct is_reader_class : std::false_type {};

  template<typename T>
  struct is_reader_class<T, std::enable_if_t<std::is_base_of<
    memory_base, std::decay_t<T>>::value>> : std::true_type {};

  template<typename T>
  inline constexpr bool is_reader_class_v = is_reader_class<T>::value;
};

template<typename T, class MemRw>
class ReadWrite<T, MemRw, Reader_OffsetValue_Struct> : public MemRw {
    static_assert(detail::is_reader_class_v<MemRw>, "读写错误");
    using TestValueOffset = OffsetValue<int>;
    using Test_Pointer = TestValueOffset*;
    static constexpr size_t count = sizeof(T) / sizeof(TestValueOffset);
    
    iovec locals[count];
    iovec remotes[count];
    
  public:
    using MemRw::read;
    using MemRw::write;
    
    void read(uintptr_t __addr, T& out) {
        set_iovec(__addr, reinterpret_cast<Test_Pointer>(&out));
        syscall(SYS_process_vm_readv, pid, locals, count,
            remotes, count, 0);
    }
    
    void write(uintptr_t __addr, T& in) {
        set_iovec(__addr, reinterpret_cast<Test_Pointer>(&in));
        syscall(SYS_process_vm_writev, pid, locals, count,
            remotes, count, 0);
    }
    
    
  private:
    void set_iovec(uintptr_t remote_addr, Test_Pointer offset_value_struct) {
        for(size_t i = 0; i < count; i++) {
            locals[i].iov_base = reinterpret_cast<void*>(offset_value_struct->get_data());
            locals[i].iov_len = offset_value_struct->get_size();
            remotes[i].iov_base = reinterpret_cast<void*>(remote_addr + offset_value_struct->get_offset());
            remotes[i].iov_len = offset_value_struct->get_size();
            offset_value_struct++;
        }
    }
};

template<typename T, class MemRw>
class ReadWrite<T, MemRw, Reader_Nomal> : public MemRw {
    static_assert(detail::is_reader_class_v<MemRw>, "读写错误");
  public:
    using MemRw::read;
    using MemRw::write;
};

template<typename STL, typename T = typename STL::value_type, typename Reader>
inline size_t ReadString(uintptr_t addr, STL& data, Reader& mem) {
    static_assert(detail::is_reader_class_v<Reader>, "读写错误");
    static constexpr size_t read_string_max = 512;
    int size = 0;
    T value;
    while(true) {
        if(size > read_string_max) {
            throw std::runtime_error("ReadString : 超出读取限制");
        }
        mem.read(addr + size * sizeof(T), 
            reinterpret_cast<void*>(&value), sizeof(T));
        if constexpr (std::is_same<T, char>::value) {
            if(value <= 0) break;
        } else {
            if(value == 0) break;
        }
        data.insert(data.end(), value);
        size++;
    }
    return size;
}


template<typename T>
using OffsetRw = ReadWrite<T, vm_memory, Reader_OffsetValue>;

template<typename T>
using StructRw = ReadWrite<T, vm_memory, Reader_OffsetValue_Struct>;

using vNormalRw = ReadWrite<void, vm_memory, Reader_Nomal>;
using pNormalRw = ReadWrite<void, p_memory, Reader_Nomal>;



}