#pragma once

#include "Array.hpp"
#include "Il2cppStruct.hpp"

namespace Mem {

namespace Il2cpp {

namespace detail {
  template<size_t __1, size_t __2>
  struct pow {
      static_assert(__2 >= 0, "pow : 参数二不能为负");
      static constexpr size_t value = __1 * pow<__1, __2 - 1>::value;
  };
  
  template<size_t __1>
  struct pow<__1, static_cast<size_t>(1)> {
      static constexpr size_t value = __1;
  };
  
  template<size_t __1>
  struct pow<__1, static_cast<size_t>(0)> {
      static constexpr size_t value = 1;
  };
    
  template<size_t __1, size_t __2>
  inline constexpr size_t pow_v = pow<__1, __2>::value;
  
};

struct ImVec2;
struct Vector2 {
    float x, y;
    Vector2(float _x = 0.f, float _y = 0.f) :
        x(_x), y(_y) {}
    Vector2(const ImVec2& im);
};

template<typename OffsetStruct, typename Reader =
    std::conditional_t<!std::is_same<OffsetStruct, uintptr_t>::value, StructRw<OffsetStruct>, vNormalRw>>
class Il2List : public Array<OffsetStruct> {
    struct __my_list {
        uintptr_t addr{1};
        int size{0};
    } __list;
    Reader __reader_;
  public:
    Il2List() = default;
    explicit Il2List(uintptr_t addr) { refresh(addr); }
    Il2List& refresh(uintptr_t addr) {
        __reader_.read(addr + 0x10, &__list, sizeof(__my_list));
        __up_value();
        return *this;
    }
  private:
    class __list_array {
        int __i = 0;
        uintptr_t __addr;
        
        Reader& reader;
        static constexpr int __max = detail::pow_v<2, 15>;
      public:
        explicit __list_array(uintptr_t addr, Reader& __reader) : __addr(addr), reader(__reader) {}
        OffsetStruct& operator()(OffsetStruct& __value) {
            if constexpr (std::is_same<OffsetStruct, uintptr_t>::value) {
                reader.read(__addr + __i * sizeof(long) + 0x20, &__value, sizeof(long));
            } else {
                uintptr_t __temp; 
                reader.read(__addr + __i * sizeof(long) + 0x20, &__temp, sizeof(long));
                reader.read(__temp, __value);
            }
            __i++;
            if(__i > __max) throw std::runtime_error("Il2List : 读取超出限制");
            return __value;
        }
        int get() noexcept {
            return __i;
        }
    };
    
    void __up_value() {
        this->resize(__list.size);
        __list_array __array(__list.addr, __reader_);
        while(__list.size > __array.get()) {
            __array(this->operator[](__array.get()));
        }
    }
};

template<typename OffsetStruct, typename Reader =
    std::conditional_t<std::is_class<OffsetStruct>::value, StructRw<OffsetStruct>, vNormalRw>>
class Il2ObjectArray : public Array<OffsetStruct> {
    int m_size;
    class __reader_nm {
        Reader __reader;
        
        Il2ObjectArray& __self;
        uintptr_t __addr;
        
        static constexpr int __max = detail::pow_v<2, 15>;
        static constexpr bool __is = std::is_class<OffsetStruct>::value;
      public:
        explicit __reader_nm(Il2ObjectArray& self) :
            __self(self)  {
        
        }
        
        void operator()(size_t i, OffsetStruct& __value) {
            if constexpr (__is) {
                uintptr_t temp;
                __reader.read(__addr + 0x20 + i * 8, &temp, sizeof(temp));
                __reader.read(temp, __value);
            } else {
                __reader.read(__addr + 0x14 + i * sizeof(OffsetStruct), &__value, sizeof(OffsetStruct));
            }
        }
        
        void set_addr(uintptr_t addr) {
            __addr = addr;
            if constexpr (__is) {
                __reader.read(addr + 0x18, &__self.m_size, 4);
            } else { __reader.read(addr + 0x10, &__self.m_size, 4); }
            if(__self.m_size > __max) {
                throw std::runtime_error("Il2ObjectArray : 读取超出限制");
            }
        }
    } m_reader{*this};
    
  public:
    Il2ObjectArray() = default;
    Il2ObjectArray& refresh(uintptr_t addr) {
        m_reader.set_addr(addr);
        this->resize(m_size);
        for(int i = 0; i < m_size; i++) {
            m_reader(i, this->operator[](i));
        }
        return *this;
    }
    
    
    explicit Il2ObjectArray(uintptr_t addr) {
        refresh(addr);
    }
};

template<>
class Il2ObjectArray<uintptr_t, vNormalRw> : public Array<uintptr_t> {
    int m_size;
    
    class __reader_nm {
        vNormalRw m_rw;
        Il2ObjectArray& m_self;
        
        uintptr_t m_addr;
      public:
        explicit __reader_nm(Il2ObjectArray& self) :
            m_self(self)  {
        }
        void operator()(size_t i, uintptr_t& __value) {
            m_rw.read(m_addr + 0x20 + i * 8, &__value, sizeof(uintptr_t));
        }
        
        void set_addr(uintptr_t addr) {
            m_addr = addr;
            m_rw.read(addr + 0x18, &m_self.m_size, 4);
        }
    } m_reader{*this};
    
  public:
    Il2ObjectArray() = default;
    Il2ObjectArray& refresh(uintptr_t addr) {
        
        m_reader.set_addr(addr);
        this->resize(m_size);
        for(int i = 0; i < m_size; i++) {
            m_reader(i, this->operator[](i));
        }
        return *this;
    }
    explicit Il2ObjectArray(uintptr_t addr) {
        refresh(addr);
    }
};

//暂时没有实现
class Il2String {
    std::u16string m_data;
    std::string m_str;
  public:
    
    const std::string& str() const noexcept {
        return m_str;
    }
    
    std::string& str() noexcept {
        return m_str;
    }
};

class Il2Camera {
  public:
    static inline int display_x = 0;
    static inline int display_y = 0;
  private:
    float matrix[16];
    
    class creader {
        vNormalRw m_rw;
        Il2Camera& m_cam;
      public:
        creader(Il2Camera& cam) : m_cam(cam) {}
        void operator()(uintptr_t addr) {
            uintptr_t temp;
            m_rw.read(addr + 0x10, &temp, sizeof(uintptr_t));
            m_rw.read(temp + 0xDC, m_cam.matrix, sizeof(m_cam.matrix));
        }
    } m_reader{*this};
  public:
    explicit Il2Camera(uintptr_t addr) {
        refresh(addr);
    }
    Il2Camera() = default;
    Il2Camera& refresh(uintptr_t addr) {
        
        m_reader(addr);
        return *this;
    }
    
    Vector2 ToScreen(const Vector2 &world_pos) const {
        float w = matrix[3] * world_pos.x + matrix[7] * world_pos.y + matrix[15];
        float camera_z = w + matrix[14] * 0.001f;
        float ndc_x = (matrix[0] * world_pos.x + matrix[4] * world_pos.y + matrix[12]) / camera_z;
        float ndc_y = (matrix[1] * world_pos.x + matrix[5] * world_pos.y + matrix[13]) / camera_z;
        float x = (ndc_x + 1) * display_x / 2;
        float y = (ndc_y + 1) * display_y / 2;
        return Vector2(x, display_y - y);
    }
    
    
};


}; // namespace Il2cpp

};