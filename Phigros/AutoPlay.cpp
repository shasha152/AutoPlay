#include "AutoPlay.h"
#include <cmath>

#include <iostream>

namespace Phigros {

inline Vector2 rotatePoint(const Vector2& original, const Vector2& center, float theta) {
    float theta_radians = theta * M_PI / 180.0f;
    float translatedX = original.x - center.x;
    float translatedY = original.y - center.y;
    float cosTheta = cos(theta_radians);
    float sinTheta = sin(theta_radians);
    float rotatedX = translatedX * cosTheta - translatedY * sinTheta;
    float rotatedY = translatedX * sinTheta + translatedY * cosTheta;

    rotatedX += center.x;
    rotatedY += center.y;

    return Vector2{rotatedX, rotatedY};
}

class FlickEvent : public Touch::FingerEvent {
    sendMessage<Pos_Message> m_send;
    int m_index;
    float m_time;
    
    struct time_and_x {
        float x;
        float time;
    };
    std::vector<time_and_x> m_pos_array;
  public:
    void run() override {
        set_event2([this](){
            for(auto& x : m_pos_array) {
                m_send.send(x.time, Pos_Message{m_index, x.time, Vector2(x.x, 0.f)});
                auto vec2 = m_send.get().pos;
                
                finger->down(Il2Camera::display_y - vec2.y, vec2.x);
        }});
    }
    
    void add(float x, float time) {
        m_pos_array.emplace_back(time_and_x{x, time});
    }
    
    FlickEvent(std::shared_ptr<recvMessage<Pos_Message>>& recv, int index, float time) :
        FingerEvent(0), m_send(recv), m_index(index), m_time(time) {}
};


template<typename Iterator>
class EventGenerate {
    Iterator m_start;
    Iterator m_end;
    
    static constexpr float max_cmp_x = 1.2f;
    static constexpr float max_cmp_y = 0.4f;
    
