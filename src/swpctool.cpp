// Bent's Sword World PC Translation Tool  
// (c) 2025 @bent86 / @shram86 / @retrodevdiscord
// CC0
//  Static dependency on wxWidgets
//  Library dependency on ICU
//

#include "swpctool.h"
#include <string>
#include <sstream>
#include <iomanip>

/// Convert integer value `val` to text in hexadecimal format.
/// The minimum width is padded with leading zeros; if not
/// specified, this `width` is derived from the type of the
/// argument. Function suitable from char to long long.
/// Pointers, floating point values, etc. are not supported;
/// passing them will result in an (intentional!) compiler error.
/// Basics from: http://stackoverflow.com/a/5100745/2932052

template <typename T>
inline std::string int_to_hex(T val, size_t width = sizeof(T) * 2)
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(width) << std::hex << (val | 0);
    return ss.str();
}


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


SearchFrame::SearchFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{
    wxSize _sz = wxSize(300, 200);
    SetSize(_sz);

    pnlSearch = new wxPanel(this, wxID_ANY);
    lblSearch = new wxStaticText(pnlSearch, wxID_ANY, "String to search:", wxPoint(80, 10));
    txtSearchStr = new wxTextCtrl(pnlSearch, wxID_ANY, "", wxPoint(10, 40), wxSize(250, 60));
    btnOK = new wxButton(pnlSearch, wxID_ANY, "Find", wxPoint(90, 120));
    btnOK->Bind(wxEVT_BUTTON, &SearchFrame::Confirm, this);
    this->Bind(wxEVT_MENU, &SearchFrame::OnFrameClose, this);
}

//////

std::string ReplaceString(std::string subject, const std::string& search,
    const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

std::string ReplaceString(std::string subject, const char search,
    const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, 1, replace);
        pos += replace.length();
    }
    return subject;
}

std::string addNewlines(const std::string& text, int x) {
    std::string result = "";
    for (size_t i = 0; i < text.length(); ++i) {
        result += text[i];
        if ((i + 1) % x == 0) {
            result += '\n';
        }
    }
    return result;
}

std::vector<std::string> split(std::string s, std::string delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, (int)(pos + delimiter.length()));
    }
    tokens.push_back(s);

    return tokens;
}


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
//EVT_MENU(swpctool_OpenGoto, MyFrame::OpenStrSel)
//EVT_MENU(swpctool_OpenSearch, MyFrame::OpenSearch)

EVT_BUTTON(swpctool_Prev, MyFrame::PrevWord)
EVT_BUTTON(swpctool_Next, MyFrame::NextWord)

EVT_TEXT(swpctool_ChangeTl, MyFrame::UpdateTlByteCount)

EVT_CHECKBOX(swpctool_SetBad, MyFrame::SetBad)
EVT_CHECKBOX(swpctool_SetComplete, MyFrame::SetComplete)
EVT_CHECKBOX(swpctool_SetInsert, MyFrame::SetInsert)

EVT_BUTTON(swpctool_Commit, MyFrame::CommitChanges)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(PickStringFrame, wxFrame)
EVT_MENU(swpctool_Goto, PickStringFrame::Confirm)
EVT_MENU(swpctool_CloseStrFrame, PickStringFrame::OnFrameClose)
//EVT_MENU(swpctool_OpenGoto, PickStringFrame::OnMenuOpen)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(SearchFrame, wxFrame)
EVT_MENU(swpctool_Search, SearchFrame::Confirm)
EVT_MENU(swpctool_CloseSearch, SearchFrame::OnFrameClose)
//EVT_MENU(swpctool_OpenSearch, SearchFrame::OnMenuOpen)
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
        int g = std::stoi(std::string(app->frmStrFrame->txtStringSel->GetValue())) - 1;
        if (g < 0) { wxLogError("Use a positive number, man"); return; }
        if (app->strFiles[g].encoding == 1) {
            wxLogError("[#%d] %s is an encoded file; pick another!", g+1, app->strFiles[g].fname);
            return;

        }
        if (app->strFiles[g].length == 1) {
            wxLogError("[#%d] %s is an empty file, pick another!", g+1, app->strFiles[g].fname);
            return;
        }

        myf->UpdateCurrentWord(g);
        app->frmStrFrame = NULL;
        Close(true);
    }
    catch (...) {
        wxLogError("Not a valid string");
    }
}


