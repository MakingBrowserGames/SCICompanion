// PreferencesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AppState.h"
#include "PreferencesDialog.h"

// CPreferencesDialog dialog

CPreferencesDialog::CPreferencesDialog(CWnd* pParent /*=NULL*/)
	: CExtResizableDialog(CPreferencesDialog::IDD, pParent)
{
    _fBrowseInfoStart = appState->_fBrowseInfo;
    _fAspectRatioStart = appState->_fUseOriginalAspectRatio;
}

CPreferencesDialog::~CPreferencesDialog()
{
}

void CPreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
    ShowSizeGrip(FALSE);
    DDX_Check(pDX, IDC_DRAWGRIDLINES, appState->_fGridLines);
    DDX_Check(pDX, IDC_SCALETRACINGIMAGES, appState->_fScaleTracingImages);
    DDX_Check(pDX, IDC_CHECKAUTOSUGGEST, appState->_fUseAutoSuggest);
    DDX_Check(pDX, IDC_CHECKAUTOLOADGAME, appState->_fAutoLoadGame);
    DDX_Check(pDX, IDC_CHECKDUPENEWCELS, appState->_fDupeNewCels);
    DDX_Check(pDX, IDC_CHECKUSEBOXEGO, appState->_fUseBoxEgo);
    DDX_Check(pDX, IDC_CHECKSCI01, appState->_fShowPolyDotted);

    DDX_Check(pDX, IDC_BROWSEINFO, appState->_fBrowseInfo);
    DDX_Check(pDX, IDC_SCRIPTNAV, appState->_fScriptNav);
    DDX_Check(pDX, IDC_CODECOMPLETION, appState->_fCodeCompletion);
    DDX_Check(pDX, IDC_HOVERTIPS, appState->_fHoverTips);
    DDX_Check(pDX, IDC_COMPILEERRORSOUND, appState->_fPlayCompileErrorSound);
    DDX_Check(pDX, IDC_CHECK_ASPECTRATIO, appState->_fUseOriginalAspectRatio);

    DDX_Text(pDX, IDC_FAKEEGOX, appState->_cxFakeEgo);
    DDV_MinMaxInt(pDX, appState->_cxFakeEgo, 10, 80);
    DDX_Text(pDX, IDC_FAKEEGOY, appState->_cyFakeEgo);
    DDV_MinMaxInt(pDX, appState->_cyFakeEgo, 10, 80);

    // Visuals
    DDX_Control(pDX, IDC_BROWSEINFO, m_wndBrowserInfo);
    DDX_Control(pDX, IDC_CODECOMPLETION, m_wndCodeCompletion);
    DDX_Control(pDX, IDC_SCRIPTNAV, m_wndScriptNav);
    DDX_Control(pDX, IDC_HOVERTIPS, m_wndHoverTips);
    DDX_Control(pDX, IDC_GROUP1, m_wndGroup1);
    DDX_Control(pDX, IDC_SCALETRACINGIMAGES, m_wndCheck1);
    DDX_Control(pDX, IDC_DRAWGRIDLINES, m_wndCheck2);
    DDX_Control(pDX, IDC_CHECKAUTOSUGGEST, m_wndCheck3);
    DDX_Control(pDX, IDC_CHECKAUTOLOADGAME, m_wndCheck4);
    DDX_Control(pDX, IDC_CHECKDUPENEWCELS, m_wndCheck5);
    DDX_Control(pDX, IDC_CHECKUSEBOXEGO, m_wndCheck6);
    DDX_Control(pDX, IDC_CHECKSCI01, m_wndCheck7);
    DDX_Control(pDX, IDC_COMPILEERRORSOUND, m_wndCheck8);
    DDX_Control(pDX, IDC_CHECK_ASPECTRATIO, m_wndCheck9);
    DDX_Control(pDX, IDOK, m_wndOk);
    DDX_Control(pDX, IDCANCEL, m_wndCancel);

    _SyncBrowseInfo();
}

void CPreferencesDialog::_SyncBrowseInfo()
{
    CButton *pCheck = static_cast<CButton*>(GetDlgItem(IDC_BROWSEINFO));
    int iChecked = pCheck->GetCheck();
    GetDlgItem(IDC_SCRIPTNAV)->EnableWindow(iChecked);
    GetDlgItem(IDC_CODECOMPLETION)->EnableWindow(iChecked);
    GetDlgItem(IDC_HOVERTIPS)->EnableWindow(iChecked);
}


BEGIN_MESSAGE_MAP(CPreferencesDialog, CExtResizableDialog)
    ON_BN_CLICKED(IDC_BROWSEINFO, OnBnClickedBrowseinfo)
END_MESSAGE_MAP()


// CPreferencesDialog message handlers

void CPreferencesDialog::OnBnClickedBrowseinfo()
{
    _SyncBrowseInfo();
}

void CPreferencesDialog::OnOK()
{
    __super::OnOK();
    if (_fBrowseInfoStart != appState->_fBrowseInfo)
    {
        if (appState->_fBrowseInfo)
        {
            appState->GenerateBrowseInfo();
        }
    }
    if (_fAspectRatioStart != appState->_fUseOriginalAspectRatio)
    {
        appState->NotifyChangeAspectRatio();
    }
}
