//
// Bent's Sword World PC Translation Tool  
// (c) 2025 @bent86 / @shram86 / @retrodevdiscord
// CC0 
//  depends only on wxWidgets
// 

#include "wx/wxprec.h" // includes "wx/wx.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
// resources
// the application icon (under Windows it is in resources and even
// though we could still include the XPM here it would be unused)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "sample.xpm"
#endif

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
    public:
        // return: if OnInit() returns false, the application terminates)
        virtual bool OnInit() wxOVERRIDE;

        wxPanel* pnlMainPanel;
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
    public:
        // ctor(s)
        MyFrame(const wxString& title);

        void OnQuit(wxCommandEvent& event);     // event handlers 
        void OnAbout(wxCommandEvent& event); // (these functions should _not_ be virtual)

    private:
        // any class wishing to process wxWidgets events must use this macro!
        wxDECLARE_EVENT_TABLE();
};

// IDs for the controls and the menu commands
enum
{
    swpctool_Quit = wxID_EXIT,
    swpctool_About = wxID_ABOUT // this macro must be used on osx so it is put in the right place
};

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(swpctool_Quit,  MyFrame::OnQuit)
    EVT_MENU(swpctool_About, MyFrame::OnAbout)
wxEND_EVENT_TABLE()

// Create a new application object
wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    // call the base class initialization method
    if ( !wxApp::OnInit() )
        return false; // < exit immediately

    // create the main application window
    MyFrame *frame = new MyFrame("Sword World PC Translation Tool");
    wxSize _sz = wxSize(640,400);
    frame->SetSize(_sz);

    this->pnlMainPanel = new wxPanel(frame, wxID_ANY);

    wxSize szMainLblSz = wxSize(100, 50);
    wxPoint ptMainLblPt = wxPoint(10, 10);
    wxStaticText* lblMainLabel = new wxStaticText(this->pnlMainPanel, wxID_ANY, "String #: 0001 / 9999", ptMainLblPt); //, wxDefaultPosition, szMainLblSz);
    lblMainLabel->SetFont(frame->GetFont().Scale(1));

    frame->Show(true);  // and show it 
    // success: wxApp::OnRun() will be called which will enter the main loop
    return true;
}

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{
    SetIcon(wxICON(sample));

    #if wxUSE_MENUBAR
        wxMenu *fileMenu = new wxMenu;
        wxMenu *helpMenu = new wxMenu;
        helpMenu->Append(swpctool_About, "&About\tF1", "Show about dialog");
        fileMenu->Append(swpctool_Quit, "E&xit\tAlt-X", "Quit this program");
        // now append to the menu bar...
        wxMenuBar *menuBar = new wxMenuBar();
        menuBar->Append(fileMenu, "&File");
        menuBar->Append(helpMenu, "&Help");
        // ... and attach this menu bar to the frame
        SetMenuBar(menuBar);
    #else // !wxUSE_MENUBAR
        // If menus are not available add a button to access the about box
        // code here taken from minimal project: 
        wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* aboutBtn = new wxButton(this, wxID_ANY, "About...");
        aboutBtn->Bind(wxEVT_BUTTON, &MyFrame::OnAbout, this);
        sizer->Add(aboutBtn, wxSizerFlags().Center());
        SetSizer(sizer);
    #endif // wxUSE_MENUBAR/!wxUSE_MENUBAR
    #if wxUSE_STATUSBAR
        CreateStatusBar(2); // create a status bar just for fun 
        SetStatusText("Current progress: 0%");
    #endif // wxUSE_STATUSBAR
}

// event handlers
void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true); // true is to force the frame to close esp on macos
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format("Welcome to the Sword World PC tool!\n\nThis is the swpctool wxWidgets sample\nrunning under" + wxGetOsDescription()),
        "About Sword World PC Tool", wxOK | wxICON_INFORMATION, this);
}
