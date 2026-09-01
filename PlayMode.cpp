#include "PlayMode.hpp"
#include "extract_assets.hpp"
#include "data_path.hpp"

#include "gl_errors.hpp"

#include <algorithm>

namespace {

bool overlaps(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
	return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

bool stand_on_platform(PlayMode::Player const &pl, PlayMode::Platform const &p) {
	return overlaps(pl.position.x, pl.position.y, PlayMode::PlayerWidth, PlayMode::PlayerHeight,
	                p.position.x, p.position.y, p.w, PlayMode::PlatformHeight);
}
bool player_overlap_box(PlayMode::Player const &pl, PlayMode::Box const &b) {
	return overlaps(pl.position.x, pl.position.y, PlayMode::PlayerWidth, PlayMode::PlayerHeight,
	                b.position.x, b.position.y, PlayMode::BoxSize, PlayMode::BoxSize);
}
bool stand_on_box(PlayMode::Player const &pl, PlayMode::Box const &b) {
	if (!player_overlap_box(pl, b)) {
		return false;
	}
	float const box_top = b.position.y + PlayMode::BoxSize;
	return pl.position.y >= box_top - 4.0f;
}
bool player_overlap_exit(PlayMode::Player const &pl, PlayMode::Exit const &e) {
	return overlaps(pl.position.x, pl.position.y, PlayMode::PlayerWidth, PlayMode::PlayerHeight,
	                e.position.x, e.position.y, PlayMode::ExitWidth, PlayMode::ExitHeight);
}
bool player_overlap_coin(PlayMode::Player const &pl, PlayMode::Coin const &c) {
	return overlaps(pl.position.x, pl.position.y, PlayMode::PlayerWidth, PlayMode::PlayerHeight,
	                c.position.x, c.position.y, PlayMode::CoinSize, PlayMode::CoinSize);
}

PPU466::Tile make_solid_tile() {
	PPU466::Tile tile;
	for (uint32_t y = 0; y < 8; ++y) {
		tile.bit0[y] = 0xff;
		tile.bit1[y] = 0x00;
	}
	return tile;
}

PPU466::Tile make_index_tile(uint8_t color_index) {
	PPU466::Tile tile;
	uint8_t const b0 = (color_index & 1) ? uint8_t(0xff) : uint8_t(0x00);
	uint8_t const b1 = (color_index & 2) ? uint8_t(0xff) : uint8_t(0x00);
	for (uint32_t y = 0; y < 8; ++y) {
		tile.bit0[y] = b0;
		tile.bit1[y] = b1;
	}
	return tile;
}

PPU466::Tile make_empty_tile() {
	PPU466::Tile tile;
	for (uint32_t y = 0; y < 8; ++y) {
		tile.bit0[y] = 0x00;
		tile.bit1[y] = 0x00;
	}
	return tile;
}

uint8_t reverse_bits(uint8_t b) {
	uint8_t r = 0;
	for (uint32_t i = 0; i < 8; ++i) {
		if (b & uint8_t(1 << i)) r |= uint8_t(1 << (7 - i));
	}
	return r;
}

PPU466::Tile flip_tile_horizontal(PPU466::Tile const &src) {
	PPU466::Tile dst;
	for (uint32_t y = 0; y < 8; ++y) {
		dst.bit0[y] = reverse_bits(src.bit0[y]);
		dst.bit1[y] = reverse_bits(src.bit1[y]);
	}
	return dst;
}

} //namespace

PlayMode::PlayMode() {
	ppu.tile_table[0] = make_empty_tile();
	ppu.tile_table[1] = make_solid_tile();
	ppu.tile_table[2] = make_index_tile(2);
	ppu.tile_table[3] = make_index_tile(3);

	extract_8_16(data_path("player1.png"), &ppu.tile_table[4], &ppu.palette_table[0]);
	ppu.tile_table[6] = flip_tile_horizontal(ppu.tile_table[4]);
	ppu.tile_table[7] = flip_tile_horizontal(ppu.tile_table[5]);
	extract_8_16(data_path("player2.png"), &ppu.tile_table[8], &ppu.palette_table[2]);
	ppu.tile_table[10] = flip_tile_horizontal(ppu.tile_table[8]);
	ppu.tile_table[11] = flip_tile_horizontal(ppu.tile_table[9]);

	extract_8_16(data_path("door1.png"), &ppu.tile_table[36], &ppu.palette_table[6]);
	extract_8_16(data_path("door2.png"), &ppu.tile_table[46], &ppu.palette_table[4]);
	extract_16_16(data_path("box.png"), &ppu.tile_table[38], &ppu.palette_table[7]);
	extract_16_8(data_path("you.png"), &ppu.tile_table[42], &ppu.palette_table[3]);
	extract_16_8(data_path("win.png"), &ppu.tile_table[44], &ppu.palette_table[3]);
	extract_16_8(data_path("die.png"), &ppu.tile_table[50], &ppu.palette_table[3]);
	extract_8_8(data_path("coin.png"), &ppu.tile_table[48], &ppu.palette_table[5]);

	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x6b, 0x3a, 0x1a, 0xff),
		glm::u8vec4(0xff, 0xf4, 0xa4, 0xff),
		glm::u8vec4(0x71, 0x6a, 0x79, 0xff),
	};

	ppu.background_color = glm::u8vec3(0xff, 0xf4, 0xa4);

	platforms = {
		{ glm::vec2(  0.0f,   0.0f), 256.0f },
		{ glm::vec2( 210.0f,  40.0f),  40.0f },
		{ glm::vec2( 20.0f,  56.0f),  80.0f },
		{ glm::vec2(144.0f,  80.0f),  40.0f },
		{ glm::vec2(72.0f,  96.0f),  28.0f },
		{ glm::vec2( 200.0f, 120.0f),  48.0f },
	};

	ppu.background.fill(0);
	int32_t const mid_tx = int32_t(PPU466::ScreenWidth / 8) / 2;
	for (int32_t ty = 0; ty < int32_t(PPU466::BackgroundHeight); ++ty) {
		for (int32_t tx = 0; tx < int32_t(PPU466::BackgroundWidth); ++tx) {
			uint8_t const tile = ((tx % int32_t(PPU466::ScreenWidth / 8)) < mid_tx) ? uint8_t(2) : uint8_t(3);
			ppu.background[uint32_t(tx) + PPU466::BackgroundWidth * uint32_t(ty)] = uint16_t(tile | (1 << 8));
		}
	}
	for (Platform const &p : platforms) {
		int32_t x0 = int32_t(p.position.x) / 8;
		int32_t y0 = int32_t(p.position.y) / 8;
		int32_t x1 = int32_t(p.position.x + p.w) / 8;
		int32_t y1 = int32_t(p.position.y + PlatformHeight) / 8;
		if (y1 <= y0) y1 = y0 + 1;
		for (int32_t ty = y0; ty < y1; ++ty) {
			for (int32_t tx = x0; tx < x1; ++tx) {
				if (tx < 0 || ty < 0) continue;
				if (tx >= int32_t(PPU466::BackgroundWidth)) continue;
				if (ty >= int32_t(PPU466::BackgroundHeight)) continue;
				ppu.background[uint32_t(tx) + PPU466::BackgroundWidth * uint32_t(ty)] = uint16_t(1 | (1 << 8));
			}
		}
	}

	player1.position = glm::vec2(10.0f, 16.0f);
	player2.position = glm::vec2(238.0f, 16.0f);
	player2.facing_front = false;

	boxes = {
		{ glm::vec2(210.0f, 8.0f) },
	};

	exit1.position = glm::vec2(80.0f, 104.0f);
	exit1.is_locked = true;
	exit2.position = glm::vec2(216.0f, 128.0f);
	exit2.is_locked = false;

	coins = {
		{ glm::vec2(148.0f, 65.0f), &exit1, false },
	};

	texts = {
		{ glm::vec2(108.0f, 200.0f), false },
		{ glm::vec2(132.0f, 200.0f), false },
	};
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			jump.downs += 1;
			jump.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_ESCAPE) {
			auto keep = shared_from_this();
			Mode::set_current(std::make_shared< PlayMode >());
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			jump.pressed = false;
			return true;
		}
	}
	return false;
}

