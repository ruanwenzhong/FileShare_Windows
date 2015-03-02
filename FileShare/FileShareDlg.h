
// FileShareDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "Inc.h"


// CFileShareDlg 对话框
class CFileShareDlg : public CDialog
{
// 构造
public:
	CFileShareDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_FILESHARE_DIALOG };//IDD_FILESHARE_DIALOG

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedRecv();
	CListCtrl m_recvList;
	afx_msg void OnNMDblclkClientList(NMHDR *pNMHDR, LRESULT *pResult);
    LRESULT FindClientMsg(WPARAM wParam, LPARAM lParam);
    LRESULT RecvFileRequestMsg(WPARAM wParam, LPARAM lParam);
    LRESULT RecvFileAcceptMsg(WPARAM wParam, LPARAM lParam);
    LRESULT RecvFileRefusedMsg(WPARAM wParam, LPARAM lParam);
	LRESULT RecvFileCancelMsg(WPARAM wParam, LPARAM lParam);
    LRESULT RefreshClientListMsg(WPARAM wParam, LPARAM lParam);
	LRESULT ConnectServerMsg(WPARAM wParam, LPARAM lParam);

public:
    SOCKET m_ListenSock;
	SOCKET m_ListenFileSock;
	SOCKET m_ConnectSock;
	BOOL bShutDown;
	BOOL bIsSender;

	HANDLE m_hListenThread;
	HANDLE m_hRecvThread;
	HANDLE m_hRecvSenderThread;
	HANDLE m_scanThread[256];
    CString m_strPCName;

    void getPCName();
	void selectFile(CClientItem *item);
	void rangeWindow();

	list<CFileItem*>  m_fileList;
	list<CClientItem*> m_clientList;
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnHistory();
	afx_msg void OnBnClickedReflesh();
};
