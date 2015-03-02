
// FileShareDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "FileShare.h"
#include "FileShareDlg.h"
#include "SendFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFileShareDlg �Ի���




CFileShareDlg::CFileShareDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileShareDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFileShareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIENT_LIST, m_recvList);
}

BEGIN_MESSAGE_MAP(CFileShareDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_SEND, &CFileShareDlg::OnBnClickedSend)
	ON_BN_CLICKED(ID_RECV, &CFileShareDlg::OnBnClickedRecv)
	ON_NOTIFY(NM_DBLCLK, IDC_CLIENT_LIST, &CFileShareDlg::OnNMDblclkClientList)
	ON_MESSAGE(WM_FINDCLIENT_MSG,CFileShareDlg::FindClientMsg)
	ON_MESSAGE(WM_RECVFILEREQUEST_MSG,CFileShareDlg::RecvFileRequestMsg)
	ON_MESSAGE(WM_RECVFILEACCEPT_MSG,CFileShareDlg::RecvFileAcceptMsg)
	ON_MESSAGE(WM_FILE_REFUSED,CFileShareDlg::RecvFileRefusedMsg)
	ON_MESSAGE(WM_REFLESH_LIST,CFileShareDlg::RefreshClientListMsg)
	ON_MESSAGE(WM_FILE_CANCEL,CFileShareDlg::RecvFileCancelMsg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_HISTORY, &CFileShareDlg::OnBnClickedBtnHistory)
	ON_BN_CLICKED(IDC_REFLESH, &CFileShareDlg::OnBnClickedReflesh)
END_MESSAGE_MAP()


// CFileShareDlg ��Ϣ�������

BOOL CFileShareDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	LONG lStyle; 
	lStyle = GetWindowLong(m_recvList.m_hWnd, GWL_STYLE);//��ȡ��ǰ����style 
	lStyle &= ~LVS_TYPEMASK; //�����ʾ��ʽλ 
	lStyle |= LVS_REPORT; //����style 
	SetWindowLong(m_recvList.m_hWnd, GWL_STYLE, lStyle);//����style 
	DWORD dwStyle = m_recvList.GetExtendedStyle(); 
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl�� 
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl�� 
	m_recvList.SetExtendedStyle(dwStyle); //������չ��� 

	m_recvList.InsertColumn( 0, _T("���շ�����"), LVCFMT_LEFT, 80 );//������ 
	m_recvList.InsertColumn( 1, _T("���շ�IP"), LVCFMT_LEFT, 100 );
   
	for(int nIndex = 0;nIndex < 256 ; nIndex++)
	{
         m_scanThread[nIndex] = NULL;
	}
	
	getPCName();
	bShutDown = FALSE;
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CFileShareDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CFileShareDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CFileShareDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//����
void CFileShareDlg::OnBnClickedSend()
{
	//���Դ���
 //   wchar_t src[MAX_BUF_SIZE];
	//char recvBuf[MAX_BUF_SIZE];
	//int store = 0;
	//memset(src,0,MAX_BUF_SIZE * sizeof(wchar_t));
 //   wcscpy_s(src,MAX_BUF_SIZE,L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
	//						   <XinZongMsg Version=\"1.0\" Type=\"HELLO_REPLY\">\
	//						   <ClientName>PC</ClientName></XinZongMsg><?xml version=\"1.0\" encoding=\"UTF-8\"?>\
	//						   <XinZongMsg Version=\"1.0\" Type=\"HELLO_REPLY\">\
	//						   <ClientName>PC</ClientName></XinZongMsg>\r\n");
	//splitRecvData(src,wcslen(src),recvBuf,&store);

/*
     CString strPathName,strFileName;
	 long intfileSize = 0;

     //�����ļ�����
	CFileDialog dlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT, _T("*.*|*.*||"), this);

    DWORD MAXFILE = 4000;
    dlg.m_ofn.nMaxFile = MAXFILE;
    wchar_t* pc = new wchar_t[MAXFILE];
    dlg.m_ofn.lpstrFile = pc;
    dlg.m_ofn.lpstrFile[0] = NULL;

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	POSITION pos = dlg.GetStartPosition();
    while (pos != NULL)
    {
		 CFileItem* fileItem = new CFileItem();

		 fileItem->strFilePath =  dlg.GetNextPathName(pos);
		 fileItem->strFileName = fileItem->strFilePath.Right(fileItem->strFilePath.GetLength() -  fileItem->strFilePath.ReverseFind('\\') - 1);

		 FILE *stream;
	     if( (stream = _wfsopen( fileItem->strFilePath, _T("rb"), _SH_DENYNO )) != NULL )
		{
			fseek(stream,   0,   SEEK_END);
			intfileSize = ftell(stream);
			fseek(stream,0,SEEK_SET);
			fileItem->longFileLength = intfileSize;
			fclose( stream );
		}
		 CTime t = CTime::GetCurrentTime();
		 CString szTime = t.Format("%Y%m%d%H%M%S");
		 CString strFileID;
		 strFileID.Format(_T("%s%d"),szTime,fileItem->longFileLength);
		 fileItem->strFileID = strFileID;
		 m_fileList.push_back(fileItem);

    }
*/
	m_hRecvThread =  CreateThread(NULL, 0, ListenSenderThread, this, 0, NULL);

	//��������
   GetDlgItem(ID_SEND)->EnableWindow(FALSE);
   GetDlgItem(ID_RECV)->EnableWindow(FALSE);
   m_hListenThread = CreateThread(NULL, 0, FindRecverThread, this, 0, NULL);
   m_hRecvSenderThread = CreateThread(NULL, 0, ListenFileSenderThread, this, 0, NULL);
}