void SearchFrame::Confirm(wxCommandEvent& e)
{
    MyApp* app = &wxGetApp();

    if (app->appMode != MODE_ARF) {
        wxLogError("Searching only supported in ARF files.");
        return;
    }

    MyFrame* myf = app->frmMainFrame;
    if (myf->datLoaded == false) {
        wxLogError("Load a translation file first!");
        return;
    }
    try {
        // convert captured text to sjis
        std::string _c = utf8ToSjis(std::string(txtSearchStr->GetValue().ToUTF8()));
        std::vector<u8> newby;
        // convert to bytes
        // remove any newlines, replace [nn] codes
        for (int s = 0; s < _c.length(); s++) {
            if ((u8)_c[s] == (u8)10) {
                newby.push_back((u8)0);
            }
            else if ((u8)_c[s] == (u8)0x5b) {
                if ((u8)_c[s + 2] == (u8)0x5d) {
                    char buf;// [2] = { 0, 0 };
                    char* p;
                    buf = (u8)_c[s + 1];
                    newby.push_back((u8)strtol(&buf, &p, 16));
                    s += 2;
                }
                else if ((u8)_c[s + 3] == (u8)0x5d) {
                    char buf[2] = { 0, 0 };
                    char* p;
                    buf[0] = (u8)_c[s + 1];
                    buf[1] = (u8)_c[s + 2];
                    newby.push_back((u8)strtol(buf, &p, 16));
                    s += 3;
                }
                else
                    newby.push_back((u8)0x5b);
            }
            else
                newby.push_back((u8)_c[s]);
        }
        // now that we have a search string, search bytes
        // step 1. compare size. if .length is < then skip that str
        // step 2. go byte by byte
        bool found = false;
        for (int i = 0; i < app->strFiles.size(); i++) {
            if (newby.size() > app->strFiles[i].length) { ; }
            else {
                for (int b = 0; b < (app->strFiles[i].length - newby.size()); b++) {
                    int a = 0;
                    if (app->strFiles[i].bytes[b] == newby[a]) {
                        while (app->strFiles[i].bytes[b] == newby[a]) {
                            b++;
                            a++;
                        }
                        if (a == newby.size()) {
                            wxLogError("String found: [#%d] %s at byte %d", i+1, app->strFiles[i].fname, b - newby.size());
                            found = true;
                            break;
                        }
                    }
                    if (found == true) b = 9999;
                }
            }
            //if (found == true) i = 9999;
        }
    }
    catch (...) {
        wxLogError("Not a valid string!");
    }
}



void MyFrame::ApplyTranslation()
{
    MyApp* app = &wxGetApp();
    if (app->appMode == MODE_DAT)
    {
        // save temporary changes to memory
        std::string tmp_u8(app->txtTranslation->GetValue().ToUTF8());
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
    }
    else { // ARF file
        if (app->strFiles[curWord].encoding == 1) return;
        if (app->lblTranslationSize->GetLabelText() != "Size OK") {
            wxLogError("Can't apply translation.");
        }
        // get string list from translation box in sjis
        std::vector<std::string> new_strs = split(
            utf8ToSjis(
                std::string(app->txtTranslation->GetValue().ToUTF8())
            ), "[0]\n");
        // lol, replace apostroph
        for (int f = 0; f < new_strs.size(); f++){
            new_strs[f] = ReplaceBadConversions(new_strs[f]);
            //tmp_sjis
        }
        // lets do some checks. first, see if there are any line breaks
        // and if so, just chop them out.
        std::vector<u8> outbytes;
        for (int f = 0; f < new_strs.size(); f++) {
            for (int j = 0; j < new_strs[f].length(); j++) {
                if ((u8)new_strs[f][j] == (u8)10) { ; } // no line breaks
                else if ((u8)new_strs[f][j] == (u8)0x5b) {
                    //if ((u8)new_strs[f][j - 1] < (u8)0x80)
                    //{
                   
                    if ((u8)new_strs[f][j + 2] == (u8)0x5d) { // 1 letter hex code
                        j++;
                        char f = new_strs[f][j];
                        char* p;
                        outbytes.push_back((&f, &p, 16));
                        j++;
                    }
                    else if ((u8)new_strs[f][j + 3] == (u8)0x5d) { // 2 letter hex codes
                        // 2 digit hex
                        char h[2];
                        j++;
                        h[0] = new_strs[f][j];
                        j++;
                        h[1] = new_strs[f][j];
                        char* p;
                        outbytes.push_back((u8)strtol(h, &p, 16));
                        j++;
                        }
                    //}
                    else // otherwise normal byte
                    {
                        outbytes.push_back((u8)0x5b);
                    }
                }
                else
                    outbytes.push_back((u8)new_strs[f][j]);
            }
            outbytes.push_back((u8)0);
        }
        outbytes.pop_back(); // get rid of final termiantor
        int bp = outbytes.size();

        //wxLogError("%d vs expected %d", (int)outbytes.size(), (int)app->strFiles[curWord].length);
        while (bp < app->strFiles[curWord].length) {
            //wxLogError("hmm, %d vs %d", outbytes.size(), app->strFiles[curWord].length);
            //outbytes.push_back(app->strFiles[curWord].bytes[bp++]);
            outbytes.push_back((u8)0);
            bp++;
        }
        // fix the bottom bytes
        int k = app->strFiles[curWord].length - 1;
        while ((u8)app->strFiles[curWord].bytes[k] != (u8)0) {
            outbytes[k] = app->strFiles[curWord].bytes[k];
            k--;
        }

        std::cout << app->strFiles[curWord].fname + "\n";
        for (int j = 0; j < app->strFiles[curWord].length; j++) {
            //wxLogError("%x | %x (%c)", outbytes[j], app->strFiles[curWord].bytes[j], outbytes[j]);
            if ((u8)app->strFiles[curWord].bytes[j] == (u8)0)
            {
                if ((u8)outbytes[j] != (u8)0) {
                    wxLogError("Null mismatch at %x", j);
                    return;
                }
            }if ((u8)outbytes[j] == (u8)0)
            {
                if ((u8)app->strFiles[curWord].bytes[j] != (u8)0) {
                    //wxLogError("Null mismatch at %x", j);
                    //return;
                }
            }
            //std::cout << std::hex << (int)outbytes[j] << "|";
            //std::cout << std::hex << (int)app->strFiles[curWord].bytes[j] << "\n";
            //if ((int)outbytes[j] != (int)app->strFiles[curWord].bytes[j]) {
            //    wxLogError("error at %d, %d vs %d", j, (u8)outbytes[j], (u8)app->strFiles[curWord].bytes[j]);
            //}
        }
        // finally, push the bytes to the TL
        int b = 0;
        for (; b < outbytes.size(); b++) {
            app->strFiles[curWord].bytes[b] = outbytes[b];
            // and update the original lines
        }
        //app->strFiles[curWord].bytes[b] = (u8)0;
        //for (int b = 0; b < )
        ParseStrFiles();
        // FILE WILL NOT BE WRITTEN UNTIL CTRL ENTER
    }
}


