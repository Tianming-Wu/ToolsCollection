/*
    This is a mimic of the grep command on Linux.

    However, the difference is that this grep does not support
    reading from files. You should use something else like irm
    to read the content, and pipe it to this grep.

    This file is part of the ToolsCollection project.
    Author: Tianming-Wu <https://github.com/Tianming-Wu>
*/

#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

class PatternMatcher {
public:
    PatternMatcher(const std::string& pattern, bool extended)
        : extended(extended)
    {
        if(extended) {
            regex_pattern = std::regex(pattern);
        } else {
            simple_pattern = pattern;
        }
    }

    bool match(const std::string& text) const {
        if(extended) {
            return std::regex_search(text, regex_pattern);
        } else {
            return simple_pattern(text);
        }
    }

private:
    class simple_match {
        enum class MatchType { Invalid, FixedText, FixedLength, ChangingLength };
        struct sm { MatchType type; char content; };
        std::vector<sm> pattern;
    public:
        simple_match() = default;
        simple_match(const std::string& ptrnstr)
        {
            MatchType lastType = MatchType::Invalid;
            sm lastPattern {lastType, '\0'};
            for(auto &c : ptrnstr) {
                if(c == '*') {
                    if(lastType == MatchType::ChangingLength) continue;
                    else if(lastType == MatchType::FixedLength) {
                        // As normal
                        pattern.push_back(lastPattern);
                        lastType = MatchType::ChangingLength;
                        lastPattern = {lastType, '\0'};
                    }
                    else {
                        pattern.push_back(lastPattern);
                        lastType = MatchType::ChangingLength;
                        lastPattern = {lastType, '\0'};
                    }
                } else if (c == '?') {
                    if(lastType == MatchType::ChangingLength) {
                        // change the order, to make sure '?' are before '*',
                        // This adds to simplicity of the matching process.

                        // pop the last changing-length pattern.
                        sm swPattern = pattern.back();
                        pattern.pop_back();

                        // add a fixed-length pattern.
                        pattern.push_back({MatchType::FixedLength, '\0'});

                        // put the changing-length pattern back.
                        pattern.push_back(swPattern);

                        // Note this bahavior is like inserting in front of the
                        // last changing-length pattern.
                        // For example,
                        // ? * ? -> ? ? *
                        // * ? * -> ? * *
                        // * ? ? -> ? ? *
                        // So we can make sure this acts like "at least", where
                        // we can stack multiple ?s and allow * to match an amount
                        // of more than 0 characters (as the effect)
                    } else {
                        pattern.push_back(lastPattern);
                        lastType = MatchType::FixedLength;
                        lastPattern = {lastType, '\0'};
                    }
                } else {
                    pattern.push_back(lastPattern);
                    lastType = MatchType::FixedText;
                    lastPattern = {lastType, c};
                }
            }

            if(lastPattern.type != MatchType::Invalid) pattern.push_back(lastPattern);
            pattern.erase(pattern.begin()); // Remove the invalid pattern at the beginning.
        }

        bool operator()(const std::string& text) const {
            for(size_t i = 0, j = 0; i < text.size() && j < pattern.size();) {
                const auto& [type, content] = pattern[j];
                switch(type) {
                    case MatchType::FixedText:
                        if(text.compare(i, 1, &content, 1) == 0) {
                            i += 1;
                            j++;
                        } else return false;
                        break;
                    case MatchType::FixedLength:
                        i++;
                        j++;
                        break;
                    case MatchType::ChangingLength:
                        if(j + 1 == pattern.size()) return true; // last pattern is '*', match the rest of the text
                        else {
                            // try to match the next pattern with the remaining text
                            // Follow the "longest priority" rule.
                            std::string nextPattern;
                            size_t nextPatternLength = 0;
                            
                            // Here when building the pattern, we intentionally merge
                            // adjacent '*' and '?' into only one '*', so there is no change
                            // for them to be followed by another '*' or '?'.

                            // So it must be fixed text.
                            for(size_t k = j + 1; k < pattern.size(); k++) {
                                const auto& [nextType, nextContent] = pattern[k];
                                if(nextType == MatchType::FixedText) {
                                    nextPattern += nextContent;
                                    nextPatternLength++;
                                } else break;
                            }

                            size_t nextPos = text.find(nextPattern, i);
                            if(nextPos == std::string::npos) return false;
                            else {
                                i = nextPos + nextPatternLength; // move i to the position after the fixed text
                                j += 1 + nextPatternLength; // skip the '*' and the fixed text after it
                            }
                        }
                        break;
                }
            }
            return true;
        }
    };

private:
    std::regex regex_pattern;
    simple_match simple_pattern;

    bool extended;
};


int main(int argc, char** argv) {
    if(argc < 2) return 1;

    std::string strptrn;
    bool complex = false;

    if(argv[1][0] == '-') {
        switch(argv[1][1]) {
            case '\0': return 1;
            case 'E': {
                complex = true;
                strptrn = argv[2];
                break;
            }
            default: return 1;
        }
    } else {
        strptrn = argv[1];
    }

    PatternMatcher matcher(strptrn, complex);
    
    std::string line;
    while(getline(std::cin, line)) {
        if(matcher.match(line)) {
            std::cout << line << std::endl;
        }
        if(std::cin.eof()) break;
    }

    return 0;
}

/*
    Here's something about grammar:

    grep [options] pattern

    By default, the pattern is not a regular expression,
    but a simple match.

    Options:

    -E      Interpret pattern as an extended regular expression (ERE).




*/