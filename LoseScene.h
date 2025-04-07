#pragma once
#include "Scene.h"

class LoseScene : public Scene {
public:
    GLuint m_font_texture_id;

    ~LoseScene();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};