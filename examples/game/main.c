#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#include "sfx_paddlehit.h"
#include "sfx_score.h"
#include "sfx_wallbounce.h"
#include "sfx_backgroundmusic.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define MAX_LIVES 5

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
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RayPong");
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

    Paddle player = {{10, SCREEN_HEIGHT / 2 - 50, 10, 100}, 6, MAX_LIVES};
    Paddle enemy = {{SCREEN_WIDTH - 20, SCREEN_HEIGHT / 2 - 50, 10, 100}, 3, MAX_LIVES};
    Ball ball = {{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, {4, 4}, 10};

    // Initialize screen shake effect variables - will be used in a future update
    int screenShakeFrames = 0;
    float lastPlayerY = player.rect.y;
    bool gameOver = false;
    char winnerText[20];

    while (!WindowShouldClose()) {
        UpdateMusicStream(bgMusic);

        if (!gameOver) {
            float playerDeltaY = player.rect.y - lastPlayerY;
            lastPlayerY = player.rect.y;

            if (IsKeyDown(KEY_UP) && player.rect.y > 0) player.rect.y -= player.speed;
            if (IsKeyDown(KEY_DOWN) && player.rect.y < SCREEN_HEIGHT - player.rect.height) player.rect.y += player.speed;

            if (ball.speed.x > 0) { 
                if (ball.position.y > enemy.rect.y + enemy.rect.height / 2) enemy.rect.y += enemy.speed + (rand() % 2);
                if (ball.position.y < enemy.rect.y + enemy.rect.height / 2) enemy.rect.y -= enemy.speed + (rand() % 2);
            }

            ball.position.x += ball.speed.x;
            ball.position.y += ball.speed.y;

            if (ball.position.y <= 0 || ball.position.y >= SCREEN_HEIGHT) {
                ball.speed.y *= -1;
                PlaySound(wallBounce);
            }

            if (CheckCollisionCircleRec(ball.position, ball.radius, player.rect)) {
                PlaySound(paddleHit);
                screenShakeFrames = 8;

                ball.speed.x *= -1;
                ball.speed.y += playerDeltaY * 0.5f;
                ball.position.x = player.rect.x + player.rect.width + ball.radius;

                ball.speed.x *= 1.05f;
                ball.speed.y *= 1.05f;
            }
            if (CheckCollisionCircleRec(ball.position, ball.radius, enemy.rect)) {
                PlaySound(paddleHit);
                screenShakeFrames = 8;

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
                ball.speed = (Vector2){4, 4};
            }
            if (ball.position.x > SCREEN_WIDTH) { 
                enemy.lives--;
                PlaySound(scoreSound);
                ball.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ball.speed = (Vector2){-4, 4};
            }

            if (player.lives <= 0) {
                gameOver = true;
                sprintf(winnerText, "ENEMY WINS!");
            }
            if (enemy.lives <= 0) {
                gameOver = true;
                sprintf(winnerText, "YOU WIN!");
            }
            
            // Apply screen shake effect if active
            if (screenShakeFrames > 0) {
                screenShakeFrames--;
            }
        } else {
            DrawText(winnerText, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2, 30, WHITE);
            DrawText("Press [R] to Restart", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 40, 20, WHITE);

            if (IsKeyPressed(KEY_R)) {
                player.lives = MAX_LIVES;
                enemy.lives = MAX_LIVES;
                gameOver = false;

                ball.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ball.speed = (Vector2){4, 4};

                player.rect.y = SCREEN_HEIGHT / 2 - 50;
                enemy.rect.y = SCREEN_HEIGHT / 2 - 50;
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