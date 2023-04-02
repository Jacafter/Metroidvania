#include "raylib.h"
#include "raymath.h"
#include <dos.h>

#define G 400
#define PLAYER_JUMP_SPD 350.0f
#define PLAYER_HOR_SPD 200.0f

// Global Variables
bool bGameStart = false;
bool bHitEnemy = false;

// Game Structures
typedef struct sPlayer
{
    Vector2 position;
    Rectangle playerRect;
    float speed;
    bool canJump;
} sPlayer;
typedef struct sPlatform
{
    Rectangle platformRect;
    int blocking;
    Color platformColor;
} sPlatform;
typedef struct sEnemy
{
    Rectangle enemyRect;
    int blocking;
    Color enemyColor;
} sEnemy;
typedef struct sGoal
{
    Vector2 position;
    Rectangle goalRect;
    Color color;
} sGoal;

// Game Updates Redirection
void UpdatePlayerMovement(sPlayer* player, sPlatform* platform, int iPlatformLength, float fDelta);
void UpdatePlayerHit(sPlayer* player, sEnemy* enemy, int iEnemyLength, float fDelta);
void UpdateCameraCenter(Camera2D* camera, sPlayer* player, sPlatform* platform, int iPlatformLength, float fDelta, int iWidth, int iHeight);

// Game Menu
void Instruction()
{
    BeginDrawing();

    ClearBackground(LIGHTGRAY);
    DrawText("Press SPACE to JUMP", 290, 210, 20, BLACK);
    DrawText("Press LEFT KEY to move LEFT", 240, 240, 20, BLACK);
    DrawText("Press RIGHT KEY to move RIGHT", 240, 270, 20, BLACK);

    EndDrawing();

    Sleep(3200);
}
void GameMenu()
{
    BeginDrawing();

    ClearBackground(LIGHTGRAY);
    DrawText("Metroidvania", 240, 190, 50, BLACK);
    DrawText("Press ENTER to Start", 290, 260, 20, BLACK);
    DrawText("Press SPACE to see instruction", 240, 290, 20, BLACK);
    DrawText("Press ESC to close", 300, 320, 20, BLACK);

    EndDrawing();

    if (IsKeyPressed(KEY_ENTER))
        bGameStart = true;

    if (IsKeyPressed(KEY_ESCAPE))
    {
        CloseWindow();
    }

    if (IsKeyPressed(KEY_SPACE))
        Instruction();

}

