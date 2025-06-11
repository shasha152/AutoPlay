#include "Phigros.h"
#include "AutoPlay.h"

#include <chrono>
#include <iostream>

namespace Phigros {

using namespace Mem;

inline Touch::TouchSrceen touch = -1;
inline constexpr double TARGET_FPS = 400.0;
inline constexpr std::chrono::duration<double, std::milli> FRAME_TIME(1000.0 / TARGET_FPS);

void PhigrosBase::init(uintptr_t array_addr) {
    assembly.parse(array_addr);
    
    auto& mdll = assembly["mscorlib.dll"];
    StringAddr = Mem::Il2cpp::Domain::GetClass(mdll, "String", "System").get_addr();
    GetTypeAddr = Mem::Il2cpp::Domain::GetClass(mdll, "Type", "System").parse_method().find_method("GetType", 1).get_addr();
    
    auto& udll = assembly["UnityEngine.CoreModule.dll"];
    FindObjectsAddr = Mem::Il2cpp::Domain::GetClass(udll, "Object", "UnityEngine").parse_method().find_method("FindObjectsOfType", 1).get_addr();
    
}

uintptr_t PhigrosBase::GetUString(const std::u16string& str) {
    static std::mutex mutex;
    
    std::lock_guard<std::mutex> lock(mutex);
    auto iter = UnityString.find(str);
    
    if(iter == UnityString.end()) {
        UnityString.emplace(str, Il2cpp::Call::CreateString(str, StringAddr));
    }
    
    return UnityString[str];
}

void PhigrosBase::set_addr(uintptr_t addr) noexcept { m_addr = addr; }

void Init(const char *package) {
    if(Mem::init(package) == 0)
        throw std::runtime_error("未找到pid");
    vNormalRw rw;
    
    uintptr_t il2cpp = get_module_base("libil2cpp.so", 2);
    rw.read(il2cpp + 0x700518, &il2cpp, 8);
    PhigrosBase::init(il2cpp);
    
    std::cout << "点击一下屏幕\n";
    touch = Touch::Create();
    
    std::cout << "初始化成功\n";
}

void Run() {
    std::unique_ptr<LevelControl> level;
    auto auto_play = std::make_unique<AutoPlay>(touch);
    
    while(true) {
        std::cout << "进入谱子 回车刷新<----";
        std::cin.get();
        
        auto object = PhigrosBase::FindObjects<uintptr_t>(u"LevelControl,Assembly-CSharp");
        if(object.empty()) {
            std::cout << "未进入谱子\n";
            continue;
        }
        level.reset(new LevelControl());
        level->set_addr(object.at(0)); 
        level->update();
        
        auto_play->setTargetLevel(level.get());
        auto_play->CreateSongEvent();
        
        std::cout << "数据获取成功\n";
        auto prev_time = std::chrono::high_resolution_clock::now();
        bool is_run = true;
        while(is_run) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed_time = current_time - prev_time;
            auto remaining_time = FRAME_TIME - elapsed_time;
            
            //降低cpu占用率
            if(remaining_time > std::chrono::milliseconds(0)) {
                std::this_thread::sleep_for(remaining_time);
            }
            level->update();
            is_run = auto_play->Play();
            prev_time = current_time;
        }
        auto_play->Destroy();
    }
}

};