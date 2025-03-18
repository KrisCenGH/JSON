/**
 * 这是一个JSON库, 可以解析JSON字符串以及将JSON对象序列化
 */

#ifndef JSON_PARSE_H__
#define JSON_PARSE_H__

#include <ostream>
#include <variant>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace JSON
{
    enum class JSON_type // JSON的七种类型(boolean算作两种) + 解析失败类型(undefined)
    {
        NULL_TYPE,
        TRUE_TYPE,
        FALSE_TYPE,
        NUMBER_TYPE,
        STRING_TYPE,
        ARRAY_TYPE,
        OBJECT_TYPE,

        UNDEFINED
    };

    enum class parse_state // JSON解析状态
    {
        OK, // 解析正确
        EXPECT_VALUE, // JSON只含有空白或空字符
        INVALID_VALUE, // 无效的JSON值
        ROOT_NOT_SINGULAR, // 多个根元素

        NUMBER_TOO_BIG, // NUMBER数字太大

        MISS_QUOTATION_MARK, // 缺失引号
        INVALID_STRING_CHAR, // 包含控制字符等不允许的字符
        INVALID_STRING_ESCAPE, // 无效的转义字符
        INVALID_UNICODE_HEX, // 无效的十六进制数
        INVALID_UNICODE_SURROGATE, // 无效的Unicode代理对或编码

        MISS_COMMA_OR_SQUARE_BRACKET, // 缺失逗号或者方括号

        MISS_KEY, // 缺失键
        MISS_COLON, // 缺失冒号
        MISS_COMMA_OR_CURLY_BRACKET // 缺失逗号或者花括号
    };

    class JSON_node final // JSON节点
    {
        public:
            JSON_node();
            ~JSON_node();
            JSON_node(const JSON_node &node);
            JSON_node(JSON_node &&node) noexcept;

            /**
             * root_obj用于标识是否将json参数视为一个完整JSON以进行解析. 如果为true, 则将*this解析为JSON对象, 否则, 解析为普通STRING_TYPE
             */
            JSON_node(const std::string &json, bool root_obj = true);
            JSON_node(const char *json, bool root_obj = true);

            JSON_node(bool flag);
            JSON_node(double num);

            static parse_state parse(JSON_node &node, const std::string &json); // 解析JSON字符串为JSON对象节点
            static std::string stringify(const JSON_node &node); // 将JSON对象节点序列化为JSON字符串

            JSON_type get_type(); // 获取当前节点类型

            void get_value(double &num); // 将要获取的值放在函数参数中返回
            void get_value(std::string &str);
            void get_value(std::vector<JSON_node> &arr);
            void get_value(std::unordered_map<std::string, JSON_node> &mp);

            void set_value_null(); // 将this->type设置为null

            JSON_node& operator[](std::string key);
            const JSON_node& operator[](std::string key) const;
            JSON_node& operator[](size_t index);
            const JSON_node& operator[](size_t index) const;

            JSON_node& operator=(const JSON_node &node); // 拷贝赋值运算符
            JSON_node& operator=(JSON_node &&node) noexcept; // 移动赋值运算符
            JSON_node& operator=(bool flag);
            JSON_node& operator=(int num);
            JSON_node& operator=(double num);
            JSON_node& operator=(const char *str); // 当右值为C风格字符串时, string参数版本的重载函数不会被正确调用, 因此需要此C风格字符串版本
            JSON_node& operator=(const std::string &str);
            JSON_node& operator=(const std::vector<JSON_node> &arr);
            JSON_node& operator=(const std::unordered_map<std::string, JSON_node> &mp);

            /**
             * +=: 为数组添加成员
             * -=: 为数组和对象删除成员
             */
            JSON_node& operator+=(bool flag); // 为数组类型添加成员
            JSON_node& operator+=(double num);
            JSON_node& operator+=(const char *str);
            JSON_node& operator+=(const std::string &str);
            JSON_node& operator+=(const std::vector<JSON_node> &arr);
            JSON_node& operator+=(const std::unordered_map<std::string, JSON_node> &mp);
            
            JSON_node& operator-=(size_t index); // 删除数组指定下标的成员

            JSON_node& operator-=(const char *key); // 根据指定key为对象类型删除成员
            JSON_node& operator-=(const std::string &key);

        private:
            JSON_type type; // 节点值的类型

            // 节点值
            std::variant<double, 
                        std::string, 
                        std::vector<JSON_node>,
                        std::unordered_map<std::string, JSON_node> 
                        > value;

        private:
            static parse_state parse_value(JSON_node &node, const std::string &json, size_t &index);
    
            static void parse_whitespace(const std::string &json, size_t &index); // 跳过空白字符

            static parse_state parse_null(JSON_node &node, const std::string &json, size_t &index); // 解析null

            static parse_state parse_true(JSON_node &node, const std::string &json, size_t &index); // 解析true

            static parse_state parse_false(JSON_node &node, const std::string &json, size_t &index); // 解析false

            static parse_state parse_number(JSON_node &node, const std::string &json, size_t &index); // 解析number

            static parse_state parse_unicode_escape(const std::string &json, size_t &index, std::string &str); // 解析Unicode转义序列

            static parse_state parse_string(JSON_node &node, const std::string &json, size_t &index); // 解析字符串

            static parse_state parse_array(JSON_node &node, const std::string &json, size_t &index); // 解析数组

            static parse_state parse_object(JSON_node &node, const std::string &json, size_t &index); // 解析字符串

            static std::string stringify_string(const std::string &str); // 序列化字符串(key, STRING_TYPE)

            static std::string output_string_parse(const JSON_node &node); // 配合operator<<进行字符串输出控制

        friend std::ostream& operator<<(std::ostream &os, const JSON_node &node);
    };

    std::ostream& operator<<(std::ostream &os, const JSON_node &node);
}

#endif
