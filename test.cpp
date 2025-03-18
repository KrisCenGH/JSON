#include "JSON.hpp"

#include <iostream>

using namespace std;
using namespace JSON;

int main(void)
{
    JSON_node obj_1; // 默认构造函数

    // 使用字符串构造一个JSON对象
    JSON_node obj_2(R"(
        {
            "test1": "Hello, World! 这是一段测试文本",
            "测试2": 123,

            "test4": false,      "test5  "   :   [12.57,  "true", {}, {"not":0}, []]

        }
    )");
    cout << obj_2 << endl;
    // 输出：{ "test4": false, "test5  ": [ 12.570000, "true", { }, { "not": 0.000000 }, [ ] ], "测试2": 123.000000, "test1": "Hello, World! 这是一段测试文本" }

    string new_s = JSON_node::stringify(obj_2);
    cout << new_s << endl;
    // 输出：{"test4":false,"test5  ":[12.570000,"true",{},{"not":0.000000},[]],"测试2":123.000000,"test1":"Hello, World! 这是一段测试文本"}

    JSON_node::parse(obj_1, new_s);
    cout << obj_1 << endl;
    // 输出：{ "test1": "Hello, World! 这是一段测试文本", "测试2": 123.000000, "test5  ": [ 12.570000, "true", { }, { "not": 0.000000 }, [ ] ], "test4": false }

    obj_2["测试2"] = 58.1;
    cout << obj_2["测试2"] << endl;
    // 输出：58.100000

    obj_2["这是一个新的测试"] = 79;
    obj_2["test5  "] += false;
    obj_2["test5  "][3]["not"] = "0000";
    cout << obj_2["test5  "] << endl;
    // 输出：[ 12.570000, "true", { }, { "not": "0000" }, [ ], false ]
    cout << obj_2["test5  "][3]["not"] << endl;
    // 输出："0000"

    return 0;
}