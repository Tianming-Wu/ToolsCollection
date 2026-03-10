#include "condition.hpp"
#include <algorithm>
#include <sstream>

namespace condition {

Condition::Condition(ConditionType type, const std::string& pattern)
    : type_(type), pattern_(pattern) {
    initializeEvaluator();
}

Condition::Condition(ConditionType type, std::vector<std::shared_ptr<Condition>> children)
    : type_(type), children_(children) {
    initializeEvaluator();
}

void Condition::initializeEvaluator() {
    switch (type_) {
        case ConditionType::ALWAYS:
            evaluator_ = [](const std::string&) { return true; };
            break;
            
        case ConditionType::NEVER:
            evaluator_ = [](const std::string&) { return false; };
            break;
            
        case ConditionType::STRING_MATCH:
            evaluator_ = [this](const std::string& input) {
                return input.find(pattern_) != std::string::npos;
            };
            break;
            
        case ConditionType::REGEX_MATCH:
            evaluator_ = [this](const std::string& input) {
                try {
                    std::regex pattern(pattern_);
                    return std::regex_search(input, pattern);
                } catch (const std::regex_error&) {
                    return false; // Invalid regex returns false
                }
            };
            break;
            
        case ConditionType::AND:
            evaluator_ = [this](const std::string& input) {
                for (const auto& child : children_) {
                    if (!child->evaluate(input)) return false;
                }
                return true;
            };
            break;
            
        case ConditionType::OR:
            evaluator_ = [this](const std::string& input) {
                for (const auto& child : children_) {
                    if (child->evaluate(input)) return true;
                }
                return false;
            };
            break;
            
        case ConditionType::NOT:
            evaluator_ = [this](const std::string& input) {
                if (children_.empty()) return true;
                return !children_[0]->evaluate(input);
            };
            break;
    }
}

bool Condition::evaluate(const std::string& input) const {
    return evaluator_(input);
}

std::string Condition::toString() const {
    switch (type_) {
        case ConditionType::ALWAYS:
            return "ALWAYS";
        case ConditionType::NEVER:
            return "NEVER";
        case ConditionType::STRING_MATCH:
            return "STRING_MATCH(\"" + pattern_ + "\")";
        case ConditionType::REGEX_MATCH:
            return "REGEX_MATCH(\"" + pattern_ + "\")";
        case ConditionType::AND: {
            std::string result = "AND(";
            for (size_t i = 0; i < children_.size(); ++i) {
                if (i > 0) result += " & ";
                result += children_[i]->toString();
            }
            return result + ")";
        }
        case ConditionType::OR: {
            std::string result = "OR(";
            for (size_t i = 0; i < children_.size(); ++i) {
                if (i > 0) result += " | ";
                result += children_[i]->toString();
            }
            return result + ")";
        }
        case ConditionType::NOT:
            return "NOT(" + (children_.empty() ? "" : children_[0]->toString()) + ")";
        default:
            return "UNKNOWN";
    }
}

// Factory methods
std::shared_ptr<Condition> Condition::createStringMatch(const std::string& pattern, bool case_sensitive) {
    // Note: This simple implementation doesn't handle case sensitivity
    return std::make_shared<Condition>(ConditionType::STRING_MATCH, pattern);
}

std::shared_ptr<Condition> Condition::createRegexMatch(const std::string& pattern) {
    return std::make_shared<Condition>(ConditionType::REGEX_MATCH, pattern);
}

std::shared_ptr<Condition> Condition::createAnd(std::vector<std::shared_ptr<Condition>> conditions) {
    return std::make_shared<Condition>(ConditionType::AND, conditions);
}

std::shared_ptr<Condition> Condition::createOr(std::vector<std::shared_ptr<Condition>> conditions) {
    return std::make_shared<Condition>(ConditionType::OR, conditions);
}

std::shared_ptr<Condition> Condition::createNot(std::shared_ptr<Condition> condition) {
    return std::make_shared<Condition>(ConditionType::NOT, std::vector<std::shared_ptr<Condition>>{condition});
}

// Simple expression parser (supports basic operations)
std::shared_ptr<Condition> Condition::parse(const std::string& expression) {
    // Remove whitespace
    std::string expr = expression;
    expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());
    
    if (expr.empty()) {
        return createStringMatch(""); // Match everything
    }
    
    // Handle NOT operator
    if (expr[0] == '!') {
        auto inner = parse(expr.substr(1));
        return createNot(inner);
    }
    
    // Handle AND/OR operators (simple version - no precedence)
    size_t and_pos = expr.find('&');
    size_t or_pos = expr.find('|');
    
    if (and_pos != std::string::npos && or_pos != std::string::npos) {
        // Both operators found - use the first one
        if (and_pos < or_pos) {
            auto left = parse(expr.substr(0, and_pos));
            auto right = parse(expr.substr(and_pos + 1));
            return createAnd({left, right});
        } else {
            auto left = parse(expr.substr(0, or_pos));
            auto right = parse(expr.substr(or_pos + 1));
            return createOr({left, right});
        }
    } else if (and_pos != std::string::npos) {
        auto left = parse(expr.substr(0, and_pos));
        auto right = parse(expr.substr(and_pos + 1));
        return createAnd({left, right});
    } else if (or_pos != std::string::npos) {
        auto left = parse(expr.substr(0, or_pos));
        auto right = parse(expr.substr(or_pos + 1));
        return createOr({left, right});
    }
    
    // Handle regex pattern: R(pattern)
    if (expr.size() >= 3 && expr[0] == 'R' && expr[1] == '(' && expr.back() == ')') {
        std::string pattern = expr.substr(2, expr.length() - 3);
        return createRegexMatch(pattern);
    }
    
    // Default: string match
    return createStringMatch(expr);
}

} // namespace condition