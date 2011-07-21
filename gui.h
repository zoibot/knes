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
	wxStopWatch frame_timer;
    bool running;
public:

    MainWindow(const wxString& title, const wxSize& size, long flags);

    //File
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    //Debug
    void OnMemoryView(wxCommandEvent& event);
    void OnTest(wxCommandEvent& event);
    //Game Loop
    void OnIdle(wxIdleEvent& event);

    DECLARE_EVENT_TABLE()
};

    enum {
    //File
    ID_Quit = 1,
    ID_About,
    ID_Open,
    //Debug
    ID_MemoryView,
    };

#endif //GUI_H
