#include "MessageDialog.h"

MessageDialog::MessageDialog(const wxString& title, const wxString& text, void (*OnOk)(void*), void (*OnCancel)(void*), void *data)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
    , OnOk(OnOk)
    , OnCancel(OnCancel)
    , data(data)
{

    wxPanel *panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText *stxt1 = new wxStaticText(panel, ID_TEXT, text);
    wxButton *btn1 = new wxButton(panel, ID_OK, wxT("OK"));
    wxButton *btn2 = new wxButton(panel, ID_CANCEL, wxT("Cancel"));

    Connect(GetId(), wxEVT_CLOSE_WINDOW,
            wxCloseEventHandler(MessageDialog::OnCloseHandler));
    Connect(ID_OK, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MessageDialog::OnOkHandler));
    Connect(ID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MessageDialog::OnCancelHandler));

    hbox->Add(btn1, 0, wxALL, 2);
    hbox->Add(btn2, 0, wxALL, 2);
    vbox->Add(stxt1, 0, wxALL, 5);
    vbox->Add(hbox, 0, wxALL, 5);
    panel->SetSizer(vbox);

    Center();
}

void MessageDialog::OnCloseHandler(wxCloseEvent& event)
{
    if (OnCancel)
    {
        OnCancel(data);
    }
    Destroy();
}
void MessageDialog::OnOkHandler(wxCommandEvent & event)
{
    if (OnOk)
    {
        OnOk(data);
    }
    Destroy();
}
void MessageDialog::OnCancelHandler(wxCommandEvent & event)
{
    if (OnCancel)
    {
        OnCancel(data);
    }
    Destroy();
}
