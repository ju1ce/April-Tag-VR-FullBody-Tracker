#include <wx/wx.h>

class MessageDialog : public wxFrame
{
public:
    const int ID_TEXT = 1;
    const int ID_OK = 2;
    const int ID_CANCEL = 3;

    MessageDialog(const wxString& title, const wxString& text, void (*OnOk)(void*), void (*OnCancel)(void*), void *data);

    void (*OnOk)(void *);
    void (*OnCancel)(void *);
    void *data;

private:
    void OnCloseHandler(wxCloseEvent& event);
    void OnOkHandler(wxCommandEvent & event);
    void OnCancelHandler(wxCommandEvent & event);
};

