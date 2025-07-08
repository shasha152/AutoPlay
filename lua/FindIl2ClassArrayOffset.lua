local classes = {};

local function getValue(addr, flag)
    return gg.getValues({{address = addr, flags = flag}})[1].value;
end

local function pointer(addr)
    return gg.getValues({{address = addr, flags = 32}})[1].value;
end

local function getName(addr)
    local str = ""
    local t = {}
    for i=1, 128 do
        t[i] = {address=addr+(i-1), flags=gg.TYPE_BYTE}
    end
    t = gg.getValues(t)
    
    for i, v in ipairs(t) do
        if v.value==0 then break end
        if v.value<0 then return "" end
        str = str..string.char(v.value&0xFF)
    end
    return str
end

function searchObject(class, dll)
    local class_addr = classes[class];
    if(class_addr == nil) then
        gg.clearResults();
        gg.setRanges(gg.REGION_C_ALLOC | gg.REGION_OTHER);
        gg.searchNumber("Q 00 \'"..class.."\' 00");
        gg.searchNumber(":"..string.sub(class, 1, 1));
        local last = gg.getResults(1000);
        local load = {};
        gg.searchPointer(0);
        if(gg.getResultsCount() == 0) then
            for k, v in pairs(last) do
                gg.searchNumber(0xB400000000000000 | v.address, 32);
                for _, vv in pairs(gg.getResults(1000)) do
                    load[#load + 1] = vv;
                end
            end
        else 
            for _, vv in pairs(gg.getResults(1000)) do
               load[#load + 1] = vv;
            end
        end
        for k, v in pairs(load) do
            local str_addr = pointer(pointer(v.address - 0x10));
            if(getName(str_addr) == dll) then
                class_addr = v.address - 0x10;
                classes[class] = class_addr;
            end
        end
    end
    gg.clearResults();
    gg.setRanges(gg.REGION_C_ALLOC);
    gg.searchNumber(class_addr, 32);
    if(gg.getResultsCount() == 0) then
        gg.setRanges(gg.REGION_C_ALLOC);
        gg.searchNumber(0xB400000000000000 | class_addr, 32);
        classes[class] = 0xB400000000000000 | class_addr;
    end
    local res = gg.getResults(1000);
    gg.clearResults();
    return res;
end

local results = {};
local load = searchObject("<Module>", "mscorlib.dll");
gg.setRanges(gg.REGION_C_BSS);
gg.loadResults(load);
gg.searchPointer(0);
if(gg.getResultsCount() == 0) then
    for k, v in pairs(load) do
        gg.searchNumber(0xB400000000000000 | v.address, 32);
        for __, vv in pairs(gg.getResults(1000)) do
            results[#results + 1] = {
                address = vv.address;
                flags = 32
            };
        end
    end
else results = gg.getResults(1000); end

local module = gg.getRangesList("libil2cpp.so")[3].start;
for k, v in pairs(results) do
    print(string.format("0x%X", v.address - module));
end