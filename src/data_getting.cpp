#include <pybind11/embed.h> // pybind11 embedding
#include <pybind11/stl.h>
#include <iostream>

namespace py = pybind11;

int main() {
    py::scoped_interpreter guard{};
    
    try {
        auto sys = py::module_::import("sys");
        sys.attr("path").attr("append")("C:/your/project/path");
        
        auto my_strategy = py::module_::import("my_strategy");
        
        // 调用 Python 函数
        auto result = my_strategy.attr("generate_signals")(py::list{1, 2, 3, 4, 5});
        
        // 转换为 C++ 类型
        std::vector<double> signals = result.cast<std::vector<double>>();
        
        std::cout << "Received signals: ";
        for (double sig : signals) {
            std::cout << sig << " ";
        }
        std::cout << std::endl;
        
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error: " << e.what() << std::endl;
    }
    
    return 0;
}