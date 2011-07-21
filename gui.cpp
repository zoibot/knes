#include <wx/wx.h>
#include <wx/event.h>
#include <wx/menu.h>


#include "gui.h"

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
    //File
    EVT_MENU(ID_Quit,  MainWindow::OnQuit)
    EVT_MENU(ID_About, MainWindow::OnAbout)
    EVT_MENU(ID_Open, MainWindow::OnOpen)
    //Debug
    EVT_MENU(ID_MemoryView, MainWindow::OnMemoryView)
END_EVENT_TABLE()

IMPLEMENT_APP(KNES)

bool KNES::OnInit()
{
    long flags = wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxSYSTEM_MENU |
                    wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN;
    MainWindow *frame = new MainWindow( _("asdfNES"), wxSize(256, 256), flags);
    frame->SetClientSize(256,240);
    frame->Show(true);
    SetTopWindow(frame);
    return true;
}

MainWindow::MainWindow(const wxString& title, const wxSize& size, long flags)
       : wxFrame(NULL, -1, title, wxDefaultPosition, size, flags)
{
    wxMenuBar *menuBar = new wxMenuBar;

    wxMenu *menuFile = new wxMenu(); 
    menuFile->Append(ID_Open, _("&Open..."));
    menuFile->Append(ID_About, _("&About..."));
    menuFile->AppendSeparator();
    menuFile->Append(ID_Quit, _("E&xit"));
    menuBar->Append(menuFile, _("&File"));

    wxMenu *menuDebug = new wxMenu();
    menuDebug->Append(ID_MemoryView, _("&MemoryView"));
    menuBar->Append(menuDebug, _("&Debug"));

    SetMenuBar( menuBar );

    mach = NULL;
    Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(MainWindow::OnIdle) );

    running = true;
    //CreateStatusBar();
    //SetStatusText( _("Welcome to wxWidgets!") );
}

void MainWindow::OnAbout(wxCommandEvent& event) {
        wxMessageBox( _("asdfNES NES Emulator (c)2011 Kevin Brackbill"),
                  _("asdfNES"), 
                  wxOK | wxICON_INFORMATION, this );
}

void MainWindow::OnQuit(wxCommandEvent& event) {
    running = false;
    delete mach;
    mach = NULL;
    Close(true);
}

void MainWindow::OnOpen(wxCommandEvent& event) {
    if(mach) {
        delete mach;
        mach = NULL;
    }
    wxString filename = wxFileSelector("Open ROM", "", "", "NES", "NES Roms (*.nes)|*.nes;*.NES",
                            wxFD_OPEN, this);
    if(!filename.empty()) {
        mach = load(filename.char_str());
        mach->set_input(new WxInputProvider());
    }
}

void MainWindow::OnMemoryView(wxCommandEvent& event) {
    wxFrame *memory_view = new wxFrame();
    memory_view->Show(true);
}

void MainWindow::OnIdle(wxIdleEvent& event) {
    double frame_rate = 1000.0f/double(frame_timer.Time());
    frame_timer.Start();
    wxClientDC dc(this);
    if(running && mach) {
        mach->run(1);
        //draw screen
        dc.DrawBitmap(mach->screenshot(), 0, 0);
        //display framerate
        wxString framerate;
        framerate << frame_rate;
        dc.SetTextBackground(wxTheColourDatabase->Find("black"));
        dc.SetTextForeground(wxTheColourDatabase->Find("green"));
        dc.DrawText(framerate, 0, 0);
    }
    event.RequestMore();
}
