#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include <math.h>


typedef struct GameData {
    //Player data
    Vector2 player_position;
    Vector2 player_velocity;
    float player_acceleration;
    float player_rotation;
    float player_rotational_velocity;
    //Asteroid data 
    Vector2 *asteroid_positions;
    Vector2 *asteroid_velocities;
    float *asteroid_rotation;
    float *asteroid_rotational_velocity;
    int *asteroid_sizes;
    int asteroid_count;
    int max_asteroids;
    //Particle data 
    Vector2 *particle_positions;
    Vector2 *particle_velocities;
    float *particle_time;
    int max_particles;
    int particle_count;
    //Missile data 
    Vector2 *missile_positions;
    Vector2 *missile_velocities;
    int missile_count;
    int max_missiles;
    //GameData 
    int screen_height;
    int screen_width;
    int lives;
    float invicibility_time;
    float player_cooldown;
}GameData;

/*
    Physics Functions
*/

Vector2 UpdatePosition(Vector2 pos, Vector2 velocity, int screen_width, int screen_height, bool wrap) {
    Vector2 v = Vector2Add(pos, velocity);
    
    if (wrap) {
        //Wrap
        if (v.x < -15) {
            v.x += screen_width+15;
        }
        
        if (v.y < -15) {
            v.y = screen_height+15;
        }
        
        if (v.x > screen_width+15) {
            v.x =-15;
        }
        
        if (v.y > screen_height+15) {
            v.y = -15;
        }
    }
    
    return v;
}


Vector2 UpdateVelocity(Vector2 velocity, float acceleration, float rotation, float delta_time) {
    float to_radians = 0.017453;
    float r = rotation+90;
    Vector2 v = velocity;
    v.x += (acceleration*(-1.0f*cos(r*to_radians)))*delta_time;
    v.y += (acceleration*(-1.0f*sin(r*to_radians)))*delta_time;
    return v;    
}

//change_in_degrees indicates how much the object will rotate in 1 second
float UpdateRotation(float current_rotation, float change_in_degrees, float delta_time) {
    float rotation = current_rotation+(change_in_degrees*delta_time);
    //switch to modulo
    
    if (rotation > 360) {
        rotation -= 360;
    }
    
    if (rotation < 0) {
        rotation += 360;
    }
    
    return rotation;
}

/*
    Game Functions
*/

GameData* InitNewGame(const int screen_height, const int screen_width, int max_asteroids, int max_particles, int max_missiles) {
    GameData *new_game = malloc(sizeof(GameData));
    Vector2 player_pos;
    player_pos.x = screen_width/2;
    player_pos.y = screen_height/2;
    
    new_game->player_position              = player_pos;
    new_game->player_velocity              = (Vector2){0.0f, 0.0f};
    new_game->player_acceleration          = 0.0f;
    new_game->player_rotation              = 0.0f;
    new_game->player_rotational_velocity   = 0.0f;
    new_game->asteroid_positions           = malloc(sizeof(Vector2)*max_asteroids);
    new_game->asteroid_velocities          = malloc(sizeof(Vector2)*max_asteroids);
    new_game->asteroid_rotation            = malloc(sizeof(float)*max_asteroids);
    new_game->asteroid_rotational_velocity = malloc(sizeof(float)*max_asteroids);
    new_game->asteroid_sizes               = malloc(sizeof(int)*max_asteroids);
    new_game->asteroid_count               = 0;
    new_game->max_asteroids                = max_asteroids;
    new_game->particle_positions           = malloc(sizeof(Vector2)*max_particles);
    new_game->max_particles                = max_particles;
    new_game->particle_count               = 0;
    new_game->particle_velocities          = malloc(sizeof(Vector2)*max_particles);
    new_game->particle_time                = malloc(sizeof(float)*max_particles);
    new_game->missile_positions            = malloc(sizeof(Vector2)*max_missiles);
    new_game->missile_velocities           = malloc(sizeof(Vector2)*max_missiles);
    new_game->missile_count                = 0;
    new_game->max_missiles                 = max_missiles;
    new_game->screen_height                = screen_height;
    new_game->screen_width                 = screen_width;
    new_game->lives                        = 3;
    new_game->player_cooldown              = 0.0f;
    new_game->invicibility_time            = 0.0f;
    
    return new_game;
}

