/*
    Enhanced Filtered "tree" command implementation.

    Since it is too hard to make a fully functional path filter, i will just
    use the regex to filter every single file.
*/

#include <iostream>
#include <filesystem>
#include <regex>
#include <vector>
#include <string>
#include <SharedCppLib2/stringlist.hpp>
#include <functional>
#include <sstream>
#include <locale>
#include <codecvt>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace fs = std::filesystem;

class TreeFilter {
public:
    // filters: vector of patterns; patterns starting with '!' are blacklist
    TreeFilter(const std::vector<std::string>& filters)
    {
        for(const std::string &rule : filters) {
            if(rule.empty()) continue;
            try {
                if(rule[0] == '!') {
                    std::string pat = rule.substr(1);
                    blacklist_.emplace_back(pat, std::regex::ECMAScript);
                } else {
                    whitelist_.emplace_back(rule, std::regex::ECMAScript);
                }
            } catch(const std::regex_error &e) {
                std::cerr << "Invalid regex '" << rule << "': " << e.what() << "\n";
            }
        }
    }

    // return true if path passes the filter: (whitelist empty or matches at least one whitelist) AND matches no blacklist
    bool matches_path(const std::string &path) const {
        // check blacklist first
        for(const auto &r : blacklist_) {
            if(std::regex_search(path, r)) return false;
        }

        if(whitelist_.empty()) return true;

        for(const auto &r : whitelist_) {
            if(std::regex_search(path, r)) return true;
        }
        return false;
    }

    std::wstring run(fs::path start_path = fs::current_path()) {
        std::wstringstream result;

        enum Pref { NONE = 0, BRANCH = 1, LAST = 2, MID = 3 };
        constexpr const wchar_t prefixes[] = L"┬├└|";
        constexpr const wchar_t bold_prefixes[] = L"┳┣┗";

        // std::function<bool(const fs::path&)> keep = [&](const fs::path& p) {
        //     std::string path = p.string();
        //     if(blacklist.filtered(path)) return false;
        //     if(whitelist.filtered(path)) return false;
        //     return true;
        // };

        if(!fs::exists(start_path)) {
            std::cout << "Error start path doesn't exists" << std::endl;
            return L"";
        }

        // single-pass traversal: traverse returns true if it printed any visible items into 'outbuf'
        std::function<bool(const fs::path&, const std::wstring&, std::wstringstream&)> traverse =
            [&](const fs::path &p, const std::wstring& prefix, std::wstringstream &outbuf)->bool {
            std::vector<fs::directory_entry> entries;
            try {
                for(const fs::directory_entry &entry : fs::directory_iterator(p)) entries.push_back(entry);
            } catch(const std::exception &e) {
                std::string msg = e.what();
                std::wstring wmsg(msg.begin(), msg.end());
                outbuf << prefix << L"[error reading directory] " << wmsg << L"\n";
                return false;
            }

            // We'll collect shown entries with optional child buffers. This lets us traverse each child once.
            struct ShownEntry { fs::directory_entry entry; bool is_dir; bool will_show; std::wstringstream child_buf; };
            std::vector<ShownEntry> shown_list;

            for(const auto &entry : entries) {
                ShownEntry se{entry, entry.is_directory(), false, std::wstringstream{}};
                try {
                    if(se.is_dir) {
                        // recurse into directory and capture its output in child_buf
                        std::wstring new_prefix = prefix + L"    "; // placeholder; actual indentation adjusted when printing
                        bool child_visible = traverse(entry.path(), new_prefix, se.child_buf);
                        bool matched = matches_path(entry.path().string());
                        se.will_show = matched || child_visible;
                    } else {
                        se.will_show = matches_path(entry.path().string());
                    }
                } catch(...) {
                    se.will_show = false;
                }
                if(se.will_show) shown_list.push_back(std::move(se));
            }

            if(shown_list.empty()) return false;

            for(size_t i = 0; i < shown_list.size(); ++i) {
                const auto &se = shown_list[i];
                bool is_last = (i + 1 == shown_list.size());
                std::wstring current_prefix = prefix + (is_last ? L"└── " : L"├── ");
                outbuf << current_prefix << se.entry.path().filename().wstring();
                if(se.is_dir) outbuf << L"/";
                outbuf << L"\n";

                if(se.is_dir) {
                    // children were produced in se.child_buf, but that buffer used placeholder indentation.
                    // We need to adjust indentation: its lines currently start with the child's own prefixes based on placeholder.
                    // Simpler approach: regenerate child output by performing a printing traversal that uses correct new_prefix.
                    std::wstringstream real_child_buf;
                    std::wstring new_prefix = prefix + (is_last ? L"    " : L"│   ");
                    // Call a printing-only recursion by invoking traverse again but with a temporary function that writes directly.
                    // To avoid duplicate traversal, we reuse se.child_buf if it's non-empty: replace placeholder by proper indentation.
                    std::wstring child_str = se.child_buf.str();
                    if(!child_str.empty()) {
                        // child_str lines currently start with prefixes relative to placeholder; they likely already contain proper tree chars.
                        // We'll output child_str directly but need to adjust its leading whitespace from placeholder to new_prefix.
                        // Replace occurrences of placeholder prefix (4 spaces) at start of lines with new_prefix.
                        std::wstring placeholder = prefix + L"    ";
                        std::wstring adjusted;
                        size_t pos = 0;
                        while(pos < child_str.size()) {
                            // find end of line
                            size_t eol = child_str.find(L'\n', pos);
                            if(eol == std::wstring::npos) eol = child_str.size();
                            std::wstring line = child_str.substr(pos, eol - pos);
                            // if line starts with placeholder, replace it with new_prefix
                            if(line.rfind(placeholder, 0) == 0) {
                                line.erase(0, placeholder.size());
                                adjusted += new_prefix + line;
                            } else {
                                // otherwise, just prefix with new_prefix for safety
                                adjusted += new_prefix + line;
                            }
                            adjusted += L"\n";
                            pos = eol + 1;
                        }
                        outbuf << adjusted;
                    } else {
                        // nothing in child buffer
                    }
                }
            }

            return true;
        };

        result << prefixes[NONE] << start_path.wstring() << L'\n';
    traverse(start_path, L"", result);

        return result.str();
    }

private:
    std::vector<std::regex> whitelist_;
    std::vector<std::regex> blacklist_;
};

