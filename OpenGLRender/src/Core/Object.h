#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Core/Component.h"

class Transform;

class Object {
public:
    std::string name;
    Transform* transform = nullptr;   // 每个 Object 自带 Transform

    explicit Object(std::string n = "Object");

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        auto c = std::make_unique<T>(std::forward<Args>(args)...);
        c->owner = this;
        T* ptr = c.get();
        m_Components.push_back(std::move(c));
        ptr->Start();
        return ptr;
    }

    template<typename T>
    T* GetComponent() {
        for (auto& c : m_Components)
            if (auto* p = dynamic_cast<T*>(c.get())) return p;
        return nullptr;
    }

    void Update(float dt);

private:
    std::vector<std::unique_ptr<Component>> m_Components;
};