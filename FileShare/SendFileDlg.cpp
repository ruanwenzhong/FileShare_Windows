// SendFileDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FileShare.h"
#include "SendFileDlg.h"
#include "FileShareDlg.h"


// CSendFileDlg 对话框

IMPLEMENT_DYNAMIC(CSendFileDlg, CDialog)

CSendFileDlg::CSendFileDlg(CClientItem* clientItem,CFileItem* fileItem,BOOL bSender,CWnd* pParent /*=NULL*/)
	: CDialog(CSendFileDlg::IDD, pParent)
{
	   m_clientItem =  clientItem;
	   m_fileItem = fileItem;
       m_sender = bSender;
       m_bClose = FALSE;
	   bytesSent = 0;
	   lastBytes = 0;
       lastTime = 0;

       Create(IDD,pParent);
}

CSendFileDlg::~CSendFileDlg()
{
}

void CSendFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
}


BEGIN_MESSAGE_MAP(CSendFileDlg, CDialog)
	ON_BN_CLICKED(ID_ACCEPT_BTN, &CSendFileDlg::OnBnClickedAcceptBtn)
	ON_MESSAGE(WM_FILEPROGRESS_MSG,&CSendFileDlg::FileProgressMsg)
	ON_MESSAGE(WM_BEGINSENDFILE_MSG,&CSendFileDlg::BeginSendFileMsg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(ID_REFUSE_BTN, &CSendFileDlg::OnBnClickedRefuseBtn)
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CSendFileDlg 消息处理程序

BOOL CSendFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    ShowWindow(SW_SHOW);
    
	if(m_fileItem)
	{
		m_strFileSize = getFileSize(m_fileItem->longFileLength);
		CString strDisplayName;
		strDisplayName.Format(_T("%s(%s)"),m_fileItem->strFileName,m_strFileSize);
		GetDlgItem(IDC_STATIC_FILENAME)->SetWindowText(strDisplayName);

		if(m_sender)//发送方
		{
			GetDlgItem(ID_ACCEPT_BTN)->ShowWindow(SW_HIDE);
			GetDlgItem(ID_REFUSE_BTN)->SetWindowText(_T("取消"));
			sendFileRequest();
		}
		else //接收方
		{
             GetDlgItem(ID_ACCEPT_BTN)->SetWindowText(_T("接收"));
			 GetDlgItem(ID_REFUSE_BTN)->SetWindowText(_T("拒绝"));
		}

		m_progress.SetRange32(0,m_fileItem->longFileLength);
		SetWindowText(m_fileItem->strFileName);
	}

	//CRect rect;
	//GetWindowRect(&rect);
    
	return TRUE;  // return TRUE unless you set the focus to a control
}


void CSendFileDlg::sendFileRequest()
{
	if(m_fileItem)
	{
	   CString strFileRequest;
	   strFileRequest.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"FILE_REQUEST\"><FileTransferID>%s</FileTransferID ><FileName>%s</FileName><FileSize>%d</FileSize></XinZongMsg>\r\n"),
	   m_fileItem->strFileID,m_fileItem->strFileName,m_fileItem->longFileLength);

	   if(m_clientItem)
	   {
		   sendBytes(m_clientItem->m_Socket,strFileRequest.GetBuffer(0));
		   strFileRequest.ReleaseBuffer();
	   }
	  
	}

}


//接收
void CSendFileDlg::OnBnClickedAcceptBtn()
{
     if(m_fileItem)
	 {
		 CFileDialog dlg(FALSE, NULL,m_fileItem->strFileName,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"*.*|*.*||", this);
		if (dlg.DoModal() != IDOK)
			return;

		GetDlgItem(ID_ACCEPT_BTN)->ShowWindow(SW_HIDE);
		m_fileItem->strFilePath = dlg.GetPathName();     //+ _T("\\") + m_fileItem->strFileName;

		//回复接收消息
        CString strFileRequest;
	    strFileRequest.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"FILE_ACCEPT\"><FileTransferID>%s</FileTransferID ><FileName>%s</FileName><FileSize>%d</FileSize></XinZongMsg>\r\n"),
	    m_fileItem->strFileID,m_fileItem->strFileName,m_fileItem->longFileLength);

	    if(m_clientItem)
	    {
		    sendBytes(m_clientItem->m_Socket,strFileRequest.GetBuffer(0));
		    strFileRequest.ReleaseBuffer();
	    }

		//开始接收文件
       //m_hRecvThread = CreateThread(NULL, 0, RecvFileThread, this, 0, NULL);
	 }
}

