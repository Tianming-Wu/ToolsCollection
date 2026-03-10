#include <SharedCppLib2/bytearray.hpp>
#include <SharedCppLib2/Base64.hpp>
#include <SharedCppLib2/arguments.hpp>

#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    std::arguments args(argc, argv, std::arguments::default_policy | std::arguments::parse_policy::EnablePrimaryCommand);

    if (args.empty()) {
        std::cout << "Usage: base64 [encode|decode] [input]" << std::endl;
        std::cout << "       base64 [encode-file|decode-file] [input_file] [output_file]" << std::endl;
        return 0;
    }

    if (args.addHelp([]() {
        std::cout << "Usage: base64 [encode|decode] [input]" << std::endl;
        std::cout << "  encode: Encodes the input string to Base64." << std::endl;
        std::cout << "  decode: Decodes the input Base64 string.\n" << std::endl;
        std::cout << "Usage: base64 [encode-file|decode-file] [input_file] [output_file]" << std::endl;
        std::cout << "  encode-file: Encodes the input file to Base64 and writes to output file." << std::endl;
        std::cout << "  decode-file: Decodes the input Base64 file and writes to output file." << std::endl;
    })) return 0;

    enum class Action {
        Invalid, Encode, Decode,
        Encode_File, Decode_File
    } action = Action::Invalid;

    std::string primary = args.getPrimaryCommand();

    if(primary == "encode") {
        action = Action::Encode;
    } else if (primary == "decode") {
        action = Action::Decode;
    } else if (primary == "encode-file") {
        action = Action::Encode_File;
    } else if (primary == "decode-file") {
        action = Action::Decode_File;
    } else {
        std::cerr << "Invalid action: " << primary << std::endl;
        std::cout << "Usage: base64 [encode|decode] [input]" << std::endl;
        return 1;
    }

    if ((action == Action::Encode || action == Action::Decode) && args.size() < 3) {
        std::cerr << "Input string is required." << std::endl;
        std::cout << "Usage: base64 [encode|decode] [input]" << std::endl;
        return 1;
    } else if ((action == Action::Encode_File || action == Action::Decode_File) && args.size() < 4) {
        std::cerr << "Input and output file paths are required." << std::endl;
        std::cout << "Usage: base64 [encode-file|decode-file] [input_file] [output_file]" << std::endl;
        return 1;
    }

    if(action == Action::Encode || action == Action::Decode) {
        const std::string input = args.anyAfter(1);
        if(action == Action::Encode) {
            std::string encoded = Base64::encode(std::bytearray(input));
            std::cout << encoded << std::endl;
        } else if (action == Action::Decode) {
            try {
                std::string decoded = std::bytearray::fromBase64(input).toStdString();
                std::cout << decoded << std::endl;
            } catch (const std::exception& ex) {
                std::cerr << "Failed to decode input: " << ex.what() << std::endl;
                return 1;
            }
        }
    } else if (action == Action::Encode_File || action == Action::Decode_File) {
        const std::string input_file = args.at(2), output_file = args.at(3);
        try {
            if(action == Action::Encode_File) {
                std::ifstream input(input_file, std::ios::binary);
                if(!input) {
                    std::cerr << "Failed to open input file: " << input_file << std::endl;
                    return 1;
                }

                std::bytearray data;
                data.readAllFromStream(input);
                std::string encoded = Base64::encode(data);
                std::ofstream ofs(output_file);
                ofs << encoded;

            } else if (action == Action::Decode_File) {
                std::ifstream ifs(input_file);
                std::stringstream ss;
                ss << ifs.rdbuf();
                std::string encoded = ss.str();
                std::bytearray decoded = std::bytearray::fromBase64(encoded);

                std::ofstream ofs(output_file, std::ios::binary);
                if(!ofs) {
                    std::cerr << "Failed to open output file: " << output_file << std::endl;
                    return 1;
                }

                decoded.writeRaw(ofs);
            }
        } catch (const std::exception& ex) {
            std::cerr << "Failed to process file: " << ex.what() << std::endl;
            return 1;
        }
    }

    return 0;
}