#pragma once
#include <string>
#include <memory>
#include <functional>
#include <regex>
#include <vector>

namespace condition {

enum class ConditionType {
    ALWAYS,
    NEVER,
    STRING_MATCH,
    REGEX_MATCH,
    AND,
    OR,
    NOT
};

class Condition {
public:
    using EvalFunc = std::function<bool(const std::string&)>;
    
    Condition(ConditionType type, const std::string& pattern = "");
    Condition(ConditionType type, std::vector<std::shared_ptr<Condition>> children);
    
    bool evaluate(const std::string& input) const;
    std::string toString() const;
    
    // Factory methods
    static std::shared_ptr<Condition> parse(const std::string& expression);
    static std::shared_ptr<Condition> createStringMatch(const std::string& pattern, bool case_sensitive = true);
    static std::shared_ptr<Condition> createRegexMatch(const std::string& pattern);
    static std::shared_ptr<Condition> createAnd(std::vector<std::shared_ptr<Condition>> conditions);
    static std::shared_ptr<Condition> createOr(std::vector<std::shared_ptr<Condition>> conditions);
    static std::shared_ptr<Condition> createNot(std::shared_ptr<Condition> condition);
    
    // Convenience methods
    static std::shared_ptr<Condition> createAlways() { 
        return std::make_shared<Condition>(ConditionType::ALWAYS); 
    }
    
    static std::shared_ptr<Condition> createNever() { 
        return std::make_shared<Condition>(ConditionType::NEVER); 
    }
    
private:
    ConditionType type_;
    std::string pattern_;
    std::vector<std::shared_ptr<Condition>> children_;
    EvalFunc evaluator_;
    
    void initializeEvaluator();
};

} // namespace condition