void DeInitGame(GameData *game) {
    free(game->asteroid_positions);
    free(game->asteroid_velocities);
    free(game->asteroid_rotation);
    free(game->asteroid_rotational_velocity);
    free(game->asteroid_sizes);
    free(game->particle_positions);
    free(game->particle_velocities);
    free(game->particle_time);
    free(game->missile_positions);
    free(game->missile_velocities);
    free(game);
}

bool OffScreen(Vector2 v, int screen_width, int screen_height) {
    
    if (v.x < 0) {
        return true;
    }
    
    if (v.y < 0) {
        return true;
    }
    
    if (v.x > screen_width) {
        return true; 
    }
    
    if (v.y > screen_height) {
        return true;
    }
    
    return false;
}

void SpawnMissile(GameData *game, Vector2 position, float rotation, float acceleration, float delta_time) {
    int index = ++game->missile_count-1;
    
    if (index >= game->max_missiles) {
        //shift all missiles down by one
        for (int i = 1; i < game->max_missiles; i++) {
            game->missile_positions[i-1] = game->particle_positions[i];
            game->missile_velocities[i-1] = game->missile_velocities[i];
        }
        
        index = game->max_missiles-1;
        game->missile_count = game->max_missiles;
    }
    
    game->missile_positions[index] = position;
    
    game->missile_velocities[index] = UpdateVelocity(
        (Vector2){0, 0},
        acceleration,
        rotation,
        delta_time
    );
}

void SpawnParticle(GameData *game, Vector2 position, Vector2 initial_velocity, float rotation, float acceleration, float delta_time) {
    int index = ++game->particle_count-1;
    
    if (index >= game->max_particles) {
        //shift all particles down an index
        for (int i = 1; i < game->max_particles; i++) {
            game->particle_positions[i-1] = game->particle_positions[i];
            game->particle_velocities[i-1] = game->particle_velocities[i];
        }
        
        index = game->max_particles-1;
        game->particle_count = game->max_particles;
    }
    
    game->particle_positions[index] = position;
    
    game->particle_velocities[index] = UpdateVelocity(
        initial_velocity,
        acceleration,
        rotation,
        delta_time
    );
}

void SpawnAsteroid(GameData *game, int size, Vector2 position, float rotation, float acceleration, float delta_time) {
    if (game->asteroid_count <= game->max_asteroids) {
        int index = ++game->asteroid_count-1;
        game->asteroid_positions[index] = position;
        game->asteroid_rotation[index] = rotation;
        game->asteroid_sizes[index] = size;
        game->asteroid_velocities[index] = UpdateVelocity(
            (Vector2){0, 0},
            acceleration,
            rotation,
            delta_time
        );
        game->asteroid_rotational_velocity[index] = GetRandomValue(-90*size, 90*size);
    }
}

void DestroyAsteroid(GameData *game, int index, float delta_time) {
    int asteroid_size = game->asteroid_sizes[index];
    Vector2 pos = game->asteroid_positions[index];
    Vector2 velocity = Vector2Scale(game->asteroid_velocities[index], 0.5);
    
    //shift asteroid array down which removes asteroid
    for (int i = index; i < game->asteroid_count; i++) {
        game->asteroid_positions[i] = game->asteroid_positions[i+1];
        game->asteroid_velocities[i] = game->asteroid_velocities[i+1];
        game->asteroid_rotation[i] = game->asteroid_rotation[i+1];
        game->asteroid_rotational_velocity[i] = game->asteroid_rotational_velocity[i+1];
        game->asteroid_sizes[i] = game->asteroid_sizes[i+1];
    }
    
    game->asteroid_count--;
    
    int angle = GetRandomValue(0, 360);
    
    //Spawn particles 
    int p = 100/asteroid_size;
    for (int i = 0; i < p; i++) {
        SpawnParticle(
            game,
            pos,
            velocity,
            GetRandomValue(-22*asteroid_size, 22*asteroid_size)+angle,
            GetRandomValue(10, 500/asteroid_size),
            delta_time
        );
        
       
    }
    
    //Spawn new asteroids if not the smallest asteroid size
    if (asteroid_size < 4) {
        for (int i = 0; i < 2; i++) {
            SpawnAsteroid(
                game,
                asteroid_size+2,
                pos,
                GetRandomValue(0, 360),
                GetRandomValue(30, 100)*asteroid_size,
                delta_time
                
            );
        }
    }
}