constexpr const char* __usage = R"(Usage:
    ftree <directory> [options] <filter_rules ...>
)";

int main(int argc, char** argv) {
    std::stringlist args(argc, argv);

    // (no test overwrite) use actual command-line args

    // if (args.size() < 2) {
    //     std::cout << __usage << std::endl;
    //     return 1;
    // }



    std::string directory = (args.size() < 2)?".":args[1];
    // collect filters from args (from index 2 onward)
    std::vector<std::string> filters;
    for(size_t i = 2; i < args.size(); ++i) filters.push_back(args[i]);
    TreeFilter tf(filters);

    try {
        // set locale to user's environment (helps wcout formatting on some systems)
        try { std::locale::global(std::locale("")); } catch(...) {}
        std::wcout.imbue(std::locale());

        std::wstring out = tf.run(directory);

        // If we're on Windows and stdout is a real console, use WriteConsoleW to reliably print wide chars
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if(hOut != INVALID_HANDLE_VALUE) {
            DWORD mode;
            if(GetConsoleMode(hOut, &mode)) {
                // real console: write wide chars directly
                DWORD written = 0;
                WriteConsoleW(hOut, out.c_str(), (DWORD)out.size(), &written, NULL);
                // also write a final newline if not present
                if(out.empty() || out.back() != L'\n') {
                    WriteConsoleW(hOut, L"\n", 1, &written, NULL);
                }
            } else {
                // redirected to file/pipe: convert to UTF-8 and write to narrow stdout
                int size = WideCharToMultiByte(CP_UTF8, 0, out.c_str(), (int)out.size(), NULL, 0, NULL, NULL);
                std::string utf8;
                if(size > 0) {
                    utf8.resize(size);
                    WideCharToMultiByte(CP_UTF8, 0, out.c_str(), (int)out.size(), &utf8[0], size, NULL, NULL);
                }
                std::cout << utf8;
                std::cout.flush();
            }
        } else {
            // fallback
            int size = WideCharToMultiByte(CP_UTF8, 0, out.c_str(), (int)out.size(), NULL, 0, NULL, NULL);
            std::string utf8;
            if(size > 0) {
                utf8.resize(size);
                WideCharToMultiByte(CP_UTF8, 0, out.c_str(), (int)out.size(), &utf8[0], size, NULL, NULL);
            }
            std::cout << utf8;
            std::cout.flush();
        }
#else
        // Non-Windows: print wide to wcout and also UTF-8 to cout as fallback
        std::wcout << out;
        std::wcout.flush();
        try {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
            std::string utf8 = conv.to_bytes(out);
            std::cout << utf8;
            std::cout.flush();
        } catch(...) {
            // conversion failed: best-effort narrow output
            std::string approx(out.begin(), out.end());
            std::cout << approx;
            std::cout.flush();
        }
#endif
    } catch(const std::exception &e) {
        std::cout << "Unhandled exception: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}
