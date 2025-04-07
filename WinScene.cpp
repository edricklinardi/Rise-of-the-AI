/**
* Author: Edrick Linardi
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "WinScene.h"
#include "Utility.h"

WinScene::~WinScene() {
    delete m_game_state.player;
}

void WinScene::initialise() {
    m_game_state.next_scene_id = -1;  // No transition yet

    Mix_HaltMusic();

    GLuint map_texture_id = Utility::load_texture("assets/tilemap_packed.png");
    m_font_texture_id = Utility::load_texture("assets/font_sprite.png");

    m_game_state.player = new Entity();
    m_game_state.player->set_lives(0);
}

void WinScene::update(float delta_time) {
}

void WinScene::render(ShaderProgram* program) {
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 ui_view_matrix = glm::mat4(1.0f); // Reset view matrix
    program->set_view_matrix(ui_view_matrix);

    Utility::draw_text(program, m_font_texture_id, "Congratulations! You WIN!", 0.3f, 0.01f, glm::vec3(-3.75f, 0.0f, 0));
    Utility::draw_text(program, m_font_texture_id, "Press R to restart", 0.3f, 0.01f, glm::vec3(-2.75f, -0.5f, 0));
}