//����
void CFileShareDlg::OnBnClickedRecv()
{
	 m_hRecvThread =  CreateThread(NULL, 0, ListenSenderThread, this, 0, NULL);

     //getPCName();
	 GetDlgItem(ID_SEND)->EnableWindow(FALSE);
     GetDlgItem(ID_RECV)->EnableWindow(FALSE);
     m_hListenThread = CreateThread(NULL, 0, FindRecverThread, this, 0, NULL);
     m_hRecvSenderThread = CreateThread(NULL, 0, ListenFileSenderThread, this, 0, NULL); 
}

void CFileShareDlg::selectFile(CClientItem *clientItem)
{
       CString strPathName,strFileName;
	 long intfileSize = 0;

     //�����ļ�����
	CFileDialog dlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT, _T("*.*|*.*||"), this);

    DWORD MAXFILE = 4000;
    dlg.m_ofn.nMaxFile = MAXFILE;
    wchar_t* pc = new wchar_t[MAXFILE];
    dlg.m_ofn.lpstrFile = pc;
    dlg.m_ofn.lpstrFile[0] = NULL;

	if (dlg.DoModal() != IDOK)
	{
		delete pc;
		return;
	}

	POSITION pos = dlg.GetStartPosition();
    while (pos != NULL)
    {
		 CFileItem* fileItem = new CFileItem();

		 fileItem->strFilePath =  dlg.GetNextPathName(pos);
		 fileItem->strFileName = fileItem->strFilePath.Right(fileItem->strFilePath.GetLength() -  fileItem->strFilePath.ReverseFind('\\') - 1);

		 FILE *stream;
	     if( (stream = _wfsopen( fileItem->strFilePath, _T("rb"), _SH_DENYNO )) != NULL )
		{
			fseek(stream,   0,   SEEK_END);
			intfileSize = ftell(stream);
			fseek(stream,0,SEEK_SET);
			fileItem->longFileLength = intfileSize;
			fclose( stream );
		}

		 CTime t = CTime::GetCurrentTime();
		 CString szTime = t.Format("%Y%m%d%H%M%S");
		 CString strFileID;
		 strFileID.Format(_T("%s%d"),szTime,fileItem->longFileLength);
		 fileItem->strFileID = strFileID;
		 m_fileList.push_back(fileItem);

		CSendFileDlg *fileDlg = new CSendFileDlg(clientItem,fileItem,TRUE,this);
		fileItem->m_pFileDlg = fileDlg;

		Sleep(1000);
    }

	delete pc;
	rangeWindow();
}
	
