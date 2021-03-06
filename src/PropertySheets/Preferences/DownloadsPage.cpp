#include "DownloadsPage.hpp"

#include "../../Configuration.hpp"
#include "../../Dialogs/OpenFileDialog.hpp"
#include "../../Translator.hpp"

const GUID DLG_BROWSE = { 0x7D5FE367, 0xE148, 0x4A96,{ 0xB3, 0x26, 0x42, 0xEF, 0x23, 0x7A, 0x36, 0x62 } };

using PropertySheets::Preferences::DownloadsPage;

DownloadsPage::DownloadsPage()
{
    m_title = TRW("downloads");
    SetTitle(m_title.c_str());
}

BOOL DownloadsPage::OnApply()
{
    Configuration& cfg = Configuration::GetInstance();

    // Save path
    TCHAR path[MAX_PATH];
    m_savePath.GetWindowText(path, ARRAYSIZE(path));
    cfg.SetDefaultSavePath(ToString(path));

    // Move completed
    cfg.SetMoveCompletedDownloads(m_moveCompleted.GetCheck() == BST_CHECKED);
    cfg.SetMoveCompletedDownloadsFromDefaultOnly(m_onlyMoveFromDefault.GetCheck() == BST_CHECKED);

    m_movePath.GetWindowText(path, ARRAYSIZE(path));
    cfg.SetMoveCompletedDownloadsPath(ToString(path));

    // Rates
    TCHAR dl[1024];
    int res = m_dl.GetWindowText(dl, ARRAYSIZE(dl));
    int dl_rate = -1;
    if (res > 0) { dl_rate = std::stoi(dl); }

    TCHAR ul[1024];
    res = m_ul.GetWindowText(ul, ARRAYSIZE(ul));
    int ul_rate = -1;
    if (res > 0) { ul_rate = std::stoi(ul); }

    if (dl_rate > 0) { dl_rate *= 1024; }
    if (ul_rate > 0) { ul_rate *= 1024; }

    cfg.Session()->SetDownloadRateLimit(dl_rate);
    cfg.Session()->SetUploadRateLimit(ul_rate);

    return TRUE;
}

void DownloadsPage::OnBrowse(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    int target = (nID == ID_PREFS_DEFSAVEPATH_BROWSE)
        ? ID_PREFS_DEFSAVEPATH
        : ID_PREFS_MOVEPATH;

    Dialogs::OpenFileDialog dlg;
    dlg.SetGuid(DLG_BROWSE);
    dlg.SetOptions(dlg.GetOptions() | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);
    dlg.SetTitle(TRW("choose_save_path"));

    // If we have a save path in the textbox,
    // use that.
    TCHAR path[MAX_PATH];
    int res = GetDlgItemText(target, path, ARRAYSIZE(path));
    if (res > 0) { dlg.SetFolder(path); }

    dlg.Show();
    auto paths = dlg.GetPaths();

    if (paths.size() > 0)
    {
        SetDlgItemText(target, paths[0].c_str());
        SetModified();
    }
}

void DownloadsPage::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    switch (nID)
    {
    case ID_PREFS_GLOBAL_DL_LIMIT:
    case ID_PREFS_GLOBAL_UL_LIMIT:
    case ID_PREFS_DEFSAVEPATH:
        if (uNotifyCode == EN_CHANGE) { SetModified(); }
        break;
    case ID_PREFS_MOVECOMPLETEDDOWNLOADS:
    {
        UpdateMoveState(m_moveCompleted.GetCheck() == BST_CHECKED);
        SetModified();
        break;
    }
    }
}

BOOL DownloadsPage::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
    SetDlgItemText(ID_TRANSFERS_GROUP, TRW("transfers"));
    SetDlgItemText(ID_DEFSAVEPATH_TEXT, TRW("path"));
    SetDlgItemText(ID_PREFS_DEFSAVEPATH_BROWSE, TRW("browse"));
    SetDlgItemText(ID_DL_PREFS_LIMITS_GROUP, TRW("global_limits"));
    SetDlgItemText(ID_PREFS_GLOBAL_DL_LIMIT_TEXT, TRW("dl_limit"));
    SetDlgItemText(ID_PREFS_GLOBAL_DL_LIMIT_HELP, TRW("dl_limit_help"));
    SetDlgItemText(ID_PREFS_GLOBAL_UL_LIMIT_TEXT, TRW("ul_limit"));
    SetDlgItemText(ID_PREFS_GLOBAL_UL_LIMIT_HELP, TRW("ul_limit_help"));
    SetDlgItemText(ID_PREFS_MOVECOMPLETEDDOWNLOADS, TRW("move_completed_downloads"));
    SetDlgItemText(ID_PREFS_MOVEPATH_BROWSE, TRW("browse"));
    SetDlgItemText(ID_PREFS_ONLYMOVEFROMDEFAULT, TRW("only_move_from_default_save_path"));

    m_savePath = GetDlgItem(ID_PREFS_DEFSAVEPATH);
    m_moveCompleted = GetDlgItem(ID_PREFS_MOVECOMPLETEDDOWNLOADS);
    m_movePath = GetDlgItem(ID_PREFS_MOVEPATH);
    m_movePathBrowse = GetDlgItem(ID_PREFS_MOVEPATH_BROWSE);
    m_onlyMoveFromDefault = GetDlgItem(ID_PREFS_ONLYMOVEFROMDEFAULT);
    m_dl = GetDlgItem(ID_PREFS_GLOBAL_DL_LIMIT);
    m_ul = GetDlgItem(ID_PREFS_GLOBAL_UL_LIMIT);

    Configuration& cfg = Configuration::GetInstance();
    m_savePath.SetWindowText(ToWideString(cfg.GetDefaultSavePath()).c_str());
    
    m_moveCompleted.SetCheck(cfg.GetMoveCompletedDownloads() ? BST_CHECKED : BST_UNCHECKED);
    m_movePath.SetWindowText(TWS(cfg.GetMoveCompletedDownloadsPath()));
    m_onlyMoveFromDefault.SetCheck(cfg.GetMoveCompletedDownloadsFromDefaultOnly() ? BST_CHECKED : BST_UNCHECKED);

    // Rate limits
    int dl_rate = cfg.Session()->GetDownloadRateLimit();
    int ul_rate = cfg.Session()->GetUploadRateLimit();
    if (dl_rate < 0) { dl_rate = 0; }
    if (dl_rate > 0) { dl_rate /= 1024; }
    if (ul_rate < 0) { ul_rate = 0; }
    if (ul_rate > 0) { ul_rate /= 1024; }

    m_dl.SetWindowText(std::to_wstring(dl_rate).c_str());
    m_ul.SetWindowText(std::to_wstring(ul_rate).c_str());

    UpdateMoveState(m_moveCompleted.GetCheck() == BST_CHECKED);

    return FALSE;
}

void DownloadsPage::UpdateMoveState(bool checked)
{
    m_movePath.EnableWindow(checked ? TRUE : FALSE);
    m_movePathBrowse.EnableWindow(checked ? TRUE : FALSE);
    m_onlyMoveFromDefault.EnableWindow(checked ? TRUE : FALSE);
}
