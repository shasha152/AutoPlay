#pragma once

#include <unordered_map>
#include <vector>
#include "DefIl2Offset.hpp"
#include "Reader.hpp"
#include "DataRef.hpp"

#include <iostream>

//这里我感觉写成答辩了

namespace Mem {

namespace Il2cpp {

namespace Domain {

template<class __Il2Offset>
class __MyIl2cppBase {
    static_assert(detail::is_il2cpp_offset_v<__Il2Offset>, "IL2CPP偏移获取失败");
    
  public:
    virtual ~__MyIl2cppBase() = default;
  protected:
    static inline __Il2Offset offset;
    using PointerType = uint64_t;
    
};

template<class Il2Offset>
class Il2Class;

template<class Il2Offset>
class Il2Method : public __MyIl2cppBase<Il2Offset> {
    using ArgCountType = uint8_t;
    using typename __MyIl2cppBase<Il2Offset>::PointerType;
    using __MyIl2cppBase<Il2Offset>::offset;
    
    std::string name;
    friend class Il2Class<Il2Offset>;
  public:
    struct Method_Impl {
        OffsetValue<PointerType> method_name{offset.method_name};
        OffsetValue<PointerType> method_class{offset.method_class};
        OffsetValue<PointerType> method_addr{offset.method_addr};
        OffsetValue<ArgCountType> args_count{offset.method_argcount};
    };
  private:
    Method_Impl impl;

    Il2Method& parse(uintptr_t addr, StructRw<Method_Impl> &reader) {
        reader.read(addr, impl);
        ReadString(impl.method_name(), name, reader);
        return *this;
    }
    
    bool is_same_class(uintptr_t addr) {
        return impl.method_class() == addr;
    }
  public:
    Il2Method() = default;
    PointerType get_addr() const noexcept {
        return impl.method_addr();
    }
    
    ArgCountType get_args_count() const noexcept {
        return impl.args_count();
    }
    
    const std::string &get_name() const noexcept { 
        return name; 
    }
};

template<class Il2Offset>
class Il2Filed : public __MyIl2cppBase<Il2Offset> {
    using OffsetType = uint16_t;
    using typename __MyIl2cppBase<Il2Offset>::PointerType;
    using __MyIl2cppBase<Il2Offset>::offset;
    
    std::string name;
    friend class Il2Class<Il2Offset>;
  public:
    struct Field_Impl {
        OffsetValue<PointerType> field_name{offset.field_name};
        OffsetValue<PointerType> field_class{offset.field_class};
        OffsetValue<OffsetType> field_offset{offset.field_offset};
    };
  private:
    Field_Impl impl;
    
    Il2Filed& parse(uintptr_t addr, StructRw<Field_Impl> &reader) {
        reader.read(addr, impl);
        ReadString(impl.field_name(), name, reader);
        return *this;
    }
    bool is_same_class(uintptr_t addr) {
        return impl.field_class() == addr;
    }
  public:
    Il2Filed() = default;
    OffsetType get_offset() const noexcept {
        return impl.field_offset();
    }
    
    OffsetType get_addr() const noexcept {
        return impl.field_offset();
    }
    
    const std::string &get_name() const noexcept { 
        return name; 
    }
};

template<class Il2Offset>
class Il2Class : public __MyIl2cppBase<Il2Offset> {
  public:
    using Il2Class_t = Il2Class<Il2Offset>;
    using Il2Method_t = Il2Method<Il2Offset>;
    using Il2Filed_t = Il2Filed<Il2Offset>;
  private:
    using MethodImpl = typename Il2Method_t::Method_Impl;
    using FieldImpl = typename Il2Filed_t::Field_Impl;
    
    using typename __MyIl2cppBase<Il2Offset>::PointerType;
    using __MyIl2cppBase<Il2Offset>::offset;
    
    std::string name;
    std::string __namespace;
    DataRef<std::shared_ptr<Il2Class_t>> __parent;
    DataRef<std::vector<Il2Method_t>> __methods;
    DataRef<std::vector<Il2Filed_t>> __fields;
    
