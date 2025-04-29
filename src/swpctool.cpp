//
// Bent's Sword World PC Translation Tool  
// (c) 2025 @bent86 / @shram86 / @retrodevdiscord
// CC0 
//  Static dependency on wxWidgets
//  Library dependency on ICU
// 

#include "swpctool.h"


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

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(swpctool_Quit, MyFrame::OnQuit)
EVT_MENU(swpctool_About, MyFrame::OnAbout)
EVT_MENU(swpctool_Open, MyFrame::LoadDB)

EVT_BUTTON(swpctool_Prev, MyFrame::PrevWord)
EVT_BUTTON(swpctool_Next, MyFrame::NextWord)

EVT_TEXT(swpctool_ChangeTl, MyFrame::UpdateTlByteCount)

EVT_CHECKBOX(swpctool_SetBad, MyFrame::SetBad)
EVT_CHECKBOX(swpctool_SetComplete, MyFrame::SetComplete)

EVT_BUTTON(swpctool_Commit, MyFrame::CommitChanges)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(PickStringFrame, wxFrame)
EVT_MENU(swpctool_Goto, PickStringFrame::Confirm)
EVT_MENU(swpctool_CloseStrFrame, PickStringFrame::OnFrameClose)
wxEND_EVENT_TABLE()

// Create a new application object
wxIMPLEMENT_APP(MyApp);


void check_null(char c)
{
    if (c != 0)
        wxLogError("Database format error:\nIs the database corrupt?\nfound %d", (int)c);
}


//////////
PickStringFrame::PickStringFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{
    wxSize _sz = wxSize(200, 150);
    SetSize(_sz);

    pnlStrPanel = new wxPanel(this, wxID_ANY);
    lblStringLabel = new wxStaticText(pnlStrPanel, wxID_ANY, "Goto string:", wxPoint(60, 10));
    txtStringSel = new wxTextCtrl(pnlStrPanel, wxID_ANY, "", wxPoint(50, 40), wxSize(100, 20));
    btnStrOK = new wxButton(pnlStrPanel, wxID_ANY, "Go", wxPoint(60, 80));
    btnStrOK->Bind(wxEVT_BUTTON, &PickStringFrame::Confirm, this);
    this->Bind(wxEVT_MENU, &PickStringFrame::OnFrameClose, this);
}


void PickStringFrame::Confirm(wxCommandEvent& event)
{
    MyApp* app = &wxGetApp();
    MyFrame* myf = app->frmMainFrame;
    if (myf->datLoaded == false) {
        wxLogError("Load a translation file first!");
        return;
    }
    try { 
        myf->UpdateCurrentWord(std::stoi(std::string(app->frmStrFrame->txtStringSel->GetValue())) - 1);
        app->frmStrFrame = NULL;
        Close(true);
    } catch(...) { 
        wxLogError("Not a valid string");
    }
}


////////
void MyFrame::NextWord(wxCommandEvent& event)
{
    MyApp* app = &wxGetApp();
    if (datLoaded == true)
    {
        // save temporary changes to memory
        //wxLogError("%s", (app->txtTranslation->GetValue()));
        std::string tmp_u8(app->txtTranslation->GetValue().ToUTF8());//utf8ToSjis(tmp_u8str);
        //wxLogError("%s", wxString(tmp_u8.c_str()));
        std::string tmp_sjis(utf8ToSjis(tmp_u8));
        //wxLogError("%s", tmp_sjis);
        for (int i = 0; i < wordList[curWord].translation.size(); i++)
        {
            wordList[curWord].translation[i] = (u8)0;
        }
        wordList[curWord].translation.clear();
        for (int i = 0; i < strlen(tmp_sjis.c_str()); i++)
        {
            wordList[curWord].translation.push_back((u8)tmp_sjis[i]);
        }
        wordList[curWord].translation.push_back((u8)0);

        //std::string s2(wordList[curWord].translation.data());
        //wxLogError("%s", s2);

        curWord++;
        if (curWord == wordList.size())
            curWord = 0;
        while (wordList[curWord].complete == true) {
            curWord++;
            if (curWord == wordList.size())
                curWord = 0;
        }
        UpdateCurrentWord(curWord);
    }
    else wxLogError("Load a translation file first!");
}


