#include <pybind11/pybind11.h>
#include <string>
#include "proto/KoalaAI_G2C.pb.h"
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp> 
#include <boost/interprocess/containers/map.hpp>

#include <fstream>
#include <chrono>
#include <unistd.h>
#include <thread>
namespace py = pybind11;

using namespace boost::interprocess;

// 定义所需的类型
typedef managed_shared_memory::segment_manager segment_manager_t;
typedef allocator<void, segment_manager_t> void_allocator;
typedef allocator<char, segment_manager_t> char_allocator;
typedef allocator<int, segment_manager_t> int_allocator;
typedef basic_string<char, std::char_traits<char>, char_allocator> shared_string;
typedef allocator<shared_string, segment_manager_t> shared_string_allocator;
typedef vector<shared_string, shared_string_allocator> shared_vector_string;
typedef vector<int, int_allocator> shared_vector_int;
typedef shared_string    KeyType;
typedef shared_string  MappedType;
typedef std::pair<shared_string, shared_string> ValueType;
typedef allocator<ValueType, segment_manager_t> string_map_allocator;
typedef map<KeyType, MappedType, std::less<KeyType>, string_map_allocator> string_map;
class Message
{
public:
   long timestep;
   shared_string req_id;
   shared_string request;
   shared_string agents_id;
   // constructor:
   Message(long ts, std::string rid, std::string req, std::string aids, char_allocator char_alloc) : timestep(ts), req_id(rid.c_str(), char_alloc), request(req.c_str(), req.length(), char_alloc), agents_id(aids.c_str(), char_alloc)
   {
   }
};
typedef allocator<Message, segment_manager_t> message_allocator;
typedef vector<Message, message_allocator> shared_vector_message;


std::string process_string(py::bytes &input) {
    std::string input_str(input);
    msg::g2c::GameMassage msg;
    // std::ifstream ifs("../proto.b", std::ios::in | std::ios::binary);
    // std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    bool ok = msg.ParseFromString(input_str);
    // print
    std::cout << msg.DebugString() << std::endl;
    return msg.DebugString();
}

std::string SHM_NAME = "MySharedMemory";
managed_shared_memory *segment;
// 获取分配器
const shared_string_allocator *alloc_inst;
const char_allocator *char_alloc_inst;
const message_allocator *message_alloc_inst;
shared_vector_message *myVector;
const string_map_allocator *string_map_alloc_inst;
string_map *action_map;
interprocess_mutex *proto_list_mtx;
interprocess_mutex *actions_map_mtx;

bool init_shm(){
    shared_memory_object::remove(SHM_NAME.c_str());
    segment = new managed_shared_memory(create_only, SHM_NAME.c_str(), 65536*1000); // 6G
    alloc_inst = new shared_string_allocator(segment->get_segment_manager()); 
    char_alloc_inst = new char_allocator(segment->get_segment_manager());
    message_alloc_inst = new message_allocator(segment->get_segment_manager());
    proto_list_mtx = segment->construct<interprocess_mutex>("proto_list_mtx")(); 
    actions_map_mtx = segment->construct<interprocess_mutex>("actions_map_mtx")();
    myVector = segment->construct<shared_vector_message>("proto_list")(*message_alloc_inst);
    
    string_map_alloc_inst = new string_map_allocator(segment->get_segment_manager());

    action_map = segment->construct<string_map>("actions_map")(std::less<KeyType>(), *string_map_alloc_inst);

    std::cout << "init shm success" << std::endl;
    return true;
}

bool open_shm(){
    segment = new managed_shared_memory(open_only, SHM_NAME.c_str());
    alloc_inst = new shared_string_allocator(segment->get_segment_manager());
    char_alloc_inst = new char_allocator(segment->get_segment_manager());
    message_alloc_inst = new message_allocator(segment->get_segment_manager());
    proto_list_mtx = segment->find<interprocess_mutex>("proto_list_mtx").first; 
    myVector = segment->find<shared_vector_message>("proto_list").first;
    std::cout << "open shm success" << std::endl;
    return true;
}

bool dispatch_proto(long timestep, std::string req_id, py::bytes &request, std::string agents_id){
    std::string data(request);
    Message msg(timestep, req_id, data, agents_id, *char_alloc_inst);
    proto_list_mtx->lock();
    myVector->push_back(msg);
    proto_list_mtx->unlock();
    return true;
}

py::tuple fetch_proto(){
    while(true){
        if(!myVector->empty()){
            proto_list_mtx->lock();
            Message msg = myVector->back();
            myVector->pop_back();
            proto_list_mtx->unlock();
            std::string req_id(msg.req_id.begin(), msg.req_id.end());
            std::string request(msg.request.begin(), msg.request.end());
            std::string agents_id(msg.agents_id.begin(), msg.agents_id.end());
            py::tuple ret = py::make_tuple(msg.timestep, req_id, py::bytes(request), agents_id);
            return ret;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

PYBIND11_MODULE(bip_msg, m) {
    m.def("process_string", &process_string, "Process a string"),
    m.def("init_shm", &init_shm, "init_shm"),
    m.def("open_shm", &open_shm, "open_shm"),
    m.def("dispatch_proto", &dispatch_proto, "dispatch_proto"),
    m.def("fetch_proto", &fetch_proto, "fetch_proto");
}
