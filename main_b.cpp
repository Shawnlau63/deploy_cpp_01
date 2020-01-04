#include <iostream>
#include <torch/torch.h>
#include <torch/script.h>


int main() {
    torch::jit::script::Module module;
    try {
        module = torch::jit::load("..\\params\\traced_mynet_model.pt");
    }catch (const c10::Error& e) {
        std::cerr << "error loading the model\n";
        return -1;
    }

    auto image = torch::tensor({1});

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(image);

    auto rst = module.forward(inputs).toTensor();

    std::cout<<rst<<std::endl;

    return 0;
}