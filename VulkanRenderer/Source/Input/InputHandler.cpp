//
// Created by Thomas on 15/04/2025.
//

#include "InputHandler.hpp"

void InputHandler::registerKeyCallback(const KeyCallback& keyCallback)
{
	keyCallbacks.push_back(keyCallback);
}

void InputHandler::glfwOnKeyPressed(int key, int scancode, int action, int mods)
{
	const InputEvent event{InputType::keyPress, key, scancode, action, mods, 0, 0};
	for (auto& keyCallback : keyCallbacks)
	{
		keyCallback(event);
	}
}

void InputHandler::glfwOnMouseMove(double xpos, double ypos)
{
	const InputEvent event{InputType::mouseMove, 0, 0, 0, 0, xpos, ypos};
	for (auto& keyCallback : keyCallbacks)
	{
		keyCallback(event);
	}
}
