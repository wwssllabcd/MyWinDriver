
// MFCApplication1Dlg.h : ���Y��
//

#pragma once
#include "afxwin.h"


// CMfcWinDriverDlg ��ܤ��
class CMfcWinDriverDlg : public CDialogEx
{
// �غc
public:
	CMfcWinDriverDlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
protected:
	HICON m_hIcon;

	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    CEdit m_txtMsg;
};