// Main Loop
int main(void)
{

    bool bGameExit = false;

    // Initialization
    const int iScreenWidth = 800;
    const int iScreenHeight = 450;

    InitWindow(iScreenWidth, iScreenHeight, "Metroidvania");

    SetTargetFPS(60);

    // Background Loading
    Texture2D Background = LoadTexture("Tiles/BackGround.png");
    
    // Initialize audio device
    InitAudioDevice();
    Music music = LoadMusicStream("Tiles/dungeon002.ogg");
    PlayMusicStream(music);

    // Elements Creation
    sPlayer player = { 0 };
    player.position = (Vector2){ 700, 150 };
    player.speed = 0;
    player.canJump = false;

    sGoal goal = { 0 };
    goal.position = (Vector2){ 900, 20 };

    sEnemy enemy[] = {
        {{ 500, -270, 20, 500 }, 1, RED }, // Start Wall
        {{ 500, 600, 2500, 20 }, 1, RED }, // Down Limit
        {{ 1020, 250, 150, 30 }, 1, RED }, // Down-Stairs Obstacles
        {{ 1200, 100, 50, 50 }, 1, RED },
        {{ 1400, 270, 25, 140 }, 1, RED },
        {{ 1400, 100, 25, 80 }, 1, RED },
        {{ 1850, 440, 50, 30 }, 1, RED }, 
        {{ 2000, 250, 100, 25 }, 1, RED }, 
        {{ 2150, 340, 30, 30 }, 1, RED },
        {{ 2300, 280, 30, 80 }, 1, RED }, 
        {{ 2500, 250, 60, 60 }, 1, RED },
        {{ 2600, 430, 300, 20 }, 1, RED }, 
        {{ 2980, -150, 20, 600 }, 1, RED },// Second Platform Wall
        {{ 2850, 150, 20, 60 }, 1, RED }, // Up-Stairs Obstacles
        {{ 2950, -10, 30, 30 }, 1, RED }, 
        {{ 2340, -120, 40, 100 }, 1, RED }, // Going-Left Obstacles
        {{ 1980, -190, 50, 100 }, 1, RED }, 
        {{ 1790, 50, 1000, 20 }, 1, RED }, // Going-Left Down Limit
        {{ 1730, -400, 40, 250 }, 1, RED }, // Goal-Platform Obstacles
        {{ 1730, -80, 40, 130 }, 1, RED }, //
    };

    int iEnemyLength = sizeof(enemy) / sizeof(enemy[0]);

    sPlatform platform[] = {
        {{ 500, 230, 500, 100 }, 1, DARKGRAY }, // Main Platform
        {{ 850, 50, 900, 50 }, 1, DARKGRAY }, // Goal Platform
        {{ 1250, 300, 100, 10 }, 1, DARKGRAY }, // Down Stairs
        {{ 1500, 400, 50, 10 }, 1, DARKGRAY },
        {{ 1900, 450, 100, 10 }, 1, DARKGRAY },
        {{ 2200, 350, 100, 10 }, 1, DARKGRAY },
        {{ 2500, 450, 500, 25 }, 1, DARKGRAY }, // Second Platform
        {{ 2925, 300, 75, 10 }, 1, DARKGRAY }, // Up Stairs
        {{ 2750, 200, 100, 10 }, 1, DARKGRAY },
        {{ 2925, 80, 25, 10 }, 1, DARKGRAY },
        {{ 2610, 0, 25, 10 }, 1, DARKGRAY }, // Going Left
        {{ 2390, -20, 25, 10 }, 1, DARKGRAY },
        {{ 2300, 50, 25, 10 }, 1, DARKGRAY },
        {{ 2100, -40, 25, 10 }, 1, DARKGRAY },
        {{ 1950, -180, 25, 10 }, 1, DARKGRAY },
        {{ 1800, 20, 25, 10 }, 1, DARKGRAY },
    };

    int iPlatformLength = sizeof(platform) / sizeof(platform[0]);

    // Camera Set
    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ iScreenWidth / 2.0f, iScreenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Camera Pointers
    void (*cameraUpdaters)(Camera2D*, sPlayer*, sPlatform*, int, float, int, int) = { UpdateCameraCenter };

    // Game Menu
    while (!bGameExit)
    {
        GameMenu();

        // Main game loop
        while (bGameStart == true)
        {

            // Game Updates
            float fDeltaTime = GetFrameTime();
            UpdatePlayerMovement(&player, platform, iPlatformLength, fDeltaTime);

            UpdatePlayerHit(&player, enemy, iEnemyLength, fDeltaTime);

            UpdateMusicStream(music);
            
            // Camera Updates
            cameraUpdaters(&camera, &player, platform, iPlatformLength, fDeltaTime, iScreenWidth, iScreenHeight);

            // Draw Elements
            BeginDrawing();

            ClearBackground(LIGHTGRAY);

            DrawTexture(Background, -20, 0, WHITE);

            // Spatial UI
            BeginMode2D(camera);

            for (int i = 0; i < iPlatformLength; i++) DrawRectangleRec(platform[i].platformRect, platform[i].platformColor); // Platforms

            for (int i = 0; i < iEnemyLength; i++) DrawRectangleRec(enemy[i].enemyRect, enemy[i].enemyColor); // Enemies

            Rectangle playerRect = { player.position.x - 20, player.position.y - 40, 40, 40 }; // Player
            DrawRectangleRec(playerRect, BLUE);

            Rectangle goalRect = { goal.position.x - 30, goal.position.y - 30, 40, 40 }; // Goal
            DrawRectangleRec(goalRect, GREEN);


            EndMode2D();

            // Non-Diegetic UI
            //bHitEnemy = false; ADD SOUND
            // ADD TIME COUNT? 

            EndDrawing();


            // Win Check
            if (CheckCollisionRecs(playerRect, goalRect))
            {
                BeginDrawing();

                ClearBackground(LIGHTGRAY);
                DrawText("YOU WON!!!!", 240, 190, 50, BLACK);

                EndDrawing();

                Sleep(2100);

                CloseWindow();

                return 0;
            }

        }

    }

}

// Game Updates
void UpdatePlayerMovement(sPlayer* player, sPlatform* platform, int iPlatformLength, float fDelta)
{
    if (IsKeyDown(KEY_LEFT)) player->position.x -= PLAYER_HOR_SPD * fDelta;
    if (IsKeyDown(KEY_RIGHT)) player->position.x += PLAYER_HOR_SPD * fDelta;
    if (IsKeyDown(KEY_SPACE) && player->canJump)
    {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    int iLanfPlatfor = 0;
    for (int i = 0; i < iPlatformLength; i++)
    {
        sPlatform* ei = platform + i;
        Vector2* p = &(player->position);
        if (ei->blocking &&
            ei->platformRect.x <= p->x &&
            ei->platformRect.x + ei->platformRect.width >= p->x &&
            ei->platformRect.y >= p->y &&
            ei->platformRect.y < p->y + player->speed * fDelta)
        {
            iLanfPlatfor = 1;
            player->speed = 0.0f;
            p->y = ei->platformRect.y;
        }
    }

    if (!iLanfPlatfor)
    {
        player->position.y += player->speed * fDelta;
        player->speed += G * fDelta;
        player->canJump = false;
    }
    else player->canJump = true;
}
void UpdatePlayerHit(sPlayer* player, sEnemy* enemy, int iEnemyLength, float fDelta)
{
    for (int i = 0; i <= iEnemyLength; i++)
    {
        sEnemy* ei = enemy + i;
        Vector2* p = &(player->position);
        if (
            ei->enemyRect.x <= p->x &&
            ei->enemyRect.x + ei->enemyRect.width >= p->x &&
            ei->enemyRect.y <= p->y &&
            ei->enemyRect.y + ei->enemyRect.height  >= p->y)
        {
            bHitEnemy = true;

            CloseAudioDevice();

            CloseWindow();

            Sleep(50);

            main();
        }
    }
}
void UpdateCameraCenter(Camera2D* camera, sPlayer* player, sPlatform* platform, int iPlatformLength, float fDelta, int iWidth, int iHeight)
{
    camera->target = player->position;
}