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


bool IsRowFull(int y) {
    for (int x = 0; x < BOARD_W; x++) {
        if (board[y][x] == 0) {
            return false;
        }
    }
    return true;
}

void ClearLines() {
  for (int y = 0; y < BOARD_H; y++) {
    if (!IsRowFull(y)) {
      continue;
    }

    // move all rows above y down by 1
    for (int yy = y; yy > 0; yy--) {
      for (int x = 0; x < BOARD_W; x++) {
        board[yy][x] = board[yy-1][x];
      }
    }

    // clear top row
    for (int x = 0; x < BOARD_W; x++) {
      board[0][x] = 0;
    }
  }
}
bool CanPlace(const Piece &piece) {
  const auto& tetrimino = TETRIMINOS[piece.type].rotations[piece.rotation];
  
  for (int i = 0; i < 4; i++) {
    const auto& rot = tetrimino[i];
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

void Place(const Piece &piece) {
  const auto& tetrimino = TETRIMINOS[piece.type].rotations[piece.rotation];
  
  for (int i = 0; i < 4; i++) {
    const auto& rot = tetrimino[i];
    int x = piece.x + rot.x;
    int y = piece.y + rot.y;

    board[y-1][x] = piece.type+1;
  }
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
  candidate.rotation = (candidate.rotation + dir) % 4;
  if (CanPlace(candidate)) {
    return candidate;
  }
  return piece;
}

void Reset() {
  for (int i = 0; i < BOARD_H; i++) {
    for (int j = 0; j < BOARD_W; j++) {
      board[i][j] = 0;
    }
  }
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

        Piece candidate = piece;

        // handle input
        if (IsKeyPressed(KEY_E)) {
            piece = Rotate(piece, 1);
        }
        if (IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A)) {
          piece = Move(piece, -1);
        }
        if (IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D)) {
          piece = Move(piece, 1);
        }
        if (IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S)) {
          piece = Drop(piece);
        }
        if (IsKeyDown(KEY_W)) { }
        if (IsKeyPressed(KEY_N)) {piece = GetPiece();}
        if (IsKeyPressed(KEY_R)) {Reset();}

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
            piece = Drop(piece);
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