    std::unordered_map<Iterator, bool> tmp_is_handle;
  public:
    EventGenerate(Iterator start, Iterator end) : 
        m_start(start), m_end(end) {
        for(auto it = start; it != end; it++) {
            tmp_is_handle.emplace(it, false);
        }
    }
    Ph_Event operator()(int index, std::shared_ptr<recvMessage<Pos_Message>>& recv) {
        auto start = m_start;
        Touch::SEvent event = nullptr;
        if(tmp_is_handle.at(start) == true) {
            m_start++;
            return make_event(event, *start, index);
        }
        switch(start->type()) {
            case Note_Click:
            case Note_Flick:
            {
                auto iter = is_has_flick(start);
                if(iter != m_end) {
                    auto fevent = Touch::CreateEvent<FlickEvent>(recv, index, start->realTime());
                    fevent->add(start->positionX(), start->realTime());
                    auto last_iter = start;
                    while(iter != m_end) {
                        if((std::abs(iter->positionX() - last_iter->positionX())) > max_cmp_x / 2.0f) {
                            float local_time = last_iter->realTime() + (iter->realTime() - last_iter->realTime()) / 2.0f;
                            float local_x = (iter->positionX() - last_iter->positionX()) / 2.0f;
                            fevent->add((last_iter->positionX() + local_x), local_time);
                        }
                        fevent->add(iter->positionX(), iter->realTime());
                        last_iter = iter;
                        iter = is_has_flick(iter);
                    }
                    event = std::move(fevent);
                } else {
                    event = Touch::CreateEvent<Touch::Down>(0);
                }
                
                break;
            }
            case Note_Hold:
            {
                uint64_t hold_time = start->holdTime() * 1e3;
                event = Touch::CreateEvent<Touch::HoldDown>(hold_time);
                break;
            }
            case Note_Drag:
            {
                auto vevent = Touch::CreateEvent<Touch::LerpMove>(75);
                vevent->set_add_touch(105, 0);
                event = std::move(vevent);
                break;
            }
            case Note_None:
            {
                break;
            }
        }
        tmp_is_handle[start] = true;
        m_start++;
        return make_event(event, *start, index);
    }
  private:
    //想写个模拟真实的黄键接住的算法，太麻烦就不写了(其实写不出来...)
    Iterator is_has_flick(Iterator start) {
        const float start_time = start->realTime();
        const float start_x = start->positionX();
    
        for(auto iter = start + 1; iter != m_end; ++iter) {
            const float tm = std::abs(iter->realTime() - start_time);
            const float fx = std::abs(iter->positionX() - start_x);
        
            if(tm > max_cmp_y)
                break;
            if(fx < max_cmp_x && iter->type() == Note_Flick && !tmp_is_handle[iter]) {
                tmp_is_handle[iter] = true;
                return iter;
            }
        }
        return m_end;
    }

    
    Ph_Event make_event(Touch::SEvent& event, const NoteChartOffsets& note, int index) {
        return Ph_Event{
            .event = std::move(event),
            .posX = note.positionX(),
            .time = note.realTime(),
            .index = index,
            .type = note.type()
        };
    }
};

Vector2 LinePosControl::get_pos() {
    if(start == end) return Vector2(0.f, 0.f);
        
    if(m_time < start->startTime()) {
        return Vector2(start->start(), start->start2());
    } else if(m_time >= start->endTime()) {
        return Vector2(start->end(), start->end2());
    } else {
        float duration = start->endTime() - start->startTime();
        float t = std::clamp((m_time - start->startTime()) / duration, 0.0f, 1.0f); //归一化时间 [0, 1]
        float x = start->start() + t * (start->end() - start->start());
        float y = start->start2() + t * (start->end2() - start->start2());
        return Vector2(x, y);
    }
}

void LinePosControl::update_index(float time) {
    m_time = time;
    if(start == end) return;
        
    while(start != end && time > start->endTime()) {
        start++;
    }
}

AutoPlay::AutoPlay(Touch::TouchSrceen src) {
    m_fingers.reserve(Touch::FingerMaxNumber);
    for(int i = 0; i < Touch::FingerMaxNumber; i++)
        m_fingers.push_back(Touch::CreateFinger(src, i));
    
    m_recvPos = std::make_shared<recvMessage<Pos_Message>>();
}

AutoPlay::~AutoPlay() {
    Destroy();
}

void AutoPlay::setTargetLevel(LevelControl* level) noexcept {
    m_level = level;
    
    const auto& lines = m_level->get_progress().
        get_judge().get_lines();
    curr_move_iter.reserve(lines.size());
    end_move_iter.reserve(lines.size());
    for(const auto& line : lines) {
        curr_move_iter.push_back(line.get_events().begin());
        end_move_iter.push_back(line.get_events().end());
    }
    
}

void AutoPlay::CreateSongEvent() {
    if(!m_level) throw std::logic_error("AutoPlay : 未设置目标Level");
    
    const auto& lines = m_level->get_progress().
        get_judge().get_lines();
    
    std::vector<Ph_Event> sortEvent;
    sortEvent.reserve(getAllNoteNumber());
    auto gevent = [this, &sortEvent](const auto& notes, int index) {
        EventGenerate generate(notes.begin(), notes.end());
        for(const auto& n : notes) {
            auto event = generate(index, m_recvPos);
            if(event.event) sortEvent.push_back(std::move(event));
        }
    };
    
    for(size_t i = 0; i < lines.size(); i++) {
        const auto& line = lines[i];
        m_lineposc.emplace_back(line.get_events().begin(),
            line.get_events().end());
        gevent(line.get_above(), i);
        gevent(line.get_below(), i);
    }
    
    std::sort(sortEvent.begin(), sortEvent.end(), [](const auto& e, const auto& b){
        return e.time < b.time;
    });
    
    float time = m_level->get_progress().get_time();
    for(size_t i = 0; i < sortEvent.size(); i++) {
        if(time < sortEvent[i].time)
        m_events.push(std::move(sortEvent[i]));
    }
     
}

bool AutoPlay::Play() {
    if(!m_level) throw std::logic_error("AutoPlay : 未设置目标Level");
    
    const auto& pro = m_level->get_progress();
    float time = pro.get_time();
    Update(time);
    //手指的绑定事件
    if(!m_events.empty()) {
        
        auto& event = m_events.front();
        if(time >= event.time) {
            setEventTouch(event);
            Touch::Finger::bind_event(event.event, getFinger());
            m_events.pop();
        }
    }
    return !m_events.empty();
}

void AutoPlay::Destroy() {
    curr_move_iter.clear();
    end_move_iter.clear();
    m_hold_events.clear();
    m_lineposc.clear();
    
    while(!m_events.empty()) m_events.pop();
    
    m_level = nullptr;
}

void AutoPlay::Update(float time) {
    
    for(auto& line : m_lineposc)
        line.update_index(time);
    
    //处理连续的flick键
    if(m_recvPos->has_message()) {
        const auto& data = m_recvPos->get_message();
        if(time >= data.time)
        m_recvPos->handle([this](float x, int index) -> Vector2 {
            return getNotePos(x, index);
        }, data.pos.x, data.index);
    }
    
    //处理移动的hold键
    for(auto it = m_hold_events.begin(); it != m_hold_events.end();) {
        if(!it->event->is_run()) {
            it = m_hold_events.erase(it);
        } else {
            setEventTouch(*it);
            it++;
        }
    }
}

void AutoPlay::setEventTouch(Ph_Event& event) {
    auto note_src = getNotePos(event);
    
    switch(event.type) {
        case Note_Click:
        case Note_Flick:
        case Note_Drag:
            event.event->set_touch(Il2Camera::display_y - note_src.y, note_src.x);
            break;
        case Note_Hold:
            event.event->set_touch(Il2Camera::display_y - note_src.y, note_src.x);
            if(event.is_add_list == false) {
                m_hold_events.emplace_back(event).is_add_list = true;
            }
            break;
        default:
            break;
    }
}

Touch::SFinger& AutoPlay::getFinger() {
    auto iter = std::find_if(m_fingers.begin(), m_fingers.end(), [](const auto& finger){
        return !finger->is_down();
    });
    if(iter == m_fingers.end())
        throw std::runtime_error("未找到可用的手指");
    return *iter;
}

size_t AutoPlay::getAllNoteNumber() {
    const auto& lines = m_level->get_progress().
        get_judge().get_lines();
    
    size_t res = 0;
    for(const auto& line : lines) {
        res += line.get_above().size();
        res += line.get_below().size();
    }
    
    return res;
}

Vector2 AutoPlay::getNotePos(float x, int index) {
    const auto& cam = m_level->get_cam();
    const auto& line = m_level->get_progress().
        get_judge().get_lines().at(index);
    //获取notechart的屏幕坐标
    auto line_pos = m_lineposc.at(index).get_pos();
    Vector2 note_pos(x + line_pos.x, line_pos.y);
    auto note_z_pos = rotatePoint(note_pos, line_pos, line.get_theta());
    auto note_src = cam.ToScreen(note_z_pos);
    
    return note_src;
}

Vector2 AutoPlay::getNotePos(const Ph_Event& event) {
    return getNotePos(event.posX, event.index);
}


};