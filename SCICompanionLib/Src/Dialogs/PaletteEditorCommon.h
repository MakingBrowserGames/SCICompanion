#pragma once

#include "resource.h"
#include "ChooseColorStatic.h"
#include "ResourceEntityDocument.h"
#include "PaletteOperations.h"
#include "AppState.h"
#include "format.h"

template<class T>
class PaletteEditorCommon : public T, public IColorDialogCallback
{
public:
    // TODO: Set the used value as 0x3 or 0x1, depending.

    PaletteEditorCommon<T>(UINT id, CWnd *pwnd) : T(id, pwnd) {}

    void UpdateCommon(CObject *pObject)
    {
        PaletteChangeHint hint = GetHint<PaletteChangeHint>(pObject);
        if (IsFlagSet(hint, PaletteChangeHint::Changed))
        {
            if (_pDoc && _pDoc->GetResource())
            {
                // _pDoc will be set for non-dialog guys (e.g. PaletteView)
                _paletteOwned = std::make_unique<PaletteComponent>(_pDoc->GetResource()->GetComponent<PaletteComponent>());
                _palette = _paletteOwned.get();
                _SyncPalette();
            }
        }
    }

    void Init(PaletteComponent &palette, const std::vector<const Cel*> &cels)
    {
        // This is the path used for dialog-based guys.
        _hasActualUsedColors = !cels.empty();
        if (_hasActualUsedColors)
        {
            CountActualUsedColors(cels, _actualUsedColors);
        }
        _mainSelectedIndex = -1;
        _palette = &palette;
        memcpy(_originalColors, palette.Colors, ARRAYSIZE(_originalColors));
    }

    // IColorDialogCallback
    void OnColorClick(BYTE bIndex, int nID, BOOL fLeftClick)
    {

        _mainSelectedIndex = bIndex;
        _SyncUI();
        _SyncSelection();

        bool multipleSelection[256];
        m_wndStatic.GetMultipleSelection(multipleSelection);
        int countUsed = 0;
        int countUnused = 0;
        int countSelected = 0;
        int countFixed = 0;
        for (int i = 0; i < ARRAYSIZE(multipleSelection); i++)
        {
            if (multipleSelection[i])
            {
                countSelected++;
                if (_palette->Colors[i].rgbReserved)
                {
                    if (_palette->Colors[i].rgbReserved == 0x3)
                    {
                        countFixed++;
                    }
                    countUsed++;
                }
                else
                {
                    countUnused++;
                }
            }
        }

        m_wndUsedCheck.EnableWindow(countSelected > 0);
        m_wndFixedCheck.EnableWindow(countSelected > 0);
        if (countUsed && countUnused)
        {
            m_wndUsedCheck.SetCheck(BST_INDETERMINATE);
        }
        else if (countUsed)
        {
            m_wndUsedCheck.SetCheck(BST_CHECKED);
        }
        else
        {
            m_wndUsedCheck.SetCheck(BST_UNCHECKED);
        }

        if (countFixed && (countFixed == countSelected))
        {
            m_wndFixedCheck.SetCheck(BST_CHECKED);
        }
        else if (countFixed)
        {
            m_wndFixedCheck.SetCheck(BST_INDETERMINATE);
        }
        else
        {
            m_wndFixedCheck.SetCheck(BST_UNCHECKED);
        }

        if (!fLeftClick)
        {
            OnBnClickedButtoneditcolor();
        }
    }

    void SetDocument(CDocument *pDoc)
    {
        _pDoc = static_cast<ResourceEntityDocument*>(pDoc);
        if (_pDoc)
        {
            UpdateCommon(&WrapHint(PaletteChangeHint::Changed));
            //_pDoc->AddNonViewClient(this);
            //std::vector<const Cel*> cels;
            //Init(_pDoc->GetResource()->GetComponent<PaletteComponent>(), cels);
        }
    }

