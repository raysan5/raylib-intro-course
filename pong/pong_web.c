/*******************************************************************************************
*
*   raylib pong web
*
*   COMPILATION (Web - emscripten):
*     emcc -o pong_web.html pong_web.c -s USE_GLFW=3 -s TOTAL_MEMORY=67108864 --preload-file resources 
*          -I../../raylib/src -L../../raylib/src -lraylib -DPLATFORM_WEB
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

typedef enum { SCREEN_LOGO = 0, SCREEN_TITLE, SCREEN_GAMEPLAY, SCREEN_ENDING } GameScreen;

static void UpdateDrawFrame(void);

// Global variables
static const int screenWidth = 800;
static const int screenHeight = 600;

static bool pause = false;
static bool finishGame = false;
static int framesCounter = 0;
static GameScreen currentScreen = SCREEN_LOGO; // 0-LOGO, 1-TITLE, 2-GAMEPLAY, 3-ENDING

// Ball
static Vector2 ballPosition = { screenWidth/2, screenHeight/2 };
static float ballRadius = 20.0f;
static int ballSpeedX = 6;
static int ballSpeedY = -4;

// Player
static Rectangle player = { 10, screenHeight/2 - 50, 25, 100 };
static float playerSpeed = 8.0f;
static int playerScore = 0;

// Enemy
static Rectangle enemy = { screenWidth - 10 - 25, screenHeight/2 - 50, 25, 100 };
static float enemySpeed = 3.0f;
static int enemyVisionRange = screenWidth/2;
static int enemyScore = 0;

static float alphaLogo = 0.0f;
static int logoState = 0;          // 0-FadeIn, 1-Wait, 2-FadeOut 

// Resources
static Texture2D texLogo = { 0 };
static Font fntTitle = { 0 };
static Sound fxStart = { 0 };
static Sound fxPong = { 0 };
static Music ambient = { 0 };
    
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    //SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_UNDECORATED);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    
    InitAudioDevice();
    
    // Resources loading
    texLogo = LoadTexture("resources/logo_raylib.png");

    //Image imLogo = LoadImage("resources/logo_raylib.png");
    //Texture2D texLogo = LoadTextureFromImage(imLogo);
    //UnloadImage(imLogo);
    
    fntTitle = LoadFont("resources/pixantiqua.ttf");
    SetTextureFilter(fntTitle.texture, TEXTURE_FILTER_POINT);
    
    fxStart = LoadSound("resources/start.wav");
    fxPong = LoadSound("resources/pong.wav");
    
    ambient = LoadMusicStream("resources/qt-plimp.xm");
    PlayMusicStream(ambient);
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texLogo);
    UnloadFont(fntTitle);
    
    UnloadSound(fxStart);
    UnloadSound(fxPong);
    UnloadMusicStream(ambient);
    
    CloseAudioDevice();

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateMusicStream(ambient);
    
    switch (currentScreen)
    {
        case SCREEN_LOGO:
        {
            if (logoState == 0)
            {
                alphaLogo +=  (1.0f/180);
                if (alphaLogo > 1.0f)
                {
                    alphaLogo = 1.0f;
                    logoState = 1;
                }
            }
            else if (logoState == 1)
            {
                framesCounter++;
                if (framesCounter >= 200)
                {
                    framesCounter = 0;
                    logoState = 2;
                }
            }
            else if (logoState == 2)
            {
                alphaLogo -=  (1.0f/180);
                if (alphaLogo < 0.0f)
                {
                    alphaLogo = 0.0f;
                    currentScreen = 1;
                }
            }

        } break;
        case SCREEN_TITLE:
        {
            framesCounter++;
            
            // Update TITLE screen
            if (IsKeyPressed(KEY_ENTER)) 
            {
                PlaySound(fxStart);
                currentScreen = 2;
            }
        } break;
        case SCREEN_GAMEPLAY:
        {
            // Update GAMEPLAY screen
            if (!pause)
            {
                // Ball movement logic
                ballPosition.x += ballSpeedX;
                ballPosition.y += ballSpeedY;
                
                if (((ballPosition.x + ballRadius) > screenWidth) || ((ballPosition.x - ballRadius) < 0)) 
                {
                    PlaySound(fxPong);
                    ballSpeedX *= -1;
                }
                
                if (((ballPosition.y + ballRadius) > screenHeight) || ((ballPosition.y - ballRadius) < 0)) 
                {
                    PlaySound(fxPong);
                    ballSpeedY *= -1;
                }
                
                if ((ballPosition.x - ballRadius) <= 0) enemyScore += 1000;
                else if ((ballPosition.x + ballRadius) > GetScreenWidth()) playerScore += 1000;
                
                // Player movement logic
                if (IsKeyDown(KEY_UP)) player.y -= 8;
                else if (IsKeyDown(KEY_DOWN)) player.y += 8;
                
                if (player.y <= 0) player.y = 0;
                else if ((player.y + player.height) >= screenHeight) player.y = screenHeight - player.height;
                
                if (CheckCollisionCircleRec(ballPosition, ballRadius, player)) 
                {
                    PlaySound(fxPong);
                    ballSpeedX *= -1;
                }
                
                // Enemy movement logic
                if (ballPosition.x > enemyVisionRange)
                {
                    if (ballPosition.y > (enemy.y + enemy.height/2)) enemy.y += enemySpeed;
                    else if (ballPosition.y < (enemy.y + enemy.height/2)) enemy.y -= enemySpeed;
                }
                
                if (CheckCollisionCircleRec(ballPosition, ballRadius, enemy)) 
                {
                    PlaySound(fxPong);
                    ballSpeedX *= -1;
                }
                
                if (IsKeyDown(KEY_RIGHT)) enemyVisionRange++;
                else if (IsKeyDown(KEY_LEFT)) enemyVisionRange--;
            }
            
            if (IsKeyPressed(KEY_P)) pause = !pause;
            
            if (IsKeyPressed(KEY_ENTER)) currentScreen = 3;
        } break;
        case SCREEN_ENDING:
        {
            // Update ENDING screen
            if (IsKeyPressed(KEY_ENTER)) 
            {
                //currentScreen = 1;
                finishGame = true;
            }
        } break;
        default: break;
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);
        
        switch (currentScreen)
        {
            case SCREEN_LOGO:
            {
                // Draw LOGO screen
                //DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
                //DrawText("SCREEN LOGO", 10, 10, 30, DARKBLUE);
                
                DrawTexture(texLogo, GetScreenWidth()/2 - texLogo.width/2, GetScreenHeight()/2 - texLogo.height/2 - 40, Fade(WHITE, alphaLogo));
            } break;
            case SCREEN_TITLE:
            {
                // Draw TITLE screen
                //DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
                //DrawText("SCREEN TITLE", 10, 10, 30, DARKGREEN);
                
                DrawTextEx(fntTitle, "SUPER PONG", (Vector2){ 200, 100 }, fntTitle.baseSize*2, 4, RED);
                
                if ((framesCounter/30)%2) DrawText("PRESS ENTER to START", 200, 300, 30, BLACK);
                
            } break;
            case SCREEN_GAMEPLAY:
            {
                DrawCircleV(ballPosition, ballRadius, RED);

                DrawRectangleRec(player, BLUE);
                
                DrawRectangleRec(enemy, DARKGREEN);
                
                DrawLine(enemyVisionRange, 0, enemyVisionRange, screenHeight, GRAY);
                
                // Draw hud
                DrawText(TextFormat("%04i", playerScore), 100, 10, 30, BLUE);
                DrawText(TextFormat("%04i", enemyScore), screenWidth - 200, 10, 30, DARKGREEN);
                
                if (pause)
                {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.8f));
                    DrawText("GAME PAUSED", 320, 200, 30, RED);
                }
            } break;
            case SCREEN_ENDING:
            {
                // Draw ENDING screen
                DrawRectangle(0, 0, screenWidth, screenHeight, RED);
                DrawText("SCREEN ENDING", 10, 10, 30, MAROON);
            } break;
            default: break;
        }

    EndDrawing();
    //----------------------------------------------------------------------------------
}