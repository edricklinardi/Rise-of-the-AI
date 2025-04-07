/**
* Author: Edrick Linardi
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Level3.h"
#include "Utility.h"

#define LEVEL3_WIDTH 30
#define LEVEL3_HEIGHT 8

extern ShaderProgram g_shader_program;

constexpr char BACKGROUND_FILEPATH[] = "assets/sky_background.png";
constexpr char PLAYER_SPRITE_FILEPATH[] = "assets/Player/Player.png";
constexpr char GUARD_SPRITE_FILEPATH[] = "assets/AI/Guard.png";
constexpr char WALKER_SPRITE_FILEPATH[] = "assets/AI/Walker.png";
constexpr char FLYER_SPRITE_FILEPATH[] = "assets/AI/Flyer.png";
constexpr char TILEMAP_FILEPATH[] = "assets/tilemap_packed.png";

constexpr char BGM_FILEPATH[] = "assets/audio/level_3.mp3";
constexpr char JUMP_SFX_FILEPATH[] = "assets/audio/jump.wav";
constexpr char AI_DEATH_SFX_FILEPATH[] = "assets/audio/ai_death.wav";
constexpr char DEATH_SFX_FILEPATH[] = "assets/audio/death.wav";

unsigned int LEVEL3_DATA[] = {
    21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    121, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    121, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    121, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 22, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    121, 0, 0, 0, 0, 0, 0, 0, 21, 22, 22, 122, 122, 122, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    121, 0, 0, 0, 0, 0, 0, 21, 122, 122, 122, 122, 122, 122, 122, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    121, 22, 22, 23, 0, 0, 21, 122, 122, 122, 122, 122, 122, 122, 122, 122, 22, 23, 0, 0, 21, 22, 22, 22, 23, 0, 0, 21, 22, 23,
    121, 122, 122, 122, 22, 22, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 22, 22, 122, 122, 122, 122, 122, 22, 22, 122, 122, 123
};

extern int lives;

Level3Scene::~Level3Scene()
{
    delete[] m_game_state.enemies;
    delete m_game_state.player;
    delete m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.death_sfx);
    Mix_FreeChunk(m_game_state.ai_death_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void Level3Scene::initialise()
{
    m_game_state.next_scene_id = -1;

    // Background setup
    GLuint background_texture_id = Utility::load_texture(BACKGROUND_FILEPATH);
    m_game_state.bg_texture_id = background_texture_id;

    // Map setup
    GLuint map_texture_id = Utility::load_texture(TILEMAP_FILEPATH);
    m_game_state.map = new Map(
        LEVEL3_WIDTH, LEVEL3_HEIGHT,
        LEVEL3_DATA,
        map_texture_id,
        1.0f,
        20, 9
    );


    // Player setup
    GLuint player_texture_id = Utility::load_texture(PLAYER_SPRITE_FILEPATH);

    int m_player_animation_indices[2][11] = {
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },               // First row for idle animation
        { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }      // Second row for running animation
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -9.8f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,          // texture id
        2.5f,                       // speed
        acceleration,               // acceleration
        5.0f,                       // jumping power
        m_player_animation_indices,        // animation index sets
        5.0f,                       // animation time
        11,                         // animation frame amount
        0,                          // current animation index
        11,                         // animation column amount
        2,                          // animation row amount
        1.0f,                       // width
        1.0f,                       // height
        PLAYER					    // entity type
    );

    m_game_state.player->set_position(glm::vec3(2.0f, -2.0f, 0.0f));
    m_game_state.player->set_lives(lives);
    m_game_state.player->face_right();

    // AI setup
    GLuint guard_texture_id = Utility::load_texture(GUARD_SPRITE_FILEPATH);
    GLuint walker_texture_id = Utility::load_texture(WALKER_SPRITE_FILEPATH);
	GLuint flyer_texture_id = Utility::load_texture(FLYER_SPRITE_FILEPATH);

    int m_enemy_animation_indices[2][11] = {
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },               // First row for idle animation
        { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }      // Second row for running animation
    };

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    // Guard AI
    m_game_state.enemies[0] = Entity(guard_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    m_game_state.enemies[0].activate();
    m_game_state.enemies[0].set_walking(m_enemy_animation_indices); // animation frames
    m_game_state.enemies[0].set_acceleration(acceleration);
    m_game_state.enemies[0].set_animation_frames(11);
    m_game_state.enemies[0].set_animation_cols(11);
    m_game_state.enemies[0].set_animation_rows(2);
    m_game_state.enemies[0].set_animation_row(1);
    m_game_state.enemies[0].set_position(glm::vec3(3.0f, -2.0f, 0.0f));

    // Walker AI
    m_game_state.enemies[1] = Entity(walker_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, WALKER, IDLE);
    m_game_state.enemies[1].activate();
    m_game_state.enemies[1].set_walking(m_enemy_animation_indices); // animation frames
    m_game_state.enemies[1].set_acceleration(acceleration);
    m_game_state.enemies[1].set_animation_frames(11);
    m_game_state.enemies[1].set_animation_cols(11);
    m_game_state.enemies[1].set_animation_rows(2);
    m_game_state.enemies[1].set_animation_row(1);
    m_game_state.enemies[1].set_position(glm::vec3(16.5f, 5.0f, 0.0f));

    // ^^^ AI 0 and 1 keeps off falling from the map only for this level, no fix found yet ^^^

    // Flyer AI
    m_game_state.enemies[2] = Entity(flyer_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, FLYER, IDLE);
    m_game_state.enemies[2].activate();
    m_game_state.enemies[2].set_walking(m_enemy_animation_indices); // animation frames
    m_game_state.enemies[2].set_acceleration(acceleration);
    m_game_state.enemies[2].set_animation_frames(11);
    m_game_state.enemies[2].set_animation_cols(11);
    m_game_state.enemies[2].set_animation_rows(2);
    m_game_state.enemies[2].set_animation_row(1);
    m_game_state.enemies[2].set_position(glm::vec3(20.0f, -1.0f, 0.0f));

    // Audio setup
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(25);

    m_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);
    m_game_state.death_sfx = Mix_LoadWAV(DEATH_SFX_FILEPATH);
    m_game_state.ai_death_sfx = Mix_LoadWAV(AI_DEATH_SFX_FILEPATH);
}

void Level3Scene::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, nullptr, 0, m_game_state.map);
    }

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (!m_game_state.enemies[i].is_active()) continue;

        if (m_game_state.player->check_collision(&m_game_state.enemies[i]))
        {
            if (m_game_state.player->check_collision_above(&m_game_state.enemies[i]))
            {
                Mix_PlayChannel(-1, m_game_state.ai_death_sfx, 0);
                m_game_state.enemies[i].deactivate();
                m_game_state.player->set_velocity(glm::vec3(
                    m_game_state.player->get_velocity().x,
                    5.0f,
                    0.0f
                ));
            }
            else // Player dies when colliding with AI
            {
                Mix_PlayChannel(-1, m_game_state.death_sfx, 0);

                m_game_state.player->lose_life(); //Decreases player life
                lives = m_game_state.player->get_lives();

                if (lives <= 0)
                {
                    m_game_state.next_scene_id = 5; // Go to lose scene
                    return;
                }

                // Restarts level when player dies
                initialise();
                m_game_state.player->set_lives(lives);
            }
        }
    }

    if (m_game_state.player->get_position().y < -10.0f) { //Player wins when character falls off the map
        m_game_state.next_scene_id = 4; 
    }
}

void Level3Scene::render(ShaderProgram* program)
{
    Utility::draw_background(program, m_game_state.bg_texture_id, LEVEL3_WIDTH, LEVEL3_HEIGHT);

    m_game_state.map->render(program);
    m_game_state.player->render(program);
	for (int i = 0; i < ENEMY_COUNT; i++)
		m_game_state.enemies[i].render(program);
}