void MyFrame::PrevWord(wxCommandEvent& event)
{
    MyApp* app = &wxGetApp();

    if (datLoaded == true) {
        // save temporary changes to memory 
        std::string tmp_u8(app->txtTranslation->GetValue().ToUTF8());//utf8ToSjis(tmp_u8str);
        std::string tmp_sjis(utf8ToSjis(tmp_u8));

        for (int i = 0; i < wordList[curWord].translation.size(); i++)
        {
            wordList[curWord].translation[i] = (u8)0;
        }
        wordList[curWord].translation.clear();
        for (int i = 0; i < strlen(tmp_sjis.c_str()); i++)
        {
            wordList[curWord].translation.push_back((u8)tmp_sjis[i]);
        }
        wordList[curWord].translation.push_back((u8)0); // to force terminate it 

        // change display to next word 
        curWord--;
        if (curWord == -1)
            curWord = wordList.size() - 1;
        while (wordList[curWord].complete == true) {
            curWord--;
            if (curWord == -1)
                curWord = wordList.size() - 1;
        }
        // skip it if needed 
        UpdateCurrentWord(curWord);
    }
    else wxLogError("Load a translation file first!");
}


void MyFrame::UpdateCurrentWord(int wordNum)
{
    MyApp* app = &wxGetApp();
    curWord = wordNum; // just in case. 
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
    // get converted size from text 
    std::string tmp_u8str(app->txtTranslation->GetValue().ToUTF8());
    std::string tmp_sjis = utf8ToSjis(tmp_u8str);
    app->lblTranslationSize->SetLabel("Size: " + std::to_string(tmp_sjis.length()));
    // < number of duplicates >
    app->lblDuplicates->SetLabel("Duplicates: " + std::to_string(wordList[wordNum].locs.size()-1));
    // bad flag, ...
    app->chkMarkBad->SetValue(wordList[wordNum].bad);
    app->chkMarkComplete->SetValue(wordList[wordNum].complete);
    //std::cout << "bad: " << std::to_string(wordList[wordNum].bad) << " compl: " << std::to_string(wordList[wordNum].complete) << "\n";
}



