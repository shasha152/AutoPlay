
#include "Touch.h"

namespace Touch {

FingerEvent::FingerEvent(uint64_t _time) 
    : time(_time) {}

FingerEvent::~FingerEvent() {

}

HoldDown::HoldDown(uint64_t time) : FingerEvent(time) {}

HoldDown::~HoldDown() {

}

LerpMove::LerpMove(uint64_t time) : FingerEvent(time),
    step(time / step_time) {}

LerpMove::~LerpMove() {

}

Down::Down(uint64_t time) 
    : FingerEvent(time) {}

Down::~Down() {

}

FingerEvent& FingerEvent::bind_finger(const SFinger& _finger) {
    if(_finger->is_down()) {
        throw std::runtime_error("bind_finger : 不能添加了在使用的手指");
    }
    this->finger = _finger;
    return *this;
}

void FingerEvent::start_clock() {
    start = std::chrono::steady_clock::now();
}

uint64_t FingerEvent::get_now_time() {
    return std::chrono::duration_cast<std::chrono::milliseconds
        >(std::chrono::steady_clock::now() - start).count();
}

void FingerEvent::set_event(const std::function<void(uint64_t)> &_event) {
    if(!finger) throw std::logic_error("run : 没有绑定手指");
    while(!m_is_run) { std::this_thread::yield(); }
    
    start_clock();
    uint64_t clock = 0;
    
    while(time >= clock) {
        clock = get_now_time();
        _event(clock);
    }
    m_is_run = false;
    finger->up();
}

void FingerEvent::set_event2(const std::function<void()> &_event) {
    if(!finger) throw std::logic_error("run : 没有绑定手指");
    
    while(!m_is_run) { std::this_thread::yield(); }
    
    start_clock();
    _event();
    m_is_run = false;
    finger->up();
}

void FingerEvent::set_touch(int _x, int _y) {
    if(x1 != _x || y1 != _y) {
        this->x1 = _x;
        this->y1 = _y;
        is_update.store(true, std::memory_order_release);
    }
    
    m_is_run = true;
}

bool FingerEvent::is_run() const noexcept {
    return m_is_run;
}

void Down::run() {
    set_event2([this](){
        finger->down(x1, y1);
    });
}

void HoldDown::run() {
    set_event([this](uint64_t now){
        if(is_update.load(std::memory_order_acquire)) {
            finger->down(x1, y1);
            is_update.store(false);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    });
}

void LerpMove::set_add_touch(int x_, int y_) {
    x2 = x_;
    y2 = y_;
    
}

void LerpMove::run() {
    set_event([this](uint64_t now){
        float ver = (float)now / time;
        float m_ver = std::clamp(ver, 0.0f, 1.0f);
        int x = (x2) * m_ver;
        int y = (y2) * m_ver;
        finger->down(x1 + x, y1 + y);
        std::this_thread::sleep_for(std::chrono::milliseconds(time / step));
    });
}



};