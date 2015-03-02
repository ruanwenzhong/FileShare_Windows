
// FileShareDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "Inc.h"


// CFileShareDlg �Ի���
class CFileShareDlg : public CDialog
{
// ����
public:
	CFileShareDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_FILESHARE_DIALOG };//IDD_FILESHARE_DIALOG

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
