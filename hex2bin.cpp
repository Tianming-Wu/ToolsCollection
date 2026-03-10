#include <SharedCppLib2/bytearray.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if(argc == 1) {
        std::cout << "Usage: hex2bin <input> [output]" << std::endl;
        return 0;
    }

    fs::path input_path = argv[1];
    fs::path output_path = (argc >= 3) ? argv[2] : input_path.filename().stem().string() + ".bin";


    std::ifstream ifs(input_path);
    if(!ifs) {
        std::cerr << "Failed to open input file: " << input_path << std::endl;
        return 1;
    }

    std::string line;
    while(getline(ifs, line)) {
        if(!line.starts_with(":")) {
            std::cerr << "Invalid hex file format. This tool is for Devboard." << std::endl;
            return 1;
        }


    }

    

    return 0;
}