/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 1.0, last time updated with raylib 1.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

typedef enum { SCREEN_LOGO = 0, SCREEN_TITLE, SCREEN_GAMEPLAY, SCREEN_ENDING } GameScreen;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;

    //SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_UNDECORATED);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    
    InitAudioDevice();
    
    // Ball
    Vector2 ballPosition = { screenWidth/2, screenHeight/2 };
    float ballRadius = 20.0f;
    int ballSpeedX = 6;
    int ballSpeedY = -4;
    
    // Player
    Rectangle player = { 10, screenHeight/2 - 50, 25, 100 };
    float playerSpeed = 8.0f;
    int playerScore = 0;

    // Enemy
    Rectangle enemy = { screenWidth - 10 - 25, screenHeight/2 - 50, 25, 100 };
    float enemySpeed = 3.0f;
    int enemyVisionRange = screenWidth/2;
    int enemyScore = 0;
    
    // Resources loading
    Texture2D texLogo = LoadTexture("resources/logo_raylib.png");
    float alphaLogo = 0.0f;
    int logoState = 0;          // 0-FadeIn, 1-Wait, 2-FadeOut 
    
    //Image imLogo = LoadImage("resources/logo_raylib.png");
    //Texture2D texLogo = LoadTextureFromImage(imLogo);
    //UnloadImage(imLogo);
    
    Font fntTitle = LoadFont("resources/pixantiqua.ttf");
    
    Sound fxStart = LoadSound("resources/start.wav");
    Sound fxPong = LoadSound("resources/pong.wav");
    
    Music ambient = LoadMusicStream("resources/qt-plimp.xm");
    PlayMusicStream(ambient);
    
    // General variables
    bool pause = false;
    bool finishGame = false;
    int framesCounter = 0;
    GameScreen currentScreen = SCREEN_LOGO; // 0-LOGO, 1-TITLE, 2-GAMEPLAY, 3-ENDING

    SetTargetFPS(60);           // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose() && !finishGame)    // Detect window close button or ESC key
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
                    
                    DrawTextEx(fntTitle, "SUPER PONG", (Vector2){ 200, 100 }, 64, 4, LIME);
                    
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