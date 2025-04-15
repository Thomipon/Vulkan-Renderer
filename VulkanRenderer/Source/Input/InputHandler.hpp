#pragma once
#include <cstdint>
#include <functional>
#include <vector>

enum class InputType : uint8_t
{
	none = 0,
	keyPress,
	mouseMove
};

struct InputEvent
{	InputEvent(InputType type, int key, int scancode, int action, int mods, double mouseX, double mouseY)
		: type(type), key(key),
		  scancode(scancode),
		  action(action),
		  mods(mods), mouseX(mouseX), mouseY(mouseY)
	{
	}

	InputType type;
	int key;
	int scancode;
	int action;
	int mods;
	double mouseX;
	double mouseY;
};

using KeyCallback = std::function<void(const InputEvent&)>;

class InputHandler
{
public:
	void registerKeyCallback(const KeyCallback& keyCallback);

private:
	void glfwOnKeyPressed(int key, int scancode, int action, int mods);
	void glfwOnMouseMove(double xpos, double ypos);

	std::vector<KeyCallback> keyCallbacks; // TODO: We need a proper event system here

	friend class Window;
};
