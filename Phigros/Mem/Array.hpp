#pragma once

#include <new>
#include <algorithm>

namespace Mem {

namespace Il2cpp {

template<typename T>
class Array {
    using value_pointer = T*;
    using value_ref = T&;
    
    using const_value_ref = const T&;
    
    value_pointer __data{nullptr};
    size_t __size{0};
    size_t __capacity{0};
    
  public:
    using value_type = T;
    using iterator = T*;
    
    Array() = default;
    Array(const Array& _array) {
        *this = _array;
    }
    
    Array(Array &&_array) noexcept {
        *this = std::move(_array);
    }
    
    Array& operator=(const Array& _array) {
        resize(_array.__size);
        std::copy(_array.begin(), _array.end(), __data);  
        return *this;
    }
    
    Array& operator=(Array&& _array) noexcept {
        __data = _array.__data;
        __size = _array.__size;
        __capacity = _array.__capacity;
        _array.__data = nullptr;
        _array.__size = 0;
        _array.__capacity = 0;
        return *this;
    }
    
    explicit Array(size_t size) {
        resize(size);
    }
    
    
    virtual ~Array() {
        if(__data) {
            delete __data;
        }
    }
    template<typename __Tp,
        std::enable_if_t<std::is_same<T, std::decay_t<__Tp>>::value>>
    value_ref push_back(__Tp &&data) {
        if(__size >= __capacity) {
            throw std::out_of_range("Array : 添加数值超出界限");
        }
        __data[__size] = std::forward<__Tp>(data);
        __size++;
        return __data[__size - 1];
    }
    
    
    void resize(size_t size) {
        if(__capacity < size) {
            __capacity = size;
            
            auto older_data = __data;
            __data = new(std::nothrow) T[size]{};
            if(__data == nullptr) throw std::runtime_error("Array : resize");
            std::move(__data, __data + __size, older_data);
            __size = size;
            delete older_data;
        }
    }
    
    void reserve(size_t size) {
        if(__capacity < size) {
            __capacity = size;
            
            auto older_data = __data;
            __data = new(std::nothrow) T[size];
            if(__data == nullptr) throw std::runtime_error("Array : reserve");
            std::move(__data, __data + __size, older_data);
            delete older_data;
        }
    }
    
    void clear() { __size = 0; }
    size_t size() const noexcept { return __size; }
    
    value_ref operator[](size_t index) noexcept {
        return __data[index];
    }
    const_value_ref operator[](size_t index) const noexcept {
        return __data[index];
    }
    
    value_ref at(size_t index) {
        if(index >= __size) {
            throw std::out_of_range("Array : 超出数组界限");
        }
        return __data[index];
    }
    
    const_value_ref at(size_t index) const {
        if(index >= __size) {
            throw std::out_of_range("Array : 超出数组界限");
        }
        return __data[index];
    }
    
    value_pointer begin() noexcept { 
        return __data;
    }
    
    const value_pointer begin() const noexcept {
        return __data;
    }
    
    value_pointer end() noexcept {
        return __data + __size;
    }
    
    const value_pointer end() const noexcept {
        return __data + __size;
    }
    
    value_pointer data() noexcept {
        return __data;
    }
    
    const value_pointer data() const noexcept {
        return __data;
    }
    
    const bool empty() const noexcept {
        return __size == 0;
    }
    
    bool empty() noexcept {
        return __size == 0;
    }
};

};

};