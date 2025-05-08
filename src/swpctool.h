#ifndef __SWPCTOOL_H__
#define __SWPCTOOL_H__


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
//#include "convtable.h"
#include <unicode/unistr.h>


#define u8 unsigned char
#define MODE_ARF 1 
#define MODE_DAT 0


std::string ReplaceBadConversions(const std::string& s);
void check_null(char c);
std::string sjisToUtf8(const std::string& value);
std::string utf8ToSjis(const std::string& value);
std::vector<std::string> split(std::string s, std::string delimiter);
std::string addNewlines(const std::string& text, int x);
std::string ReplaceString(std::string subject, const char search,
    const std::string& replace);
std::string ReplaceString(std::string subject, const std::string& search,
    const std::string& replace);

class Location
{
public:
    Location();
    Location(unsigned long address, int disk);

    unsigned long address;
    int disk;

private:
};


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

class STRFile
{
public:
    STRFile();

    std::vector<u8> bytes;

    std::string fname;
    std::vector<std::string> lines;
    std::vector<std::string> tl_lines;
    u8 encoding = 0;
    int offset = 0;
    int length = 0;
    int compr_length = 0;
};


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

    void UpdateTlByteCount();
    void UpdateTlByteCount(wxCommandEvent& e);
    void OpenStrSel(wxCommandEvent& e);
    void CommitChanges(wxCommandEvent& e);
    void SetBad(wxCommandEvent& e);
    void SetComplete(wxCommandEvent& e);
    void SetInsert(wxCommandEvent& e);

    void ParseStrFiles();
    void SyncLinesOnly();
    void ApplyTranslation();

    std::vector<char> translation_dat;
    std::vector<TlWord> wordList;

    int curWord;

    std::string currentFilePath;

    bool datLoaded;
    bool strWinOpen = false;

private:
    // any class wishing to process wxWidgets events must use this macro!
    wxDECLARE_EVENT_TABLE();
};

class PickStringFrame : public wxFrame
{
public:
    PickStringFrame(const wxString& title);

    void OnFrameClose(wxCommandEvent& event); // always need this on macos, doh

    void Confirm(wxCommandEvent& event); // goto string function

    wxPanel* pnlStrPanel;
    wxStaticText* lblStringLabel;
    wxTextEntry* txtStringSel;
    wxButton* btnStrOK;

private:
    wxDECLARE_EVENT_TABLE();
};

// define app (must be of type wxApp)
class MyApp : public wxApp
{
public:
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit() wxOVERRIDE;
    int FilterEvent(wxEvent& event) override;

    MyFrame* frmMainFrame;
    wxPanel* pnlMainPanel;
    PickStringFrame* frmStrFrame;

    wxStaticText* lblMainLabel;
    wxStaticText* lblDuplicates;
    wxStaticText* lblOriginalSizeLabel;
    wxStaticText* lblTranslationSize;

    wxTextCtrl* txtOriginalText;
    wxTextCtrl* txtTranslation;

    wxCheckBox* chkMarkBad;
    wxCheckBox* chkMarkComplete;
    wxCheckBox* chkMarkInsert;

    wxButton* btnBack;
    wxButton* btnNext;
    wxButton* btnCommit;
    wxButton* btnSelectString;

    bool shiftKey = false;
    bool backspacing = false;
    bool isPasting = false;

    u8 appMode = 0;

    int numStrFiles;
    int arfHeaderOffset;
    std::vector<STRFile> strFiles;

};

// IDs for the controls and the menu commands
enum
{
    swpctool_Quit = wxID_EXIT,
    swpctool_About = wxID_ABOUT, // this macro must be used on osx so it is put in the right place
    swpctool_Open = wxID_ANY,
    swpctool_Prev = wxID_ANY,
    swpctool_Next = wxID_ANY,
    swpctool_ChangeTl = wxID_ANY,
    swpctool_Goto = wxID_ANY,
    swpctool_CloseStrFrame = wxID_ANY,
    swpctool_SetComplete = wxID_ANY,
    swpctool_SetBad = wxID_ANY,
    swpctool_Commit = wxID_ANY,
    swpctool_SetInsert = wxID_ANY
};


#endif 
