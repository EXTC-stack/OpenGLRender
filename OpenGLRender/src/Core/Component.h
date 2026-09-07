#pragma once

class Object;  // 前向声明，避免循环引用

class Component {
public:
    Object* owner = nullptr;
    virtual ~Component() = default;
    virtual void Start() {}
    virtual void Update(float dt) {}
};