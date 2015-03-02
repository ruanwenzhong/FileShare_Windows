
#include "stdafx.h"
#include "Inc.h"
#include "TransformChar.h"
#include "FileShareDlg.h"
#include "SendFileDlg.h"

BOOL SOCKET_Select(SOCKET hSocket, int nTimeOut, BOOL bRead)
{
	fd_set fdset;
	timeval tv;
	FD_ZERO(&fdset);
	FD_SET(hSocket, &fdset);
	nTimeOut = nTimeOut > 1000 ? 1000 : nTimeOut;
	tv.tv_sec  = 0;
	tv.tv_usec = nTimeOut;

	int iRet = 0;
	if ( bRead ) {
		iRet = select(0, &fdset, NULL , NULL, &tv);
	}else{
		iRet = select(0, NULL , &fdset, NULL, &tv);
	}

	if(iRet <= 0) {
		return FALSE;
	} else if (FD_ISSET(hSocket, &fdset)){
		return TRUE;
	}
	return FALSE;
}

BOOL  sendBytes(SOCKET hSock,TCHAR* data )
{   
	if(hSock <=0)
	{
         return FALSE;
	}

    //TCHAR -> utf-8
	//char* result;
	
	int textlen;
	textlen = WideCharToMultiByte( CP_UTF8, 0, data, -1, NULL, 0, NULL, NULL );
	//result =(char *)malloc((textlen)*sizeof(char));
	char result[2048];
	memset(result, 0, sizeof(char) * 2048);
	WideCharToMultiByte( CP_UTF8, 0, data, -1, result, 2048, NULL, NULL );
    
	//int bytes = send(hSock,result,textlen,0);
	int bytes = send(hSock,result,strlen(result),0);
	//TRACE(_T("send data : %s\r\n"),data);
	//TRACE(_T("send %d bytes \r\n"),bytes);
	//free(result);
	return TRUE;
}

int SendData(const SOCKET sock,const char *sendData,int len)
{
	int sendBytes = 0,lastTotal = 0;
	char sendBuf[TCP_PACKET_SIZE];
	lastTotal = len;
	memset(sendBuf,0,TCP_PACKET_SIZE * sizeof(char));
	memcpy(sendBuf,sendData,lastTotal);

	do
	{
		sendBytes = send(sock,sendBuf,lastTotal,0);
		if(sendBytes == SOCKET_ERROR)//套接字出异常
		{
			break;
		}

		lastTotal = lastTotal - sendBytes;
		char buf[TCP_PACKET_SIZE];
		memset(buf,0,TCP_PACKET_SIZE);
		memcpy(buf,sendBuf + sendBytes,lastTotal);
		memset(sendBuf,0,TCP_PACKET_SIZE);
		memcpy(sendBuf,buf,lastTotal);
	}while(lastTotal > 0);
	return 1;
}

CString getHostIP()
{
     char szHostName[128];
	 CString strHostIP = _T("");
 
     if( gethostname(szHostName, 128) == 0 )
    {
		  struct hostent * pHost; 
		  int i;  
		  pHost = gethostbyname(szHostName);
		  for( i = 0; pHost!= NULL && pHost->h_addr_list[i]!= NULL; i++ )  
		  {
		      LPCSTR psz=inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
		      strHostIP = psz;
		  }  
	 }

	 return strHostIP;
}


int  splitRecvData(wchar_t *src,DWORD len,char* szBuf,DWORD *store,CClientItem* item)
{
	   int result = 0;
       if(src == NULL)
	   {
            return -1;
	   }
	   if(item == NULL)
	   {
            return -1;
	   }

	   while(1)
	   {	 
		   wchar_t* pch = wcsstr(src,L"</XinZongMsg>");
		   if(pch == NULL)
		   {
			   //把多余的字节存回buf
			   char *uftBuf = UnicodeToUTF8(src);
               strcpy(szBuf,uftBuf);
			   free(uftBuf);
			   return 0;
		   }
		   else
		   {
				 result = (int)(pch - src);
                 wchar_t xmlBuf[MAX_BUF_SIZE];
				 memset(xmlBuf,0,MAX_BUF_SIZE * sizeof(wchar_t));
				 wcsncpy(xmlBuf,src,result + wcslen(L"</XinZongMsg>") );
				 *store =  wcslen(src) - result - wcslen(L"</XinZongMsg>");
				  src = src + result + wcslen(L"</XinZongMsg>");
 
				  //解析支付串
                  resolveXml(xmlBuf,item);
		   }
	   }
}

