#include <raylib.h>
#include <random>

std::mt19937 rng(std::random_device{}());

int RandomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

struct Tileset {
  Texture2D texture;
  int tileWidth;
  int tileHeight;
  int columns;
  int rows;

  Rectangle GetTile(int index) const {
      int x = index % this->columns;
      int y = index / this->columns;
      return {
          static_cast<float>(x * this->tileWidth),
          static_cast<float>(y * this->tileHeight),
          static_cast<float>(this->tileWidth),
          static_cast<float>(this->tileHeight)
      };
  }
};

Tileset LoadTileset(const char *path, int tileWidth, int tileHeight) {
  Tileset ts{};
  ts.texture = LoadTexture(path);
  ts.tileWidth = tileWidth;
  ts.tileHeight = tileHeight;

  ts.columns = ts.texture.width / tileWidth;
  ts.rows = ts.texture.height / tileHeight;
  return ts;
}

class Ticker {
public:
    Ticker(float tick = 2.0f) : tick(tick) { }

    bool IsTick(float dt) {
        acc += dt;
        if (acc >= tick) {
            acc -= tick;
            return true;
        }
        return false;
    }

    void Reset() {
        acc = 0.0f;
    }

    void SetTick(float t) {
        tick = t;
    }

  private:
    float tick = 0.0f;
    float acc = 0.0f;
};

static constexpr int BOARD_W = 10;
static constexpr int BOARD_H = 20;
static constexpr int NUM_TETRIMINOS = 7;

// 0 means empty, 1-7 means a tetrimino type
int board[BOARD_H][BOARD_W] = {0};

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
   }}
};

Piece GetPiece() {
  int idx = GetRandomValue(0, 6);
  return {idx, 0, 3, 0};
}

int main()
{
    constexpr int width = 600;
    constexpr int height = 800;

    constexpr int tileSize = 32;

    InitWindow(width, height, "raylib Tetris");

    SetTargetFPS(60);

    Tileset ts = LoadTileset("assets/blocks.png", 32, 32);

    Ticker ticker(1.0f);
    SetTargetFPS(60);

    Piece piece = GetPiece();

    while (!WindowShouldClose())
    {
        // Render
        BeginDrawing();

        ClearBackground(BLACK);

        // draw the tetris board
        DrawLine(0, 0, 0, BOARD_H*tileSize, WHITE);
        DrawLine(0, BOARD_H*tileSize, BOARD_W*tileSize, BOARD_H*tileSize, WHITE);
        DrawLine(BOARD_W*tileSize, 0, BOARD_W*tileSize, BOARD_H*tileSize, WHITE);

        // handle input
        if (IsKeyPressed(KEY_E)) {
            piece.rotation = (piece.rotation + 1) % 4;
        }
        if (IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A)) {
          piece.x -= 1;
        }
        if (IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D)) {
          piece.x += 1;
        }
        if (IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S)) {
          piece.y += 1;
        }
        if (IsKeyDown(KEY_W)) { }
        if (IsKeyPressed(KEY_N)) {piece = GetPiece();}

        for (int i = 0; i < BOARD_W; i++) {
            for (int j = 0; j < BOARD_H; j++) {
                int c = board[j][i];
                if (c > 0)
                    DrawTextureRec(ts.texture, ts.GetTile(c-1), {static_cast<float>(i * tileSize), static_cast<float>(j * tileSize)}, WHITE);
            }
        }


        // draw board

        float dt = GetFrameTime();
        if (ticker.IsTick(dt)) {
            piece.y++;
        };

        const auto& tetrimino = TETRIMINOS[piece.type];
        for (int i = 0; i < 4; i++) {
            const auto& rot = tetrimino.rotations[piece.rotation][i];
            int x = piece.x + rot.x;
            int y = piece.y + rot.y;
            DrawTextureRec(ts.texture, ts.GetTile(piece.type), {static_cast<float>(x * tileSize), static_cast<float>(y * tileSize)}, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
}
