#include "JSON.hpp"

using namespace JSON;

JSON_node::JSON_node() : type(JSON_type::OBJECT_TYPE), value(std::unordered_map<std::string, JSON_node>())
{
    /* empty */
}

JSON_node::~JSON_node()
{
    /* empty */
}

JSON_node::JSON_node(const JSON_node &node) : type(node.type), value(node.value)
{
    /* empty */
}

JSON_node::JSON_node(JSON_node &&node) noexcept : type(node.type), value(node.value)
{
    /* empty */
}

JSON_node::JSON_node(const std::string &json, bool root_obj) : type(JSON_type::OBJECT_TYPE), value(std::unordered_map<std::string, JSON_node>())
{
    if (root_obj) {
        auto state = parse(*this, json);
        if (state != parse_state::OK) {
            throw std::runtime_error("JSON format is invalid!");
        }
    } else {
        this->type = JSON_type::STRING_TYPE;
        this->value = json;
    }
}

JSON_node::JSON_node(const char *json, bool root_obj) : type(JSON_type::OBJECT_TYPE), value(std::unordered_map<std::string, JSON_node>())
{
    if (root_obj) {
        auto state = parse(*this, json);
        if (state != parse_state::OK) {
            throw std::runtime_error("JSON format is invalid!");
        }
    } else {
        this->type = JSON_type::STRING_TYPE;
        this->value = json;
    }
}

JSON_node::JSON_node(bool flag)
{
    if (flag)
        this->type = JSON_type::TRUE_TYPE;
    else
        this->type = JSON_type::FALSE_TYPE;
    this->value = 0.0;
}

JSON_node::JSON_node(double num) : type(JSON_type::NUMBER_TYPE), value(num)
{

}

parse_state JSON_node::parse(JSON_node &node, const std::string &json)
{
    if (node.type == JSON_type::UNDEFINED)
        return parse_state::INVALID_VALUE;

    size_t index = 0; // 字符索引
    node.type = JSON_type::UNDEFINED; // 如解析失败, 则为undefined

    parse_whitespace(json, index);

    auto res = parse_value(node, json, index);
    if (res == parse_state::OK) {
        parse_whitespace(json, index);
        if (index < json.size() && json[index] != '\0') { // 格式不合法
            node.type = JSON_type::UNDEFINED;
            res = parse_state::ROOT_NOT_SINGULAR;
        }
    }

    return res;
}

std::string JSON_node::stringify(const JSON_node &node)
{
    if (node.type == JSON_type::UNDEFINED)
        return "";

    std::string json = "";
    if (node.type == JSON_type::ARRAY_TYPE) {
        json = "[";
    } else if (node.type == JSON_type::OBJECT_TYPE) {
        json = "{";
    }

    switch (node.type)
    {
        case JSON_type::NULL_TYPE:
            json += "null";
            break;

        case JSON_type::TRUE_TYPE:
            json += "true";
            break;

        case JSON_type::FALSE_TYPE:
            json += "false";
            break;

        case JSON_type::NUMBER_TYPE:
            json += std::to_string(std::get<double>(node.value));
            break;

        case JSON_type::STRING_TYPE:
            json += stringify_string(std::get<std::string>(node.value));
            break;

        case JSON_type::ARRAY_TYPE: {
            auto &arr = std::get<std::vector<JSON_node>>(node.value);
            for (const auto &sub_node : arr) {
                json += stringify(sub_node) + ',';
            }

            if (json.back() == ',')
                json.pop_back(); // 弹出最后一个逗号
            json += ']';
            break;
        }

        case JSON_type::OBJECT_TYPE: {
            auto &kv = std::get<std::unordered_map<std::string, JSON_node>>(node.value);
            for (const auto &[key, value] : kv) {
                json += stringify_string(key) + ':'; // 解析key
                json += stringify(value) + ','; // 解析value
            }

            if (json.back() == ',')
                json.pop_back(); // 弹出最后一个逗号
            json += '}';
            break;
        }

        default:
            json = "";
    }

    return json;
}

JSON_type JSON_node::get_type()
{
    return this->type;
}

