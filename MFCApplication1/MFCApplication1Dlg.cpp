
// MFCApplication1Dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"

#include "WinDriverUtility.h"

#include "Utility\Observer.h"
#include "UtilityDialog\DialogUtility.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMfcWinDriverDlg 對話方塊



CMfcWinDriverDlg::CMfcWinDriverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMfcWinDriverDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_txtMsg, m_txtMsg);
}

BEGIN_MESSAGE_MAP(CMfcWinDriverDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CMfcWinDriverDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMfcWinDriverDlg 訊息處理常式
DialogUtility m_dlgDu;
CEdit* m_dlgMsg;
void dialog_show_message(estring_r tmsg, bool isClean) {
    m_dlgDu.show_txt_msg(m_dlgMsg, isClean, tmsg);
}

BOOL CMfcWinDriverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定
    m_dlgMsg = &this->m_txtMsg;
    Observer::observerRegister(0, dialog_show_message);

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CMfcWinDriverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CMfcWinDriverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMfcWinDriverDlg::OnBnClickedOk() {
    
    WinDriverUtility wdu;
    wdu.run();
}
