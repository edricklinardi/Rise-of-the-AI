#pragma once
#include "Scene.h"

class Level2Scene : public Scene {
public:
    static constexpr int ENEMY_COUNT = 2;

    ~Level2Scene();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