    void DoDataExchange(CDataExchange* pDX) override
    {
        __super::DoDataExchange(pDX);
        DDX_Control(pDX, IDC_STATIC1, m_wndStatic);

        DDX_Control(pDX, IDC_EDITRANGE, m_wndEditRange);

        _SyncPalette();
        bool initialSelection[256] = {};
        m_wndStatic.SetShowHover(false);
        m_wndStatic.SetSelection(initialSelection);
        m_wndStatic.ShowSelection(TRUE);
        m_wndStatic.SetAutoHandleSelection(true);
        m_wndStatic.ShowUnused(true);
        m_wndStatic.SetCallback(this);
        m_wndStatic.SetActualUsedColors(_actualUsedColors);

        DDX_Control(pDX, IDC_CHECK1, m_wndUsedCheck);
        m_wndUsedCheck.EnableWindow(FALSE);

        DDX_Control(pDX, IDC_CHECK2, m_wndFixedCheck);
        m_wndFixedCheck.EnableWindow(FALSE);
        m_wndFixedCheck.ShowWindow(appState->GetVersion().sci11Palettes ? SW_SHOW : SW_HIDE);

        DDX_Control(pDX, IDC_BUTTONEDITCOLOR, m_wndButtonChooseColor);
        m_wndButtonChooseColor.EnableWindow(FALSE);

        // Visuals
        if (GetDlgItem(IDCANCEL))
        {
            DDX_Control(pDX, IDCANCEL, m_wndCancel);
        }
        if (GetDlgItem(IDC_BUTTONREVERT))
        {
            DDX_Control(pDX, IDC_BUTTONREVERT, m_wndRevert);
        }
        DDX_Control(pDX, IDC_EDIT1, m_wndEditDescription);
        DDX_Control(pDX, IDC_BUTTON_SAVE, m_wndSave);
        DDX_Control(pDX, IDC_BUTTON_LOAD, m_wndLoad);

        m_wndEditDescription.SetWindowTextA(
            "Shift-click selects a range of colors. Ctrl-click toggles selection. Right-click opens the color chooser."
            );
    }

protected:
    void _SyncSelection()
    {
        bool multipleSelection[256];
        m_wndStatic.GetMultipleSelection(multipleSelection);
        std::string rangeText;
        // Calculate the ranges
        bool on = false;
        int startRange = 0;
        for (int i = 0; i < 256; i++)
        {
            if (multipleSelection[i] && !on)
            {
                startRange = i;
                on = true;
            }
            if (!multipleSelection[i] && on)
            {
                if (!rangeText.empty())
                {
                    rangeText += ",\r\n";
                }
                int endRange = i - 1;
                if (endRange == startRange)
                {
                    rangeText += fmt::format("{0}", startRange);
                }
                else
                {
                    rangeText += fmt::format("{0}-{1}", startRange, (i - 1));
                }
                on = false;
            }
        }
        m_wndEditRange.SetWindowTextA(rangeText.c_str());
    }

    void _UpdateDocument()
    {
        if (_pDoc)
        {
            _pDoc->ApplyChanges<PaletteComponent>(
                [this](PaletteComponent &palette)
            {
                palette = *this->_palette;
                return WrapHint(PaletteChangeHint::Changed);
            }
                );
        }
    }

    void _SyncPalette()
    {
        if (_palette && m_wndStatic.GetSafeHwnd())
        {
            m_wndStatic.SetPalette(16, 16, reinterpret_cast<const EGACOLOR *>(_palette->Mapping), 256, _palette->Colors, false);
            m_wndStatic.Invalidate(FALSE);
        }
    }

    void _SyncUI()
    {
        m_wndButtonChooseColor.EnableWindow(_mainSelectedIndex != -1);
    }

    DECLARE_MESSAGE_MAP()

    CChooseColorStatic m_wndStatic;
    CExtEdit m_wndEditRange;
    CExtCheckBox m_wndUsedCheck;
    CExtCheckBox m_wndFixedCheck;
    CExtButton m_wndButtonChooseColor;

    PaletteComponent *_palette;
    std::unique_ptr<PaletteComponent> _paletteOwned;   // For the _pDoc scenario where we need a copy of the paletter to work on.
    bool _hasActualUsedColors;
    bool _actualUsedColors[256];
    int _mainSelectedIndex;

    RGBQUAD _originalColors[256];

    // Visuals
    CExtButton m_wndCancel;
    CExtButton m_wndRevert;
    CExtButton m_wndSave;
    CExtButton m_wndLoad;
    CExtEdit m_wndEditDescription;