void JSON_node::get_value(double &num) // 将要获取的值放在函数参数中返回
{
    if (this->type != JSON_type::NUMBER_TYPE)
        throw std::runtime_error("This type is not number!");
    num = std::get<double>(this->value);
}

void JSON_node::get_value(std::string &str)
{
    if (this->type != JSON_type::STRING_TYPE)
        throw std::runtime_error("This type is not string!");
    str = std::get<std::string>(this->value);
}

void JSON_node::get_value(std::vector<JSON_node> &arr)
{
    if (this->type != JSON_type::ARRAY_TYPE)
        throw std::runtime_error("This type is not array!");
    arr = std::get<std::vector<JSON_node>>(this->value);
}

void JSON_node::get_value(std::unordered_map<std::string, JSON_node> &mp)
{
    if (this->type != JSON_type::OBJECT_TYPE)
        throw std::runtime_error("This type is not object!");
    mp = std::get<std::unordered_map<std::string, JSON_node>>(this->value);
}

void JSON_node::set_value_null()
{
    this->type = JSON_type::NULL_TYPE;
    this->value = 0.0;
}

parse_state JSON_node::parse_value(JSON_node &node, const std::string &json, size_t &index)
{
    if (index > json.size()) // 越界检查, 注意'\0'字符
        return parse_state::INVALID_VALUE;

    switch (json[index])
    {
        case 'n':
            return parse_null(node, json, index);

        case 't':
            return parse_true(node, json, index);

        case 'f':
            return parse_false(node, json, index);

        case '"':
            return parse_string(node, json, index);

        case '[':
            return parse_array(node, json, index);

        case '{':
            return parse_object(node, json, index);

        case '\0':
            return parse_state::EXPECT_VALUE;
        
        default:                                        // 解析number
            return parse_number(node, json, index);
    }
}

void JSON_node::parse_whitespace(const std::string &json, size_t &index) // 跳过空白字符
{
    while (index < json.size() && (json[index] == ' ' || json[index] == '\n' || json[index] == '\t' || json[index] == '\r'))
        ++index;
}

parse_state JSON_node::parse_null(JSON_node &node, const std::string &json, size_t &index) // 解析null
{
    if (index + 3 >= json.size() || json[index] != 'n' || json[index + 1] != 'u' || json[index + 2] != 'l' || json[index + 3] != 'l')
        return parse_state::INVALID_VALUE;
    node.type = JSON_type::NULL_TYPE;
    index += 4;

    return parse_state::OK;
}

parse_state JSON_node::parse_true(JSON_node &node, const std::string &json, size_t &index) // 解析true
{
    if (index + 3 >= json.size() || json[index] != 't' || json[index + 1] != 'r' || json[index + 2] != 'u' || json[index + 3] != 'e')
        return parse_state::INVALID_VALUE;
    node.type = JSON_type::TRUE_TYPE;
    index += 4;

    return parse_state::OK;
}

parse_state JSON_node::parse_false(JSON_node &node, const std::string &json, size_t &index) // 解析false
{
    if (index + 4 >= json.size() || json[index] != 'f' || json[index + 1] != 'a' || json[index + 2] != 'l' || json[index + 3] != 's' || json[index + 4] != 'e')
        return parse_state::INVALID_VALUE;
    node.type = JSON_type::FALSE_TYPE;
    index += 5;

    return parse_state::OK;
}

