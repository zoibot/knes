#include <iostream>

#include <SFML/Window.hpp>

#include "input.h"

SFMLInputProvider::SFMLInputProvider() {

}

bool SFMLInputProvider::pressed(int button, int controller) {
	return sf::Keyboard::IsKeyPressed(keymap[button]);
}

TestInputProvider::TestInputProvider() {
	for(int i = 0; i < 16; i++)
		buttons[i] = false;
}

bool TestInputProvider::pressed(int button, int controller) {
	return buttons[button + (8 * (controller-1))];
}

void TestInputProvider::set_button(int button, int controller, bool state) {
	buttons[button + (8 * (controller-1))] = state;
}