void KillOffscreenParticles(GameData *game) {
    int index = 0;
    
    while (index < game->particle_count) {
        //check if on screen
        Vector2 p = game->particle_positions[index];
        bool offscreen = OffScreen(p, game->screen_width, game->screen_height);
        
        //Also kill if too old
        if (game->particle_time[index] > 3.0f) {
            
            if (GetRandomValue(0, 1000) < 10) {
                offscreen = true;
            }
        }
        
        if (offscreen) {
            //shift particles over and shorten particle list
            for (int i = index; i < game->particle_count; i++) {
                game->particle_positions[i] = game->particle_positions[i+1];
                game->particle_velocities[i] = game->particle_velocities[i+1];
                game->particle_time[i] = game->particle_time[i+1];
            }
            
            game->particle_count--;
        }
        
        else {
            index++;
        }
    }
}

void KillOffscreenMissiles(GameData *game) {
    int index = 0;
    
    while (index < game->missile_count) {
        Vector2 m = game->missile_positions[index];
        bool offscreen = OffScreen(m, game->screen_width, game->screen_height);
        
        if (offscreen) {
            //shift missiles over and shorten missile list 
            for (int i = index; i < game->missile_count; i ++) {
                game->missile_positions[i] = game->missile_positions[i+1];
                game->missile_velocities[i] = game->missile_velocities[i+1];
            }
            
            game->missile_count--;
        }
        
        else {
            index++;
        }
    }
}

bool CheckMissileCollisions(GameData *game, float delta_time) {
    int asteroid_radius = 50;
    int missile_radius = 8;
    bool missile_collision = false;
    
    for (int i = 0; i < game->missile_count; i++) {
        for (int j = 0; j < game->asteroid_count; j++) {
            
            Vector2 missile_pos = game->missile_positions[i];
            Vector2 aster_pos = game->asteroid_positions[j];
            int asteroid_size = game->asteroid_sizes[j];
            
            if (CheckCollisionCircles(missile_pos, missile_radius, aster_pos, asteroid_radius/asteroid_size)) {
                game->missile_positions[i] = (Vector2){-1000, -1000};
                DestroyAsteroid(game, j, delta_time);
                missile_collision = true;
            }
        }
    }
    
    return missile_collision;
}

bool CheckPlayerCollision(GameData *game, float delta_time, Vector2 *player_graphic, int ship_l, Vector2 *asteroid_graphic, int asteroid_l) {
    Vector2 player_pos = game->player_position;
    
    Vector2 translated_ship[ship_l];
    float to_radians = 0.01745f;
    
    //translate ship point coordinates
    for (int i = 0; i < ship_l; i++) {
        translated_ship[i] = player_graphic[i];
        translated_ship[i] = Vector2Rotate(translated_ship[i], game->player_rotation*to_radians);
        translated_ship[i] = Vector2Add(translated_ship[i], game->player_position);
    }
    
    for (int i = 0; i < game->asteroid_count; i++) {
        
        Vector2 asteroid_pos = game->asteroid_positions[i];
        float asteroid_rotation = game->asteroid_rotation[i];
        int asteroid_size = game->asteroid_sizes[i];
        
        Vector2 translated_asteroid_graphic[asteroid_l];
        
        //translate asteroid point coordinates
        for(int j = 0; j < asteroid_l; j++) {
            translated_asteroid_graphic[j] = asteroid_graphic[j];
            translated_asteroid_graphic[j] = Vector2Scale(translated_asteroid_graphic[j], 1/(float)asteroid_size);
            translated_asteroid_graphic[j] = Vector2Rotate(translated_asteroid_graphic[j], asteroid_rotation);
            translated_asteroid_graphic[j] = Vector2Add(translated_asteroid_graphic[j], asteroid_pos);
        }
        
        
        //Check collision of points in players ship against asteroid polygon
        for (int j = 0; j < ship_l; j++) {
            bool collided = CheckCollisionPointPoly(translated_ship[j], translated_asteroid_graphic, asteroid_l);
            
            if (collided) {
                printf("Destroyed asteroid index: %d, number of asteroids: %d\n", i, game->asteroid_count);
                DestroyAsteroid(game, i, delta_time);
                return true;
            }
        }
        
    }
    
    return false;
}