parse_state JSON_node::parse_number(JSON_node &node, const std::string &json, size_t &index) // 解析number
{
    std::size_t pos;
    std::string str = "";
    for (size_t i = index; i < json.size(); ++i) {
        if (json[i] == ' ' || json[i] == ',' || json[i] == ']' || json[i] == '}' || json[i] == '\n' || json[i] == '\t' || json[i] == '\r')
            break;
        str += json[i];
    }

    try {
        // 部分格式JSON并不允许, 但也能成功转换而不抛出异常, 需要自行判断
        size_t p = 0;
        if (str[p] == '-')
            ++p;
        if (str[p] == '0') {
            ++p;
            if (str[p] != '.' && str[p] != '\0') { //不允许前导0
                return parse_state::ROOT_NOT_SINGULAR;
            }
        } else {
            if (!(str[p] >= '1' && str[p] <= '9'))
                throw std::invalid_argument("Incorrect format.");
            while (str[p] >= '0' && str[p] <= '9')
                ++p;
        }
        if (str[p] == '.') {
            ++p;
            if (!(str[p] >= '0' && str[p] <= '9'))
                throw std::invalid_argument("Incorrect format.");
            while (str[p] >= '0' && str[p] <= '9')
                ++p;
        }
        if (str[p] == 'e' || str[p] == 'E') {
            ++p;
            if (str[p] == '+' || str[p] == '-')
                ++p;
            if (!(str[p] >= '0' && str[p] <= '9'))
                throw std::invalid_argument("Incorrect format.");
            while (str[p] >= '0' && str[p] <= '9')
                ++p;
        }
            
        node.value = std::stod(str, &pos);
    } catch(const std::invalid_argument& e) { // 无效参数
        return parse_state::INVALID_VALUE;
    } catch(const std::out_of_range& e) { // 超出范围
        return parse_state::NUMBER_TOO_BIG;
    }

    if (json[index] == pos)
        return parse_state::INVALID_VALUE;
    node.type = JSON_type::NUMBER_TYPE;
    index += pos;

    return parse_state::OK;
}

parse_state JSON_node::parse_unicode_escape(const std::string &json, size_t &index, std::string &str)
{
    // 解析第一个 "\uXXXX"
    if (index + 4 >= json.size())
        return parse_state::INVALID_UNICODE_HEX;
    uint32_t codepoint = 0;
    for (int i = 1; i <= 4; ++i) {
        char hexChar = json[index + i];
        uint32_t digit = 0;
        if (hexChar >= '0' && hexChar <= '9')
            digit = hexChar - '0';
        else if (hexChar >= 'A' && hexChar <= 'F')
            digit = hexChar - 'A' + 10;
        else if (hexChar >= 'a' && hexChar <= 'f')
            digit = hexChar - 'a' + 10;
        else
            return parse_state::INVALID_UNICODE_HEX;
        codepoint = (codepoint << 4) | digit;
    }
    index += 4; // 消耗四个16进制数字
    
    // 检查是否为代理项对的高代理
    if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
        // 高代理项，必须存在低代理项 "\uXXXX"
        if (index + 6 < json.size() &&
            json[index+1] == '\\' &&
            json[index+2] == 'u') {
            index += 2; // 进入低代理的 "\u"
            uint32_t low = 0;
            for (int i = 1; i <= 4; ++i) {
                char hexChar = json[index + i];
                uint32_t digit = 0;
                if (hexChar >= '0' && hexChar <= '9')
                    digit = hexChar - '0';
                else if (hexChar >= 'A' && hexChar <= 'F')
                    digit = hexChar - 'A' + 10;
                else if (hexChar >= 'a' && hexChar <= 'f')
                    digit = hexChar - 'a' + 10;
                else
                    return parse_state::INVALID_UNICODE_HEX;
                low = (low << 4) | digit;
            }
            // 检查 low 是否在有效范围内
            if (low < 0xDC00 || low > 0xDFFF)
                return parse_state::INVALID_UNICODE_SURROGATE;
            index += 4; // 消耗低代理的四个16进制数字
        
            // 计算真实 Unicode code point
            codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
        }
    }
    
    // 将 codepoint 转换为 UTF-8 字符串
    if (codepoint <= 0x7F) {
        str.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        str.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
        str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        str.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
        str.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0x10FFFF) {
        str.push_back(static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07)));
        str.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        str.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        return parse_state::INVALID_UNICODE_HEX;
    }
    
    return parse_state::OK;
}