DWORD WINAPI SendFileThread(LPVOID pParam)
{
	CoInitialize(NULL); // 初始化COM

	CSendFileDlg *pThis = (CSendFileDlg *)pParam;
    ASSERT(pThis != NULL);

	fd_set readfds;
	fd_set writefds;
	int      maxfd = 0,recvsize = 0;
	timeval to;
	int bytesSend = 0,bytesTotal = 0,lastTotal = 0,bytesRecv = 0;
	char recvBuf[TCP_PACKET_SIZE];
	char sendBuf[TCP_PACKET_SIZE];
	int recvBufSize = 0;
	int iError = 0;
	FILE *file = NULL;
	int retval = 0;
	CString strText;

	to.tv_sec = 1;
	to.tv_usec = 0;

	SOCKET connsock =  socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);;
	//设置套接字发送缓冲区
	int value=64 * 1024;//64K
	int tmpCode=0;
	tmpCode=::setsockopt(connsock, SOL_SOCKET, SO_SNDBUF, (char*)&value, sizeof(value));

    SOCKADDR_IN addrServ;
	char* host = NULL;
	int iPort = LISTEN_FILE_PORT;
	memset(&addrServ, 0, sizeof(addrServ));
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(iPort);	
	host = UnicodeToANSI(pThis->m_clientItem->m_strIp);
	addrServ.sin_addr.s_addr =  inet_addr(host);
	free(host);

	retval = connect(connsock,(SOCKADDR *)&addrServ,sizeof(SOCKADDR));//不阻塞

	if (SOCKET_ERROR == retval) //连接失败
	{
		closesocket(connsock);
		::PostMessage(pThis->GetSafeHwnd(),WM_CLOSE,0,0);//关闭对话框
		CoUninitialize(); // 结束对COM的使用。
		return 0;
	}

	//发送一条FILE_SEND消息
	CString strMsg;
	strMsg.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"FILE_SEND\"><FileTransferID>%s</FileTransferID><FileName>%s</FileName><FileSize>%d</FileSize></XinZongMsg>\r\n"),
					 pThis->m_fileItem->strFileID,pThis->m_fileItem->strFileName,pThis->m_fileItem->longFileLength);
	sendBytes(connsock,strMsg.GetBuffer(0));
	strMsg.ReleaseBuffer();

	//file = _wfsopen(pThis->m_fileItem->strFilePath,L"rb",_SH_DENYNO);

	//等待服务器回应
	for(;;)
	{
		if(pThis->m_bClose)
		{
			closesocket(connsock);
			if(file)
			{
				fclose(file);
				file = NULL;
			}
            ::PostMessage(pThis->GetSafeHwnd(),WM_CLOSE,0,0);//关闭对话框
			return 0;
		}

		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		FD_SET(connsock,&readfds);
		FD_SET(connsock,&writefds);
		if (connsock>maxfd)
		{
			maxfd = connsock;
		}

		int n = select(maxfd + 1, &readfds, &writefds, NULL, &to);
		if (n>0)
		{
			if(FD_ISSET(connsock,&readfds))  //用数据需要接收
			{
				memset(recvBuf,0,TCP_PACKET_SIZE);
				recvBufSize = 0;
				bytesRecv = 0;
				if((bytesRecv = recv(connsock, recvBuf +recvBufSize , sizeof(recvBuf) - recvBufSize, 0)) > 0)
				{
					clean_string(recvBuf);
					int textlen ;
					wchar_t * result;
					textlen = MultiByteToWideChar( CP_UTF8,0, recvBuf,-1, NULL,0 ); 
					result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
					memset(result,0,(textlen+1)*sizeof(wchar_t)); 
					MultiByteToWideChar(CP_UTF8, 0,recvBuf,-1,(LPWSTR)result,textlen);

                    CString strType = getXmlType(result);
					if(strType == _T("FILE_RECV"))
					{                 
                           file = _wfsopen(pThis->m_fileItem->strFilePath,L"rb",_SH_DENYNO);
					}
					free(result);
				}
				else
				{

					if(file)
					{
						fclose(file);
						file = NULL;
					}
					
					closesocket(connsock);
					::PostMessage(pThis->GetSafeHwnd(),WM_CLOSE,0,0);//关闭对话框
					return 0;
				}
				recvBufSize = 0;
			}

			if(FD_ISSET(connsock,&writefds)) //有数据要写
			{
				if(file)
				{
					char buf[8192];
					memset(buf,0,8192 * sizeof(char) );
					int n = fread(buf,1,sizeof(buf), file);
					int byteSend = send(connsock,buf,n,0);
					if(byteSend < n)
					{
						memset(sendBuf,0,TCP_PACKET_SIZE);
						memcpy(sendBuf, buf+ byteSend,n - byteSend);
						SendData(connsock,sendBuf,n - byteSend);
					}				

					pThis->bytesSent += n;
					//SendMessage 修改进度条
					::PostMessage(pThis->GetSafeHwnd(),WM_FILEPROGRESS_MSG,pThis->bytesSent,0);
					//TRACE(_T("FileData is sending!"));
					if (pThis->bytesSent >= pThis->m_fileItem->longFileLength)
					{
						fclose(file);
						file = NULL;
					}
					Sleep(2);
				}
			}
		}
	}
	CoUninitialize(); // 结束对COM的使用。
	return 0;
}