    ResourceEntityDocument *_pDoc;

public:
    afx_msg void OnBnClickedButtonrevert();
    afx_msg void OnBnClickedCheck1();
    afx_msg void OnBnClickedCheck2();
    afx_msg void OnBnClickedButtoneditcolor();
    afx_msg void OnEnChangeEdit1();
    afx_msg void OnImportPalette();
    afx_msg void OnExportPalette();
};

BEGIN_TEMPLATE_MESSAGE_MAP(PaletteEditorCommon, T, T)
    ON_BN_CLICKED(IDC_BUTTONREVERT, OnBnClickedButtonrevert)
    ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
    ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
    ON_BN_CLICKED(IDC_BUTTONEDITCOLOR, OnBnClickedButtoneditcolor)
    ON_BN_CLICKED(IDC_BUTTON_LOAD, OnImportPalette)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, OnExportPalette)
END_MESSAGE_MAP()

template<class T>
void PaletteEditorCommon<T>::OnBnClickedButtonrevert()
{
    memcpy(_palette->Colors, _originalColors, ARRAYSIZE(_originalColors));
    _SyncPalette();
    m_wndStatic.Invalidate(FALSE);
}

template<class T>
void PaletteEditorCommon<T>::OnBnClickedCheck1()
{
    bool multipleSelection[256];
    m_wndStatic.GetMultipleSelection(multipleSelection);

    int checkValue = m_wndUsedCheck.GetCheck();

    if ((checkValue == BST_CHECKED) || (checkValue == BST_UNCHECKED))
    {
        for (int i = 0; i < ARRAYSIZE(multipleSelection); i++)
        {
            if (multipleSelection[i])
            {
                if (checkValue == BST_CHECKED)
                {
                    _palette->Colors[i].rgbReserved |= 0x1;
                }
                else
                {
                    _palette->Colors[i].rgbReserved = 0;
                }
                
            }
        }
    }
    _SyncPalette();
    _UpdateDocument();
    m_wndStatic.Invalidate(FALSE);
}

template<class T>
void PaletteEditorCommon<T>::OnBnClickedCheck2()
{
    bool multipleSelection[256];
    m_wndStatic.GetMultipleSelection(multipleSelection);

    int checkValue = m_wndFixedCheck.GetCheck();

    if ((checkValue == BST_CHECKED) || (checkValue == BST_UNCHECKED))
    {
        for (int i = 0; i < ARRAYSIZE(multipleSelection); i++)
        {
            if (multipleSelection[i])
            {
                if (checkValue == BST_CHECKED)
                {
                    _palette->Colors[i].rgbReserved = 0x3;
                }
                else
                {
                    _palette->Colors[i].rgbReserved &= ~0x2;
                }
            }
        }
    }
    _SyncPalette();
    _UpdateDocument();
    m_wndStatic.Invalidate(FALSE);
}

template<class T>
void PaletteEditorCommon<T>::OnBnClickedButtoneditcolor()
{
    if (_mainSelectedIndex != -1)
    {
        uint8_t usedValue = _palette->Colors[_mainSelectedIndex].rgbReserved;
        COLORREF color = RGB_TO_COLORREF(_palette->Colors[_mainSelectedIndex]);
        CExtColorDlg colorDialog(color, color);
        colorDialog.m_strCaption = fmt::format("Edit color {0}", _mainSelectedIndex).c_str();
        if (IDOK == colorDialog.DoModal())
        {
            _palette->Colors[_mainSelectedIndex] = RGBQUADFromCOLORREF(colorDialog.m_clrNew);
            _palette->Colors[_mainSelectedIndex].rgbReserved = usedValue;
            _UpdateDocument();
            _SyncPalette();
        }
    }
}

template<class T>
void PaletteEditorCommon<T>::OnImportPalette()
{
    CFileDialog fileDialog(TRUE, nullptr, nullptr, 0, "PAL files (*.pal)|*.pal|All Files|*.*|" );
    if (IDOK == fileDialog.DoModal())
    {
        CString strFileName = fileDialog.GetPathName();
        try
        {
            LoadPALFile((PCSTR)strFileName, *_palette);
            _UpdateDocument();
        }
        catch (std::exception &e)
        {
            AfxMessageBox(e.what(), MB_OK | MB_ICONWARNING);
        }
    }
}


template<class T>
void PaletteEditorCommon<T>::OnExportPalette()
{
    // TODO: http://worms2d.info/Palette_file#File_format
}
