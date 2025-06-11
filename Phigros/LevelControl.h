#pragma once

#include "Phigros.h"

namespace Phigros {

using namespace Mem::Il2cpp;

enum NoteType : int {
    Note_None = 0,
    Note_Click = 1,
    Note_Flick = 2,
    Note_Hold = 3,
    Note_Drag = 4
};

struct LevelControlOffsets {
    Mem::OffsetValue<uintptr_t> ProgressControl;
    Mem::OffsetValue<uintptr_t> Camera;
    Mem::OffsetValue<float> screenW;
    Mem::OffsetValue<float> screenH;
};

struct ProgressControlOffsets {
    Mem::OffsetValue<uintptr_t> JudgeControl;
    Mem::OffsetValue<float> nowTime;
    Mem::OffsetValue<bool> leave;
};

struct JudgeControlOffset {
    Mem::OffsetValue<uintptr_t> judgeLineControls;
    
};

struct JudgeLineOffsets {
    Mem::OffsetValue<float> theta;
    Mem::OffsetValue<uintptr_t> judgeLineMoveEvents;
    Mem::OffsetValue<uintptr_t> notesAbove;
    Mem::OffsetValue<uintptr_t> notesBelow;
};

//因为偏移不太可能变就写死了
struct NoteChartOffsets {
    Mem::OffsetValue<NoteType> type{0x10};
    Mem::OffsetValue<float> positionX{0x18};
    Mem::OffsetValue<float> holdTime{0x1C};
    Mem::OffsetValue<bool> isJudged{0x28};
    Mem::OffsetValue<float> realTime{0x2C};
};

struct JudgeLineEventOffsets {
    Mem::OffsetValue<float> startTime{0x10};
    Mem::OffsetValue<float> endTime{0x14};
    Mem::OffsetValue<float> start{0x18};
	Mem::OffsetValue<float> end{0x1C};
	Mem::OffsetValue<float> start2{0x20};
	Mem::OffsetValue<float> end2{0x24};
	
};

class LineControl : public PhigrosBase {
    bool is_init{false};
    
    Il2List<NoteChartOffsets> m_above;
    Il2List<NoteChartOffsets> m_below;
    Il2List<JudgeLineEventOffsets> Events;
    
    JudgeLineOffsets m_date;
    Mem::StructRw<JudgeLineOffsets> m_rw1;
    Mem::OffsetRw<Mem::OffsetValue<float>> m_rw2;
  public:
    LineControl();
    void update() override;
    
    const Il2List<JudgeLineEventOffsets>& get_events() const;
    Il2List<JudgeLineEventOffsets>& get_events();
    const float get_theta() const;
    
    const Il2List<NoteChartOffsets>& get_above() const;
    const Il2List<NoteChartOffsets>& get_below() const;
};

class JudgeControl : public PhigrosBase {
    Il2List<uintptr_t> m_judgeslist;
    std::vector<LineControl> m_judges;
    
    Mem::StructRw<JudgeControlOffset> rw;
    JudgeControlOffset m_date;
    bool is_init{false};
  public:
    JudgeControl();
    void update() override;
    
    const std::vector<LineControl>& get_lines() const;
    std::vector<LineControl>& get_lines();
};
class ProgressControl : public PhigrosBase {
    JudgeControl m_judge;
    
    ProgressControlOffsets m_date;
    Mem::OffsetRw<Mem::OffsetValue<uintptr_t>> m_rw1;
    Mem::OffsetRw<Mem::OffsetValue<float>> m_rw2;
  public:
    ProgressControl();
    void update() override;
    
    const JudgeControl& get_judge() const;
    JudgeControl& get_judge();
    const float get_time() const;
    const bool get_leave() const;
};

class LevelControl : public PhigrosBase {
    Il2Camera m_cam;
    ProgressControl m_progress;
    
    LevelControlOffsets m_date;
    Mem::StructRw<LevelControlOffsets> m_rw;
    bool is_init{false};
  public:
    LevelControl();
    void update() override;
    
    const Il2Camera& get_cam() const;
    const ProgressControl& get_progress() const;
    ProgressControl& get_progress();
};







};