void CFileShareDlg::OnNMDblclkClientList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// ѡ���ͷ�
    
   DWORD dwPos = GetMessagePos(); 
   CPoint point( LOWORD(dwPos), HIWORD(dwPos) ); 
   m_recvList.ScreenToClient(&point); 
   LVHITTESTINFO lvinfo; 
   lvinfo.pt = point; 
   lvinfo.flags = LVHT_ABOVE; 
   int nItem = m_recvList.SubItemHitTest(&lvinfo); 
   if(nItem != -1) 
   { 
         CString strDeviceName = m_recvList.GetItemText(lvinfo.iItem,0);
 
         list<CClientItem* >::iterator it;
		 for(it = m_clientList.begin();it != m_clientList.end();it++)
		  {
               CClientItem* tempItem = (CClientItem*) *it;
			   if(tempItem)
			   {
				   if(tempItem->m_strClientName == strDeviceName)
				   {   //���ҵ��ͻ���
                       //�����ļ�����Ի���
					   selectFile(tempItem);
					 //   list<CFileItem* >::iterator it;
						//for(it = m_fileList.begin(); it != m_fileList.end();it++)
						//{
      //                       CFileItem* itFileDlg = (CFileItem*) *it;
					 //        CSendFileDlg *fileDlg = new CSendFileDlg(tempItem,itFileDlg,TRUE,this);
						//	 itFileDlg->m_pFileDlg = fileDlg;

						//}
				   }
			   }
		  }
   } 

	*pResult = 0;
}

LRESULT CFileShareDlg::FindClientMsg(WPARAM wParam, LPARAM lParam)
{
	m_recvList.ShowWindow(SW_SHOW);

	DWORD dwRet = (DWORD)wParam;

	if(dwRet == 0)//�ͻ�������
	{
         CClientItem *clientItem = (CClientItem *)lParam;
		 int nRow = m_recvList.InsertItem(0,clientItem->m_strClientName);// ������
		 m_recvList.SetItemText(nRow, 1, clientItem->m_strIp);//��������������
	}
	else//�ͻ�������
	{
         
	}

	return TRUE;
}

LRESULT CFileShareDlg::RecvFileRequestMsg(WPARAM wParam, LPARAM lParam)
{
    CClientItem_FileItem* lpStruct = (CClientItem_FileItem*)lParam;
    ASSERT(lpStruct != NULL);

	CSendFileDlg *fileDlg = new CSendFileDlg(lpStruct->clientItem,lpStruct->fileItem,FALSE,this);
	lpStruct->fileItem->m_pFileDlg = fileDlg;

	//��fileItem��ӵ�����
	m_fileList.push_back(lpStruct->fileItem);
	delete lpStruct;

	rangeWindow();
	return TRUE;
}

LRESULT CFileShareDlg::RecvFileAcceptMsg(WPARAM wParam, LPARAM lParam)
{
	//��ʼ�����ļ�
    TCHAR* pstrFileID = (TCHAR*) lParam;
     
     list<CFileItem* >::iterator it;
	 for(it = m_fileList.begin(); it != m_fileList.end();it++)
	 {
         CFileItem* itFileDlg = (CFileItem*) *it;
		 if(itFileDlg  && itFileDlg->strFileID ==  pstrFileID)
		 {
		     itFileDlg->bAccept = TRUE;
			 itFileDlg->m_pFileDlg->beginSendFileData();
		 }

	 }
    
	delete pstrFileID;
	return TRUE;
}

LRESULT CFileShareDlg::RecvFileRefusedMsg(WPARAM wParam, LPARAM lParam)
{
	//��ʼ�����ļ�
    TCHAR* pstrFileID = (TCHAR*) lParam;
     
     list<CFileItem* >::iterator it;
	 for(it = m_fileList.begin(); it != m_fileList.end();it++)
	 {
         CFileItem* itFileDlg = (CFileItem*) *it;
		 if(itFileDlg  && itFileDlg->strFileID ==  pstrFileID)
		 {
			// itFileDlg->m_pFileDlg->OnClose();
			 ::PostMessage(itFileDlg->m_pFileDlg->GetSafeHwnd(),WM_CLOSE,0,0);
			 break;
		 }

	 }
    
	delete pstrFileID;
	return TRUE;
}

