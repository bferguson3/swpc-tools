//
// Bent's Sword World PC Translation Tool  
// (c) 2025 @bent86 / @shram86 / @retrodevdiscord
// CC0 
//  Static dependency on wxWidgets
//  Library dependency on ICU
// 

#include "wx/wxprec.h" // includes "wx/wx.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
// resources
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "sample.xpm"
#endif
#include <wx/wfstream.h>
#include <iostream>
#include <fstream>
#include <cstddef>
#include "convtable.h"
#include <unicode/unistr.h>


#define u8 unsigned char

class Location
{
    public:
        Location();
        Location(unsigned long address, int disk);

        unsigned long address;
        int disk;

    private:
};

Location::Location()
{
    address = 0LU;
    disk = 0;
}

Location::Location(unsigned long _address, int _disk)
{
    address = _address;
    disk = _disk;
}

class TlWord
{
    public:
        TlWord();
        TlWord(std::vector<char> text);

        std::vector<Location> locs;
        unsigned int count;
        unsigned long bytesize;
        
        std::vector<char> text;
        std::vector<char> translation;

        bool bad = false;
        bool complete = false;
    
    private:
};

TlWord::TlWord()
{
    count = 1u;
    bytesize = 0lu;
  
}

TlWord::TlWord(std::vector<char> _text)
{
    text = _text;
    count = 1u;
    bytesize = strlen(_text.data());
}

//////
// taken from @deviantfan@stackoverflow
// requires convtable.h created by doing:
//  $ cat SHIFTJIS.TXT | ./jistool >> convtable.bin ; python3 ./convtable.py >> ./src/convtable.h
/*
    std::string sj2utf8(const std::string &input)
    {
        std::string output(3 * input.length(), ' '); //ShiftJis won't give 4byte UTF8, so max. 3 byte per input char are needed
        size_t indexInput = 0, indexOutput = 0;

        while(indexInput < input.length())
        {
            char arraySection = ((uint8_t)input[indexInput]) >> 4;

            size_t arrayOffset;
            if(arraySection == 0x8) arrayOffset = 0x100; //these are two-byte shiftjis
            else if(arraySection == 0x9) arrayOffset = 0x1100;
            else if(arraySection == 0xE) arrayOffset = 0x2100;
            else arrayOffset = 0; //this is one byte shiftjis

            //determining real array offset
            if(arrayOffset)
            {
                arrayOffset += (((uint8_t)input[indexInput]) & 0xf) << 8;
                indexInput++;
                if(indexInput >= input.length()) break;
            }
            arrayOffset += (uint8_t)input[indexInput++];
            arrayOffset <<= 1;

            //unicode number is...
            uint16_t unicodeValue = (convTable[arrayOffset] << 8) | convTable[arrayOffset + 1];

            //converting to UTF8
            if(unicodeValue < 0x80)
            {
                output[indexOutput++] = unicodeValue;
            }
            else if(unicodeValue < 0x800)
            {
                output[indexOutput++] = 0xC0 | (unicodeValue >> 6);
                output[indexOutput++] = 0x80 | (unicodeValue & 0x3f);
            }
            else
            {
                output[indexOutput++] = 0xE0 | (unicodeValue >> 12);
                output[indexOutput++] = 0x80 | ((unicodeValue & 0xfff) >> 6);
                output[indexOutput++] = 0x80 | (unicodeValue & 0x3f);
            }
        }

        output.resize(indexOutput); //remove the unnecessary bytes
        return output;
    }
*/

/// Taken from Kilfu0701@github
std::string utf8ToSjis(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "utf8");
    int length = src.extract(0, src.length(), NULL, "shift_jis");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "shift_jis");

    return std::string(result.begin(), result.end() - 1);
}

std::string sjisToUtf8(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "shift_jis");
    int length = src.extract(0, src.length(), NULL, "utf8");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "utf8");

    return std::string(result.begin(), result.end() - 1);
}
///


// main frame
class MyFrame : public wxFrame
{
    public:
        MyFrame(const wxString& title); // ctor(s)