////////
void MyFrame::NextWord(wxCommandEvent& event)
{
    MyApp* app = &wxGetApp();
    if (datLoaded == true)
    {
        ApplyTranslation();
        //ParseStrFiles();
            // FILE WILL NOT BE WRITTEN UNTIL CTRL ENTER

        curWord++;
        if (app->appMode == MODE_DAT) {
            if (curWord == wordList.size())
                curWord = 0;
        }
        if (app->appMode == MODE_ARF) {
            if (curWord == app->strFiles.size())
                curWord = 0;
            while (app->strFiles[curWord].length == 1) {
                curWord++;
                if (curWord == app->strFiles.size())
                    curWord = 0;
            }
        }

        if (app->appMode == MODE_DAT) {
            UpdateCurrentWord(curWord);

            // Ensure we arent looking at an empty one
            while ((app->txtOriginalText->GetValue() == ""))
            {
                curWord++;
                if (app->appMode == MODE_DAT)
                    if (curWord == wordList.size())
                        curWord = 0;
               
                UpdateCurrentWord(curWord);

            }
        }
        else {
            while ((app->strFiles[curWord].length == 1) || (app->strFiles[curWord].encoding == 1)) {
                curWord++;
                //if (app->appMode == MODE_ARF)
                if (curWord == app->strFiles.size())
                   curWord = 0;
            }
            UpdateCurrentWord(curWord);
        }

    }
    else wxLogError("Load a translation file first!");
}

void MyFrame::PrevWord(wxCommandEvent& event)
{
    MyApp* app = &wxGetApp();

    if (datLoaded == true) {
        ApplyTranslation();
        // change display to next word
        curWord--;
        if (curWord == -1) {
            if (app->appMode == MODE_DAT) {
                curWord = wordList.size() - 1;
            }
            else {
                curWord = app->strFiles.size() - 1;
                while (app->strFiles[curWord].length == 1) {
                    curWord--;
                    if (curWord == 0)
                        curWord = app->strFiles.size() - 1;
                }
            }
        }
        if (app->appMode == MODE_DAT) {
            UpdateCurrentWord(curWord);
            // Ensure we arent looking at an empty one
            while (app->txtOriginalText->GetValue() == "")
            {
                curWord--;
                if (curWord == -1)
                    if (app->appMode == MODE_DAT) curWord = wordList.size() - 1;
                    //else curWord = app->strFiles.size() - 1;
                UpdateCurrentWord(curWord);
            }
        }
       
        else {
            while ((app->strFiles[curWord].length == 1) || (app->strFiles[curWord].encoding == 1)) {
                curWord--;
                //if (app->appMode == MODE_ARF)
                if (curWord == -1)
                   curWord = app->strFiles.size() - 1;
            }
            UpdateCurrentWord(curWord);
        }
    }
    else wxLogError("Load a translation file first!");
}