parse_state JSON_node::parse_string(JSON_node &node, const std::string &json, size_t &index)
{
    std::string str = "";
    ++index;

    while (index <= json.size()) { // 注意'\0', size()所返回的大小不包括空字符
        switch (json[index])
        {
            case '\"': // 处理到字符串的末尾, 成功退出
                node.type = JSON_type::STRING_TYPE;
                node.value = str;
                ++index;
                return parse_state::OK;

            case '\\': // 处理转义字符
                ++index;
                switch (json[index])
                {
                    case '\\': str += '\\'; break;
                    case '\"': str += '\"'; break;
                    case '/': str += '/'; break;
                    case 'b': str += '\b'; break;
                    case 'f': str += '\f'; break;
                    case 'n': str += '\n'; break;
                    case 'r': str += '\r'; break;
                    case 't': str += '\t'; break;
                    case 'u': {
                        auto state = parse_unicode_escape(json, index, str);
                        if (state != parse_state::OK)
                            return state;
                        break;
                    }
                    default:
                        return parse_state::INVALID_STRING_ESCAPE;
                }
                break;

            case '\0':
                return parse_state::MISS_QUOTATION_MARK;

            default: { // 非\u样式的UTF8字符以及控制字符解析
                if (json[index] >= 0 && json[index] < 32) { // 根据ASCII码, 排除控制字符
                    return parse_state::INVALID_STRING_CHAR;
                }

                int length = 1;
                if ((json[index] & 0xf8) == 0xf0)
                    length = 4;
                else if ((json[index] & 0xf0) == 0xe0)
                    length = 3;
                else if ((json[index] & 0xe0) == 0xc0)
                    length = 2;

                if (index + length > json.size())
                    length = 1;

                str += json.substr(index, length);
                index += length - 1;
            }
        }
        ++index;
    }

    return parse_state::MISS_QUOTATION_MARK;
}

