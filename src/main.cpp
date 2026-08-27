#include "tetris.h"

int main()
{
    constexpr int width = 600;
    constexpr int height = 800;

    const char *title = "raylib Tetris";

    TetrisGame game(title, width, height);
    game.Run();
    
    return 0;
}
