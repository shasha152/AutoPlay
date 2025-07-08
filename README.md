# AutoPlay

Phigros自动打歌，主要由Android模拟触摸和读写内存实现
因为是解析内存中的数据，所以自动打歌适用于全部歌曲

大部分歌曲能打到S,小部分全金

# 附加功能

解锁歌曲和无限Money

# 遇到错误
1.如果你无法使用，可能是Il2Class数组偏移发生了变法，修改Phigros.cpp的init函数rw.read(il2cpp + [新的偏移], &il2cpp, 8)

使用gg修改器执行脚本FindIl2ClassArrayOffset.lua获取偏移
```c++
void Init(const char *package) {
    if(Mem::init(package) == 0)
        throw std::runtime_error("未找到pid");
    vNormalRw rw;
    
    uintptr_t il2cpp = get_module_base("libil2cpp.so", 2);
    rw.read(il2cpp + 0x700518, &il2cpp, 8);
    PhigrosBase::init(il2cpp);
    std::cout << "\033[92;40;5m";
    std::cout << "点击一下屏幕\n";
    touch = Touch::Create();
    
    std::cout << "初始化成功\n";
}
```

2.如果是未找到类，在启动一次程序

3.要是以上都没解决问题告知我
