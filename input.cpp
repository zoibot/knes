#include <iostream>

#include <SFML/Window.hpp>

#include "input.h"

WxInputProvider::WxInputProvider() {

}

bool WxInputProvider::pressed(int button, int controller) {
	return wxGetKeyState(keymap[button]);
}

TestInputProvider::TestInputProvider() {
	for(int i = 0; i < 16; i++)
		buttons[i] = false;
}

bool TestInputProvider::pressed(int button, int controller) {
	return buttons[button];
	// + (8 * (controller-1))
}

void TestInputProvider::set_button(int button, int controller, bool state) {
	buttons[button] = state;
	// + (8 * (controller-1))
}