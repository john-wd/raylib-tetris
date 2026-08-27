#pragma once
#include <random>
#include <raylib.h>


std::mt19937 rng(std::random_device{}());

int RandomInt(int min, int max) {
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
    return {static_cast<float>(x * this->tileWidth),
            static_cast<float>(y * this->tileHeight),
            static_cast<float>(this->tileWidth),
            static_cast<float>(this->tileHeight)};
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