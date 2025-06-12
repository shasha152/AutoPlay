#pragma once

#include <iostream>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <map>

namespace Phigros {

template<typename T>
class sendMessage;

template<typename T>
struct sendMg {
    std::atomic<bool> is_finish;
    T pos;
    
    sendMg(bool _is_finish, T _pos) : is_finish(_is_finish), pos(_pos) {}
};

template<typename T>
class recvMessage {
    friend class sendMessage<T>;
    std::multimap<float, std::shared_ptr<sendMg<T>>> m_pos_mes;
    
    std::condition_variable condition;
    mutable std::mutex mutex;
  public:
    bool has_message() const;
    const T& get_message() const;
    
    template<typename Fun, typename... Args>
    void handle(Fun, Args&&...);
    
};

template<typename T>
class sendMessage {
    std::shared_ptr<recvMessage<T>> m_recv;
    std::shared_ptr<sendMg<T>> m_mg;
    
  public:
    sendMessage(std::shared_ptr<recvMessage<T>>&);
    const T& get() const;
    
    void send(float, const T&);
};

template<typename T>
bool recvMessage<T>::has_message() const {
    return !m_pos_mes.empty();
}

template<typename T>
const T& recvMessage<T>::get_message() const {
    return m_pos_mes.begin()->second->pos;
}

template<typename T>
template<typename Fun, typename... Args>
void recvMessage<T>::handle(Fun fun, Args&&... args) {
    auto Vec2 = fun(std::forward<Args>(args)...);
    std::unique_lock<std::mutex> lock(mutex);
    
    auto& ms = m_pos_mes.begin()->second;
    ms->pos = Vec2;
    ms->is_finish.store(true);
    condition.notify_all();
    m_pos_mes.erase(m_pos_mes.begin());
}


template<typename T>
sendMessage<T>::sendMessage(std::shared_ptr<recvMessage<T>>& rc) :
    m_recv(rc), m_mg(std::make_shared<sendMg<T>>(false, T())) {

}

template<typename T>
void sendMessage<T>::send(float key, const T& data) {
    m_mg->is_finish = false;
    m_mg->pos = data;
    std::unique_lock<std::mutex> lock(m_recv->mutex);
    m_recv->m_pos_mes.emplace(key, m_mg);
    
    
}

template<typename T>
const T& sendMessage<T>::get() const {
    std::unique_lock<std::mutex> lock(m_recv->mutex);
    m_recv->condition.wait(lock, [this](){
        std::this_thread::yield();
        return m_mg->is_finish.load(std::memory_order_acquire);
    });
    
    return m_mg->pos;
}

};