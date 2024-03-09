#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include "proto/KoalaAI_G2C.pb.h"
using namespace boost::interprocess;

// 定义所需的类型
typedef managed_shared_memory::segment_manager segment_manager_t;
typedef allocator<void, segment_manager_t> void_allocator;
typedef allocator<char, segment_manager_t> char_allocator;
typedef basic_string<char, std::char_traits<char>, char_allocator> shared_string;
typedef allocator<shared_string, segment_manager_t> shared_string_allocator;
typedef vector<shared_string, shared_string_allocator> shared_vector;

int producer()
{
   struct shm_remove
      {
         shm_remove() { shared_memory_object::remove("MySharedMemory"); }
         ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
      } remover;
    // 创建共享内存
    managed_shared_memory segment(create_only, "MySharedMemory", 65536);

    // 获取分配器
    const shared_string_allocator alloc_inst(segment.get_segment_manager());
    const char_allocator char_alloc_inst(segment.get_segment_manager());

    // 构造共享的vector
    shared_vector* myVector = segment.construct<shared_vector>("MyVector")(alloc_inst);

    // 在 vector 中添加字符串
    myVector->push_back(shared_string("Hello", char_alloc_inst));
    myVector->push_back(shared_string("World", char_alloc_inst));
   
   for (int i=0; i<100; i++){
      //每秒添加一个字符串
      myVector->push_back(shared_string(std::to_string(i).c_str(), char_alloc_inst));
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   }

   //  // 打印 vector 中的字符串
   //  for (const auto& str : *myVector) {
   //      std::cout << str << std::endl;
   //  }

    // 销毁共享内存
    segment.destroy<shared_vector>("MyVector");

    return 0;
}

int consumer()
{
   managed_shared_memory segment;
   // 等待并打开已存在的共享内存
   while (1){
   try{
      segment = managed_shared_memory(open_only, "MySharedMemory");
      break;
   }
   catch(...){
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
   }
   }
    
   // 获取共享内存中的vector
   shared_vector* myVector = segment.find<shared_vector>("MyVector").first;

   if (myVector) {
      // 打印 vector 中的字符串
      while(1){
         if (myVector->size() > 0) {
            std::cout << myVector->front() << std::endl;
            myVector->erase(myVector->begin());
            // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
         }
         else{
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
         }
      }
      
   } else {
      std::cerr << "Vector not found!" << std::endl;
   }

   return 0;
}

int main(int argc, char* argv[]){
   msg::g2c::GameMassage msg;
   std::ifstream ifs("../proto.b", std::ios::in | std::ios::binary);
   std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
   bool ok = msg.ParseFromString(data);
   if (!ok) {
      std::cerr << "ParseFromString failed" << std::endl;
      return -1;
   }

   std::cout << msg.DebugString() << std::endl;

   if (argc == 1){
      //start a process to do the work
      std::string s(argv[0]); s += " child ";
      //启动子进程，不等待结束
      producer();
      return 0;
      }
   else
      consumer();
   return 0;
}