void MyFrame::UpdateCurrentWord(int wordNum)
{
    MyApp* app = &wxGetApp();
    curWord = wordNum; // just in case.

    switch (app->appMode) {
        case MODE_DAT: {
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
            app->lblDuplicates->SetLabel("Duplicates: " + std::to_string(wordList[wordNum].locs.size() - 1));
            // bad flag, ...
            app->chkMarkBad->SetValue(wordList[wordNum].bad);
            app->chkMarkComplete->SetValue(wordList[wordNum].complete);
            //std::cout << "bad: " << std::to_string(wordList[wordNum].bad) << " compl: " << std::to_string(wordList[wordNum].complete) << "\n";
            break;
        }
        case MODE_ARF: {
           
            // set string count label,
            std::string s = "Current string: [#" + std::to_string(wordNum+1) + "]" + app->strFiles[curWord].fname + " (" + std::to_string(app->strFiles[curWord].lines.size()) + ")";
            app->lblMainLabel->SetLabel(s);

            std::vector<std::string> sjislines;
            std::vector<std::string> tlines;
            for (int i = 0; i < app->strFiles[curWord].lines.size(); i++)
            {
                app->strFiles[curWord].lines[i] = ReplaceString(app->strFiles[curWord].lines[i],
                    "[0]\n", "\n");
                app->strFiles[curWord].tl_lines[i] = ReplaceString(app->strFiles[curWord].tl_lines[i],
                    "[0]\n", "\n");
                sjislines.push_back(utf8ToSjis(app->strFiles[curWord].lines[i]));
                tlines.push_back(utf8ToSjis(app->strFiles[curWord].tl_lines[i]));
            }

            ///////////
            std::string finalstr = ""; // wait!!! append \n every 40 chars
            int bc = 0;
            int len = sjislines.size() - 1;
            if (len == 0) len = 1;
            for (int i = 0; i < len; i++) {
                bool firstline = true;
                for (int j = 0; j < sjislines[i].length(); j++) {
                    if ((u8)sjislines[i][j] < (u8)0x20) {
                        if (sjislines[i][j] == 10) {
                            finalstr += "[0]\n";
                            firstline = true;
                        }

                        else {
                            std::string hx = int_to_hex((u8)sjislines[i][j],1);
                            finalstr += "[" + hx + "]";
                        }
                    }
                    else if ((u8)sjislines[i][j] < (u8)0x80) { // ascii
                        finalstr += sjislines[i][j];
                        bc++;
                    }
                    else {
                        finalstr += sjislines[i][j];
                        finalstr += sjislines[i][j + 1];
                        j++;
                        bc += 2;
                    }
                    //int mbc = 27;
                    //if (firstline == true) { mbc = 25; firstline = false; }
                    //if ((bc > mbc)) {
                    //    if (sjislines[i][j + 2] == 0x42) {
                    //        bc -= 2;
                    //    }
                    //    else if (sjislines[i][j + 2] == 0x48) {
                    //        bc -= 2;
                    //    }
                    //    else {
                    //        if (finalstr[finalstr.length() - 1] != '\n')
                    //            finalstr += '\n';
                    //        bc = 0;
                    //    }
                    //}
                }
                finalstr += "";
                bc = 0;
            }
            std::string newfinal(sjisToUtf8(finalstr));
            std::cout << newfinal << std::endl;
            app->txtOriginalText->ChangeValue(wxString::FromUTF8(newfinal.c_str()));
            ///////////
            ///////////
            finalstr = ""; // wait!!! append \n every 40 chars
            bc = 0;
            int sz = tlines.size() - 1;
            if (sz == 0) sz = 1;
            for (int i = 0; i < sz; i++) {
                bool firstline = true;
                for (int j = 0; j < tlines[i].length(); j++) {
                    if ((u8)tlines[i][j] < (u8)0x20) {
                        if (tlines[i][j] == 10) {
                            finalstr += "[0]\n";
                            firstline = true;
                        }
                        else {
                            std::string hx = int_to_hex((u8)tlines[i][j],1);
                            finalstr += "[" +hx + "]";
                        }
                    }
                    else if ((u8)tlines[i][j] < (u8)0x80) { // ascii
                        finalstr += tlines[i][j];
                        bc++;
                    }
                    else {
                        finalstr += tlines[i][j];
                        finalstr += tlines[i][j + 1];
                        j++;
                        bc += 2;
                    }
                    //int cs = 27;
                    //if (firstline == true) { cs = 25; firstline = false; }
                    //if ((bc > cs)) {
                    //    if (tlines[i][j + 2] == 0x42) {
                    //        bc -= 2;
                    //    }
                    //    else if (tlines[i][j + 2] == 0x48) {
                    //        bc -= 2;
                    //    }
                    //    else {
                    //        if (finalstr[finalstr.length() - 1] != '\n')
                    //            finalstr += '\n';
                    //        bc = 0;
                    //    }
                    //}
                }
                finalstr += "";
                bc = 0;
            }
            newfinal = sjisToUtf8(finalstr);
            std::cout << newfinal << std::endl;
            app->txtTranslation->ChangeValue(wxString::FromUTF8(newfinal.c_str()));
            ///////////

            app->lblOriginalSizeLabel->SetLabel("Size: " + std::to_string(app->strFiles[wordNum].length));

            UpdateTlByteCount();
            // end case arf
            break;
        }

    }


}


STRFile::STRFile()
{
    fname = "";
    offset = 0;
    length = 0;
}

void MyFrame::SyncLinesOnly()
{
    // Syncs all LINES and TL_LINES to BYTES

    MyApp* app = &wxGetApp();

    for (int i = 0; i < app->strFiles.size(); i++) {
        // parse ENTIRE file, replacing non-valid bytes with [xn] codes
        //for (int f = 0; f < app->strFiles[i].lines.size(); f++)
        //    for(int k = 0; k < app->strFiles[i].lines[f].length(); k++)
        //        app->strFiles[i].lines[f][k] = (u8)0;
        app->strFiles[i].lines.clear();
        //app->strFiles[i].tl_lines.clear();

        std::string tmp = "";
        int _sct = 0;
        for (int _i = 0; _i < app->strFiles[i].length; _i++) {
            if (app->strFiles[i].bytes[_i] == 0) {
                tmp += "[0]\n";
                //tmp = ReplaceBadConversions(tmp);
                tmp = std::string(sjisToUtf8(tmp));
                app->strFiles[i].lines.push_back(tmp);
                //app->strFiles[i].tl_lines.push_back(tmp);
                tmp = "";
                //std::cout << app->strFiles[i].lines[_sct] << std::endl;
                _sct++;
            }
            else if (app->strFiles[i].bytes[_i] == 0x10) {
                tmp += "[10]";
            }
            else if (app->strFiles[i].bytes[_i] == 0x11) {
                tmp += "[11]";
            }
            else if (app->strFiles[i].bytes[_i] == 0x12) {
                tmp += "[12]";
            }
            else {
                tmp += app->strFiles[i].bytes[_i];
            }
        }
        // This will save the bytes at the end of the file without making a string out of them.
        if (tmp.length() > 0) {
            //tmp = ReplaceBadConversions(tmp);
            tmp = std::string(sjisToUtf8(tmp));
            app->strFiles[i].lines.push_back(tmp);
            //app->strFiles[i].tl_lines.push_back(tmp);
        }
    }
}

