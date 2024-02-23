#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"
#include <stdio.h>
#include <iostream>
class MNNcore{
    public:
    std::shared_ptr<MNN::Interpreter> mnn_interpreter;
    MNN::Session *mnn_session = nullptr;
    MNN::Tensor *input_tensor = nullptr; // assume single input.
    MNN::ScheduleConfig schedule_config;

    void init_mnn() {
        std::string model_path = "./model.mnn";
        mnn_interpreter = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(model_path.c_str()));
        schedule_config.numThread = 1;
        MNN::BackendConfig backend_config;
        backend_config.precision = MNN::BackendConfig::Precision_High; // default Precision_High
        schedule_config.backendConfig = &backend_config;
        mnn_session = mnn_interpreter->createSession(schedule_config);
        input_tensor = mnn_interpreter->getSessionInput(mnn_session, nullptr);

        auto input = new MNN::Tensor(input_tensor, MNN::Tensor::CAFFE);
        std::cout<< "input: ";
        for (auto in: input->shape()){
            std::cout << in << " ";
        }
        std::cout<<std::endl;
        std::cout<< "input_tensor: " ;
        for(auto shape : input_tensor->shape()){
            std::cout << shape << " ";
        }
        std::cout<< std::endl;
        input->host<float>()[0] = 1.0f;
        input_tensor->copyFromHostTensor(input);
        delete input;

        // time it
        auto start_time = std::chrono::high_resolution_clock::now();
        mnn_interpreter->runSession(mnn_session);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;

        // time it
        start_time = std::chrono::high_resolution_clock::now();
        mnn_interpreter->runSession(mnn_session);
        end_time = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;


        // std::cout<< "input_tensor: " << input_tensor->size() << std::endl;
        auto output_tensors = mnn_interpreter->getSessionOutputAll(mnn_session);
        auto output_tensor = output_tensors["output"];
        std::cout << "output_tensor: " ;
        for(auto shape : output_tensor->shape()){
            std::cout << shape << " ";
        }
        std::cout<< std::endl;
    }
};

int main(){
    MNNcore mnncore;
    mnncore.init_mnn();
    return 0;
}