#include <stdio.h>
#include <stdlib.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <time.h>

#include "Touch.h"

namespace Touch {


int getTouchFd() {
    std::vector<int> events;
    
    for(int i = 0;;i++) {
        std::string path = std::string("/dev/input/event") + std::to_string(i);
        int fd = open(path.c_str(), O_RDWR | O_NONBLOCK);
        if(fd == -1) break;
        events.push_back(fd);
    }
    input_event ev;
    while(true) {
        for(int fd : events) {
            memset(&ev, 0, sizeof(ev));
            
            read(fd, &ev, sizeof(ev));
            if(ev.type == EV_ABS) {
                return fd;
            }
        }
    }
    
    return -1;
}

int Create() {
    struct uinput_user_dev uinp;
    int fb = getTouchFd();
	int uinp_fd = open("/dev/uinput", O_WRONLY|O_NONBLOCK);
		
	if(uinp_fd <= 0) {
		throw std::runtime_error("无法打开 /dev/uinput 请检查是否有root权限");
		return -1;
	}
		
	memset(&uinp, 0, sizeof(uinp));
    char name[UINPUT_MAX_NAME_SIZE] = "myTouch";
        
    strncpy(uinp.name, name, UINPUT_MAX_NAME_SIZE);
    uinp.id.bustype = BUS_SPI;
    uinp.id.vendor = 1;
    uinp.id.product = 1;
    uinp.id.version = 1;
        
    ioctl(uinp_fd, UI_SET_PHYS, name);
    
    struct input_absinfo absX;
    struct input_absinfo absY;
    ioctl(fb, EVIOCGABS(ABS_MT_POSITION_X), &absX);
    ioctl(fb, EVIOCGABS(ABS_MT_POSITION_Y), &absY);
    
    uinp.absmin[ABS_MT_SLOT] = 0;
	uinp.absmax[ABS_MT_SLOT] = FingerMaxNumber - 1;
	uinp.absmin[ABS_MT_TRACKING_ID] = 0;
	uinp.absmax[ABS_MT_TRACKING_ID] = 65535;

    uinp.absmin[ABS_MT_POSITION_X] = absX.minimum;
    uinp.absmax[ABS_MT_POSITION_X] = absX.maximum;

    uinp.absmin[ABS_MT_POSITION_Y] = absY.minimum;
    uinp.absmax[ABS_MT_POSITION_Y] = absY.maximum;

    ioctl(uinp_fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);
    
	ioctl(uinp_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
    ioctl(uinp_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    ioctl(uinp_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
    ioctl(uinp_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
    
    ioctl(uinp_fd, UI_SET_EVBIT, EV_SYN);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_ABS);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);

    write(uinp_fd, &uinp, sizeof(uinp));
	ioctl(uinp_fd, UI_DEV_CREATE);
	return uinp_fd;
}

void Finger::down(int x, int y) {
    timeval time;
    gettimeofday(&time, 0);
	inputs.emplace_back(input_event{time, EV_ABS, ABS_MT_SLOT, slot});
    inputs.emplace_back(input_event{time, EV_ABS, ABS_MT_TRACKING_ID, slot});
    inputs.emplace_back(input_event{time, EV_ABS, ABS_MT_POSITION_X, x});
	inputs.emplace_back(input_event{time, EV_ABS, ABS_MT_POSITION_Y, y});
	
    inputs.emplace_back(input_event{time, EV_SYN, SYN_REPORT, 0});
    
    {
        std::unique_lock<std::mutex> lock{*mutex};
        write(fd, inputs.data(), sizeof(input_event) * inputs.size());
    }
    inputs.clear();
	isdown = true;
}

void Finger::up() {
    timeval time;
    gettimeofday(&time, 0);
	inputs.emplace_back(input_event{time, EV_ABS, ABS_MT_SLOT, slot});
    inputs.emplace_back(input_event{time, EV_ABS, ABS_MT_TRACKING_ID, -1});
    inputs.emplace_back(input_event{time, EV_SYN, SYN_REPORT, 0});
    {
        std::unique_lock<std::mutex> lock{*mutex};
        write(fd, inputs.data(), sizeof(input_event) * inputs.size());
    }
    inputs.clear();
	isdown = false;
}


Finger& Finger::bind_event(const SEvent& fevent, const SFinger& finger) {
    if(finger) fevent->bind_finger(finger);
    finger->isdown = true;
    pool.enqueue([fevent](){
        fevent->run();
    });
    return *finger;
}

SFinger CreateFinger(TouchSrceen fd, int slot) {
    if(slot < 0 || slot > FingerMaxNumber - 1) {
        throw std::logic_error("超出创建finger的slot限制 slot: 0~9");
    }
    return std::make_shared<Finger>(fd, slot);
}

};