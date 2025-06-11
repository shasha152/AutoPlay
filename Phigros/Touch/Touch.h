#pragma once

#include "ThreadPool.h"

#include <chrono>
#include <memory>
#include <atomic>
#include <functional>

#include <linux/input.h>

namespace Touch {

using TouchSrceen = int;
inline constexpr uint8_t FingerMaxNumber = 10;

TouchSrceen Create();

class FingerEvent;
class Finger;

using SFinger = std::shared_ptr<Finger>;
using SEvent = std::shared_ptr<FingerEvent>;

class Finger {
    static inline std::atomic<int> curr_x = -1;
    static inline std::atomic<int> curr_y = -1;
    
    bool isdown;
    uint16_t slot;
    TouchSrceen fd;
    
    std::vector<input_event> inputs;
    static inline int g_slot{-1};
    static inline ThreadPool pool{FingerMaxNumber};
    static inline std::mutex* mutex{new std::mutex()};
  public:
    Finger(TouchSrceen _fd, uint16_t _slot) : isdown(false), fd(_fd), slot(_slot) {}
    void down(int x, int y);
    void up();
    
    bool is_down() const noexcept { return isdown; }
    static Finger& bind_event(const SEvent& fevent, const SFinger& finger);
    
    int get_x() const;
    int get_y() const;
};

class FingerEvent {
  protected:
    std::chrono::steady_clock::time_point start;
    SFinger finger;
    uint64_t time;
    bool m_is_run{false};
    int x1, y1;
    std::atomic<bool> is_update{false};
    
    virtual ~FingerEvent();
    
    void start_clock();
    uint64_t get_now_time();
    
    void set_event(const std::function<void(uint64_t)> &_event);
    void set_event2(const std::function<void()> &_event);
  public:
    virtual void run() = 0;
    void set_touch(int _x, int _y);
    
    FingerEvent& bind_finger(const SFinger& _finger);
    FingerEvent(uint64_t _time);
    
    bool is_run() const noexcept;
    bool is_down() { return finger && finger->is_down(); }
};

class Down : public FingerEvent {
  public:
    void run() override;
    
    Down(uint64_t time = 0);
    ~Down();
};

class HoldDown : public FingerEvent {
  public:
    
    void run() override;
    
    HoldDown(uint64_t time = 0);
    ~HoldDown();
};

class LerpMove : public FingerEvent {
    int x2, y2;
    
    int step;
    static constexpr int step_time = 15;
  public:
    void set_add_touch(int x_, int y_);
    
    void run() override;
    LerpMove(uint64_t time);
    ~LerpMove();
};

SFinger CreateFinger(TouchSrceen fd, int slot);

template<typename Event, typename... Args>
std::shared_ptr<Event> CreateEvent(Args&&... args) {
    static_assert(std::is_base_of<FingerEvent, Event>::value,
        "事件类型必须继承FingerEvent");
    return std::make_shared<Event>(std::forward<Args>(args)...);
}

template<typename Event>
std::pair<SFinger, std::shared_ptr<Event>> 
StartEvent(TouchSrceen fd, uint64_t time, int slot) {
    auto v1 = CreateFinger(fd, slot);
    auto v2 = CreateEvent<Event>(time);
    
    Finger::bind_event(v2, v1);
    return std::make_pair(v1, v2);
}

};