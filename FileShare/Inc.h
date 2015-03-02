#pragma once
class CFileShareDlg;
class CSendFileDlg;

#define MAX_BUF_SIZE 1024
#define TCP_PACKET_SIZE  2048

#define WM_FINDCLIENT_MSG                   (WM_USER+100)
#define WM_RECVFILEREQUEST_MSG    (WM_USER + 101)
#define WM_RECVFILEACCEPT_MSG       (WM_USER + 102)
#define WM_FILEPROGRESS_MSG            (WM_USER + 103)
#define WM_BEGINSENDFILE_MSG           (WM_USER + 104)
#define WM_FILE_REFUSED                        (WM_USER + 105)
#define WM_REFLESH_LIST                         (WM_USER + 106)
#define WM_FILE_CANCEL                            (WM_USER + 107)
#define WM_CONNECT_SERVER                 (WM_USER + 108)

#define  LISTEN_CLIENT_PORT           9000
#define  LISTEN_FILE_PORT                9001
#define  FIND_COUNT                             5

class CClientItem {
public:
	CString m_strIp;
	SOCKET m_Socket;
	HANDLE hThread;                          //连接线程
	CFileShareDlg *m_pMainWnd;
	CString m_strClientName;
	BOOL bOnline;
	BOOL bDisconnect;
	int intConnectType;  //0:消息   1:文件数据

	CClientItem(){
		m_pMainWnd = NULL;
		m_Socket = INVALID_SOCKET;
		hThread = NULL;
		bOnline = FALSE;
		bDisconnect = FALSE;
		intConnectType = 0;
	}
};

class CFileItem{

public:
	CString strFileName;
	CString strFilePath;
	CString strFileID;
	long       longFileLength;
	BOOL   bAccept;

    CSendFileDlg *m_pFileDlg;

   CFileItem()
   {
         m_pFileDlg = NULL;
         longFileLength = 0;
		 bAccept = FALSE;
   }
};

class CClientItem_FileItem{
    
public:
	CClientItem* clientItem;
    CFileItem* fileItem;

    CClientItem_FileItem()
	{
         clientItem = NULL;
         fileItem = NULL;
	}
};

class CThreadStruct{
public:
     CFileShareDlg *m_pMainWnd;
	 CString strSrvIp;

    CThreadStruct()
	{
         m_pMainWnd = NULL;
	}
};

DWORD WINAPI FindRecverThread(LPVOID pParam);
DWORD WINAPI ConnectRecverThread(LPVOID pParam);
DWORD WINAPI ListenSenderThread(LPVOID pParam);
DWORD WINAPI ListenFileSenderThread(LPVOID pParam);
DWORD WINAPI ClientThreadProc(LPVOID lpParameter);   
DWORD WINAPI ConnectServerThead(LPVOID pParam);

DWORD WINAPI SendFileThread(LPVOID pParam);
DWORD WINAPI RecvFileThread(LPVOID pParam);

BOOL SOCKET_Select(SOCKET hSocket, int nTimeOut = 100, BOOL bRead = FALSE);
BOOL sendBytes(SOCKET hSock,TCHAR* data );
int SendData(const SOCKET sock,const char *sendData,int len);
int  splitRecvData(wchar_t *src,DWORD len,char* szBuf,DWORD *store,CClientItem *);

BOOL resolveXml(wchar_t* xml,CClientItem* clientItem);
CString getXmlType(wchar_t* xml);
CString resolveFileSendXml(wchar_t* strXml,CClientItem* clientItem);
void clean_string(char *str);

CString getHostIP();