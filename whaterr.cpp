#include <SharedCppLib2/platform.hpp>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage: whaterr [err_id]" << std::endl; 
        return 1;
    }

    const std::string input(argv[1]);

    DWORD dwerror = 0;

    try {
        std::size_t pos = 0;
        // base 0 lets stoul auto-detect 0x (hex), 0 (octal), or decimal
        unsigned long parsed = std::stoul(input, &pos, 0);

        // ensure we consumed the entire string (aside from trailing spaces)
        while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos]))) {
            ++pos;
        }

        if (pos != input.size()) {
            std::cerr << "Invalid characters in error code: " << input << std::endl;
            return 1;
        }

        dwerror = static_cast<DWORD>(parsed);
    } catch (const std::exception& ex) {
        std::cerr << "Failed to parse error code: " << ex.what() << std::endl;
        return 1;
    }

    std::string result = platform::windows::TranslateError(dwerror);

    std::cout << result << std::endl;

    return 0;
}