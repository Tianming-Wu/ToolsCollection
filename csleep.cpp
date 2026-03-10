/*
    Command Intergrated Sleep

    This command allows you to execute a command after a specified time period or at a specific time.

    Usage:
        csleep <[-p <time_point>] | [-m|-s|-M|-d] <duration>> <command>

        -p <time_point> : Specifies a specific time point to execute the command. This must be the first argument if used.
            The supported format of the time point is below. 
        -m : Specifies that the duration is in milliseconds. Note that the command is not accurate at this scale.
        -s : Specifies that the duration is in seconds. This is the default unit.
        -M : Specifies that the duration is in minutes.
        -d : Specifies that the duration is in days.

    Supported time point formats:
    Note: since having spaces in the definition can be inconvenient, comma is used instead.
        - YYYY-MM-DD,HH:MM:SS
        - YYYY-MM-DD (time is considered as 00:00:00)
        - HH:MM:SS (today or tomorrow if time already passed)
        - HH:MM (today or tomorrow if time already passed), in this case seconds are considered as 0.

    Error handing:
        If you entered an invalid time point or duration, the program will return 114 and does
        not execute the command. Otherwise the return code is the command's return code.

    Default cases:
        If called with only a duration, the program will sleep for the specified duration and do nothing.
*/

#include <iostream>
#include <chrono>
#include <thread>

#include <regex>

#ifdef DEBUG
    #define CSLEEP_DEBUG
#endif

#include <SharedCppLib2/stringlist.hpp>

static constexpr const char* __usage = R"(Usage:
    csleep <[-p <time_point>] | [-m|-s|-M|-d] <duration>> <command>

    -p <time_point> : Specifies a specific time point to execute the command. This must be the first argument if used.
        The supported format of the time point is below. 
    -m : Specifies that the duration is in milliseconds. Note that the command is not accurate at this scale.
    -s : Specifies that the duration is in seconds. This is the default unit.
    -M : Specifies that the duration is in minutes.
    -d : Specifies that the duration is in days.

    Supported time point formats:
    Note: since having spaces in the definition can be inconvenient, comma is used instead.
        - YYYY-MM-DD,HH:MM:SS
        - YYYY-MM-DD (time is considered as 00:00:00)
        - HH:MM:SS (today or tomorrow if time already passed)
        - HH:MM (today or tomorrow if time already passed), in this case seconds are considered as 0.
    
    Error handing:
        If you entered an invalid time point or duration, the program will return 114 and does not execute the command. Otherwise the return code is the command's return code.

    Default cases:
        If called with only a duration, the program will sleep for the specified duration and do nothing.
)";

constexpr int return_invalid_input = 114;