    using vCountType = uint16_t;
    struct Class_Impl {
        OffsetValue<PointerType> class_name{offset.class_name};
        OffsetValue<PointerType> class_namespace{offset.class_namespace};
        OffsetValue<PointerType> class_parent{offset.class_parent};
        OffsetValue<PointerType> class_method{offset.class_methods};
        OffsetValue<PointerType> class_field{offset.class_fields};
        OffsetValue<vCountType> class_mthcount{offset.class_mthcount};
        OffsetValue<vCountType> class_fldcount{offset.class_fldcount};
    } impl;
    PointerType __addr;
  public:
    
    Il2Class& parse(uintptr_t addr) {
        __addr = addr;
        StructRw<Class_Impl> reader;
        reader.read(addr, impl);
        
        ReadString(impl.class_name(), name, reader);
        ReadString(impl.class_namespace(), __namespace, reader);
        return *this;
    }
    
    Il2Class& parse_method() {
        StructRw<MethodImpl> reader;
        __methods().clear();
        auto count = impl.class_mthcount();
        auto array_addr = impl.class_method();
        uintptr_t my_addr;
        __methods().reserve(count);
        for(vCountType i = 0; i < count; i++) {
            reader.read(array_addr + i * sizeof(PointerType), &my_addr, sizeof(PointerType));
            if(!__methods().emplace_back().parse(my_addr, reader).is_same_class(__addr)) {
                __methods().pop_back();
                break;
            }
        }
        return *this;
    }
    
    Il2Class& parse_field() {
        StructRw<FieldImpl> reader;
        __fields().clear();
        auto count = impl.class_fldcount();
        auto array_addr = impl.class_field();
        uintptr_t my_addr;
        __fields().reserve(count);
        for(vCountType i = 0; i < count; i++) {
            my_addr = array_addr + offset.field_struct_size * i;
            if(!__fields().emplace_back().parse(my_addr, reader).is_same_class(__addr)) {
                __fields().pop_back();
                break;
            }
        }
        return *this;
    }
    
    Il2Class& parse_parent() {
        if(impl.class_parent() != 0) {
            __parent() = std::make_shared<Il2Class>();
            __parent()->parse(impl.class_parent());
        } else {
            throw std::runtime_error("parse_parent : 没有父类了");
        }
        return *this;
    }
    
    const std::vector<Il2Method_t> &get_method_data() const noexcept {
        return __methods();
    }
    
    const std::vector<Il2Filed_t> &get_field_data() const noexcept {
        return __fields();
    }
    
    const Il2Method_t& find_method(const std::string &name) const {
        return *__find_by_name(__methods(), name);
    }
    
    const Il2Method_t& find_method(const std::string &name, uint8_t arg_count) const {
        auto iter = __find_by_name(__methods(), name);
        auto result = __methods().end();
        for(; iter != __methods().end(); iter++) {
            if(iter->get_args_count() == arg_count && name == iter->get_name()) {
                result = iter;
                break;
            }
        }
        if(result == __methods().end()) {
            throw std::runtime_error("il2cpp find : by arg查找method失败");
        }
        
        return *result;
    }
    
    const Il2Filed_t& find_field(const std::string &name) const {
        return *__find_by_name(__fields(), name);
    }
    
    const std::string &get_namespace() const noexcept {
        return __namespace; 
    }
    
    const std::string &get_name() const noexcept {
        return name; 
    }
    
    Il2Class& get_parent() {
        if(__parent == nullptr) {
            throw std::runtime_error("get_parent : 没有父类");
        }
        return *__parent;
    }
    
    PointerType get_addr() const noexcept {
        return __addr;
    }
    
