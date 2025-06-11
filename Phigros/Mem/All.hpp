#ifndef _IL2CPP_ALL_HPP_
#define _IL2CPP_ALL_HPP_

#include "Il2cppClass.hpp"
#include "Il2cppCall.hpp"

#ifdef IMGUI_VERSION

Mem::Il2cpp::Vector2(const ImVec2& im) :
    x(im.x), y(im.y) {}

#endif

#endif