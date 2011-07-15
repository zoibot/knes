#ifndef INPUT_H
#define INPUT_H

#include <map>
using namespace std;

#include <SFML/Window.hpp>

class Machine;

enum buttons {
	bA,
	bB,
	bS,
	bT,
	bU,
	bD,
	bL,
	bR
};

class InputProvider {
public:
	virtual bool pressed(int button, int controller) = 0;
};

const sf::Keyboard::Key keymap[8] = {
	sf::Keyboard::Z, //a
    sf::Keyboard::X, //b
    sf::Keyboard::S, //Select
    sf::Keyboard::Return, //Start
    sf::Keyboard::Up,
    sf::Keyboard::Down,
	sf::Keyboard::Left,
    sf::Keyboard::Right
};


class SFMLInputProvider : public InputProvider {
public:
	SFMLInputProvider();
	bool pressed(int button, int controller);
};

class TestInputProvider : public InputProvider {
	bool buttons[16];
public:
	TestInputProvider();
	bool pressed(int button, int controller);
	void set_button(int button, int controller, bool state);
};

#endif //INPUT_H