int main(int argc, char** argv) {
    std::stringlist args(argc, argv);
    // Unfortunately, std::arguments is not suitable for this program.

    if(args.size() < 2) {
        std::cout << __usage << std::endl;
        return 0;
    } else if (args.size() < 3) {
        // if only one argument is provided, consider it as duration in seconds, and execute nothing
        try {
            int duration_sec = std::stoi(args[1]);
            std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
            return 0;
        } catch(...) {
            if(args[1].ends_with("m") || args[1].ends_with("s") || args[1].ends_with("M") || args[1].ends_with("d")) {
                std::chrono::system_clock::duration duration;
                try {
                    int value = std::stoi(args[1].substr(0, args[1].size() - 1));
                    char unit = args[1].back();

                    switch(unit) {
                        case 'm':
                            duration = std::chrono::milliseconds(value);
                            break;
                        case 's':
                            duration = std::chrono::seconds(value);
                            break;
                        case 'M':
                            duration = std::chrono::minutes(value);
                            break;
                        case 'd':
                            duration = std::chrono::hours(24 * value);
                            break;
                        default:
                            return return_invalid_input;
                    }

                    std::this_thread::sleep_for(duration);
                    return 0;
                } catch(...) {
                    return return_invalid_input;
                }
            } else {
                return return_invalid_input;
            }
        }
    }

    std::string time_point_str;
    if(args[1] == "-p") {
        // specific time point mode
        if(args.size() < 4) {
            // not enough arguments for time point mode
            return return_invalid_input;
        }
        time_point_str = args[2];

        // Mixed command type is not allowed.
        if(args.contains("-m") || args.contains("-s") || args.contains("-M") || args.contains("-d")) {
            return return_invalid_input;
        }

        // parse the time point string and calculate the duration to sleep
        std::chrono::system_clock::time_point target_time;

        std::regex tp_type_full_fullmatch (R"(\d{4}-\d{2}-\d{2},\d{2}:\d{2}:\d{2})"),
            tp_type_date_fullmatch (R"(\d{4}-\d{2}-\d{2})"),
            tp_type_time_fullmatch (R"(\d{2}:\d{2}:\d{2})"),
            tp_type_time_min_fullmatch (R"(\d{2}:\d{2})");

        if(std::regex_match(time_point_str, tp_type_full_fullmatch)) {
            std::tm tm = {};
            std::regex tp_type_full_get (R"((\d{4})-(\d{2})-(\d{2}),(\d{2}):(\d{2}):(\d{2}))");
            std::smatch match;
            std::regex_search(time_point_str, match, tp_type_full_get);

            tm.tm_year = std::stoi(match[1]) - 1900;
            tm.tm_mon = std::stoi(match[2]) - 1;
            tm.tm_mday = std::stoi(match[3]);
            tm.tm_hour = std::stoi(match[4]);
            tm.tm_min = std::stoi(match[5]);
            tm.tm_sec = std::stoi(match[6]);

            target_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        } else if(std::regex_match(time_point_str, tp_type_date_fullmatch)) {
            std::tm tm = {};
            std::regex tp_type_date_get (R"((\d{4})-(\d{2})-(\d{2}))");
            std::smatch match;
            std::regex_search(time_point_str, match, tp_type_date_get);

            tm.tm_year = std::stoi(match[1]) - 1900;
            tm.tm_mon = std::stoi(match[2]) - 1;
            tm.tm_mday = std::stoi(match[3]);
            // time is considered as 00:00:00

            target_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        } else if(std::regex_match(time_point_str, tp_type_time_fullmatch)) {
            std::tm tm = {};
            std::regex tp_type_time_get (R"((\d{2}):(\d{2}):(\d{2}))");
            std::smatch match;
            std::regex_search(time_point_str, match, tp_type_time_get);

            time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto now_tp = std::chrono::system_clock::now();
            tm = *std::localtime(&now);

            tm.tm_hour = std::stoi(match[1]);
            tm.tm_min = std::stoi(match[2]);
            tm.tm_sec = std::stoi(match[3]);

            target_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            
            // if time already passed, consider it as tomorrow
            if(target_time < now_tp) {
                target_time += std::chrono::hours(24);
            }
        } else if(std::regex_match(time_point_str, tp_type_time_min_fullmatch)) {
            std::tm tm = {};
            std::regex tp_type_time_min_get (R"((\d{2}):(\d{2}))");
            std::smatch match;
            std::regex_search(time_point_str, match, tp_type_time_min_get);

            time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto now_tp = std::chrono::system_clock::now();
            tm = *std::localtime(&now);

            tm.tm_hour = std::stoi(match[1]);
            tm.tm_min = std::stoi(match[2]);
            // seconds are considered as 0
            tm.tm_sec = 0;
        } else {
            // invalid time point format
            return return_invalid_input;
        }

        std::string &command = args[3];

        std::this_thread::sleep_until(target_time);

        int return_code = system(command.c_str());

        return return_code;
    } else {
        // duration mode

        int ac_1 = args.contains("-m") + args.contains("-s") + args.contains("-M") + args.contains("-d") > 1;
        
        if(ac_1 > 1) {
            // Mixed command type is not allowed.
            return return_invalid_input;
        }

        int duration_arg_index = 1;

        if(ac_1) duration_arg_index = 2; // We had the option, so the duration is at index 2, otherwise it's at index 1

        std::chrono::system_clock::duration duration;
        if(args.contains("-m")) {
            duration = std::chrono::milliseconds(std::stoi(args[duration_arg_index]));
        } else if(args.contains("-M")) {
            duration = std::chrono::minutes(std::stoi(args[duration_arg_index]));
        } else if(args.contains("-d")) {
            duration = std::chrono::hours(24 * std::stoi(args[duration_arg_index]));
        } else {
            // default to seconds
            duration = std::chrono::seconds(std::stoi(args[duration_arg_index]));
        }

        std::string &command = args[duration_arg_index + 1];

        std::this_thread::sleep_for(duration);

        int return_code = system(command.c_str());

        return return_code;
    }

    return return_invalid_input;
}