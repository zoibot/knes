#include <wx/wx.h>
#include <wx/event.h>
#include <wx/menu.h>

#include "gui.h"

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(ID_Quit,  MainWindow::OnQuit)
    EVT_MENU(ID_About, MainWindow::OnAbout)
END_EVENT_TABLE()

IMPLEMENT_APP(KNES)

bool KNES::OnInit()
{
    MainWindow *frame = new MainWindow( _("Hello World"), wxPoint(50, 50), wxSize(256, 256) );
    frame->Show(true);
    SetTopWindow(frame);
    return true;
}

MainWindow::MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame(NULL, -1, title, pos, size)
{
    wxMenu *menuFile = new wxMenu;

    menuFile->Append( ID_About, _("&About...") );
    menuFile->AppendSeparator();
    menuFile->Append( ID_Quit, _("E&xit") );

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, _("&File") );

    SetMenuBar( menuBar );

    mach = load("c:\\battletoads.nes");
    mach->set_input(new WxInputProvider());
    Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(MainWindow::OnIdle) );
    running = true;

    //CreateStatusBar();
    //SetStatusText( _("Welcome to wxWidgets!") );
}

void MainWindow::OnAbout(wxCommandEvent& event) {
        wxMessageBox( _("This is a wxWidgets Hello world sample"),
                  _("About Hello World"), 
                  wxOK | wxICON_INFORMATION, this );
}

void MainWindow::OnQuit(wxCommandEvent& event) {
    Close(true);
}

void MainWindow::OnIdle(wxIdleEvent& event) {
    if(running) {
        mach->run(1);
    }
    //read
    wxClientDC dc(this);
    dc.DrawBitmap(mach->screenshot(), 0, 0);
    event.RequestMore();
}