void MyFrame::ParseStrFiles()
{
    // Syncs all LINES and TL_LINES to BYTES

    MyApp* app = &wxGetApp();

    for (int i = 0; i < app->strFiles.size(); i++) {
        // parse ENTIRE file, replacing non-valid bytes with [xn] codes
        //for (int f = 0; f < app->strFiles[i].lines.size(); f++)
        //    for(int k = 0; k < app->strFiles[i].lines[f].length(); k++)
        //        app->strFiles[i].lines[f][k] = (u8)0;
        //app->strFiles[i].lines.clear();
        app->strFiles[i].tl_lines.clear();

        std::string tmp = "";
        int _sct = 0;
        for (int _i = 0; _i < app->strFiles[i].length; _i++) {
            if (app->strFiles[i].bytes[_i] == 0) {
                tmp += "[0]\n";
                tmp = std::string(sjisToUtf8(tmp));
                //app->strFiles[i].lines.push_back(tmp);
                app->strFiles[i].tl_lines.push_back(tmp);
                tmp = "";
                //std::cout << app->strFiles[i].lines[_sct] << std::endl;
                _sct++;
            }
            else if (app->strFiles[i].bytes[_i] == 0x10) {
                tmp += "[10]";
            }
            else if (app->strFiles[i].bytes[_i] == 0x11) {
                tmp += "[11]";
            }
            else if (app->strFiles[i].bytes[_i] == 0x12) {
                tmp += "[12]";
            }
            else {
                tmp += app->strFiles[i].bytes[_i];
            }
        }
        // This will save the bytes at the end of the file without making a string out of them.
        if (tmp.length() > 0) {
            //app->strFiles[i].lines.push_back(tmp);
            app->strFiles[i].tl_lines.push_back(tmp);
        }
    }
}

