#pragma once
#include "Scene.h"

class WinScene : public Scene {
public:
    GLuint m_font_texture_id;

    ~WinScene();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};