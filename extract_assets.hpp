#pragma once

#include "PPU466.hpp"

#include <string>

// Load an 8x16 PNG into two tiles: top 8x8, bottom 8x8.
void extract_8_16(
	std::string const &filename,
	PPU466::Tile *tiles,
	PPU466::Palette *palette
);

// Load a 16x8 PNG into two tiles: left 8x8, right 8x8.
void extract_16_8(
	std::string const &filename,
	PPU466::Tile *tiles,
	PPU466::Palette *palette
);

// Load an 8x8 PNG into a tile.
void extract_8_8(
	std::string const &filename,
	PPU466::Tile *tiles,
	PPU466::Palette *palette
);

// Load an 16x16 PNG into four tiles.
void extract_16_16(
	std::string const &filename,
	PPU466::Tile *tiles,
	PPU466::Palette *palette
);