LRESULT CFileShareDlg::RecvFileCancelMsg(WPARAM wParam, LPARAM lParam)
{
    TCHAR* pstrFileID = (TCHAR*) lParam;
     
     list<CFileItem* >::iterator it;
	 for(it = m_fileList.begin(); it != m_fileList.end();it++)
	 {
         CFileItem* itFileDlg = (CFileItem*) *it;
		 if(itFileDlg  && itFileDlg->strFileID ==  pstrFileID)
		 {
			 ::PostMessage(itFileDlg->m_pFileDlg->GetSafeHwnd(),WM_CLOSE,0,0);
			 break;
		 }

	 }
    
	delete pstrFileID;
	return TRUE;
}

LRESULT CFileShareDlg::RefreshClientListMsg(WPARAM wParam, LPARAM lParam)
{
	CClientItem* clientItem = (CClientItem*)lParam;

    m_recvList.DeleteAllItems();
	//����ˢ���б�
	list<CClientItem*>::iterator it;
	for(it = m_clientList.begin();it != m_clientList.end();it++)
	{

         CClientItem* item = (CClientItem*)*it;

		 if(item->bOnline)
		 {
             int nRow = m_recvList.InsertItem(0,item->m_strClientName);// ������
             m_recvList.SetItemText(nRow, 1, item->m_strIp);//��������������
		 }
	}

	//�����͵��ļ��Ի���

	
   return TRUE;
}

LRESULT CFileShareDlg::ConnectServerMsg(WPARAM wParam, LPARAM lParam)
{//û��ʹ�õ�
     //int sock = (int)wParam;
	 //CString strSrvIp = (CString) *lParam;
	return TRUE;
}

void CFileShareDlg::OnClose()
{
    bShutDown = TRUE;

	if(m_ListenSock)
		closesocket(m_ListenSock);
	if(m_ListenFileSock)
		closesocket(m_ListenFileSock);
	if(m_ConnectSock)
		closesocket(m_ConnectSock);

    if(m_hRecvThread)
	{
          WaitForSingleObject(m_hRecvThread,INFINITE);
	}

	if(m_hListenThread)
	{
          WaitForSingleObject(m_hListenThread,INFINITE);
	}

	if(m_hRecvSenderThread)
	{
         WaitForSingleObject(m_hRecvSenderThread,INFINITE); 
	}

    //�����б�
	list<CClientItem *>::iterator it;
	for(it = m_clientList.begin(); it != m_clientList.end(); it++)
	{
          CClientItem *item = (CClientItem *)*it;
		  if(item)
		  {
			  delete item;
			  item = NULL;
		  }
	}
	m_clientList.clear();
	
	list<CFileItem*>::iterator fileItor;
	for(fileItor = m_fileList.begin(); fileItor != m_fileList.end(); fileItor++)
	{
         CFileItem *item  = (CFileItem *) *fileItor;
		 if(item)
		 {
              delete item;
			  item = NULL;
		 }
	}
	m_fileList.clear();

	CDialog::OnClose();
}

void CFileShareDlg::getPCName()
{
     TCHAR  szBuffer[MAX_PATH];  
     DWORD dwNameLen;  
  
    dwNameLen = MAX_PATH;  
    if (GetComputerName(szBuffer, &dwNameLen))   
	{
         m_strPCName = szBuffer;
	}
    else  
	{
          m_strPCName = "unknow device";
	}
}

void CFileShareDlg::rangeWindow()
{
	CRect windowRect;
	this->GetWindowRect(&windowRect);
	DWORD rectTop = windowRect.top;
	DWORD rectRight = windowRect.right;

     int nIndex = 0;
     list<CFileItem* >::iterator it;
	 for(it = m_fileList.begin(); it != m_fileList.end();it++)
	 {
         CFileItem* itFileDlg = (CFileItem*) *it;
         
		 ::MoveWindow(itFileDlg->m_pFileDlg->GetSafeHwnd(),rectRight,rectTop +  147 * nIndex,315,147,TRUE); 
		 nIndex++;
	 }
}
void CFileShareDlg::OnBnClickedBtnHistory()
{

}

void CFileShareDlg::OnBnClickedReflesh()
{
	m_hListenThread = CreateThread(NULL, 0, FindRecverThread, this, 0, NULL);
}