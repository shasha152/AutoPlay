#include "OtherPlay.h"

namespace Phigros {


void OtherPlay::openUnlockSong() {
    auto objects = FindObjects<uintptr_t>(u"SongSelector,Assembly-CSharp");
    for(auto addr : objects) {
        Mem::Il2cpp::Il2List<uintptr_t> list;
        list.refresh(rw.read<uintptr_t>(addr + 0x40));
        for(auto _addr : list) {
            if(rw.read<bool>(_addr + 0x18) == 0) {
                Mem::Il2cpp::Call::PtraceCall(unlock_addr, {
                    static_cast<long>(_addr)
                });
            
            }
        }
    }
    
}

void OtherPlay::setMoney(int num) {
    auto objects = FindObjects<uintptr_t>(u"MoneyControl,Assembly-CSharp");
    
    //构建Money对象
    auto money_addr = Mem::Il2cpp::Call::PtraceCall(formb_addr, {
        static_cast<long>(num)
    });
    for(auto addr : objects) {
        rw.write(addr + 0x18, &money_addr, sizeof(money_addr));
    }
}


void OtherPlay::update() {
    unlock_addr = Mem::Il2cpp::Domain::GetClass(assembly["Assembly-CSharp.dll"], 
    "SongSelectorItem", "").
        parse_method().
        find_method("Unlock").
        get_addr();
    
    formb_addr = Mem::Il2cpp::Domain::GetClass(assembly["Assembly-CSharp.dll"], 
    "Money", "").
        parse_method().
        find_method("ForMB").
        get_addr();
}

};