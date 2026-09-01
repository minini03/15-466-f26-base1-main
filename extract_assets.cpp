#include "extract_assets.hpp"
#include "load_save_png.hpp"

#include <stdexcept>
#include <vector>

static int fill_palette_from_png(
	std::vector< glm::u8vec4 > const &data,
	PPU466::Palette *palette,
	std::string const &filename
) {
	*palette = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};
	int used = 1;
	for (glm::u8vec4 px : data) {
		if (px.a < 128) continue;
		px.a = 0xff;
		bool found = false;
		for (uint32_t i = 1; i < used; ++i) {
			if ((*palette)[i] == px) {
				found = true;
				break;
			}
		}
		if (found) continue;
		(*palette)[used] = px;
		used += 1;
	}
	return used;
}

static uint8_t palette_index(glm::u8vec4 px, PPU466::Palette const &palette, int used) {
	if (px.a < 128) return 0;
	px.a = 0xff;
	for (uint32_t i = 1; i < used; ++i) {
		if (px == palette[i]) return uint8_t(i);
	}
	return 0;
}

void extract_8_16(
	std::string const &filename,
	PPU466::Tile *tiles,
	PPU466::Palette *palette
) {
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(filename, &size, &data, LowerLeftOrigin);

	int used = fill_palette_from_png(data, palette, filename);

	auto encode_tile = [&](uint32_t x0, uint32_t y0) {
		PPU466::Tile tile{};
		for (uint32_t y = 0; y < 8; ++y) {
			uint8_t bit0 = 0;
			uint8_t bit1 = 0;
			uint32_t py = y0 + y;
			for (uint32_t x = 0; x < 8; ++x) {
				glm::u8vec4 px(0, 0, 0, 0);
				uint32_t px_x = x0 + x;
				if (py < size.y && px_x < size.x) {
					px = data[px_x + py * size.x];
				}
				uint8_t idx = palette_index(px, *palette, used);
				bit0 |= uint8_t((idx & 1) << x);
				bit1 |= uint8_t(((idx >> 1) & 1) << x);
			}
			tile.bit0[y] = bit0;
			tile.bit1[y] = bit1;
		}
		return tile;
	};

	tiles[0] = encode_tile(0, 0); // bottom
	tiles[1] = encode_tile(0, 8); // top
}

void extract_16_8(
	std::string const &filename,
	PPU466::Tile *tiles,
	PPU466::Palette *palette
) {
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(filename, &size, &data, LowerLeftOrigin);

	int used = fill_palette_from_png(data, palette, filename);

	auto encode_tile = [&](uint32_t x0, uint32_t y0) {
		PPU466::Tile tile{};
		for (uint32_t y = 0; y < 8; ++y) {
			uint8_t bit0 = 0;
			uint8_t bit1 = 0;
			uint32_t py = y0 + y;
			for (uint32_t x = 0; x < 8; ++x) {
				glm::u8vec4 px(0, 0, 0, 0);
				uint32_t px_x = x0 + x;
				if (py < size.y && px_x < size.x) {
					px = data[px_x + py * size.x];
				}
				uint8_t idx = palette_index(px, *palette, used);
				bit0 |= uint8_t((idx & 1) << x);
				bit1 |= uint8_t(((idx >> 1) & 1) << x);
			}
			tile.bit0[y] = bit0;
			tile.bit1[y] = bit1;
		}
		return tile;
	};

	tiles[0] = encode_tile(0, 0); // left
	tiles[1] = encode_tile(8, 0); // right
}

void extract_8_8(
	std::string const &filename,
	PPU466::Tile *tiles,
	PPU466::Palette *palette
) {
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(filename, &size, &data, LowerLeftOrigin);

	int used = fill_palette_from_png(data, palette, filename);

	auto encode_tile = [&](uint32_t x0, uint32_t y0) {
		PPU466::Tile tile{};
		for (uint32_t y = 0; y < 8; ++y) {
			uint8_t bit0 = 0;
			uint8_t bit1 = 0;
			uint32_t py = y0 + y;
			for (uint32_t x = 0; x < 8; ++x) {
				glm::u8vec4 px(0, 0, 0, 0);
				uint32_t px_x = x0 + x;
				if (py < size.y && px_x < size.x) {
					px = data[px_x + py * size.x];
				}
				uint8_t idx = palette_index(px, *palette, used);
				bit0 |= uint8_t((idx & 1) << x);
				bit1 |= uint8_t(((idx >> 1) & 1) << x);
			}
			tile.bit0[y] = bit0;
			tile.bit1[y] = bit1;
		}
		return tile;
	};

	tiles[0] = encode_tile(0, 0);
}

void extract_16_16(
	std::string const &filename,
	PPU466::Tile *tiles,
	PPU466::Palette *palette
) {
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(filename, &size, &data, LowerLeftOrigin);

	int used = fill_palette_from_png(data, palette, filename);

	auto encode_tile = [&](uint32_t x0, uint32_t y0) {
		PPU466::Tile tile{};
		for (uint32_t y = 0; y < 8; ++y) {
			uint8_t bit0 = 0;
			uint8_t bit1 = 0;
			uint32_t py = y0 + y;
			for (uint32_t x = 0; x < 8; ++x) {
				glm::u8vec4 px(0, 0, 0, 0);
				uint32_t px_x = x0 + x;
				if (py < size.y && px_x < size.x) {
					px = data[px_x + py * size.x];
				}
				uint8_t idx = palette_index(px, *palette, used);
				bit0 |= uint8_t((idx & 1) << x);
				bit1 |= uint8_t(((idx >> 1) & 1) << x);
			}
			tile.bit0[y] = bit0;
			tile.bit1[y] = bit1;
		}
		return tile;
	};

	tiles[0] = encode_tile(0, 0); // bottom-left
	tiles[1] = encode_tile(8, 0); // bottom-right
	tiles[2] = encode_tile(0, 8); // top-left
	tiles[3] = encode_tile(8, 8); // top-right
}
