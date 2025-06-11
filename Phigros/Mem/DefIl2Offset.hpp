#pragma once
#include <type_traits>

namespace Mem {

namespace Il2cpp {

using u16 = uint16_t;

struct OffsetBase {
    u16 class_name{0x10};
    u16 class_namespace{0x18};
    u16 class_parent{0x58};
    u16 class_static{0xB8};
    u16 class_methods{0x98};
    u16 class_fields{0x80};
    u16 class_mthcount{0x118};
    u16 class_fldcount{0x11C};
    u16 method_name{0x10};
    u16 method_addr{0x0};
    u16 method_class{0x18};
    u16 method_argcount{0x4A};
    u16 field_name{0x0};
    u16 field_class{0x10};
    u16 field_offset{0x18};
    u16 field_struct_size{0x20};
    
    virtual const char *UnityVersoin() = 0;
    virtual ~OffsetBase() = default;
};

namespace detail {
  template<typename T, typename = void>
  struct is_il2cpp_offset : std::false_type {};

  template<typename T>
  struct is_il2cpp_offset<T, 
    std::enable_if_t<std::is_base_of<OffsetBase, std::decay_t<T>>::value>> : std::true_type {};
  
  template<typename T>
  inline constexpr bool is_il2cpp_offset_v = is_il2cpp_offset<T>::value;
};
struct DefaultOffset : OffsetBase {
    const char *UnityVersoin() override {
        return "2019.4.31f1c1";
    }
};

struct PhigrosOffset : OffsetBase {
    PhigrosOffset() {
    
    }
    
    const char *UnityVersoin() override {
        return "2019.4.31f1c1";
    }
};




};

};