//开始发送文件数据
void CSendFileDlg::beginSendFileData()
{
     GetDlgItem(ID_REFUSE_BTN)->SetWindowText(_T("中断"));

     if(m_clientItem)
	{
		  m_hSendThread = CreateThread(NULL, 0, SendFileThread, this, 0, NULL);
	}
}

CString CSendFileDlg::getFileSize(long length)
{
    CString strSize;
	if(length < 1024)
	{
		 strSize.Format(_T("%db"),length);
	}
	else if(length >= 1024 && length < 1024 * 1024 )
	{
		strSize.Format(_T("%.2fKB"),(length * 1.0)/1024);
	}
	else if(length >= 1024 * 1024 && length < 1024 * 1024 * 1024 )
	{
		strSize.Format(_T("%.2fMB"),(length * 1.0)/(1024*1024));
	}

	return strSize;
}

LRESULT CSendFileDlg::FileProgressMsg(WPARAM wParam, LPARAM lParam)
{
	 m_progress.SetPos(bytesSent);

    CString strRecvSize = getFileSize(bytesSent);
	CString strDisplayRecv;
	strDisplayRecv.Format(_T("%s/%s"),strRecvSize,m_strFileSize);
	GetDlgItem(IDC_STATIC_RECVBYTE)->SetWindowText(strDisplayRecv);

	 DWORD thisTime = GetTickCount();
	 if(lastBytes == 0)
	 {
           lastTime = thisTime;
	 }
	 else
	 {
           if (thisTime - lastTime >= 500)
		   {
				float speed = (float) (bytesSent - lastBytes) / (thisTime - lastTime) / 1.024f;
				lastTime = thisTime;
				lastBytes = bytesSent;

				CString strSpeed;
				strSpeed.Format(L"%.1fKB/S", speed);
                GetDlgItem(IDC_STATIC_FILE_SPEED)->SetWindowText(strSpeed);
		   }
	 }

     lastBytes = bytesSent;
	 UpdateData(FALSE);
	 return 0;
}

LRESULT CSendFileDlg::BeginSendFileMsg(WPARAM wParam, LPARAM lParam)
{
    CClientItem *item = (CClientItem *)lParam;
    
	if(m_clientItem)
	{
          m_clientItem = item;
		  m_hSendThread = CreateThread(NULL, 0, SendFileThread, this, 0, NULL);
	}

	return 0;
}

void CSendFileDlg::OnClose()
{
	m_bClose = TRUE;
    //收到关闭通知  执行清理操作
    if(m_clientItem && m_fileItem)
	{
		list<CFileItem*>::iterator it;
		for(it = m_clientItem->m_pMainWnd->m_fileList.begin(); it != m_clientItem->m_pMainWnd->m_fileList.end(); it++)
		{
               CFileItem *item = (CFileItem*) *it;
			   if(item && item->strFileID  == m_fileItem->strFileID)
			   {
				   m_clientItem->m_pMainWnd->m_fileList.remove(m_fileItem);
				   delete m_fileItem;
				   break;
			   }
		}
	}
    

	CDialog::OnClose();
}

void CSendFileDlg::OnBnClickedRefuseBtn()
{
	  if(m_fileItem && !m_sender) //拒收文件
	 {
          //回复接收消息
        CString strFileRequest;
	    strFileRequest.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"FILE_REFUSED\"><FileTransferID>%s</FileTransferID ><FileName>%s</FileName><FileSize>%d</FileSize></XinZongMsg>\r\n"),
	    m_fileItem->strFileID,m_fileItem->strFileName,m_fileItem->longFileLength);

	    if(m_clientItem)
	    {
		    sendBytes(m_clientItem->m_Socket,strFileRequest.GetBuffer(0));
		    strFileRequest.ReleaseBuffer();
	    }

		PostMessage(WM_CLOSE,0,0);
		//OnClose();
	 }
	  else if(m_fileItem && m_sender)//取消文件传输
	  {
            m_bClose = TRUE;

	        //传输文件取消消息
			CString strFileRequest;
			strFileRequest.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"FILE_CANCEL\"><FileTransferID>%s</FileTransferID ><FileName>%s</FileName><FileSize>%d</FileSize></XinZongMsg>\r\n"),
			m_fileItem->strFileID,m_fileItem->strFileName,m_fileItem->longFileLength);

			if(m_clientItem)
			{
				sendBytes(m_clientItem->m_Socket,strFileRequest.GetBuffer(0));
				strFileRequest.ReleaseBuffer();
			}

            PostMessage(WM_CLOSE,0,0);
	  }
}

void CSendFileDlg::OnNcDestroy()
{
	CDialog::OnNcDestroy();

	delete this;
}
