/**
* Author: Edrick Linardi
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h> 
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "Entity.h"
#include "Utility.h"
#include "Map.h"
#include "Scene.h"
#include "MenuScene.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "LoseScene.h"
#include "WinScene.h"

#include <vector>
#include <cstdlib>
#include <ctime>

// ————— STRUCTS AND ENUMS ————— //
enum AppStatus { RUNNING, TERMINATED };

// ————— CONSTANTS ————— //

constexpr int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr float MILLISECONDS_IN_SECOND = 1000.0f;

// ————— TIMING ————— //
float g_previous_ticks = 0.0f;
float g_time_accumulator = 0.0f;

// ————— MATRICES & VECTORS ————— //
glm::mat4 g_view_matrix, g_projection_matrix;


// ————— SCENE SETUP ————— //
Scene* g_current_scene;
MenuScene* g_menu;
Level1Scene* g_level1;
Level2Scene* g_level2;
Level3Scene* g_level3;
WinScene* g_win_scene;
LoseScene* g_lose_scene;

Scene* g_levels[6];

int lives;

GLuint heart_texture_id;

void switch_to_scene(Scene* scene) 
{
    g_current_scene = scene;
    g_current_scene->initialise();
}

void render_hearts(ShaderProgram* program, GLuint texture_id, int life_count) {
    // Reset to UI space
    glm::mat4 ui_view_matrix = glm::mat4(1.0f);
    program->set_view_matrix(ui_view_matrix);

    // Render hearts from right to left
    for (int i = 0; i < life_count; ++i) {
        glm::mat4 model_matrix = glm::mat4(1.0f);
        float x = 4.3f - i * 0.7f;  // adjust spacing
        float y = 3.2f;

        model_matrix = glm::translate(model_matrix, glm::vec3(x, y, 0.0f));
        model_matrix = glm::scale(model_matrix, glm::vec3(0.6f, 0.6f, 1.0f)); // adjust size

        program->set_model_matrix(model_matrix);

        float vertices[] = {
            -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f
        };

        float tex_coords[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
        };

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());

        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    }

    // Restore the game view matrix after drawing UI
    program->set_view_matrix(g_view_matrix);
}

void initialise();
void process_input();
void update();
void render();
void shutdown();

void initialise()
{
    // Initialise video
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    g_display_window = SDL_CreateWindow("Rise of the AI",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
		shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    heart_texture_id = Utility::load_texture("assets/heart.png");

	// SCENE SETUP
    g_menu = new MenuScene();
    g_level1 = new Level1Scene();
	g_level2 = new Level2Scene();
	g_level3 = new Level3Scene();
	g_win_scene = new WinScene();
	g_lose_scene = new LoseScene();

	g_levels[0] = g_menu;
	g_levels[1] = g_level1;
	g_levels[2] = g_level2;
    g_levels[3] = g_level3;
	g_levels[4] = g_win_scene;
	g_levels[5] = g_lose_scene;

    lives = 3;

	switch_to_scene(g_menu);
}

void process_input()
{
    if (g_current_scene != g_menu) 
    {
        g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q:
				g_app_status = TERMINATED;
                break;
            case SDLK_RETURN:
				if (g_current_scene == g_menu)
				{
					switch_to_scene(g_level3);
				}
                break;
			case SDLK_r:
                lives = 3;
				switch_to_scene(g_menu);
				break;
            case SDLK_SPACE:
                // Jump
                if (g_current_scene != g_menu) 
                {
                    if (g_current_scene->get_state().player->get_collided_bottom())
                    {
                        g_current_scene->get_state().player->jump();
                        Mix_PlayChannel(-1, g_current_scene->get_state().jump_sfx, 0);
                    }
                }

                break;
            default: break;
            }

        default:
            break;
        }
    }

    if (g_current_scene->get_state().player != nullptr)
    {
        const Uint8* key_state = SDL_GetKeyboardState(NULL);

        if (key_state[SDL_SCANCODE_A])
        {
            g_current_scene->get_state().player->set_animation_row(1);
            g_current_scene->get_state().player->move_left();
        }
        else if (key_state[SDL_SCANCODE_D])
        {
            g_current_scene->get_state().player->set_animation_row(1);
            g_current_scene->get_state().player->move_right();
        }
        else
        {
            g_current_scene->get_state().player->set_animation_row(0);
            if (g_current_scene->get_state().player->get_movement().x < 0) 
                g_current_scene->get_state().player->face_left();
            else 
                g_current_scene->get_state().player->face_right();
        }
            

        if (glm::length(g_current_scene->get_state().player->get_movement()) > 1.0f)
            g_current_scene->get_state().player->normalise_movement();
    }
}

void update()
{
    // ————— DELTA TIME ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // ————— FIXED TIMESTEP ————— //
    delta_time += g_time_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_time_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_current_scene->update(FIXED_TIMESTEP);
        delta_time -= FIXED_TIMESTEP;
    }

    g_time_accumulator = delta_time;

    g_view_matrix = glm::mat4(1.0f);

	if (g_current_scene->get_state().player != nullptr)
	{
        if (g_current_scene->get_state().player->get_position().x > 5.0f) {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
        }
        else {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
        }
	}

    if (g_current_scene->get_state().next_scene_id != -1) {
        switch (g_current_scene->get_state().next_scene_id) {
        case 1:
            switch_to_scene(g_level1);
            break;
        case 2:
			switch_to_scene(g_level2);
			break;
        case 3:
			switch_to_scene(g_level3);
			break;
		case 4:
			switch_to_scene(g_win_scene);
            break;
        case 5:
			switch_to_scene(g_lose_scene);
			break;
        }
    }
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    g_current_scene->render(&g_shader_program);

    if (g_current_scene->get_state().player != nullptr && g_current_scene != g_win_scene && g_current_scene != g_lose_scene) {
        int current_lives = g_current_scene->get_state().player->get_lives();
        render_hearts(&g_shader_program, heart_texture_id, current_lives);
    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() 
{ 
    SDL_Quit(); 

    delete g_menu;
	delete g_level1;
}

int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        
        process_input();
		update();
        render();
    }

    shutdown();
    return 0;
}