DWORD WINAPI FindRecverThread(LPVOID pParam)
{
	CoInitialize(NULL);

	CFileShareDlg *pFileDlg = (CFileShareDlg *)pParam;
	ASSERT(pFileDlg != NULL);

	CString strHostIP = getHostIP();
	CString strPrefixIP =  strHostIP.Left(strHostIP.ReverseFind('.') + 1);
	BOOL bExsit = FALSE;

	CString strServIp;
	//int iPort = LISTEN_CLIENT_PORT;

	//int nCount = 0;
    //while(nCount < FIND_COUNT)
	//{
		//创建255个线程

    for(int nIndex = 1; nIndex < 256; nIndex++)
	{
          if(pFileDlg->bShutDown)
		  {
			  CoUninitialize(); // 结束对COM的使用。
               return 0;
		  }
         
		 strServIp.Format(_T("%s%d"),strPrefixIP,nIndex);
         if(strServIp == strHostIP)
		 {
               //本机
			   continue;
		 }

		 bExsit = FALSE;
		 list<CClientItem* >::iterator it;
		 for(it = pFileDlg->m_clientList.begin();it != pFileDlg->m_clientList.end();it++)
		  {
			   CClientItem* tempItem = (CClientItem*) *it;
			   if(tempItem)
			   {
				   if(tempItem->m_strIp == strServIp)
				   { 
					   //已存在的接收方，不再请求连接
                       bExsit = TRUE;
				   }
			   }
		 }

		 if(bExsit)
		 {
               continue;
		 }

		 //创建线程
		CThreadStruct * pStruct = new CThreadStruct();
		pStruct->strSrvIp = strServIp;
		pStruct->m_pMainWnd = pFileDlg;
		pFileDlg->m_scanThread[nIndex] =  CreateThread(NULL, 0,ConnectServerThead ,pStruct, 0, NULL);

		//	SOCKET connSock = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
		//	if (connSock == INVALID_SOCKET )
		//	{
		//		TRACE(_T("创建套接字失败！\r\n"));
		//		return 0;
		//	}

		//    u_long ul=1;//代表非阻塞
	 //       int ret  = ioctlsocket(connSock,FIONBIO,&ul);//设置为非阻塞连接

		//	 fd_set rfd;      //描述符集 这个将测试连接是否可用
		//	 struct timeval timeout;  //时间结构体
		//	 FD_ZERO(&rfd);//先清空一个描述符集
		//	 timeout.tv_sec = 0;//秒
		//	 timeout.tv_usec = 10000;//一百万分之一秒，微秒

		//	 char szIpAddr[16] = {0};
		//	 USES_CONVERSION;
		//	 strcpy_s(szIpAddr, 16, T2A(strServIp));

		//	 sockaddr_in server;
		//	 server.sin_family = AF_INET;
		//	 server.sin_port = htons(iPort);
		//	 server.sin_addr.s_addr = inet_addr(szIpAddr);

		//	 ret = connect(connSock, (SOCKADDR*)&server, sizeof(SOCKADDR));
  //           
		//	 FD_SET(connSock,&rfd);
		//	 ret = select(connSock, 0, &rfd, 0, &timeout);  
		//	 if ( ret <= 0 )
		//	 {
		//		 closesocket(connSock);
		//		 connSock = NULL;
		//		 continue;
		//	 }
		//	 else
		//	 {//发现客户端
  //                  //设回阻塞模式
		//	       ul = 0 ;
		//	       ret = ioctlsocket(connSock, FIONBIO, (unsigned long*)&ul);  //返回之后，记得要设置为阻塞模式哟
  //            	   CClientItem* tItem = new CClientItem;
		//			tItem->m_Socket = connSock;
		//			tItem->m_pMainWnd = pFileDlg;
		//			tItem->m_strIp = strServIp;
		//			pFileDlg->m_clientList.push_back(tItem);

		//		   //创建线程发送消息
		//			HANDLE tThread = CreateThread(NULL, 0, ConnectRecverThread, tItem, 0, NULL);
		//			tItem->hThread = tThread;
		//	 }
		//}

  //      nCount++;
		//Sleep(1000);
	}

    CoUninitialize(); // 结束对COM的使用。
	return 0;
}


