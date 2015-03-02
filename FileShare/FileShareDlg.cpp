
// FileShareDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FileShare.h"
#include "FileShareDlg.h"
#include "SendFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CFileShareDlg 对话框




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


// CFileShareDlg 消息处理程序

BOOL CFileShareDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	LONG lStyle; 
	lStyle = GetWindowLong(m_recvList.m_hWnd, GWL_STYLE);//获取当前窗口style 
	lStyle &= ~LVS_TYPEMASK; //清除显示方式位 
	lStyle |= LVS_REPORT; //设置style 
	SetWindowLong(m_recvList.m_hWnd, GWL_STYLE, lStyle);//设置style 
	DWORD dwStyle = m_recvList.GetExtendedStyle(); 
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl） 
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl） 
	m_recvList.SetExtendedStyle(dwStyle); //设置扩展风格 

	m_recvList.InsertColumn( 0, _T("接收方姓名"), LVCFMT_LEFT, 80 );//插入列 
	m_recvList.InsertColumn( 1, _T("接收方IP"), LVCFMT_LEFT, 100 );
   
	for(int nIndex = 0;nIndex < 256 ; nIndex++)
	{
         m_scanThread[nIndex] = NULL;
	}
	
	getPCName();
	bShutDown = FALSE;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFileShareDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFileShareDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//发送
void CFileShareDlg::OnBnClickedSend()
{
	//测试代码
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

     //发送文件申请
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

	//启动监听
   GetDlgItem(ID_SEND)->EnableWindow(FALSE);
   GetDlgItem(ID_RECV)->EnableWindow(FALSE);
   m_hListenThread = CreateThread(NULL, 0, FindRecverThread, this, 0, NULL);
   m_hRecvSenderThread = CreateThread(NULL, 0, ListenFileSenderThread, this, 0, NULL);
}

//接收
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

     //发送文件申请
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
	// 选择发送方
    
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
				   {   //查找到客户端
                       //创建文件传输对话框
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

	if(dwRet == 0)//客户端上线
	{
         CClientItem *clientItem = (CClientItem *)lParam;
		 int nRow = m_recvList.InsertItem(0,clientItem->m_strClientName);// 插入行
		 m_recvList.SetItemText(nRow, 1, clientItem->m_strIp);//设置其它列数据
	}
	else//客户端离线
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

	//将fileItem添加到链表
	m_fileList.push_back(lpStruct->fileItem);
	delete lpStruct;

	rangeWindow();
	return TRUE;
}

LRESULT CFileShareDlg::RecvFileAcceptMsg(WPARAM wParam, LPARAM lParam)
{
	//开始发送文件
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
	//开始发送文件
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
	//重新刷新列表
	list<CClientItem*>::iterator it;
	for(it = m_clientList.begin();it != m_clientList.end();it++)
	{

         CClientItem* item = (CClientItem*)*it;

		 if(item->bOnline)
		 {
             int nRow = m_recvList.InsertItem(0,item->m_strClientName);// 插入行
             m_recvList.SetItemText(nRow, 1, item->m_strIp);//设置其它列数据
		 }
	}

	//清理发送的文件对话框

	
   return TRUE;
}

LRESULT CFileShareDlg::ConnectServerMsg(WPARAM wParam, LPARAM lParam)
{//没有使用到
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

    //清理列表
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