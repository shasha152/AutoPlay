# AutoPlay

Phigros自动打歌，主要由Android模拟触摸和读写内存实现
# 附加功能

解锁歌曲和无限Money

# 遇到错误
1.如果你无法使用，可能是Il2Class数组偏移发生了变法，修改rw.read(il2cpp + [新的偏移], &il2cpp, 8)
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