DWORD WINAPI ConnectRecverThread(LPVOID pParam)
{
     CoInitialize(NULL);

    CClientItem *clientItem = (CClientItem *) pParam;
	ASSERT(clientItem != NULL);
    
	 //发送请求
	 CString strHello;
	 strHello.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"HELLO\"><ClientName>%s</ClientName></XinZongMsg>\r\n"),clientItem->m_pMainWnd->m_strPCName);
	 sendBytes(clientItem->m_Socket,strHello.GetBuffer(0));
	 strHello.ReleaseBuffer();

	 while( TRUE && !(clientItem->bDisconnect)) {

		if ( SOCKET_Select(clientItem->m_Socket) ) {
			
			char szBuf[MAX_BUF_SIZE] = {0};
			DWORD nRecvBytes = 0,nStoreBytes = 0;

			int iRet = recv(clientItem->m_Socket, (char *)szBuf + nStoreBytes, MAX_BUF_SIZE, 0);
			if ( iRet > 0 ) {
				clean_string(szBuf);
                int textlen ;
				wchar_t * result;
				textlen = MultiByteToWideChar( CP_UTF8,0, szBuf,-1, NULL,0 ); 
				result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
				memset(result,0,(textlen+1)*sizeof(wchar_t)); 
				MultiByteToWideChar(CP_UTF8, 0,szBuf,-1,(LPWSTR)result,textlen);
                   
				//resolveXml(result,clientItem);
				//拆包
                CString strTemp = result;
				strTemp.Replace(L"\r\n",L"");
                
				splitRecvData(strTemp.GetBuffer(0),textlen,szBuf,&nStoreBytes,clientItem);
				strTemp.ReleaseBuffer();

				free(result);
			}else{
				//链接已断开，执行清理操作
                
				break;
			}
		}
		Sleep(500);
	}
    
	CFileShareDlg *pFileDlg = clientItem->m_pMainWnd;
	pFileDlg->m_clientList.remove(clientItem);
	//发送消息通知界面
	SendMessage(pFileDlg->GetSafeHwnd(),WM_REFLESH_LIST,0,( LPARAM)clientItem);
    closesocket(clientItem->m_Socket);
	delete clientItem;
	CoUninitialize(); // 结束对COM的使用。
	return TRUE;
}



DWORD WINAPI ConnectServerThead(LPVOID pParam)
{
	//CString strSrvIp = (CString) *pParam;
    //TCHAR *pSrvIP = (TCHAR *) pParam;
    CThreadStruct *pStruct =  (CThreadStruct *) pParam;
	if(pStruct == NULL)
	{
         return 0;
	}

	int iPort = LISTEN_CLIENT_PORT;
	SOCKET connSock = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
	if (connSock == INVALID_SOCKET )
	{
		//TRACE(_T("创建套接字失败！\r\n"));
		delete pStruct;
		return 0;
	}

    u_long ul=1;//代表非阻塞
    int ret  = ioctlsocket(connSock,FIONBIO,&ul);//设置为非阻塞连接

	 fd_set rfd;      //描述符集 这个将测试连接是否可用
	 struct timeval timeout;  //时间结构体
	 FD_ZERO(&rfd);//先清空一个描述符集
	 timeout.tv_sec = 10;//秒
	 timeout.tv_usec = 0;//一百万分之一秒，微秒

	 char szIpAddr[16] = {0};
	 USES_CONVERSION;
	 strcpy_s(szIpAddr, 16, T2A(pStruct->strSrvIp));

	 sockaddr_in server;
	 server.sin_family = AF_INET;
	 server.sin_port = htons(iPort);
	 server.sin_addr.s_addr = inet_addr(szIpAddr);

     ret = connect(connSock, (SOCKADDR*)&server, sizeof(SOCKADDR));
         
	 FD_SET(connSock,&rfd);
	 ret = select(connSock, 0, &rfd, 0, &timeout);  
	 if ( ret <= 0 )
	 {
		 closesocket(connSock);
		 connSock = NULL;
		 delete pStruct;
		 return 0;
	 }
	else
	 {//发现客户端
           //发送消息给主界面

	       ul = 0 ;
	       ret = ioctlsocket(connSock, FIONBIO, (unsigned long*)&ul);  //返回之后，记得要设置为阻塞模式哟
      	   CClientItem* tItem = new CClientItem;
		   tItem->m_Socket = connSock;
		   tItem->m_pMainWnd = pStruct->m_pMainWnd;
		   tItem->m_strIp = pStruct->strSrvIp;
		   pStruct->m_pMainWnd->m_clientList.push_back(tItem);

		   //创建线程发送消息
		   HANDLE tThread = CreateThread(NULL, 0, ConnectRecverThread, tItem, 0, NULL);
		   tItem->hThread = tThread;
	 }
	 delete pStruct;
	 return 0;
}