        void OnQuit(wxCommandEvent& event);     // event handlers 
        void OnAbout(wxCommandEvent& event);    // (these functions should _not_ be virtual)

        void LoadDB(wxCommandEvent& event);
        void UpdateCurrentWord(int wordNum);

        void NextWord(wxCommandEvent& event);
        void PrevWord(wxCommandEvent& event);
        void UpdateTlByteCount(wxCommandEvent& e);

        std::vector<char> translation_dat;
        std::vector<TlWord> wordList; 
        int curWord;

        bool datLoaded;

    private:
        // any class wishing to process wxWidgets events must use this macro!
        wxDECLARE_EVENT_TABLE();
};

// define app (must be of type wxApp)
class MyApp : public wxApp
{
    public:
        // return: if OnInit() returns false, the application terminates)
        virtual bool OnInit() wxOVERRIDE;

        MyFrame* frmMainFrame;
        wxPanel* pnlMainPanel;
        
        wxStaticText* lblMainLabel;
        wxStaticText* lblDuplicates;
        wxStaticText* lblOriginalSizeLabel;
        wxStaticText* lblTranslationSize;
        
        wxTextCtrl* txtOriginalText;
        wxTextCtrl* txtTranslation;
        
        wxCheckBox* chkMarkBad; 
        wxCheckBox* chkMarkComplete;
        
        wxButton* btnBack;
        wxButton* btnNext;
        wxButton* btnCommit;
        wxButton* btnSelectString;


};

// IDs for the controls and the menu commands
enum
{
    swpctool_Quit = wxID_EXIT,
    swpctool_About = wxID_ABOUT, // this macro must be used on osx so it is put in the right place
    swpctool_Open = wxID_ANY,
    swpctool_Prev = wxID_ANY,
    swpctool_Next = wxID_ANY,
    swpctool_ChangeTl = wxID_ANY
};

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(swpctool_Quit, MyFrame::OnQuit)
    EVT_MENU(swpctool_About, MyFrame::OnAbout)
    EVT_MENU(swpctool_Open, MyFrame::LoadDB)
    
    EVT_MENU(swpctool_Prev, MyFrame::PrevWord)
    EVT_MENU(swpctool_Next, MyFrame::NextWord)
    
    EVT_TEXT(swpctool_ChangeTl, MyFrame::UpdateTlByteCount)
wxEND_EVENT_TABLE()

// Create a new application object
wxIMPLEMENT_APP(MyApp);

void check_null(char c)
{
    if(c != 0) 
        wxLogError("Database format error:\nIs the database corrupt?\nfound %d", (int)c);
}

void MyFrame::NextWord(wxCommandEvent& event)
{
    if (datLoaded == true)
    {
        curWord++;
        if (curWord == wordList.size()) curWord = 0;

        UpdateCurrentWord(curWord);
    }
    else wxLogError("Load a translation file first!");
}

void MyFrame::PrevWord(wxCommandEvent& event)
{
    if (datLoaded == true) {
        curWord--;
        if (curWord == -1) curWord = wordList.size() - 1;

        UpdateCurrentWord(curWord);
    }
    else wxLogError("Load a translation file first!");
}

void MyFrame::UpdateCurrentWord(int wordNum)
{
    MyApp* app = &wxGetApp();

    // set string count label, 
    std::string s = "Current string: " + std::to_string(wordNum + 1) + " / " + std::to_string(wordList.size());
    app->lblMainLabel->SetLabel(s);

    // decode and set tl text 
    // original text box and bytesz label,
    s = sjisToUtf8(std::string(wordList[wordNum].text.data()));
    app->txtOriginalText->ChangeValue(wxString::FromUTF8(s.c_str()));
    app->lblOriginalSizeLabel->SetLabel("Size: " + std::to_string(wordList[wordNum].bytesize));

    // tled text box and bytesz label,
    s = sjisToUtf8(std::string(wordList[wordNum].translation.data()));
    app->txtTranslation->ChangeValue(wxString::FromUTF8(s.c_str()));
    app->lblTranslationSize->SetLabel("Size: " + std::to_string(wordList[wordNum].bytesize));
    // < number of duplicates >

    // bad flag, ...
    app->chkMarkBad->SetValue(wordList[wordNum].bad);

    
}

