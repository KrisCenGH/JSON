# Modern C++ JSON 解析库

## 简介

这是一个基于递归下降解析原理的轻量级 JSON 库。库中的 `JSON_node` 类能够轻松地将 JSON 字符串转换为 C++ 数据结构，同时支持将数据结构序列化为 JSON 格式字符串。无论是解析、修改还是输出，都可以通过直观的运算符重载实现，简化了使用者对 JSON 数据的操作。

## 特性

- **以` std::variant `类型存储多值**

- **以` double `存储` number `类型 **

- **` object `类型基于` unordered_map `构建，不支持重复键**

- **仅支持UTF-8文本解析**

- **便捷的数据访问和修改**
  通过重载运算符（如 `operator[]`、`operator+=`、`operator-=`）提供直观的对象和数组操作接口，极大提高代码可读性。

## 依赖

- **C++ 17**

## 基本用户 API

### 1. 构造与解析

- **直接构造 JSON 对象**

  可以直接通过字符串构造一个 `JSON_node` 对象。

  ```cpp
  #include "JSON.hpp"
  using namespace JSON;

  // 使用原始字符串构造 JSON 对象
  JSON_node obj(R"({
      "name": "Alice",
      "age": 30,
      "isMember": true
  })");```
