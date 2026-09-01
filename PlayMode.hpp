#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	};

	struct Platform {
		glm::vec2 position = glm::vec2(0.0f);
		float w = 0.0f;
	};
	std::vector< Platform > platforms;

	struct Box {
		glm::vec2 position = glm::vec2(0.0f);
		float vel_y = 0.0f;
		bool on_ground = false;
	};
	std::vector<Box> boxes;


	struct Exit {
		glm::vec2 position = glm::vec2(0.0f);
		bool is_locked = false;
		bool reached = false;
	};

	// relate to two players seperately.
	Exit exit1;
	Exit exit2;
	bool won = false;
	bool lost = false;

	struct Text {
		glm::vec2 position = glm::vec2(0.0f);
		bool is_exposed = false;
	};
	std::vector<Text> texts;

	struct Coin {
		glm::vec2 position = glm::vec2(0.0f);
		Exit *exit = nullptr;
		bool collected = false;
	};
	std::vector<Coin> coins;

	Button left, right, jump;
	struct Player {
		glm::vec2 position;
		float vel_x = 0.0f;
		float vel_y = 0.0f;
		bool on_ground = false;
		bool facing_front = true;
	};
	Player player1, player2;


	static constexpr float PlatformHeight = 8.0f;
	static constexpr float PlayerWidth = 8.0f;
	static constexpr float PlayerHeight = 16.0f;
	static constexpr float BoxSize = 16.0f;
	static constexpr float ExitWidth = 8.0f;
	static constexpr float ExitHeight = 16.0f;
	static constexpr float TextWidth = 16.0f;
	static constexpr float TextHeight = 8.0f;
	static constexpr float CoinSize = 8.0f;
	static constexpr float MoveSpeed = 90.0f;
	static constexpr float Gravity = -520.0f;
	static constexpr float JumpSpeed = 210.0f;

	PPU466 ppu;
};
