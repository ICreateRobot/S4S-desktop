#include <cstdlib>
#include <cerrno>
#include <string>
#include <iostream>

enum class ValueType
{
    INVALID,
    INT,
    FLOAT,
    STRING
};

struct ParseValue
{
    ValueType type = ValueType::INVALID;
    int i_val = 0;
    float d_val = 0.0;
    std::string s_val;
};

ParseValue parse_value(const std::string &input)
{
    ParseValue result;

    if (input.empty())
        return result;

    /* ---------- 1. 引号字符串 ---------- */
    if (input.size() >= 2 &&
        input.front() == '"' &&
        input.back() == '"')
    {
        result.type = ValueType::STRING;
        result.s_val = input.substr(1, input.size() - 2); // 去掉引号
        return result;
    }

    /* ---------- 2. 整数 ---------- */
    {
        char *end = nullptr;
        errno = 0;
        long v = std::strtol(input.c_str(), &end, 10);

        if (errno == 0 && end != input.c_str() && *end == '\0')
        {
            result.type = ValueType::INT;
            result.i_val = static_cast<int>(v);
            return result;
        }
    }

    /* ---------- 3. 浮点 ---------- */
    {
        char *end = nullptr;
        errno = 0;
        float v = std::strtod(input.c_str(), &end);

        if (errno == 0 && end != input.c_str() && *end == '\0')
        {
            result.type = ValueType::FLOAT;
            result.d_val = v;
            return result;
        }
    }

    /* ---------- 4. 非法 ---------- */
    return result;
}

void test(const std::string &s)
{
    auto v = parse_value(s);

    std::cout << "Input: " << s << " -> ";

    switch (v.type)
    {
    case ValueType::INT:
        std::cout << "INT: " << v.i_val;
        break;
    case ValueType::FLOAT:
        std::cout << "FLOAT: " << v.d_val;
        break;
    case ValueType::STRING:
        std::cout << "STRING: \"" << v.s_val << "\"";
        break;
    default:
        std::cout << "INVALID";
        break;
    }
    std::cout << std::endl;
}

int main()
{
    test("123");
    test("-23");
    test("-23.4");
    test("\"123\"");
    test("\"adbc\"");
    test("abc");
}
