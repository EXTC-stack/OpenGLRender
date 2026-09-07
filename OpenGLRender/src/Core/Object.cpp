#include "Core/Object.h"
#include "Components/Transform.h"

Object::Object(std::string n) : name(std::move(n)) {
    transform = AddComponent<Transform>();
}

void Object::Update(float dt) {
    for (auto& c : m_Components) c->Update(dt);
}