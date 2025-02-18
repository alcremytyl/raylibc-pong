
/*
 * win screen  have "R" to reset
 * */

#include "raylib.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define BALL_SIZE 10
#define BALL_VEL 10
#define PLAYER_H 80
#define PLAYER_W 10
#define PLAYER_VEL 10
#define SCREEN_H 600.0f
#define SCREEN_W 800.0f
#define WIN_SCORE 2

enum State { START, PLAYING, PAUSED, WIN };

struct Game {
  State state = START;
  int scores[2] = {0, 0};
  int winner = -1;
  bool debug = false;
  Vector2 p_pos[2] = {{SCREEN_W * 0.1, (SCREEN_H - PLAYER_H) / 2.0},
                      {SCREEN_W - SCREEN_W * 0.1, (SCREEN_H - PLAYER_H) / 2.0}};
};

struct Ball {
  Vector2 pos = {(SCREEN_W - BALL_SIZE) / 2.0, (SCREEN_H - BALL_SIZE) / 2.0};
  Vector2 vel = {0, 0};
};

void HandleInputs(Game &game, Ball &ball);
void HandlePhysics(Game &game, Ball &ball);
void TogglePause(Game &game);
void ResetGame(Game &game, Ball &ball);
void DrawGame(Game &game, Ball &ball);
void CheckWin(Game &game);

int main() {
  Game game{};
  Ball ball{};

  srand(time(nullptr));
  InitWindow(SCREEN_W, SCREEN_H, "Pong");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    HandleInputs(game, ball);
    HandlePhysics(game, ball);
    DrawGame(game, ball);
  }

  CloseWindow();
  return 0;
}

void HandleInputs(Game &game, Ball &ball) {
  if (IsKeyPressed(KEY_P))
    TogglePause(game);

  if (IsKeyPressed(KEY_SPACE) && ball.vel.x == 0) {
    ball.vel.x = (std::rand() % 2 == 1 ? 1 : -1) * BALL_VEL;
    game.state = PLAYING;
  }

  if (IsKeyPressed(KEY_R) && game.state == PLAYING)
    ResetGame(game, ball);

  if (IsKeyPressed(KEY_F3))
    game.debug = !game.debug;

  int displace;
  int is_active = game.state == PLAYING;

  displace = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S)) * PLAYER_VEL * is_active;
  displace = std::min(SCREEN_H - PLAYER_H, game.p_pos[0].y - displace);
  game.p_pos[0].y = std::max(0, displace);

  displace = (IsKeyDown(KEY_UP) - IsKeyDown(KEY_DOWN)) * PLAYER_VEL * is_active;
  displace = std::min(SCREEN_H - PLAYER_H, game.p_pos[1].y - displace);
  game.p_pos[1].y = std::max(0, displace);
}

void HandlePhysics(Game &game, Ball &ball) {
  for (int i = 0; i < 2; i++) {
    auto pos = game.p_pos[i];

    if (ball.pos.x < pos.x + PLAYER_W && ball.pos.x + BALL_SIZE > pos.x &&
        ball.pos.y < pos.y + PLAYER_H && ball.pos.y + BALL_SIZE > pos.y) {

      bool side = ball.pos.x > SCREEN_W / 2.0;
      auto vel = (ball.pos.y + BALL_SIZE / 2.0) - (pos.y + PLAYER_H / 2.0);

      ball.vel.x *= -1;
      ball.pos.x = game.p_pos[side].x;
      ball.vel.y = (vel > 0 ? 1 : -1) * std::min(BALL_VEL * 1.5, std::abs(vel));
    }
  }

  // vertical bounce
  if (ball.pos.y <= 0 || ball.pos.y >= SCREEN_H)
    ball.vel.y *= -1;

  // left-right bounds & score
  if (ball.pos.x <= 0 || ball.pos.x >= SCREEN_W) {
    game.scores[ball.pos.x <= 0]++;
    CheckWin(game);
    ResetGame(game, ball);
  }

  ball.pos.x += !(game.state == PAUSED) * ball.vel.x;
  ball.pos.y += !(game.state == PAUSED) * ball.vel.y;
}

void TogglePause(Game &game) {
  if (game.state == PLAYING)
    game.state = PAUSED;
  else if (game.state == PAUSED)
    game.state = PLAYING;
}

void ResetGame(Game &game, Ball &ball) {
  game.p_pos[0].y = (SCREEN_H - PLAYER_H) / 2.0;
  game.p_pos[1].y = (SCREEN_H - PLAYER_H) / 2.0;
  ball.vel.x = 0;
  ball.vel.y = 0;
  ball.pos.x = (SCREEN_W - BALL_SIZE) / 2.0;
  ball.pos.y = (SCREEN_H - BALL_SIZE) / 2.0;

  if (game.state == WIN) {
    game.scores[0] = 0;
    game.scores[1] = 0;
  }
}

void DrawGame(Game &game, Ball &ball) {

  BeginDrawing();
  ClearBackground(RAYWHITE);

  DrawText(TextFormat("%d", game.scores[0]), SCREEN_W * .25, SCREEN_H / 12, 64,
           GRAY);
  DrawText(TextFormat("%d", game.scores[1]), SCREEN_W * .75, SCREEN_H / 12, 64,
           GRAY);

  if (game.debug) {
    auto text =
        TextFormat("scores: %d, %d\n"
                   "state: %d\n"
                   "positions: (%.1f, %.1f), (%.1f, %.1f)\n",
                   game.scores[0], game.scores[1], game.state, game.p_pos[0].x,
                   game.p_pos[0].y, game.p_pos[1].x, game.p_pos[1].y);

    DrawText(text, 10, SCREEN_H - 5 * 16, 16, DARKGRAY);
  }

  int center;

  switch (game.state) {
  case PLAYING:
    break;
  case PAUSED:
    center = MeasureText("PAUSED", 64) / 2.0;
    DrawText("PAUSED", SCREEN_W / 2.0 - center, (SCREEN_H - center) / 2.0, 64,
             RED);
    break;
  case START:
    center = MeasureText("PRESS SPACE", 64) / 2.0;
    DrawText("PRESS SPACE", SCREEN_W / 2.0 - center, (SCREEN_H - center) / 2.0,
             64, RED);
    center = MeasureText("TO START", 64) / 2.0;
    DrawText("TO START", SCREEN_W / 2.0 - center,
             (SCREEN_H - center + 64) / 2.0, 64, RED);
    break;
  case WIN:
    auto text = TextFormat("PLAYER %d WINNER", game.winner);
    center = MeasureText(text, 64) / 2.0;
    DrawText(text, SCREEN_W / 2.0 - center, (SCREEN_H - center + 64) / 2.0, 64,
             RED);
    break;
  }

  DrawRectangle(game.p_pos[0].x, game.p_pos[0].y, PLAYER_W, PLAYER_H, BLACK);
  DrawRectangle(game.p_pos[1].x, game.p_pos[1].y, PLAYER_W, PLAYER_H, BLACK);

  for (int i = 0; i < (SCREEN_H / 20); i++) {
    DrawRectangle((SCREEN_W - 2.5) / 2.0, i * 20 + 5, 5, 5, GRAY);
  }

  DrawRectangle(ball.pos.x, ball.pos.y, BALL_SIZE, BALL_SIZE, BLACK);

  EndDrawing();
}

void CheckWin(Game &game) {
  if (game.scores[0] == WIN_SCORE) {
    game.winner = 1;
    game.state = WIN;
  }

  else if (game.scores[1] == WIN_SCORE) {
    game.winner = 2;
    game.state = WIN;
  }
}
