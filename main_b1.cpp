#include <iostream>
#include <torch/torch.h>
#include <torch/script.h>


int main() {
    torch::jit::script::Module module;
    try {
        module = torch::jit::load("..\\params\\model_1.pt");
    } catch (const c10::Error &e) {
        std::cerr << "error loading the model\n";
        return -1;
    }

//    auto data = torch::ones({2, 2, 2, 2});
    auto data = torch::arange(16).reshape({2, 2, 2, 2});
    std::cout << data << std::endl;

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(data);

    auto rst = module.forward(inputs).toTensor();

    std::cout << rst << std::endl;

    return 0;
}