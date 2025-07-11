#include "LevelControl.h"

#define OFFSET_TO_INT(val) reinterpret_cast<Mem::OffsetValue<int>*>(&val)

namespace Phigros {

using namespace Mem::Il2cpp::Domain;
using Il2cppClass = Il2Class<Mem::Il2cpp::PhigrosOffset>;

template<typename Dlls>
Il2cppClass& AtClass(Dlls& dll, const std::string& klass, const std::string& names = "") {
    
    auto& adll = dll["Assembly-CSharp.dll"];
    
    auto& mclass = GetClass(adll, klass, names);
    if(mclass.get_field_data().empty()) {
        mclass.parse_field();
    }
    return mclass;
}

void __Il2OffsetValueFieldSet(Mem::OffsetValue<int>*, Il2cppClass&) {}

template<typename T, typename... Args>
void __Il2OffsetValueFieldSet(Mem::OffsetValue<int>* offval, Il2cppClass& klass, T one, Args... fields) {
    static_assert(std::is_same<T, const char*>::value,
        "参数必须为const char*");
        
    uint16_t offset = klass.find_field(one).get_offset();
    offval->set_offset(offset);
    __Il2OffsetValueFieldSet(offval + 1, klass, fields...);
}

template<typename... Args>
void Il2OffsetValueFieldSet(Mem::OffsetValue<int>* offval, Il2cppClass& klass, Args... fields) {
    __Il2OffsetValueFieldSet(offval, klass, fields...);
}


// LevelControl
LevelControl::LevelControl() {
    auto& levelclass = AtClass(assembly, "LevelControl");
    Il2OffsetValueFieldSet(OFFSET_TO_INT(m_date), levelclass, "progressControl", "backgroundCamera", "screenW", "screenH");
}

void LevelControl::update() {
    if(m_addr <= 0x800000) {
        throw std::runtime_error("LevelControl : m_addr");
    }
    if(is_init == false) {
        m_rw.read(m_addr, m_date);
        Il2Camera::display_x = m_date.screenW();
        Il2Camera::display_y = m_date.screenH();
        m_progress.set_addr(m_date.ProgressControl());
        
        is_init = true;
    }
    //std::cout << "1\n";
    m_cam.refresh(m_date.Camera());
    m_progress.update();
}

const Il2Camera& LevelControl::get_cam() const {
    return m_cam;
}

const ProgressControl& LevelControl::get_progress() const {
    return m_progress;
}

ProgressControl& LevelControl::get_progress() {
    return m_progress;
}

// ProgressControl
ProgressControl::ProgressControl() {
    auto& progress_class = AtClass(assembly, "ProgressControl");
    Il2OffsetValueFieldSet(OFFSET_TO_INT(m_date), progress_class, "judgeControl", "nowTime");
}

void ProgressControl::update() {
    if(m_addr <= 0x800000) {
        throw std::runtime_error("ProgressControl : m_addr");
    }
    
    if(m_date.JudgeControl() == 0) {
        m_rw1.read(m_addr, m_date.JudgeControl);
        m_judge.set_addr(m_date.JudgeControl());
    }
    m_rw2.read(m_addr, m_date.nowTime);
    m_judge.update();
}

const JudgeControl& ProgressControl::get_judge() const {
    return m_judge;
}

JudgeControl& ProgressControl::get_judge() {
    return m_judge;
}

const float ProgressControl::get_time() const {
    return m_date.nowTime();
}

const bool ProgressControl::get_leave() const {
    return m_date.leave();
}

// JudgeControl
JudgeControl::JudgeControl() {
    auto& judge_class = AtClass(assembly, "JudgeControl");
    Il2OffsetValueFieldSet(OFFSET_TO_INT(m_date), judge_class, "judgeLineControls");
}

void JudgeControl::update() {
    if(m_addr <= 0x800000) {
        throw std::runtime_error("JudgeControl : m_addr");
    }
    
    if(is_init == false) {
        rw.read(m_addr, m_date);
        m_judgeslist.refresh(m_date.judgeLineControls());
    
        for(uintptr_t addr : m_judgeslist) {
            m_judges.emplace_back().set_addr(addr);
        }
        is_init = true;
    }
    //std::cout << "3\n";
    for(auto& judge : m_judges) {
        judge.update();
    }
}

const std::vector<LineControl>& JudgeControl::get_lines() const {
    return m_judges;
}

std::vector<LineControl>& JudgeControl::get_lines() {
    return m_judges;
}


// LineControl
LineControl::LineControl() {
    auto& line_class = AtClass(assembly, "JudgeLineControl");
    Il2OffsetValueFieldSet(OFFSET_TO_INT(m_date), line_class, "theta", "judgeLineMoveEvents", "notesAbove", "notesBelow");
}

void LineControl::update() {
    if(m_addr <= 0x800000) {
        throw std::runtime_error("LineControl : m_addr");
    }
    
    if(is_init == false) {
        m_rw1.read(m_addr, m_date);
        Events.refresh(m_date.judgeLineMoveEvents());
        
        m_above.refresh(m_date.notesAbove());
        m_below.refresh(m_date.notesBelow());
        is_init = true;
    }
    
    m_rw2.read(m_addr, m_date.theta);
}

const Il2List<JudgeLineEventOffsets>& LineControl::get_events() const {
    return Events;
}

Il2List<JudgeLineEventOffsets>& LineControl::get_events() {
    return Events;
}

const float LineControl::get_theta() const {
    return m_date.theta();
}


const Il2List<NoteChartOffsets>& LineControl::get_above() const {
    return m_above;
}

const Il2List<NoteChartOffsets>& LineControl::get_below() const {
    return m_below;
}

};

#undef OFFSET_TO_INT