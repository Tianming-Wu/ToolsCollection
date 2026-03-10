/*
    Sha256 Verification tool, also serves as example of using SharedCppLib2 sha256 lib.
*/

#include <iostream>
#include <filesystem>
#include <fstream>
#include <SharedCppLib2/sha256.hpp>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    
    if(argc < 2) {
        std::cout << "Usage: mysha256 <filename>" << std::endl;
        return 1;
    }

    fs::path filename = argv[1];
    
    if(!fs::exists(filename)) {
        std::cout << "Error: file not exists." << std::endl;
        return 1;
    }

    std::ifstream ifs(filename, std::ios::binary);
    std::bytearray ba;

    ba.readAllFromStream(ifs);
    
    std::bytearray result = sha256::getMessageDigest(ba);

    std::cout<< std::hex << result << std::endl;
    return 0;
}