void MyFrame::LoadDB(wxCommandEvent& e)
{
    MyApp* app = &wxGetApp();

    wxFileDialog openFileDialog(this, _("Open translation .DAT/.ARF file"), "", "", "DAT/ARF files (*.dat;*.arf)|*.dat;*.arf",
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
    bool isTldat = true;
    char hdr[5] = { 'T', 'L', 'D', 'A', 'T' };
    for (int i = 0; i < 5; i++) {
        if (buffer[i] != hdr[i]) {
            isTldat = false;
            //wxLogError("Database error!\nDatabase is corrupt.");
            //return;
        }
    }

    app->strFiles.clear();
   

    // #struct ARFFileHeader {
    //     #    char name[13];     : (12+null) (bytes between 0 and index 13 are junk)
    //     #    u8 file_type;      : 00 is lzss, 01 is binary, 02 is uncompressed
    //     #    u16 uncomp_size;
    //     #    u16 compr_size;
    //     #    u24 file_offset;
    //     #    u16 nullterm = 0;
    //     #};
    if (!isTldat) {
        // try ARF
        // 1. Get number of files
        app->numStrFiles = (u8)buffer[0] + ((u8)buffer[1] << 8);
        wxLogError("%d files found.", app->numStrFiles);
        int bc = 2;
        // 2. Populate "arf database"
        app->arfHeaderOffset = (app->numStrFiles * 22) + 2;
        for (int i = 0; i < app->numStrFiles; i++) {
            STRFile f = STRFile();
            app->strFiles.push_back(f);
            /// get file names
            std::string _fn = "";
            for (int _c = 0; _c < 12; _c++) {
                _fn += buffer[bc + _c];
            }
            app->strFiles[i].fname = _fn;
            bc += 13;
            // file type
            if ((u8)buffer[bc] == (u8)0) {
                wxLogError("LZSS string found - is this a translation file?");
                return;
            }
            app->strFiles[i].encoding = (u8)buffer[bc];
            bc++;
            // uncompressed size
            int ucsz = (u8)buffer[bc] + ((u8)buffer[bc + 1] << 8);
            app->strFiles[i].length = ucsz;
            bc += 2;
            // compressed size
            app->strFiles[i].compr_length = (u8)buffer[bc] + ((u8)buffer[bc + 1] << 8);
            bc += 2;
            /// get file offsets
            app->strFiles[i].offset = (u8)buffer[bc] + ((u8)buffer[bc + 1] << 8) + ((u8)buffer[bc + 2] << 16);
            std::cout << app->strFiles[i].fname << " " << ucsz << " " << std::hex << app->strFiles[i].offset << std::endl;
            bc += 4;
            /// and get the bytes
            for (int j = app->strFiles[i].offset; j < (app->strFiles[i].offset + app->strFiles[i].compr_length); j++) {
                app->strFiles[i].bytes.push_back(buffer[j]);
            }
        }

        app->appMode = MODE_ARF;
        datLoaded = true;

        SyncLinesOnly();
        ParseStrFiles();

        UpdateCurrentWord(1);

        wxMessageDialog* arfmsg = new wxMessageDialog(this, "ARF bundle loaded OK!");
        arfmsg->ShowModal();

        return;
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

    app->appMode = MODE_DAT;

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
    fileMenu->Append(swpctool_Open, "&Open\tAlt-O", "Open translation file");
    //fileMenu->Append(swpctool_OpenGoto, "&Goto\tAlt-G", "Goto specific line/STR");
    //fileMenu->Append(swpctool_OpenSearch, "Sea&rch\tAlt-R", "Search for string");
    fileMenu->Append(swpctool_Quit, "E&xit\tAlt-X", "Quit this program");
   
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

std::string ReplaceBadConversions(const std::string& s)
{
    std::string p;
    p = ReplaceString(s, "\x81\x66", "'");
    //p = ReplaceString(p, "\x81\x67", "\"");
    //p = ReplaceString(p, "\x81\x68", "\"");
    return p;
}

void MyFrame::UpdateTlByteCount()
{
    MyApp* app = &wxGetApp();

    long pos = app->txtTranslation->GetInsertionPoint();

    // 1 get all strings as a list (utfw > uft8 > sjis)
    // 2 get string lengths from jp text
    // 3 compare and pad with 0x20 if not equal
    // 4 reset input insertion point
    try {

        std::vector<std::string> new_strs = split(utf8ToSjis(std::string(app->txtTranslation->GetValue().ToUTF8())), "[0]\n");
        std::vector<std::string> jp_strs = split(utf8ToSjis(std::string(app->txtOriginalText->GetValue().ToUTF8())), "[0]\n");
        std::string finalstr = "";

        // string lengths
        std::vector<int> strlens;
        for (int i = 0; i < jp_strs.size(); i++) {
           
            int ct = 0;
            for (int f = 0; f < jp_strs[i].length(); f++) {
                if ((u8)jp_strs[i][f] == (u8)10) { }
                else if ((u8)jp_strs[i][f] == (u8)0x5b) {
                    if ((u8)jp_strs[i][f + 2] == (u8)0x5d) {
                        f += 2;
                        ct++;
                    }
                    else if ((u8)jp_strs[i][f + 3] == (u8)0x5d) {
                        f += 3;
                        ct++;
                    }
                    else {
                        ct++;
                    }
                }
                else
                    ct++;
            }
            //ct++; // null term?
           
            strlens.push_back(ct);
            //strlens.push_back(jp_strs[i].length());
        }

        // MY DOUBLE CHECKER!
        // For each string, append spaces until it matches the jp equivalent index
        for (int i = 0; i < new_strs.size(); i++)
        {
            new_strs[i] = ReplaceBadConversions(new_strs[i]);
            jp_strs[i] = ReplaceBadConversions(jp_strs[i]);
           
        }

        app->lblTranslationSize->SetLabel("Size OK");
        for (int i = 0; i < new_strs.size(); i++)
        {   // to properly count, we only count non-newline charaters.
            int bc = 0;
            for (int a = 0; a < new_strs[i].length(); a++) {
                if ((u8)new_strs[i][a] == (u8)10) { ; }
                else if ((u8)new_strs[i][a] == (u8)0x5b) {
                    if ((u8)new_strs[i][a + 2] == (u8)0x5d) {
                        a += 2;
                        bc++;
                    }
                    else if ((u8)new_strs[i][a + 3] == (u8)0x5d) {
                        a += 3;
                        bc++;
                    }
                    else
                        bc++;
                }
                else
                    bc++;
            }
            //bc++; // endl
           
            if (bc > strlens[i]) {
                app->lblTranslationSize->SetLabel("Size over on line" + std::to_string(i));
                //wxLogError("Line %d: need %d, got %d", i, bc, strlens[i]);
            }
        }

        ///////////
        // Split each new line at 28 chars < stop
       
        for (int i = 0; i < new_strs.size(); i++) {
            int bc = 0;
            //bool first = true;
            //int cb = 25;
            finalstr += new_strs[i];
            for (int j = 0; j < new_strs[i].length(); j++) {
                // count line lengths
                if ((u8)new_strs[i][j] == (u8)0x5b) {
                    if ((u8)new_strs[i][j + 2] == (u8)0x5d) {
                        j += 2;
                        bc++;
                    }
                    else if ((u8)new_strs[i][j + 3] == (u8)0x5d) {
                        j += 3;
                        bc++;
                    }
                    else
                        bc++;
                }
                else
                    bc++;
            } // str break
            //

            while (bc < strlens[i]) {
                finalstr += " ";
                bc++;
            }
            if (i != (new_strs.size() - 1)){
                finalstr += "[0]\n";
                //bc++;
            }
        }
       

        //std::string s = sjisToUtf8(std::string(app->strFiles[curWord].lines[0].c_str()));
        //app->txtTranslation->ChangeValue(wxString::FromUTF8(app->strFiles[curWord].lines[0].c_str()));
        //app->txtTranslation->ChangeValue(wxString::FromUTF8(sjisToUtf8(app->strFiles[curWord].lines[0]).c_str()));// wxString(sjisToUtf8(utf8ToSjis(std::string(app->txtTranslation->GetValue().ToUTF8())))));
        app->txtTranslation->ChangeValue(wxString::FromUTF8(sjisToUtf8(finalstr)));

        // to double check, have to iterate everything
        //app->lblTranslationSize->SetLabel("Size: " + std::to_string(tmp_sjis.length() + app->strFiles[curWord].lines[app->strFiles[curWord].lines.size() - 1].length()));
        if (app->chkMarkInsert->GetValue() == 1) {
            if (app->backspacing == true) {
                if (pos > 0)
                    pos--;
                app->backspacing = false;
            }
            app->txtTranslation->SetInsertionPoint(pos);
            app->txtTranslation->SetSelection(pos, pos + 1);
            if (app->txtTranslation->GetStringSelection() == "[")
                app->txtTranslation->SelectNone();
            app->txtTranslation->ShowPosition(pos);
        }
        else {
            app->txtTranslation->SetInsertionPoint(pos);
        }
    }
    catch (...) {
        std::cout << "ERROR\n";
    }
}

void MyFrame::UpdateTlByteCount(wxCommandEvent& e)
{
    UpdateTlByteCount();
   
}
///////////////

void SearchFrame::OnFrameClose(wxCommandEvent& e)
{
    MyApp* app = &wxGetApp();
    app->frmStrFrame = NULL;

    Close(true);
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

void MyFrame::SetInsert(wxCommandEvent& WXUNUSED(e)) {
    return;
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

void MyFrame::OpenSearch(wxCommandEvent& WXUNUSED(e))
{
    MyApp* app = &wxGetApp();

    app->frmSearch = new SearchFrame("Text Search");
    app->frmSearch->Show(true);
}


void MyFrame::CommitChanges(wxCommandEvent& WXUNUSED(e))
{
    MyApp* app = &wxGetApp();

    if (app->frmMainFrame->datLoaded == false) {
        wxLogError("Load a file first!");
        return;
    }

    if (app->appMode == MODE_DAT) {
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
                wxLogError("You can't!\nString #%d has an invalid number of characters.", f + 1);
                return;
            }
        }

        // Recreate translation.dat file
        //# TLDAT header
        //# bytect[2] \0 transl[bytect] \0 text[bytect] \0 bad[1] \0 complete[1] \0 locct[1] \0 locs[ [disk[1] addr[3]] ][locct] \0d \0a
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
        for (int i = 0; i < wordList.size(); i++) {
            if (wordList[i].complete == true) tc++;
        }
        float pct = (float)tc;
        float den = (float)wordList.size();
        pct = (pct / den) * 100.0f;
        int _p = (int)(pct);
        SetStatusText("Current progress: " + std::to_string(_p) + "%");

    }
    else {
        // is ARF file
        ApplyTranslation(); // < calls parse str files

        std::vector<char> outbytes;
        // write 2 bytes of file count
        // write headers:
            // #struct ARFFileHeader {
    //     #    char name[13];     : (12+null) (bytes between 0 and index 13 are junk)
    //     #    u8 file_type;      : 00 is lzss, 01 is binary, 02 is uncompressed
    //     #    u16 uncomp_size;
    //     #    u16 compr_size;
    //     #    u24 file_offset;
    //     #    u16 nullterm = 0;
    //     #};
        outbytes.push_back(((u8)app->strFiles.size() & 0xff));
        outbytes.push_back((u8)((app->strFiles.size() & 0xff00) >> 8));
        //  for each str file
        for (int c = 0; c < app->strFiles.size(); c++) {
            // fname
            for (int j = 0; j < 12; j++) {
                while(app->strFiles[c].fname[j] != 0){
                    outbytes.push_back((u8)app->strFiles[c].fname[j]);
                    j++;
                }
                outbytes.push_back((u8)0);
            }
            // ftype
            outbytes.push_back((u8)app->strFiles[c].encoding);
            outbytes.push_back((u8)((app->strFiles[c].length & 0xff)));
            outbytes.push_back((u8)((app->strFiles[c].length & 0xff00) >> 8));
            outbytes.push_back((u8)((app->strFiles[c].compr_length & 0xff)));
            outbytes.push_back((u8)((app->strFiles[c].compr_length & 0xff00) >> 8));
            outbytes.push_back((u8)((app->strFiles[c].offset & 0xff)));
            outbytes.push_back((u8)((app->strFiles[c].offset & 0xff00) >> 8));
            outbytes.push_back((u8)((app->strFiles[c].offset & 0xff0000) >> 16));
            outbytes.push_back((u8)0);
        }

        //  write the .length bytes from bytes at the addr to the arf file
        int bc = 0;
        int ofsct = 2 + (22 * app->strFiles.size());
        for (int c = 0; c < app->strFiles.size(); c++) {
            bc = 0;
            if (ofsct != app->strFiles[c].offset) {
                wxLogError("offset mismatch");
            }
            wxLogError("%s %x", app->strFiles[c].fname, app->strFiles[c].offset);
            for (int b = 0; b < app->strFiles[c].compr_length; b++) {
                outbytes.push_back((u8)(app->strFiles[c].bytes[b]));
                bc++;
            }
            ofsct += bc;
           
        }
        wxLogError("%d written", bc);

        std::string tf = split(currentFilePath, ".ARF")[0];// [0] + "_E.ARF";
        if (tf[tf.length() - 1] != 'E') tf += "_E.ARF";
        else tf += ".ARF";
        std::ofstream os(tf, std::ios_base::binary);
        if (!os) { wxLogError("Cannot write to DB!\nIs it read only?"); return; }
        os.write(outbytes.data(), outbytes.size());
        os.close();

        wxLogError("%s written successfully.", tf);
    }
    //wxLogError("Complete");
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
    wxSize _sz = wxSize(500, 640);
    frmMainFrame->SetSize(_sz);

    pnlMainPanel = new wxPanel(frmMainFrame, wxID_ANY);

    wxFont fwfont(16, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    wxPoint ptMainLblPt = wxPoint(10, 10);
    lblMainLabel = new wxStaticText(pnlMainPanel, wxID_ANY, "Current string: 0 / 0", ptMainLblPt);
    lblDuplicates = new wxStaticText(pnlMainPanel, wxID_ANY, "Total # duplicates: 0", wxPoint(330, 20));

    wxStaticText* lblOrigTxtLabel = new wxStaticText(pnlMainPanel, wxID_ANY, "Original text:", wxPoint(10, 30));
    txtOriginalText = new wxTextCtrl(pnlMainPanel, wxID_ANY, "", wxPoint(50, 50), wxSize(400, 150), wxTE_READONLY | wxTE_MULTILINE | wxTE_CHARWRAP);
    lblOriginalSizeLabel = new wxStaticText(pnlMainPanel, wxID_ANY, "Size: ", wxPoint(350, 132 + 70));
    txtOriginalText->SetFont(fwfont);

    wxStaticText* lblTranslationLabel = new wxStaticText(pnlMainPanel, wxID_ANY, "Translation:", wxPoint(10, 125 + 80));
    txtTranslation = new wxTextCtrl(pnlMainPanel, wxID_ANY, "", wxPoint(50, 130 + 100), wxSize(400, 150), wxTE_MULTILINE | wxTE_CHARWRAP);
    lblTranslationSize = new wxStaticText(pnlMainPanel, wxID_ANY, "Size OK", wxPoint(350, 240 + 140));
    txtTranslation->SetFont(fwfont);

    chkMarkBad = new wxCheckBox(pnlMainPanel, wxID_ANY, "Flag (B)ad string", wxPoint(10, 250 + 160));
    chkMarkBad->Disable();
    chkMarkComplete = new wxCheckBox(pnlMainPanel, wxID_ANY, "Mark (D)one (don't show again)", wxPoint(10, 270 + 160));
    chkMarkComplete->Disable();
    chkMarkInsert = new wxCheckBox(pnlMainPanel, wxID_ANY, "(I)nsert mode", wxPoint(10, 290 + 160));

    btnBack = new wxButton(pnlMainPanel, wxID_ANY, "<<", wxPoint(60, 300 + 200));
    btnNext = new wxButton(pnlMainPanel, wxID_ANY, ">>", wxPoint(340, 300 + 200));
    btnCommit = new wxButton(pnlMainPanel, wxID_ANY, ".       Commit       .\n", wxPoint(180, 275 + 200));
    btnSelectString = new wxButton(pnlMainPanel, wxID_ANY, "Goto string...", wxPoint(10, 320 + 210));
    btnSearch = new wxButton(pnlMainPanel, wxID_ANY, "Search...", wxPoint(400, 530));

    //btnCommit->Bind(wxEVT_BUTTON, &MyFrame::OnQuit, frmMainFrame);
    btnBack->Bind(wxEVT_BUTTON, &MyFrame::PrevWord, frmMainFrame);
    btnNext->Bind(wxEVT_BUTTON, &MyFrame::NextWord, frmMainFrame);
    btnSelectString->Bind(wxEVT_BUTTON, &MyFrame::OpenStrSel, frmMainFrame);
    btnSearch->Bind(wxEVT_BUTTON, &MyFrame::OpenSearch, frmMainFrame);

    txtTranslation->Bind(wxEVT_TEXT, &MyFrame::UpdateTlByteCount, frmMainFrame);

    chkMarkBad->Bind(wxEVT_CHECKBOX, &MyFrame::SetBad, frmMainFrame);
    chkMarkComplete->Bind(wxEVT_CHECKBOX, &MyFrame::SetComplete, frmMainFrame);
    chkMarkInsert->Bind(wxEVT_CHECKBOX, &MyFrame::SetInsert, frmMainFrame);


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
        if (keyEvent.GetKeyCode() == 308) {
            shiftKey = true;
        }
        if (keyEvent.GetKeyCode() == 8) {
            if (frmMainFrame->datLoaded == true)
                backspacing = true;
        }
        if (keyEvent.GetKeyCode() == 86) {
            if (shiftKey == true) {
                isPasting = true;
            }
        }
        if (keyEvent.GetKeyCode() == 314) {
            if (shiftKey)
                frmMainFrame->PrevWord(static_cast<wxCommandEvent&>(event));
        }
        else if (keyEvent.GetKeyCode() == 316) {
            if (shiftKey)
                frmMainFrame->NextWord(static_cast<wxCommandEvent&>(event));
        }
        else if (keyEvent.GetKeyCode() == 66) {//b = 66
            if (shiftKey == true)
                if (frmMainFrame->datLoaded == true) {
                    chkMarkBad->SetValue(!chkMarkBad->GetValue());
                    frmMainFrame->SetBad(static_cast<wxCommandEvent&>(event));
                }
        }
        else if (keyEvent.GetKeyCode() == 68) {//d
            if (shiftKey)
                if (frmMainFrame->datLoaded) {
                    chkMarkComplete->SetValue(!chkMarkComplete->GetValue());
                    frmMainFrame->SetComplete(static_cast<wxCommandEvent&>(event));
                }
        }
        else if (keyEvent.GetKeyCode() == 13) {//enter
            if (shiftKey)
                if (frmMainFrame->datLoaded)
                    frmMainFrame->CommitChanges(static_cast<wxCommandEvent&>(event));
        }
        else if (keyEvent.GetKeyCode() == 71) {//g = 71 h i 73
            if (shiftKey)
                if (frmMainFrame->datLoaded)
                    frmMainFrame->OpenStrSel(static_cast<wxCommandEvent&>(event));
        }
        else if (keyEvent.GetKeyCode() == 73) {//i jklmnopqrstuv
            if (shiftKey)
                chkMarkInsert->SetValue(!chkMarkInsert->GetValue());
        }

        //else if(keyEvent.GetKeyCode() == )
    }
    if (event.GetEventType() == wxEVT_KEY_UP) {
        wxKeyEvent& keyEvent = static_cast<wxKeyEvent&>(event);
        //wxLogMessage("Key pressed: %d", keyEvent.GetKeyCode());
        if (keyEvent.GetKeyCode() == 308) {
            shiftKey = false;
            isPasting = false;
        }
    }
    return wxApp::FilterEvent(event);
}
