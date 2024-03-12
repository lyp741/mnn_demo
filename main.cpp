#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>

// 定义共享内存相关的类型
typedef boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager> CharAllocator;
typedef boost::interprocess::basic_string<char, std::char_traits<char>, CharAllocator> SharedString;
typedef std::pair<const SharedString, SharedString> ValueType;
typedef boost::interprocess::allocator<ValueType, boost::interprocess::managed_shared_memory::segment_manager> ShmemAllocator;
typedef boost::interprocess::map<SharedString, SharedString, std::less<SharedString>, ShmemAllocator> SharedMap;

int main() {
    // 创建共享内存段
    boost::interprocess::managed_shared_memory segment(boost::interprocess::open_or_create, "MySharedMemory", 65536);

    // 获取分配器
    CharAllocator char_alloc(segment.get_segment_manager());
    ShmemAllocator alloc_inst(segment.get_segment_manager());

    // 在共享内存中创建 map

    SharedMap *myMap = segment.find_or_construct<SharedMap>("MyMap")(std::less<SharedString>(), alloc_inst);


    //insert data
    myMap->insert(ValueType(SharedString("key1"), SharedString("value1")));
    myMap->insert(ValueType(SharedString("key2"), SharedString("value2")));
    myMap->insert(ValueType(SharedString("key3"), SharedString("value3")));

    // 遍历输出
    for (auto& pair : *myMap) {
        std::cout << pair.first << " : " << pair.second << std::endl;
    }

    return 0;
}
