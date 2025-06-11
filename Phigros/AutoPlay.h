#pragma once

#include "Touch/Touch.h"
#include "LevelControl.h"
#include "Message.h"

#include <list>

namespace Phigros {

using EventIterator = Il2List<JudgeLineEventOffsets>::iterator;

struct Ph_Event {
    Touch::SEvent event;
    float posX;
    float time;
    int index;
    NoteType type;
    
    bool is_add_list{false};
};

struct Pos_Message {
    int index;
    float time;
    Vector2 pos;
    
    Pos_Message& operator=(const Vector2& _pos) {
        pos = _pos;
        return *this;
    }
};


class LinePosControl {
    EventIterator start;
    EventIterator end;
    
    float m_time;
  public:
    LinePosControl(EventIterator _start, EventIterator _end) : start(_start), end(_end) {}
    
    Vector2 get_pos();
    void update_index(float time);
};


class AutoPlay {
    std::vector<Touch::SFinger> m_fingers;
    
    std::queue<Ph_Event> m_events;
    std::list<Ph_Event> m_hold_events;
    
    std::vector<EventIterator> curr_move_iter;
    std::vector<EventIterator> end_move_iter;
    
    std::vector<LinePosControl> m_lineposc;
    
    std::shared_ptr<recvMessage<Pos_Message>> m_recvPos;
    
    LevelControl* m_level;
  public:
    void CreateSongEvent();
    bool Play();
    
    void setTargetLevel(LevelControl* level) noexcept;
    void Destroy();
    AutoPlay(Touch::TouchSrceen src);
    ~AutoPlay();
  private:
    Touch::SFinger& getFinger();
    
    void setEventTouch(Ph_Event& event);
    size_t getAllNoteNumber();
    Vector2 getNotePos(float x, int index);
    Vector2 getNotePos(const Ph_Event& event);
    
    void Update(float time);
};

};