//Prepares a list for drawing vector graphic
Vector2* RenderTranslation(Vector2 *array, int array_length, float rotation, Vector2 position, float scale) {
    Vector2 *translated = malloc(sizeof(Vector2)*array_length);
    /*Multiply the rotation angle by to_radians to convert angle in to radians to
      be used by Vector2 manipulation functions
    */
    float to_radians = 0.01745f;
    
    for (int i = 0; i < array_length; i++) {
        //Load coordinates in to translation array 
        translated[i] = array[i];
        //fix coordinates from bottem left origin to top left origin 
        translated[i].y *= -1.0f;
        //Rotate
        translated[i] = Vector2Rotate(translated[i], rotation*to_radians);
        //Scale
        translated[i] = Vector2Scale(translated[i], scale);
        //Translate
        translated[i] = Vector2Add(position, translated[i]);
    }
    
    return translated;
}


int main(void) {
    const int screen_height = 600;
    const int screen_width = 800;
    int max_asteroids = 20;
    int max_particles = 1000000;
    int max_missiles = 10;
    Vector2 background_speed = {10, 30};
    float background_rotation = 0.0f;
    
    GameData *game = InitNewGame(screen_height, screen_width, max_asteroids, max_particles, max_missiles);
    
    Vector2 ship_graphic[] = {
        {0.0f, 12.0f},
        {10.0f, -10.0f},
        {0.0f, -5.0f},
        {-10.0f, -10.0f},
        {0.0f, 12.0f}
    };
    
    Vector2 asteroid_graphic[9] = {
        {0, 50},
        {0, 55},
        {0, 40},
        {0, 55},
        {0, 30},
        {0, 45},
        {0, 55},
        {0, 45},
        {0, 50}
    };
    
    int asteroid_graphic_length = 9;
    
    for (int i = 0; i < asteroid_graphic_length; i++) {
        asteroid_graphic[i] = Vector2Rotate(asteroid_graphic[i], (45*i)*0.017453);
    }
    

    
    Vector2 flame_graphic[] = {{4, -7}, {0, -20}, {-4, -7}};
    
    int ship_graphic_length = 5;
    int flame_graphic_length = 3;
    
    unsigned char flame_toggle = 0;
    bool render_flame = false;
    int thrust_time = 0;
    bool nuke = false;
    float nuke_radius = 0.0;
    
    InitWindow(screen_width, screen_height, "Asteroids");
    int target_fps = 144;
    SetTargetFPS(target_fps);
    
    //Load sounds
    InitAudioDevice();
    
    Sound gun = LoadSoundFromWave(LoadWave("8-bit-kit-lazer-1_B_major.wav"));
    Sound explosion = LoadSoundFromWave(LoadWave("8-bit-kit-hi-hat-open-3.wav"));
    Sound player_explosion = LoadSoundFromWave(LoadWave("8-bit-kit-explosion-2.wav"));
    Sound thrust = LoadSoundFromWave(LoadWave("white-noise.wav"));
    
    RenderTexture2D target = LoadRenderTexture(screen_width, screen_height);
    
    BeginTextureMode(target);
        ClearBackground(BLACK);
    EndTextureMode();
    
    Shader fade = LoadShader(0, "fade.fs");
    
    
    //Main Loop
    while (!WindowShouldClose()) {
        /*
            Update
        */
        float delta_time = GetFrameTime();
        flame_toggle++;
        
        //Spawn asteroids if none
        if (game->asteroid_count == 0) {
            
            int asteroids_to_spawn = game->max_asteroids/3;
            
            while (asteroids_to_spawn > 0) {
                //find a location greater than 70 pixels from player 
                Vector2 apos = {
                    GetRandomValue(0, game->screen_width), 
                    GetRandomValue(0, game->screen_height)
                };
                
                float distance = Vector2Distance(game->player_position, apos);
                
                if (distance < 70) {
                    continue;
                }
                
                SpawnAsteroid(
                    game,
                    1,
                    apos,
                    GetRandomValue(0, 360),
                    1,
                    0.16
                );
                
                asteroids_to_spawn--;
            }
            
        }
        
        //Get player input
        if (game->player_cooldown == 0) {
        
            if (IsKeyDown(KEY_RIGHT)) {
                game->player_rotation = UpdateRotation(game->player_rotation, 360, delta_time);
            }
            
            if (IsKeyDown(KEY_LEFT)) {
                game->player_rotation = UpdateRotation(game->player_rotation, -360, delta_time);
            }
            
            if (IsKeyPressed(KEY_SPACE)) {
                
                SpawnMissile(
                    game,
                    game->player_position,
                    game->player_rotation,
                    game->player_acceleration+500,
                    delta_time
                );
                
                PlaySound(gun);
            }
            
            if (IsKeyDown(KEY_UP)) {
                
                game->player_acceleration = 1.0f;
                render_flame = true;
                thrust_time += 4;
                
                if (!IsSoundPlaying(thrust)) {
                    PlaySound(thrust);
                }
            }
            
        }
        
        if (IsKeyUp(KEY_UP)||game->player_cooldown > 0) {
            game->player_acceleration = 0.0f;
            render_flame = false;
            thrust_time = 0;
            
            if (IsSoundPlaying(thrust)) {
                StopSound(thrust);
            }
        }
        
        //Update Player
        
        game->invicibility_time -= delta_time;
        game->player_cooldown -= delta_time;
        
        if (game->invicibility_time < 0.0f) {
            game->invicibility_time = 0.0f;
        }
        
        if (game->player_cooldown < 0.0f) {
            game->player_cooldown = 0.0f;
        }
        
        bool check_player_collision = true;
        //Check player collision and kill player if hit asteroid
        if (game->invicibility_time > 0.0f||game->player_cooldown > 0.0f) {
            check_player_collision = false;
        }
        
        
        if (check_player_collision) {
            bool player_collided = CheckPlayerCollision(
                game, 
                delta_time,     
                ship_graphic,
                sizeof(ship_graphic)/sizeof(ship_graphic[0]),
                asteroid_graphic,
                sizeof(asteroid_graphic)/sizeof(asteroid_graphic[0])
                );
            
            if (player_collided) {
                Vector2 player_v = game->player_velocity;
                game->player_cooldown = 3.0f;
                game->invicibility_time = 6.0f;
                game->lives--;
                game->player_velocity = (Vector2){0, 0};
                
                
                //spawn particles 
                int player_explosion_particles = 900;
                nuke = true;
                
                for (int i =0; i < player_explosion_particles; i++) {
                    //set particle speed; super particle speed for looks
                    int super_particle = GetRandomValue(0, 1000);
                    int particle_speed;
                    if (super_particle < 500) {
                        particle_speed = 300;
                    }
                    else {
                        particle_speed = GetRandomValue(1, 100);
                    }
                    SpawnParticle(
                        game,
                        game->player_position,
                        (Vector2){0, 0},
                        GetRandomValue(0, 360),
                        particle_speed,
                        delta_time
                    );
                }
                
                PlaySound(player_explosion);
            }
        }
        
        game->player_velocity = UpdateVelocity(
            game->player_velocity,
            game->player_acceleration,
            game->player_rotation,
            delta_time
        );
        
        game->player_position = UpdatePosition(
            game->player_position,
            game->player_velocity,
            game->screen_width,
            game->screen_height,
            true
        );
        
        //Update Asteroids 
        for (int i = 0; i < game->asteroid_count; i++) {
            game->asteroid_rotation[i] = UpdateRotation(
                game->asteroid_rotation[i],
                game->asteroid_rotational_velocity[i],
                delta_time
            );
            
            game->asteroid_positions[i] = UpdatePosition(
                game->asteroid_positions[i],
                game->asteroid_velocities[i],
                game->screen_width,
                game->screen_height,
                true
            );
        }
        
        //Update Particles 
        for (int i = 0; i < game->particle_count; i++) {
            
            game->particle_positions[i] = UpdatePosition(
                game->particle_positions[i],
                game->particle_velocities[i],
                game->screen_width,
                game->screen_height,
                false
            );
            game->particle_time[i] += delta_time;
        }
        
        //Update Missiles
        for (int i =0; i < game->missile_count; i++) {
            game->missile_positions[i] = UpdatePosition(
                game->missile_positions[i],
                game->missile_velocities[i],
                game->screen_width,
                game->screen_height,
                false
            );
        }
        
        
        KillOffscreenParticles(game);
        KillOffscreenMissiles(game);
        bool asteroid_explosion = CheckMissileCollisions(game, delta_time);
        
        if (asteroid_explosion) {
            PlaySound(explosion);
        }
        
        //Background
        background_rotation = UpdateRotation(background_rotation, 5.0f, delta_time);
        if (flame_toggle%1 == 0) {
            
            Vector2 particle_pos = {
                GetRandomValue(0, screen_width),
                GetRandomValue(0, screen_height)
            };
            
            SpawnParticle(
                game,
                particle_pos,
                (Vector2){0, 0},
                background_rotation+180,
                GetRandomValue(background_speed.x, background_speed.y),
                delta_time
            );

        }
        
        
        
        //Draw 
        BeginTextureMode(target);
        //BeginShaderMode(fade);
            
            ClearBackground(BLACK);
            
            //Translate coordinates for screen drawing 
            Vector2 *player_graphic_translation = RenderTranslation(
                ship_graphic,
                ship_graphic_length,
                game->player_rotation,
                game->player_position,
                1
            );
     
            
            //Draw player ship (flashing if invincible)
            
            bool draw_ship = false;
            
            
            if (game->invicibility_time == 0.0f) {
                draw_ship = true;
            }
            
            else {
                if (flame_toggle%10 == 0) {
                    
                    if (draw_ship == true) {
                        draw_ship = false;
                    }
                    else {
                        draw_ship = true;
                    }
                }
            }
            
            if (game->player_cooldown > 0.0f) {
                draw_ship = false;
                
            }
            
            if (draw_ship) {
                DrawLineStrip(player_graphic_translation, ship_graphic_length, WHITE);
            }
            
            //Draw player thrust flame and spawn particles
            if (render_flame) {
                if (flame_toggle%(target_fps/6) == 0) {
                    
                    //fix position and rotation
                    Vector2 *fg = RenderTranslation(
                        flame_graphic,
                        flame_graphic_length,
                        game->player_rotation,
                        game->player_position,
                        1
                    );
                    
                    DrawLineStrip(fg, flame_graphic_length, WHITE);
                    
                    
                    free(fg);
                }
                //spawn particle
                
                Vector2 flame_point = player_graphic_translation[2];
                
                
                SpawnParticle(
                    game,
                    flame_point,
                    (Vector2){0, 0},
                    game->player_rotation+180+GetRandomValue(-10, 10),
                    game->player_acceleration+100.0f+GetRandomValue(0, thrust_time),
                    delta_time
                );
            }
            
            //Draw Asteroids 
            for (int i = 0; i < game->asteroid_count; i++) {
                Vector2 *translated_asteroid_graphic = RenderTranslation(
                    asteroid_graphic,
                    asteroid_graphic_length,
                    game->asteroid_rotation[i],
                    game->asteroid_positions[i],
                    1/(float)game->asteroid_sizes[i]
                );

                
                DrawLineStrip(translated_asteroid_graphic, asteroid_graphic_length, WHITE);
                free(translated_asteroid_graphic);
                
            }
            
            //Draw Particles
            for (int i = 0; i < game->particle_count; i++) {
                Vector2 particle_start = game->particle_positions[i];
                float time = game->particle_time[i];
                
                if (time > 0.9f) {
                    time = 0.9f;
                }
                
                Vector2 particle_end = UpdatePosition(
                    particle_start,
                    Vector2Scale(game->particle_velocities[i], 8*(1-time)),
                    game->screen_width,
                    game->screen_height,
                    false
                );
                
                
                DrawLineV(particle_start, particle_end, WHITE);
            }
            
            //Draw Missiles 
            for (int i = 0; i < game->missile_count; i++) {
                DrawCircleV(game->missile_positions[i], 1.0f, WHITE);
            }
            //Release memory of translated coordinates 
            free(player_graphic_translation);
         
         //EndShaderMode();
         EndTextureMode();
         
         BeginDrawing();
            
            ClearBackground(BLACK);

            DrawTextureRec(
                target.texture,
                (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height}, 
                (Vector2){0, 0}, 
                WHITE
                );
            
            DrawFPS(10, 10);
            
            
            
        
        EndDrawing();
    }
    
    DeInitGame(game);
    
    return 0;
}