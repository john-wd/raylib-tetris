#pragma once
#include <raylib.h>


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