  private:
    using __vIl2Method_t = std::vector<Il2Method_t>;
    using __vIl2Filed_t = std::vector<Il2Filed_t>;
    template<class __Il2_Mth_Fld,
        typename __Tp = std::decay_t<__Il2_Mth_Fld>,
        std::enable_if_t<std::is_same<__Tp, __vIl2Method_t>::value || 
        std::is_same<__Tp, __vIl2Filed_t>::value, int> = 0>
    typename __Il2_Mth_Fld::iterator
    __find_by_name(__Il2_Mth_Fld &mf, const std::string &nm) {
        if(mf.empty()) {
            throw std::runtime_error("il2cpp find : 无法查找空数组");
        }
        
        auto iterator_find = std::find_if(mf.begin(), mf.end(), [&nm](const auto& p){
            return p.get_name() == nm;
        });
        if(iterator_find == mf.end()) {
            throw std::runtime_error("il2cpp find : 没查找到,数组无对应字符串");
        }
        
        return iterator_find;
    }
    
    template<class __Il2_Mth_Fld,
        typename __Tp = std::decay_t<__Il2_Mth_Fld>,
        std::enable_if_t<std::is_same<__Tp, __vIl2Method_t>::value || 
        std::is_same<__Tp, __vIl2Filed_t>::value, int> = 0>
    typename __Il2_Mth_Fld::const_iterator
    __find_by_name(const __Il2_Mth_Fld &mf, const std::string &nm) const {
        if(mf.empty()) {
            throw std::runtime_error("il2cpp find : 无法查找空数组");
        }
        
        auto iterator_find = std::find_if(mf.begin(), mf.end(), [&nm](const auto& p){
            return p.get_name() == nm;
        });
        if(iterator_find == mf.end()) {
            throw std::runtime_error("il2cpp find : 没查找到,数组无对应字符串");
        }
        
        return iterator_find;
    }
};


template<class Il2Offset>
class Il2Assembly : public __MyIl2cppBase<Il2Offset> {
  public:
    using Il2Class_t = Il2Class<Il2Offset>;
    using vIl2Class_t = std::vector<Il2Class_t>;
  private:
    using typename __MyIl2cppBase<Il2Offset>::PointerType;
    using __MyIl2cppBase<Il2Offset>::offset;
    
    DataRef<std::unordered_map<std::string, vIl2Class_t>> __classes;
  public:
    Il2Assembly& parse(uintptr_t array_addr) {
        PointerType addr;
        vNormalRw rw;
        for(size_t i = 0; i < 20000; i++) {
            rw.read(array_addr + i * 8, &addr, sizeof(addr));
            if(addr == 0) continue;
            std::string _name;
            ReadString(rw.read<uintptr_t>(rw.read<uintptr_t>(addr)), 
                _name, rw);
            auto iter = __classes().find(_name);
            if(iter == __classes().end()) {
                if(_name.find(".dll") == std::string::npos) {
                    break;
                }
                __classes().emplace(std::move(_name), vIl2Class_t{}).
                    first->second.
                    emplace_back().
                    parse(addr);
            } else {
                iter->second.emplace_back().parse(addr);
            }
            
        }
        return *this;
    }
    
    std::vector<Il2Class_t>& operator[](const std::string& key) {
        return __classes().at(key);
    }
    
    const std::vector<Il2Class_t>& operator[](const std::string& key) const {
        return __classes().at(key);
    }
    
    
};

template<class Il2Offset = DefaultOffset>
inline typename Il2Assembly<Il2Offset>::vIl2Class_t&
Get(const std::string& dll_str, uintptr_t array_addr = 0) {
    static Il2Assembly<Il2Offset> assembly;
    if(array_addr != 0) {
        assembly.parse(array_addr);
    }
    return assembly[dll_str];
}

template<typename VClass>
inline typename VClass::reference
GetClass(VClass& vclass, const std::string& klass, const std::string& namesp) {
    auto iter = std::find_if(vclass.begin(), vclass.end(), [&klass, &namesp](const auto& v){
        return v.get_name() == klass && v.get_namespace() == namesp;
    });
    
    if(iter == vclass.end()) {    
        throw std::runtime_error("GetClass : 未找到class");
    }
    
    return *iter;
}

}; // namespace Domain

}; // namespace Il2cpp

}; // namespace Mem