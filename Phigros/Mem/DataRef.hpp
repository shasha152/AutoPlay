#pragma once

#include <memory>

namespace Mem {

template<typename T>
class DataRef {
    std::shared_ptr<T> __data;
    
    using value_ref = T&;
    using const_value_ref = const T&;
  public:
    //DataRef() : __data(nullptr) {}
    
    
    template<typename... Args>
    DataRef(Args&&... args) {
        make(std::forward<Args>(args)...);
    }
    
    DataRef(const DataRef& other) : __data(other.__data) {}
    DataRef(DataRef&& other) : __data(std::move(other.__data)) {}
    
    DataRef& operator=(const DataRef& other) {
        __data = other.__data;
        return *this;
    }
    
    DataRef& operator=(DataRef&& other) noexcept {
        __data = std::move(other.__data);
        return *this;
    }
    
    template<typename... Args>
    void make(Args&&... args) {
        __data = std::make_shared<T>(std::forward<Args>(args)...);
    }
    
    template<typename _Tp,
        std::enable_if_t<std::is_same<T, std::decay_t<_Tp>>::value>>
    DataRef& value(_Tp&& v) {
        *__data = std::forward<_Tp>(v);
        return *this;
    }
    
    template<typename _Tp,
        std::enable_if_t<std::is_same<T, std::decay_t<_Tp>>::value>>
    DataRef& operator=(_Tp&& v) {
        return value(std::forward<_Tp>(v));
    }
    
    const_value_ref get() const noexcept {
        return *__data;
    }
    
    value_ref get() noexcept {
        return *__data;
    }
    
    value_ref operator()() noexcept {
        return get();
    }
    
    const_value_ref operator()() const noexcept {
        return get();
    }
};

};