#pragma once
#include "Inc.h"
#include "afxcmn.h"


// CSendFileDlg �Ի���

class CSendFileDlg : public CDialog
{
	DECLARE_DYNAMIC(CSendFileDlg)

public:
	CSendFileDlg(CClientItem* clientItem,CFileItem* fileItem,BOOL bSender,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSendFileDlg();

// �Ի�������
	enum { IDD = IDD_SENDFILE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

    CClientItem* m_clientItem;
    CFileItem*    m_fileItem;
	BOOL            m_sender;//TRUE:������ FALSE:������
	BOOL            m_bClose;
	DWORD       bytesSent;
	CString          m_strFileSize;
    long               lastBytes;
	DWORD       lastTime;

	CProgressCtrl m_progress;
	HANDLE m_hSendThread;
	HANDLE m_hRecvThread;

	void beginSendFileData();
	void sendFileData();
	CString getFileSize(long length);
	void sendFileRequest();

	afx_msg void OnBnClickedAcceptBtn();
	LRESULT FileProgressMsg(WPARAM wParam, LPARAM lParam);
	LRESULT BeginSendFileMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnBnClickedRefuseBtn();
	afx_msg void OnNcDestroy();
};
