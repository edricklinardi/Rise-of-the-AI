#pragma once
#include "Scene.h"

class Level3Scene : public Scene {
public:

	static constexpr int ENEMY_COUNT = 3;

    ~Level3Scene();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
