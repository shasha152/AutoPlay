#pragma once

#include "Phigros.h"

namespace Phigros {

class OtherPlay : public PhigrosBase {
    Mem::vNormalRw rw;
    
    uintptr_t unlock_addr;
    uintptr_t formb_addr;
  public:
    void openUnlockSong();
    void setMoney(int num);
    
    void update() override;
};

};