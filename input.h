#ifndef INPUT_H
#define INPUT_H

#include <map>
using namespace std;

#include <wx/utils.h>
#include <wx/defs.h>

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

const wxKeyCode keymap[8] = {
    (wxKeyCode)'z', //a
    (wxKeyCode)'x', //b
    (wxKeyCode)'s', //Select
    WXK_RETURN, //Start
    WXK_UP,
    WXK_DOWN,
    WXK_LEFT,
    WXK_RIGHT
};


class WxInputProvider : public InputProvider {
public:
    WxInputProvider();
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