void MyFrame::LoadDB(wxCommandEvent& e)
{
    wxFileDialog openFileDialog(this, _("Open translation .DAT file"), "", "", "DAT files (*.dat)|*.dat", 
        wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;     

    std::string path(openFileDialog.GetPath());
    std::ifstream is (path, std::ios_base::binary);
    if (!is) {
        wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
        return;
    }
    is.seekg (0, is.end);
    int length = is.tellg();
    is.seekg (0, is.beg);

    char * buffer = new char[length];
    is.read ((char*)buffer, length);
    is.close();

    // check header 
    char hdr[5] = { 'T', 'L', 'D', 'A', 'T' };
    for(int i = 0; i < 5; i++){
        if(buffer[i] != hdr[i]){
            wxLogError("Database error!\nDatabase is corrupt.");
            return;
        }
    }

    // clean 
    wordList.clear();
    // parse database 
    // format: 
    //  size[2] \0 trans[size] \0 orig[size] \0 bad[1] \0 locct[1] \0 locations[D[1],addr[3]][locct] \0d \0a
    unsigned long byte_it = 5;
    while(byte_it < length){
        TlWord _word = TlWord();

        unsigned long _tempsize = 0 + ((u8)buffer[byte_it] << 8) + (u8)buffer[byte_it+1];
        _word.bytesize = _tempsize;
        
        byte_it += 2;
        check_null((u8)buffer[byte_it++]);

        for(unsigned int _sz = 0; _sz < _word.bytesize; _sz += 1){
            _word.text.push_back((u8)buffer[byte_it++]);
        }
        _word.text.push_back((u8)0); // test zero delim

        check_null((u8)buffer[byte_it++]);

        for(unsigned int _sz = 0; _sz < _word.bytesize; _sz += 1){
            _word.translation.push_back((u8)buffer[byte_it++]);
        }
        _word.translation.push_back((u8)0); // test zero delim

        check_null((u8)buffer[byte_it++]);

        _word.bad = (bool)((u8)buffer[byte_it++]);

        check_null((u8)buffer[byte_it++]);

        _word.complete = (bool)((u8)buffer[byte_it++]);

        check_null((u8)buffer[byte_it++]);

        u8 loc_ct = (u8)buffer[byte_it++];
        check_null((u8)buffer[byte_it++]);
        for(u8 i = 0; i < loc_ct; i++){
            Location _loc = Location();
            _loc.disk = (u8)buffer[byte_it];
            _loc.address = 0 + ((u8)(buffer[byte_it+1]) << 16) + ((u8)(buffer[byte_it+2]) << 8) + ((u8)(buffer[byte_it+3]));
            _word.locs.push_back(_loc);
            byte_it += 4;
        }
        
        if((u8)(buffer[byte_it]) == '\x0d'){
            if((u8)buffer[byte_it+1] == '\x0a'){
            } else{ 
                std::cout << byte_it << std::endl;
                wxLogError("Database error!\nDatabase is corrupt.");
                return;
            }
        } else{ 
            std::cout << byte_it << std::endl;
            wxLogError("Database error!\nDatabase is corrupt.");
            return;
        }
        
        wordList.push_back(_word);

        byte_it += 2;
    }

    // wordList is fully populated now.
    this->UpdateCurrentWord(0);

    wxMessageDialog* msg = new wxMessageDialog(this, "Database loaded OK!");
    datLoaded = true;
    msg->ShowModal();
    
}

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{
    SetIcon(wxICON(sample));

    curWord = 0;
    datLoaded = false;

    #if wxUSE_MENUBAR
        wxMenu *fileMenu = new wxMenu;
        wxMenu *helpMenu = new wxMenu;
        helpMenu->Append(swpctool_About, "&About\tF1", "Show about dialog");
        fileMenu->Append(swpctool_Quit, "E&xit\tAlt-X", "Quit this program");
        fileMenu->Append(swpctool_Open, "&Open\tAlt-O", "Open translation file");
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

void MyFrame::UpdateTlByteCount(wxCommandEvent& e)
{
    MyApp* app = &wxGetApp();

    std::string tmp_u8str = app->txtTranslation->GetValue().ToUTF8();
    std::string tmp_sjis = utf8ToSjis(tmp_u8str);

    app->lblTranslationSize->SetLabel("Size: " + std::to_string(tmp_sjis.length()));
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
/////////////////


bool MyApp::OnInit()
{
    // call the base class initialization method
    if ( !wxApp::OnInit() )
        return false; // < exit immediately

    wxLocale* loc = new wxLocale(wxLANGUAGE_JAPANESE);
    loc->Init(wxLANGUAGE_JAPANESE, wxLOCALE_LOAD_DEFAULT);
    wxSetlocale(LC_ALL, "jp_JP");
    // create the main application window
    frmMainFrame = new MyFrame("Sword World PC Translation Tool");
    //frmMainFrame->myApp = this;
    wxSize _sz = wxSize(640,440);
    frmMainFrame->SetSize(_sz);

    this->pnlMainPanel = new wxPanel(frmMainFrame, wxID_ANY);

    wxPoint ptMainLblPt = wxPoint(10, 10);
    this->lblMainLabel = new wxStaticText(this->pnlMainPanel, wxID_ANY, "Current string: 0 / 0", ptMainLblPt);
    this->lblDuplicates = new wxStaticText(this->pnlMainPanel, wxID_ANY, "Total # duplicates: 0", wxPoint(480, 20));

    wxStaticText* lblOrigTxtLabel = new wxStaticText(this->pnlMainPanel, wxID_ANY, "Original text:", wxPoint(10, 30));
    this->txtOriginalText = new wxTextCtrl(this->pnlMainPanel, wxID_ANY, "", wxPoint(10, 50), wxSize(600, 80), wxTE_READONLY | wxTE_MULTILINE | wxTE_CHARWRAP);
    //this->txtOriginalText->SetDefaultStyle(wxTextAttr())
    this->lblOriginalSizeLabel = new wxStaticText(this->pnlMainPanel, wxID_ANY, "Size: 0", wxPoint(550, 132));
    
    wxStaticText* lblTranslationLabel = new wxStaticText(this->pnlMainPanel, wxID_ANY, "Translation:", wxPoint(10, 140));
    this->txtTranslation = new wxTextCtrl(this->pnlMainPanel, wxID_ANY, "", wxPoint(10, 160), wxSize(600, 80), wxTE_MULTILINE | wxTE_CHARWRAP);
    this->lblTranslationSize = new wxStaticText(this->pnlMainPanel, wxID_ANY, "Size: 0", wxPoint(550, 240));
    
    this->chkMarkBad = new wxCheckBox(this->pnlMainPanel, wxID_ANY, "Flag bad string", wxPoint(10, 250));
    this->chkMarkComplete = new wxCheckBox(this->pnlMainPanel, wxID_ANY, "Mark complete (don't show again)", wxPoint(10, 270));

    this->btnBack = new wxButton(this->pnlMainPanel, wxID_ANY, "<<", wxPoint(160, 300));
    this->btnNext = new wxButton(this->pnlMainPanel, wxID_ANY, ">>", wxPoint(440, 300));
    this->btnCommit = new wxButton(this->pnlMainPanel, wxID_ANY, ".     Commit (n)     .\n", wxPoint(280, 275));
    this->btnSelectString = new wxButton(this->pnlMainPanel, wxID_ANY, "Goto string...", wxPoint(10, 320));

    //this->btnCommit->Bind(wxEVT_BUTTON, &MyFrame::OnQuit, this->frmMainFrame);
    this->btnBack->Bind(wxEVT_BUTTON, &MyFrame::PrevWord, this->frmMainFrame);
    this->btnNext->Bind(wxEVT_BUTTON, &MyFrame::NextWord, this->frmMainFrame);
    this->txtTranslation->Bind(wxEVT_TEXT, &MyFrame::UpdateTlByteCount, this->frmMainFrame);
    frmMainFrame->Show(true);  // and show it 

    // success: wxApp::OnRun() will be called which will enter the main loop
    return true;
}

