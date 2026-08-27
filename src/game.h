#pragma once

#include "util.h"
#include <raylib.h>
#include <unordered_map>

static constexpr int BOARD_W = 10;
static constexpr int BOARD_H = 20;
static constexpr int TILE_SIZE = 32;
static constexpr int NUM_TETRIMINOS = 7;

class Ticker {
public:
  Ticker(float tick = 2.0f) : tick(tick) {}

  bool IsTick(float dt) {
    acc += dt;
    if (acc >= tick) {
      acc -= tick;
      return true;
    }
    return false;
  }

  void Reset() { acc = 0.0f; }
  void SetTick(float t) { tick = t; }

private:
  float tick = 0.0f;
  float acc = 0.0f;
};

struct Cell {
  int x;
  int y;
};

struct Tetrimino {
  Cell rotations[4][4];
};

struct Piece {
  int type;
  int rotation;
  int x;
  int y;
};

Tetrimino TETRIMINOS[NUM_TETRIMINOS] = {
    // I
    {{
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}},
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}},
    }},
    // J
    {{
        {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
        {{1, 0}, {1, 1}, {0, 2}, {1, 2}},
        {{0, 1}, {0, 2}, {1, 2}, {2, 2}},
        {{0, 0}, {1, 0}, {0, 1}, {0, 2}},
    }},
    // L
    {{
        {{0, 0}, {0, 1}, {0, 2}, {1, 2}},
        {{0, 1}, {1, 1}, {2, 1}, {0, 2}},
        {{0, 0}, {1, 0}, {1, 1}, {1, 2}},
        {{2, 1}, {0, 2}, {1, 2}, {2, 2}},
    }},
    // O
    {{
        {{1, 1}, {2, 1}, {1, 2}, {2, 2}},
        {{1, 1}, {2, 1}, {1, 2}, {2, 2}},
        {{1, 1}, {2, 1}, {1, 2}, {2, 2}},
        {{1, 1}, {2, 1}, {1, 2}, {2, 2}},
    }},
    // T
    {{
        {{1, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {2, 1}, {1, 2}},
        {{0, 1}, {1, 1}, {2, 1}, {1, 2}},
        {{1, 0}, {0, 1}, {1, 1}, {1, 2}},
    }},
    // Z
    {{
        {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
        {{2, 0}, {1, 1}, {2, 1}, {1, 2}},
        {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
        {{2, 0}, {1, 1}, {2, 1}, {1, 2}},
    }},
    // S
    {{
        {{1, 1}, {2, 1}, {0, 2}, {1, 2}},
        {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
        {{1, 1}, {2, 1}, {0, 2}, {1, 2}},
        {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
    }}};

class Game {
public:
  Game(const char *title, int width = 600, int height = 800, int targetFps = 60)
      : winWidth(width), winHeight(height), title(title), targetFps(targetFps) {

  }

  virtual void Init() = 0;
  virtual void Update(float dt) = 0;
  virtual void Close() {};

  void Run() {
    InitWindow(winWidth, winHeight, title);
    InitAudioDevice();
    Init();
    SetTargetFPS(targetFps);

    while (!WindowShouldClose()) {
      float dt = GetFrameTime();
      Update(dt);
    }

    Close();
    CloseAudioDevice();
    CloseWindow();
  }

private:
  int winWidth;
  int winHeight;
  int targetFps;
  const char *title;
};

class TetrisGame : public Game {
public:
  TetrisGame(const char *title, int width = 600, int height = 800,
             int targetFps = 60)
      : Game(title, width, height, targetFps) {}
  void Init() override {
    ticker = Ticker(1.0f);

    music = LoadMusicStream("assets/play.wav");

    sounds.emplace("move", LoadSound("assets/sfx/move.wav"));
    sounds.emplace("drop", LoadSound("assets/sfx/drop.wav"));
    sounds.emplace("rotate", LoadSound("assets/sfx/rotate.wav"));
    sounds.emplace("clear", LoadSound("assets/sfx/clear.wav"));

    ts = LoadTileset("assets/blocks.png", TILE_SIZE, TILE_SIZE);

    // Initialize the first piece
    piece = GetPiece();
    PlayMusicStream(music);
  };
  void Close() override { StopMusicStream(music); }
  void Update(float dt) override {
    UpdateMusicStream(music);
    Piece candidate = piece;

    // handle input
    if (IsKeyPressed(KEY_K)) {
      piece = Rotate(piece, 1);
      PlaySound(sounds["rotate"]);
    }
    if (IsKeyPressed(KEY_J)) {
      piece = Rotate(piece, -1);
      PlaySound(sounds["rotate"]);
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A)) {
      piece = Move(piece, -1);
      PlaySound(sounds["move"]);
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D)) {
      piece = Move(piece, 1);
      PlaySound(sounds["move"]);
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S)) {
      piece = Drop(piece);
    }
    if (IsKeyPressed(KEY_W)) {
      piece = HardDrop(piece);
    }
    if (IsKeyPressed(KEY_N)) {
      piece = GetPiece();
    }
    if (IsKeyPressed(KEY_R)) {
      Reset();
    }

    // simulate
    if (ticker.IsTick(dt)) {
      piece = Drop(piece);
    };

    // render
    BeginDrawing();
    ClearBackground(BLACK);

    // draw board boundary
    for (int i = 0; i < BOARD_W; i++) {
      for (int j = 0; j < BOARD_H; j++) {
        int c = board[j][i];
        if (c > 0)
          DrawTextureRec(ts.texture, ts.GetTile(c - 1),
                         {static_cast<float>(i * TILE_SIZE),
                          static_cast<float>(j * TILE_SIZE)},
                         WHITE);
      }
    }

    // draw the tetris board
    DrawLine(0, 0, 0, BOARD_H * TILE_SIZE, WHITE);
    DrawLine(0, BOARD_H * TILE_SIZE, BOARD_W * TILE_SIZE, BOARD_H * TILE_SIZE,
             WHITE);
    DrawLine(BOARD_W * TILE_SIZE, 0, BOARD_W * TILE_SIZE, BOARD_H * TILE_SIZE,
             WHITE);

    const auto &tetrimino = TETRIMINOS[piece.type];
    for (int i = 0; i < 4; i++) {
      const auto &rot = tetrimino.rotations[piece.rotation][i];
      int x = piece.x + rot.x;
      int y = piece.y + rot.y;
      DrawTextureRec(ts.texture, ts.GetTile(piece.type),
                     {static_cast<float>(x * TILE_SIZE),
                      static_cast<float>(y * TILE_SIZE)},
                     WHITE);
    }

    EndDrawing();
  };

private:
  void Reset() {
    for (int i = 0; i < BOARD_H; i++) {
      for (int j = 0; j < BOARD_W; j++) {
        board[i][j] = 0;
      }
    }
    piece = GetPiece();
    ticker.Reset();
  }
  void Place(const Piece &piece) {
    const auto &tetrimino = TETRIMINOS[piece.type].rotations[piece.rotation];

    for (int i = 0; i < 4; i++) {
      const auto &rot = tetrimino[i];
      int x = piece.x + rot.x;
      int y = piece.y + rot.y;

      board[y - 1][x] = piece.type + 1;
    }
    PlaySound(sounds["drop"]);
  }
  Piece GetPiece() {
    int idx = GetRandomValue(0, 6);
    return {idx, 0, 3, 0};
  }

  bool IsRowFull(int y) {
    for (int x = 0; x < BOARD_W; x++) {
      if (board[y][x] == 0) {
        return false;
      }
    }
    return true;
  }

  void ClearLines() {
    bool cleared = false;
    for (int y = 0; y < BOARD_H; y++) {
      if (!IsRowFull(y)) {
        continue;
      }

      cleared = true;
      // move all rows above y down by 1
      for (int yy = y; yy > 0; yy--) {
        for (int x = 0; x < BOARD_W; x++) {
          board[yy][x] = board[yy - 1][x];
        }
      }

      // clear top row
      for (int x = 0; x < BOARD_W; x++) {
        board[0][x] = 0;
      }
    }

    if (cleared)
      PlaySound(sounds["clear"]);
  }
  bool CanPlace(const Piece &piece) {
    const auto &tetrimino = TETRIMINOS[piece.type].rotations[piece.rotation];

    for (int i = 0; i < 4; i++) {
      const auto &rot = tetrimino[i];
      int x = piece.x + rot.x;
      int y = piece.y + rot.y;

      // check board bounds
      if (x < 0 || x >= BOARD_W || y < 0 || y >= BOARD_H) {
        return false;
      }

      // check board occupancy
      if (y > 0 && board[y][x] != 0) {
        return false;
      }
    }

    return true;
  }

  Piece HardDrop(const Piece &piece) {
    Piece candidate = piece;
    while (CanPlace(candidate)) {
      candidate.y++;
    }
    Place(candidate);
    ClearLines();
    return GetPiece();
  }

  Piece Drop(const Piece &piece) {
    Piece candidate = piece;
    candidate.y++;

    while (CanPlace(candidate)) {
      return candidate;
    }

    Place(candidate);
    ClearLines();
    return GetPiece();
  }

  Piece Move(const Piece &piece, int dx) {
    Piece candidate = piece;
    candidate.x += dx;
    if (CanPlace(candidate)) {
      return candidate;
    }
    return piece;
  }

  Piece Rotate(const Piece &piece, int dir) {
    Piece candidate = piece;
    int rot = candidate.rotation + dir;
    if (rot < 0) {
      rot += 4;
    }
    candidate.rotation = rot % 4;
    if (CanPlace(candidate)) {
      return candidate;
    }
    return piece;
  }

private:
  std::unordered_map<std::string, Sound> sounds;
  Music music;
  Ticker ticker;
  Tileset ts;

  Piece piece;
  int board[BOARD_H][BOARD_W] = {0};
};
