#ifndef GUI_H
#define GUI_H

#include <wx/wx.h>

#include "machine.h"

class KNES : public wxApp {
public:
    virtual bool OnInit();
};

class MainWindow : public wxFrame {
    Machine *mach;
    bool running;
public:

    MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size);

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnIdle(wxIdleEvent& event);

    DECLARE_EVENT_TABLE()
};

    enum {
    ID_Quit = 1,
    ID_About,
    };

#endif //GUI_H