void PlayMode::update(float elapsed) {
	if (won || lost) {
		// game end.
		left.downs = 0;
		right.downs = 0;
		jump.downs = 0;
		return;
	}

	auto move_horizontal = [&](Player &player, bool is_symmetric) {
		player.vel_x = 0.0f;
		if (left.pressed) player.vel_x -= MoveSpeed;
		if (right.pressed) player.vel_x += MoveSpeed;
		if (player.vel_x > 0.0f) player.facing_front = true;
		else if (player.vel_x < 0.0f) player.facing_front = false;

		if (is_symmetric) {
			// player2
			player.vel_x = -player.vel_x;
		}
		player.position.x += player.vel_x * elapsed;
		for (Platform const &p : platforms) {
			if (!stand_on_platform(player, p)) {
				// no collision
				continue;
			}

			if (player.vel_x > 0.0f) {
				// left collision
				player.position.x = p.position.x - PlayerWidth;
			}
			else if (player.vel_x < 0.0f) {
				// right collision
				player.position.x = p.position.x + p.w;
			}
		}
	};

	// push boxes
	auto push_boxes = [&](Player &player) {
		if (player.vel_x == 0.0f) return;

		for (Box &box : boxes) {
			if (!player_overlap_box(player, box)) {
				continue;
			}
			// standing on box
			if (player.position.y >= box.position.y + BoxSize - 1.0f) {
				continue;
			}
			if (player.vel_x != 0) {
				// two players versus one box
				if ((&player == &player1 && player_overlap_box(player2, box)) || (&player == &player2 && player_overlap_box(player1, box))) {
					player1.vel_x = 0;
					player2.vel_x = 0;
					player1.position.x = box.position.x - PlayerWidth;
					player2.position.x = box.position.x + BoxSize;
					continue;
				}
			}

			if (player.vel_x > 0.0f) {
				// left push
				box.position.x = player.position.x + PlayerWidth;
			} else {
				// right push
				box.position.x = player.position.x - BoxSize;
			}
		}
	};

	auto try_jump = [&](Player &player) {
		if (player.on_ground && jump.downs > 0) {
			player.vel_y = JumpSpeed;
			player.on_ground = false;
		}
	};


	auto apply_gravity = [&](Player &player) {
		player.vel_y += Gravity * elapsed;
		player.position.y += player.vel_y * elapsed;

		player.on_ground = false;
		for (Platform const &p : platforms) {
			if (!stand_on_platform(player, p)) continue;
			if (player.vel_y <= 0.0f) {
				player.position.y = p.position.y + PlatformHeight;
				player.vel_y = 0.0f;
				player.on_ground = true;
			} else {
				player.position.y = p.position.y - PlayerHeight;
				player.vel_y = 0.0f;
			}
		}

		for (Box const &b : boxes) {
			// stand on a box
			if (!stand_on_box(player, b)) continue;
			if (player.vel_y <= 0.0f) {
				player.position.y = b.position.y + BoxSize;
				player.vel_y = 0.0f;
				player.on_ground = true;
			}
		}
	};


	auto clamp_to_screen = [&](Player &player) {
		// avoid players go outside the screen.
		player.position.x = std::clamp(player.position.x, 0.0f, float(PPU466::ScreenWidth) - PlayerWidth);
		player.position.y = std::clamp(player.position.y, 0.0f, float(PPU466::ScreenHeight) - PlayerHeight);
	};

	float const mid = float(PPU466::ScreenWidth) * 0.5f;
	if (!exit1.reached) move_horizontal(player1, false);
	if (!exit2.reached) move_horizontal(player2, true);
	if (!exit1.reached) push_boxes(player1);
	if (!exit2.reached) push_boxes(player2);
	if (!exit1.reached) try_jump(player1);
	if (!exit2.reached) try_jump(player2);
	if (!exit1.reached) apply_gravity(player1);
	if (!exit2.reached) apply_gravity(player2);
	if (!exit1.reached) clamp_to_screen(player1);
	if (!exit2.reached) clamp_to_screen(player2);

	bool const p1_other_half = player1.position.x + PlayerWidth > mid;
	bool const p2_other_half = player2.position.x < mid;
	if (p1_other_half || p2_other_half) {
		// player enter other's zone.
		lost = true;
		texts[0].is_exposed = true;
		texts[1].is_exposed = true;
	}

	if (!lost) {
		for (Coin &coin : coins) {
			if (coin.collected) continue;
			if (player_overlap_coin(player1, coin) || player_overlap_coin(player2, coin)) {
				coin.collected = true;
				if (coin.exit) coin.exit->is_locked = false;
			}
		}

		if (!exit1.is_locked && player_overlap_exit(player1, exit1)) {
			exit1.reached = true;
		}
		if (!exit2.is_locked && player_overlap_exit(player2, exit2)) {
			exit2.reached = true;
		}
		if (exit1.reached && exit2.reached) {
			won = true;
			texts[0].is_exposed = true;
			texts[1].is_exposed = true;
		}
	}

	left.downs = 0;
	right.downs = 0;
	jump.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	auto draw_player = [&](Player const &player, uint8_t sprite_bottom, uint8_t tile_front, uint8_t palette) {
		uint8_t bottom = player.facing_front ? tile_front : uint8_t(tile_front + 2);
		uint8_t top = uint8_t(bottom + 1);
		ppu.sprites[sprite_bottom].x = uint8_t(player.position.x);
		ppu.sprites[sprite_bottom].y = uint8_t(player.position.y);
		ppu.sprites[sprite_bottom].index = bottom;
		ppu.sprites[sprite_bottom].attributes = palette;
		ppu.sprites[sprite_bottom + 1].x = uint8_t(player.position.x);
		ppu.sprites[sprite_bottom + 1].y = uint8_t(player.position.y + 8.0f);
		ppu.sprites[sprite_bottom + 1].index = top;
		ppu.sprites[sprite_bottom + 1].attributes = palette;
	};

	draw_player(player1, 0, 4, 0);
	draw_player(player2, 2, 8, 2);

	for (size_t i = 0; i < boxes.size(); ++i) {
		uint8_t const s = uint8_t(8 + i * 4);
		uint8_t const x = uint8_t(boxes[i].position.x);
		uint8_t const y = uint8_t(boxes[i].position.y);
		ppu.sprites[s + 0].x = x;
		ppu.sprites[s + 0].y = y;
		ppu.sprites[s + 0].index = 38;
		ppu.sprites[s + 0].attributes = 7;
		ppu.sprites[s + 1].x = uint8_t(x + 8);
		ppu.sprites[s + 1].y = y;
		ppu.sprites[s + 1].index = 39;
		ppu.sprites[s + 1].attributes = 7;
		ppu.sprites[s + 2].x = x;
		ppu.sprites[s + 2].y = uint8_t(y + 8);
		ppu.sprites[s + 2].index = 40;
		ppu.sprites[s + 2].attributes = 7;
		ppu.sprites[s + 3].x = uint8_t(x + 8);
		ppu.sprites[s + 3].y = uint8_t(y + 8);
		ppu.sprites[s + 3].index = 41;
		ppu.sprites[s + 3].attributes = 7;
	}

	if (!exit1.is_locked) {
		ppu.sprites[6].x = uint8_t(exit1.position.x);
		ppu.sprites[6].y = uint8_t(exit1.position.y);
		ppu.sprites[6].index = 36;
		ppu.sprites[6].attributes = 6;
		ppu.sprites[7].x = uint8_t(exit1.position.x);
		ppu.sprites[7].y = exit1.is_locked ? uint8_t(240) : uint8_t(exit1.position.y + 8.0f);
		ppu.sprites[7].index = 37;
		ppu.sprites[7].attributes = 6;
	}

	ppu.sprites[16].x = uint8_t(exit2.position.x);
	ppu.sprites[16].y = uint8_t(exit2.position.y);
	ppu.sprites[16].index = 46;
	ppu.sprites[16].attributes = 4;
	ppu.sprites[17].x = uint8_t(exit2.position.x);
	ppu.sprites[17].y = uint8_t(exit2.position.y + 8.0f);
	ppu.sprites[17].index = 47;
	ppu.sprites[17].attributes = 4;

	for (size_t i = 0; i < coins.size(); ++i) {
		uint8_t const y = coins[i].collected ? uint8_t(240) : uint8_t(coins[i].position.y);
		ppu.sprites[18 + i].x = uint8_t(coins[i].position.x);
		ppu.sprites[18 + i].y = y;
		ppu.sprites[18 + i].index = 48;
		ppu.sprites[18 + i].attributes = 5;
	}

	for (size_t i = 0; i < texts.size(); ++i) {
		uint8_t const s = uint8_t(12 + i * 2);
		uint8_t const x = uint8_t(texts[i].position.x);
		uint8_t const y = texts[i].is_exposed ? uint8_t(texts[i].position.y) : uint8_t(240);
		uint8_t const tile = (i == 1 && lost) ? uint8_t(50) : uint8_t(42 + i * 2);
		ppu.sprites[s + 0].x = x;
		ppu.sprites[s + 0].y = y;
		ppu.sprites[s + 0].index = tile;
		ppu.sprites[s + 0].attributes = 3;
		ppu.sprites[s + 1].x = uint8_t(x + 8);
		ppu.sprites[s + 1].y = y;
		ppu.sprites[s + 1].index = uint8_t(tile + 1);
		ppu.sprites[s + 1].attributes = 3;
	}

	ppu.draw(drawable_size);
}
