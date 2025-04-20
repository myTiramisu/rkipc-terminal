#pragma once
#include <string>
#include <string>
#include <vector>
#include <sstream>

// 观察者接口
class Observer {
public:
    virtual void update(const std::string& message, const std::string& mode) = 0;
    virtual ~Observer() {}

protected:
    // 辅助函数：分割字符串
    std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
};