/**
 *   IncludeBuild - Raylib Pong Example
 *
 *   A simple Pong game using Raylib to demonstrate IncludeBuild with external libraries
 *
 *   This example will work on Windows, Linux and macOS with appropriate compiler setup
 *
********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#include "paddle_hit.h"
#include "score.h"
#include "wall_bounce.h"
#include "background_music.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define MAX_LIVES 5
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 100
#define BALL_RADIUS 10
#define PADDLE_SPEED 300
#define BALL_SPEED 300

void WriteEmbeddedFile(const char *filename, const unsigned char *data, unsigned int length) {
    FILE *file = fopen(filename, "wb");
    if (file) {
        fwrite(data, 1, length, file);
        fclose(file);
    }
}

typedef struct {
    Rectangle rect;
    int speed;
    int lives;
} Paddle;

typedef struct {
    Vector2 position;
    Vector2 speed;
    float radius;
} Ball;

void DrawRoundedPaddle(Rectangle rect, float roundness) {
    DrawRectangleRounded(rect, roundness, 6, WHITE);
}

void DrawLives(int lives, Vector2 position, bool alignRight) {
    for (int i = 0; i < MAX_LIVES; i++) {
        float xPos = position.x + i * 25;
        if (alignRight) xPos = position.x - i * 25;

        if (i < lives) {
            DrawCircle(xPos, position.y, 8, WHITE);
        } else {
            DrawCircleLines(xPos, position.y, 8, WHITE);
        }
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "IncludeBuild - Raylib Pong");
    InitAudioDevice();

    WriteEmbeddedFile("paddle_hit.wav", paddle_hit_wav, paddle_hit_wav_len);
    WriteEmbeddedFile("score.wav", score_wav, score_wav_len);
    WriteEmbeddedFile("wall_bounce.wav", wall_bounce_wav, wall_bounce_wav_len);
    WriteEmbeddedFile("background_music.ogg", background_music_ogg, background_music_ogg_len);

    Music bgMusic = LoadMusicStream("background_music.ogg");

    if (bgMusic.stream.buffer == NULL) {
        printf("Failed to load background music!\n");
    } else {
        PlayMusicStream(bgMusic);
        SetMusicVolume(bgMusic, 0.2f);
    }

    Sound paddleHit = LoadSound("paddle_hit.wav");
    Sound scoreSound = LoadSound("score.wav");
    Sound wallBounce = LoadSound("wall_bounce.wav");

    SetTargetFPS(60);

    Paddle player = {{50, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT}, PADDLE_SPEED, MAX_LIVES};
    Paddle enemy = {{SCREEN_WIDTH - 50 - PADDLE_WIDTH, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT}, PADDLE_SPEED, MAX_LIVES};
    Ball ball = {{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, {BALL_SPEED * (GetRandomValue(0, 1) ? 1 : -1), BALL_SPEED * (GetRandomValue(0, 1) ? 1 : -1)}, BALL_RADIUS};

    int screenShakeTimer = 0;
    float lastPlayerY = player.rect.y;
    bool gameOver = false;
    char winnerText[20];
    int scoreLeft = 0, scoreRight = 0;

    while (!WindowShouldClose()) {
        UpdateMusicStream(bgMusic);

        if (!gameOver) {
            float playerDeltaY = player.rect.y - lastPlayerY;
            lastPlayerY = player.rect.y;

            if (IsKeyDown(KEY_W) && player.rect.y > 0) player.rect.y -= player.speed;
            if (IsKeyDown(KEY_S) && player.rect.y < SCREEN_HEIGHT - player.rect.height) player.rect.y += player.speed;

            if (ball.speed.x > 0) { 
                if (ball.position.y > enemy.rect.y + enemy.rect.height / 2) enemy.rect.y += playerDeltaY;
                if (ball.position.y < enemy.rect.y + enemy.rect.height / 2) enemy.rect.y -= playerDeltaY;
            }

            ball.position.x += ball.speed.x;
            ball.position.y += ball.speed.y;

            if (ball.position.y <= 0 || ball.position.y >= SCREEN_HEIGHT) {
                ball.speed.y *= -1;
                PlaySound(wallBounce);
            }

            if (CheckCollisionCircleRec(ball.position, ball.radius, player.rect)) {
                PlaySound(paddleHit);
                screenShakeTimer = 8;

                ball.speed.x *= -1;
                ball.speed.y += playerDeltaY * 0.5f;
                ball.position.x = player.rect.x + player.rect.width + ball.radius;

                ball.speed.x *= 1.05f;
                ball.speed.y *= 1.05f;
            }
            if (CheckCollisionCircleRec(ball.position, ball.radius, enemy.rect)) {
                PlaySound(paddleHit);
                screenShakeTimer = 8;

                ball.speed.x *= -1;
                ball.speed.y += (rand() % 3) - 1;
                ball.position.x = enemy.rect.x - ball.radius;

                ball.speed.x *= 1.05f;
                ball.speed.y *= 1.05f;
            }

            if (ball.position.x < 0) { 
                player.lives--;
                PlaySound(scoreSound);
                ball.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ball.speed = (Vector2){BALL_SPEED * (GetRandomValue(0, 1) ? 1 : -1), BALL_SPEED * (GetRandomValue(0, 1) ? 1 : -1)};
                scoreRight++;
            }
            if (ball.position.x > SCREEN_WIDTH) { 
                player.lives--;
                PlaySound(scoreSound);
                ball.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ball.speed = (Vector2){-BALL_SPEED * (GetRandomValue(0, 1) ? 1 : -1), BALL_SPEED * (GetRandomValue(0, 1) ? 1 : -1)};
                scoreLeft++;
            }

            if (player.lives <= 0) {
                gameOver = true;
                sprintf(winnerText, "ENEMY WINS!");
            }
        } else {
            DrawText(winnerText, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2, 30, WHITE);
            DrawText("Press [R] to Restart", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 40, 20, WHITE);

            if (IsKeyPressed(KEY_R)) {
                player.lives = MAX_LIVES;
                scoreLeft = 0;
                scoreRight = 0;
                gameOver = false;

                ball.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ball.speed = (Vector2){BALL_SPEED * (GetRandomValue(0, 1) ? 1 : -1), BALL_SPEED * (GetRandomValue(0, 1) ? 1 : -1)};

                player.rect.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
                enemy.rect.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < SCREEN_WIDTH; i += 20) {
            DrawLine(i, 0, i - 100, SCREEN_HEIGHT, Fade(WHITE, 0.05f));
        }

        DrawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 150, Fade(WHITE, 0.07f));

        DrawRoundedPaddle(player.rect, 0.5f);
        DrawRoundedPaddle(enemy.rect, 0.5f);

        DrawCircleV(ball.position, ball.radius, WHITE);

        DrawLives(player.lives, (Vector2){20, 20}, false);
        DrawLives(enemy.lives, (Vector2){SCREEN_WIDTH - 20, 20}, true);

        char scoreText[10];
        sprintf(scoreText, "%d", scoreLeft);
        DrawText(scoreText, SCREEN_WIDTH / 4, 20, 40, BLUE);
        
        sprintf(scoreText, "%d", scoreRight);
        DrawText(scoreText, 3 * SCREEN_WIDTH / 4, 20, 40, RED);

        DrawText("W/S to move left paddle", 10, SCREEN_HEIGHT - 20, 10, LIGHTGRAY);
        DrawText("UP/DOWN to move right paddle", SCREEN_WIDTH - 220, SCREEN_HEIGHT - 20, 10, LIGHTGRAY);

        DrawText("IncludeBuild - Raylib Pong Example", SCREEN_WIDTH / 2 - 170, 10, 20, GREEN);

        EndDrawing();
    }

    UnloadSound(paddleHit);
    UnloadSound(scoreSound);
    UnloadSound(wallBounce);
    remove("paddle_hit.wav");
    remove("score.wav");
    remove("wall_bounce.wav");
    remove("background_music.ogg");
    
    StopMusicStream(bgMusic);
    UnloadMusicStream(bgMusic);
    
    CloseAudioDevice();
    CloseWindow();
    return 0;
}