parse_state JSON_node::parse_array(JSON_node &node, const std::string &json, size_t &index)
{
    std::vector<JSON_node> arr;

    ++index;
    parse_whitespace(json, index);

    if (index < json.size() && json[index] == ']') { // 空数组
        node.value = std::move(arr);
        node.type = JSON_type::ARRAY_TYPE;
        ++index;

        return parse_state::OK;
    }

    while (1) {
        JSON_node sub_node;
        auto state = parse_value(sub_node, json, index);
        if (state != parse_state::OK)
            return state;

        arr.emplace_back(std::move(sub_node));

        parse_whitespace(json, index);
        if (index < json.size() && json[index] == ',') {
            ++index;
            parse_whitespace(json, index);
        } else if (index < json.size() && json[index] == ']') { // 数组解析完毕
            node.value = std::move(arr);
            node.type = JSON_type::ARRAY_TYPE;
            ++index;

            return parse_state::OK;
        } else {
            return parse_state::MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }

    return parse_state::INVALID_VALUE;
}

parse_state JSON_node::parse_object(JSON_node &node, const std::string &json, size_t &index)
{
    std::unordered_map<std::string, JSON_node> kv; // 临时键值对

    ++index;
    parse_whitespace(json, index);

    if (index < json.size() && json[index] == '}') { // 空对象
        node.value = std::move(kv);
        node.type = JSON_type::OBJECT_TYPE;
        ++index;

        return parse_state::OK;
    }

    while (1) {
        if (index <= json.size() && json[index] != '"')
            return parse_state::MISS_KEY;

        JSON_node key; // 临时存储键
        auto state = parse_string(key, json, index); // 解析键
        if (state != parse_state::OK)
            return parse_state::INVALID_VALUE;

        parse_whitespace(json, index);
        if (index <= json.size() && json[index] != ':')
            return parse_state::MISS_COLON;
        ++index;
        parse_whitespace(json, index);

        JSON_node value; // 存储值
        state = parse_value(value, json, index);
        if (state != parse_state::OK)
            return state;
        
        std::string temp_s = std::get<std::string>(key.value);
        kv[temp_s] = std::move(value); // 插入键值对

        parse_whitespace(json, index);
        if (index < json.size() && json[index] == ',') {
            ++index;
            parse_whitespace(json, index);
        } else if (index < json.size() && json[index] == '}') { // 对象解析完毕
            node.value = std::move(kv);
            node.type = JSON_type::OBJECT_TYPE;
            ++index;

            return parse_state::OK;
        } else {
            return parse_state::MISS_COMMA_OR_CURLY_BRACKET;
        }
    }

    return parse_state::INVALID_VALUE;
}

std::string JSON_node::stringify_string(const std::string &str)
{
    std::string res = "\"";

    for (size_t i = 0; i < str.size(); ++i) {
        switch (str[i]) {
            case '\\': res += "\\\\"; break;
            case '\"': res += "\\\""; break;
            case '\b': res += "\\b"; break;
            case '\f': res += "\\f"; break;
            case '\n': res += "\\n"; break;
            case '\r': res += "\\r"; break;
            case '\t': res += "\\t"; break;
            default:
                res += str[i];
        }
    }

    res += '"';
    return res;
}

JSON_node& JSON_node::operator[](std::string key)
{
    if (this->type == JSON_type::OBJECT_TYPE) {
        auto& map = std::get<std::unordered_map<std::string, JSON_node>>(this->value);
        return map[key];
    } else {
        throw std::runtime_error("This type is not object!");
    }
}

const JSON_node& JSON_node::operator[](std::string key) const
{
    if (this->type == JSON_type::OBJECT_TYPE) {
        auto &map = std::get<std::unordered_map<std::string, JSON_node>>(this->value);
        return map.at(key);
    } else {
        throw std::runtime_error("This type is not object!");
    }
}

JSON_node& JSON_node::operator[](size_t index)
{
    if (this->type == JSON_type::ARRAY_TYPE) {
        auto &arr = std::get<std::vector<JSON_node>>(this->value);
        return arr[index];
    } else {
        throw std::runtime_error("This type is not array!");
    }
}

const JSON_node& JSON_node::operator[](size_t index) const
{
    if (this->type == JSON_type::ARRAY_TYPE) {
        auto &arr = std::get<std::vector<JSON_node>>(this->value);
        return arr[index];
    } else {
        throw std::runtime_error("This type is not array!");
    }
}

JSON_node& JSON_node::operator=(const JSON_node &node) // 拷贝赋值运算符
{
    if (this != &node) {
        this->type = node.type;
        this->value = node.value;
    }
    return *this;
}

JSON_node& JSON_node::operator=(JSON_node &&node) noexcept // 移动赋值运算符
{
    if (this != &node) {
        this->type = node.type;
        this->value = node.value;
    }
    return *this;
}

JSON_node& JSON_node::operator=(bool flag)
{
    if (flag)
        this->type = JSON_type::TRUE_TYPE;
    else
        this->type = JSON_type::FALSE_TYPE;
    this->value = 0.0;
    return *this;
}

JSON_node& JSON_node::operator=(int num)
{
    this->type = JSON_type::NUMBER_TYPE;
    this->value = static_cast<double>(num);
    return *this;
}

JSON_node& JSON_node::operator=(double num)
{
    this->type = JSON_type::NUMBER_TYPE;
    this->value = num;
    return *this;
}

JSON_node& JSON_node::operator=(const char *str)
{
    std::string temp_str(str);

    this->type = JSON_type::STRING_TYPE;
    this->value = temp_str;
    return *this;
}

JSON_node& JSON_node::operator=(const std::string &str)
{
    this->type = JSON_type::STRING_TYPE;
    this->value = str;
    return *this;
}

JSON_node& JSON_node::operator=(const std::vector<JSON_node> &arr)
{
    this->type = JSON_type::ARRAY_TYPE;
    this->value = arr;
    return *this;
}

JSON_node& JSON_node::operator=(const std::unordered_map<std::string, JSON_node> &mp)
{
    this->type = JSON_type::OBJECT_TYPE;
    this->value = mp;
    return *this;
}

JSON_node& JSON_node::operator+=(bool flag) // 为数组类型添加成员
{
    if (this->type != JSON_type::ARRAY_TYPE)
        throw std::runtime_error("This type is not array!");
    JSON_node node(flag);
    std::get<std::vector<JSON_node>>(this->value).emplace_back(std::move(node));
    return *this;
}

JSON_node& JSON_node::operator+=(double num)
{
    if (this->type != JSON_type::ARRAY_TYPE)
        throw std::runtime_error("This type is not array!");
    JSON_node node(num);
    std::get<std::vector<JSON_node>>(this->value).emplace_back(std::move(node));
    return *this;
}

JSON_node& JSON_node::operator+=(const char *str)
{
    if (this->type != JSON_type::ARRAY_TYPE)
        throw std::runtime_error("This type is not array!");
    JSON_node node(str, false);
    std::get<std::vector<JSON_node>>(this->value).emplace_back(std::move(node));
    return *this;
}

JSON_node& JSON_node::operator+=(const std::string &str)
{
    if (this->type != JSON_type::ARRAY_TYPE)
        throw std::runtime_error("This type is not array!");
    JSON_node node(str, false);
    std::get<std::vector<JSON_node>>(this->value).emplace_back(std::move(node));
    return *this;
}

JSON_node& JSON_node::operator+=(const std::vector<JSON_node> &arr)
{
    if (this->type != JSON_type::ARRAY_TYPE)
        throw std::runtime_error("This type is not array!");
    JSON_node node;
    node = arr;
    std::get<std::vector<JSON_node>>(this->value).emplace_back(std::move(node));
    return *this;
}

JSON_node& JSON_node::operator+=(const std::unordered_map<std::string, JSON_node> &mp)
{
    if (this->type != JSON_type::ARRAY_TYPE)
        throw std::runtime_error("This type is not array!");
    JSON_node node;
    node = mp;
    std::get<std::vector<JSON_node>>(this->value).emplace_back(std::move(node));
    return *this;
}

JSON_node& JSON_node::operator-=(size_t index) // 删除数组指定下标的成员
{
    if (this->type != JSON_type::ARRAY_TYPE)
        throw std::runtime_error("This type is not array!");
    auto &arr = std::get<std::vector<JSON_node>>(this->value);
    arr.erase(arr.begin() + index);
    return *this;
}

JSON_node& JSON_node::operator-=(const char *key) // 根据指定key为对象类型删除成员
{
    if (this->type != JSON_type::OBJECT_TYPE)
        throw std::runtime_error("This type is not object!");
    auto &mp = std::get<std::unordered_map<std::string, JSON_node>>(this->value);
    mp.erase(key);
    return *this;
}

JSON_node& JSON_node::operator-=(const std::string &key)
{
    if (this->type != JSON_type::OBJECT_TYPE)
        throw std::runtime_error("This type is not object!");
    auto &mp = std::get<std::unordered_map<std::string, JSON_node>>(this->value);
    mp.erase(key);
    return *this;
}

std::string JSON_node::output_string_parse(const JSON_node &node)
{
    std::string str = "";
    switch (node.type)
    {
        case JSON_type::NULL_TYPE:
            str = "null";
            break;
        
        case JSON_type::TRUE_TYPE:
            str = "true";
            break;

        case JSON_type::FALSE_TYPE:
            str = "false";
            break;

        case JSON_type::NUMBER_TYPE:
            str = std::to_string(std::get<double>(node.value));
            break;

        case JSON_type::STRING_TYPE:
            str = "\"" + std::get<std::string>(node.value) + "\"";
            break;

        case JSON_type::ARRAY_TYPE: {
            str += "[";
            for (const auto &sub_node : std::get<std::vector<JSON_node>>(node.value)) {
                str += " " + output_string_parse(sub_node) + ",";
            }
            if (str.back() == ',')
                str.pop_back();
            str += " ]";
            break;
        }

        case JSON_type::OBJECT_TYPE: {
            str += "{";
            for (const auto &[key, value] : std::get<std::unordered_map<std::string, JSON_node>>(node.value)) {
                str += " \"" +  key + "\": " + output_string_parse(value) + ",";
            }
            if (str.back() == ',')
                str.pop_back();
            str += " }";
            break;
        }

        default:
            throw std::runtime_error("Output failed!");
    }

    return str;
}

std::ostream& JSON::operator<<(std::ostream &os, const JSON_node &node)
{
    os << JSON_node::output_string_parse(node);
    return os;
}
