#pragma once
#include "Scene.h"

class Level1Scene : public Scene {
public:
    static constexpr int ENEMY_COUNT = 1;

    ~Level1Scene();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