void MyFrame::LoadDB(wxCommandEvent& e)
{
    wxFileDialog openFileDialog(this, _("Open translation .DAT file"), "", "", "DAT files (*.dat)|*.dat",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    currentFilePath = std::string(openFileDialog.GetPath());
    std::ifstream is(currentFilePath, std::ios_base::binary);
    if (!is) {
        wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
        return;
    }
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    char* buffer = new char[length];
    is.read((char*)buffer, length);
    is.close();

    // check header 
    char hdr[5] = { 'T', 'L', 'D', 'A', 'T' };
    for (int i = 0; i < 5; i++) {
        if (buffer[i] != hdr[i]) {
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
    while (byte_it < length) {
        TlWord _word = TlWord();

        unsigned long _tempsize = 0 + ((u8)buffer[byte_it] << 8) + (u8)buffer[byte_it + 1];
        _word.bytesize = _tempsize;

        byte_it += 2;
        check_null((u8)buffer[byte_it++]);

        for (unsigned int _sz = 0; _sz < _word.bytesize; _sz += 1) {
            _word.translation.push_back((u8)buffer[byte_it++]);
        }
        _word.translation.push_back((u8)0); // test zero delim

        check_null((u8)buffer[byte_it++]);

        for (unsigned int _sz = 0; _sz < _word.bytesize; _sz += 1) {
            _word.text.push_back((u8)buffer[byte_it++]);
        }
        _word.text.push_back((u8)0); // test zero delim

        check_null((u8)buffer[byte_it++]);

        _word.bad = (bool)((u8)buffer[byte_it++]);

        check_null((u8)buffer[byte_it++]);

        _word.complete = (bool)((u8)buffer[byte_it++]);

        check_null((u8)buffer[byte_it++]);

        u8 loc_ct = (u8)buffer[byte_it++];
        check_null((u8)buffer[byte_it++]);
        for (u8 i = 0; i < loc_ct; i++) {
            Location _loc = Location();
            _loc.disk = (u8)buffer[byte_it];
            _loc.address = 0 + ((u8)(buffer[byte_it + 1]) << 16) + ((u8)(buffer[byte_it + 2]) << 8) + ((u8)(buffer[byte_it + 3]));
            _word.locs.push_back(_loc);
            byte_it += 4;
        }

        if ((u8)(buffer[byte_it]) == '\x0d') {
            if ((u8)buffer[byte_it + 1] == '\x0a') {
            }
            else {
                //std::cout << byte_it << std::endl;
                wxLogError("Database error!\nDatabase is corrupt.");
                return;
            }
        }
        else {
            //std::cout << byte_it << std::endl;
            wxLogError("Database error!\nDatabase is corrupt.");
            return;
        }

        wordList.push_back(_word);

        byte_it += 2;
    }

    MyApp* app = &wxGetApp();
    app->chkMarkBad->Enable();
    app->chkMarkComplete->Enable();

    // wordList is fully populated now.
    UpdateCurrentWord(0);

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
    wxMenu* fileMenu = new wxMenu;
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(swpctool_About, "&About\tF1", "Show about dialog");
    fileMenu->Append(swpctool_Quit, "E&xit\tAlt-X", "Quit this program");
    fileMenu->Append(swpctool_Open, "&Open\tAlt-O", "Open translation file");
    // now append to the menu bar...
    wxMenuBar* menuBar = new wxMenuBar();
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

    std::string tmp_u8str(app->txtTranslation->GetValue().ToUTF8());
    std::string tmp_sjis = utf8ToSjis(tmp_u8str);

    app->lblTranslationSize->SetLabel("Size: " + std::to_string(tmp_sjis.length()));

}


void PickStringFrame::OnFrameClose(wxCommandEvent& event)
{
    MyApp* app = &wxGetApp();
    app->frmStrFrame = NULL;

    Close(true);
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


void MyFrame::SetComplete(wxCommandEvent& WXUNUSED(e))
{
    MyApp* app = &wxGetApp();

    wordList[curWord].complete = app->chkMarkComplete->GetValue();
    //std::cout << std::to_string(wordList[curWord].complete).c_str();
}


void MyFrame::SetBad(wxCommandEvent& WXUNUSED(e))
{
    MyApp* app = &wxGetApp();

    wordList[curWord].bad = app->chkMarkBad->GetValue();
    //std::cout << std::to_string(wordList[curWord].bad).c_str();
}


void MyFrame::OpenStrSel(wxCommandEvent& WXUNUSED(e))
{
    MyApp* app = &wxGetApp();

    app->frmStrFrame = new PickStringFrame("Select String by ID");
    app->frmStrFrame->Show(true);
}

#define NULLBYTE (u8)0

void MyFrame::CommitChanges(wxCommandEvent& WXUNUSED(e))
{
    MyApp* app = &wxGetApp();

    // In case it has any changes: 
    std::string tmp_u8(app->txtTranslation->GetValue().ToUTF8());//utf8ToSjis(tmp_u8str);
    std::string tmp_sjis(utf8ToSjis(tmp_u8));
    for (int i = 0; i < wordList[curWord].translation.size(); i++)
    {
        wordList[curWord].translation[i] = (u8)0;
    }
    wordList[curWord].translation.clear();
    for (int i = 0; i < strlen(tmp_sjis.c_str()); i++)
    {
        wordList[curWord].translation.push_back((u8)tmp_sjis[i]);
    }
    wordList[curWord].translation.push_back((u8)0); // to force terminate it 

    // before you save, make sure 
    for (int f = 0; f < wordList.size(); f++) {
        std::string tmp_sjis(wordList[f].translation.data());

        if (wordList[f].bytesize != tmp_sjis.length()) {
            wxLogError("You can't!\nString #%d has an invalid number of characters.", f+1);
            return;
        }
    }

    // Recreate translation.dat file 
    //# TLDAT header
    //# bytect[2] \0 transl[bytect] \0 text[bytect] \0 bad[1] \0 complete[1] \0 locct[1] \0 locs[ [disk[1] addr[3]] ][locct] \0d \0a
    std::vector<char> outbytes;
    outbytes.push_back((u8)'T');
    outbytes.push_back((u8)'L');
    outbytes.push_back((u8)'D');
    outbytes.push_back((u8)'A');
    outbytes.push_back((u8)'T');
    for (int i = 0; i < wordList.size(); i++) {
        // bytect[2]
        outbytes.push_back((u8)((wordList[i].bytesize & 0xff00) >> 8));
        outbytes.push_back((u8)(wordList[i].bytesize & 0xff));
        outbytes.push_back(NULLBYTE);
        for (int sl = 0; sl < wordList[i].bytesize; sl++) {
            outbytes.push_back((u8)wordList[i].translation[sl]);
        }
        outbytes.push_back(NULLBYTE);
        for (int sl = 0; sl < wordList[i].bytesize; sl++) {
            outbytes.push_back((u8)wordList[i].text[sl]);
        }
        outbytes.push_back(NULLBYTE);
        outbytes.push_back((u8)wordList[i].bad);
        outbytes.push_back(NULLBYTE);
        outbytes.push_back((u8)wordList[i].complete);
        outbytes.push_back(NULLBYTE);
        outbytes.push_back((u8)wordList[i].locs.size());
        outbytes.push_back(NULLBYTE);
        for (int sl = 0; sl < wordList[i].locs.size(); sl++) {
            outbytes.push_back((u8)wordList[i].locs[sl].disk);
            outbytes.push_back((u8)((wordList[i].locs[sl].address & 0xff0000) >> 16));
            outbytes.push_back((u8)((wordList[i].locs[sl].address & 0xff00) >> 8));
            outbytes.push_back((u8)((wordList[i].locs[sl].address & 0xff)));
        }
        outbytes.push_back((u8)0x0d);
        outbytes.push_back((u8)0x0a);
    }

    std::ofstream os(currentFilePath, std::ios_base::binary);
    if (!os) { wxLogError("Cannot write to DB!\nIs it read only?"); return; }
    os.write(outbytes.data(), outbytes.size());
    os.close();

    // update %
    int tc = 0;
    for(int i = 0; i < wordList.size(); i++){
        if(wordList[i].complete==true) tc++;
    }
    float pct = (float)tc;
    float den = (float)wordList.size();
    pct = (pct / den) * 100.0f;
    int _p = (int)(pct);
    SetStatusText("Current progress: " + std::to_string(_p) + "%");

    wxLogError("Complete");
}


/////////////////

bool MyApp::OnInit()
{
    // call the base class initialization method
    if (!wxApp::OnInit())
        return false; // < exit immediately

    frmStrFrame = NULL;
    wxSetlocale(LC_ALL, "jp_JP");
    // create the main application window
    frmMainFrame = new MyFrame("Sword World PC Translation Tool");
    //frmMainFrame->myApp = this;
    wxSize _sz = wxSize(640, 440);
    frmMainFrame->SetSize(_sz);

    pnlMainPanel = new wxPanel(frmMainFrame, wxID_ANY);

    wxPoint ptMainLblPt = wxPoint(10, 10);
    lblMainLabel = new wxStaticText(pnlMainPanel, wxID_ANY, "Current string: 0 / 0", ptMainLblPt);
    lblDuplicates = new wxStaticText(pnlMainPanel, wxID_ANY, "Total # duplicates: 0", wxPoint(480, 20));

    wxStaticText* lblOrigTxtLabel = new wxStaticText(pnlMainPanel, wxID_ANY, "Original text:", wxPoint(10, 30));
    txtOriginalText = new wxTextCtrl(pnlMainPanel, wxID_ANY, "", wxPoint(10, 50), wxSize(600, 80), wxTE_READONLY | wxTE_MULTILINE | wxTE_WORDWRAP);
    //txtOriginalText->SetDefaultStyle(wxTextAttr())
    lblOriginalSizeLabel = new wxStaticText(pnlMainPanel, wxID_ANY, "Size: 0", wxPoint(550, 132));

    wxStaticText* lblTranslationLabel = new wxStaticText(pnlMainPanel, wxID_ANY, "Translation:", wxPoint(10, 140));
    txtTranslation = new wxTextCtrl(pnlMainPanel, wxID_ANY, "", wxPoint(10, 160), wxSize(600, 80), wxTE_MULTILINE | wxTE_WORDWRAP);
    lblTranslationSize = new wxStaticText(pnlMainPanel, wxID_ANY, "Size: 0", wxPoint(550, 240));

    chkMarkBad = new wxCheckBox(pnlMainPanel, wxID_ANY, "Flag bad string", wxPoint(10, 250));
    chkMarkBad->Disable();
    chkMarkComplete = new wxCheckBox(pnlMainPanel, wxID_ANY, "Mark complete (don't show again)", wxPoint(10, 270));
    chkMarkComplete->Disable();

    btnBack = new wxButton(pnlMainPanel, wxID_ANY, "<<", wxPoint(160, 300));
    btnNext = new wxButton(pnlMainPanel, wxID_ANY, ">>", wxPoint(440, 300));
    btnCommit = new wxButton(pnlMainPanel, wxID_ANY, ".       Commit       .\n", wxPoint(280, 275));
    btnSelectString = new wxButton(pnlMainPanel, wxID_ANY, "Goto string...", wxPoint(10, 320));

    //btnCommit->Bind(wxEVT_BUTTON, &MyFrame::OnQuit, frmMainFrame);
    btnBack->Bind(wxEVT_BUTTON, &MyFrame::PrevWord, frmMainFrame);
    btnNext->Bind(wxEVT_BUTTON, &MyFrame::NextWord, frmMainFrame);
    btnSelectString->Bind(wxEVT_BUTTON, &MyFrame::OpenStrSel, frmMainFrame);
    txtTranslation->Bind(wxEVT_TEXT, &MyFrame::UpdateTlByteCount, frmMainFrame);

    chkMarkBad->Bind(wxEVT_CHECKBOX, &MyFrame::SetBad, frmMainFrame);
    chkMarkComplete->Bind(wxEVT_CHECKBOX, &MyFrame::SetComplete, frmMainFrame);

    frmMainFrame->Show(true);  // and show it 

    btnCommit->Bind(wxEVT_BUTTON, &MyFrame::CommitChanges, frmMainFrame);
    //frmStrFrame = new PickStringFrame("Select String By ID");

    //frmStrFrame->Show();

    // success: wxApp::OnRun() will be called which will enter the main loop
    return true;
}


int MyApp::FilterEvent(wxEvent& event) {
    if (event.GetEventType() == wxEVT_KEY_DOWN) {
        wxKeyEvent& keyEvent = static_cast<wxKeyEvent&>(event);
        //wxLogMessage("Key pressed: %d", keyEvent.GetKeyCode());
        if(keyEvent.GetKeyCode() == 308){
            shiftKey = true;
        }
        if(keyEvent.GetKeyCode() == 314){
            if(shiftKey)
                frmMainFrame->PrevWord(static_cast<wxCommandEvent&>(event));
        }
        else if(keyEvent.GetKeyCode() == 316){
            if(shiftKey)
                frmMainFrame->NextWord(static_cast<wxCommandEvent&>(event));
        }
        else if(keyEvent.GetKeyCode() == 66){//b = 66
            if(shiftKey)
                if(frmMainFrame->datLoaded){
                    chkMarkBad->SetValue(!chkMarkBad->GetValue());
                    frmMainFrame->SetBad(static_cast<wxCommandEvent&>(event));
                }
        }
        else if(keyEvent.GetKeyCode() == 67){//c
            if(shiftKey)
                if(frmMainFrame->datLoaded){
                    chkMarkComplete->SetValue(!chkMarkComplete->GetValue());
                    frmMainFrame->SetComplete(static_cast<wxCommandEvent&>(event));
                }
        }
        else if(keyEvent.GetKeyCode() == 13){//enter
            if(shiftKey)
                if(frmMainFrame->datLoaded)
                    frmMainFrame->CommitChanges(static_cast<wxCommandEvent&>(event));
        }
        else if (keyEvent.GetKeyCode() == 71){//g = 71
            if(shiftKey)
                if(frmMainFrame->datLoaded)
                    frmMainFrame->OpenStrSel(static_cast<wxCommandEvent&>(event));
        }
        
        //else if(keyEvent.GetKeyCode() == )
    }
    if (event.GetEventType() == wxEVT_KEY_UP) {
        wxKeyEvent& keyEvent = static_cast<wxKeyEvent&>(event);
        //wxLogMessage("Key pressed: %d", keyEvent.GetKeyCode());
        if(keyEvent.GetKeyCode() == 308){
            shiftKey = false;
        }
    }
    return wxApp::FilterEvent(event);
}
