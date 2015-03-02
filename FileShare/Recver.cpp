
#include "stdafx.h"
#include "FileShareDlg.h"
#include "SendFileDlg.h"
#include "TransformChar.h"


DWORD WINAPI RecvFileThread(LPVOID pParam)
{
	CoInitialize(NULL); // 初始化COM

	CClientItem *clientItem = (CClientItem *)pParam;
	ASSERT(clientItem);
    CFileItem* fileItem = NULL;

	fd_set readfds;
	fd_set writefds;
	int      maxfd = 0,recvsize = 0;
	timeval to;
	int bytesSend = 0,bytesTotal = 0,lastTotal = 0,bytesRecv = 0;
	char recvBuf[TCP_PACKET_SIZE];
	int recvBufSize = 0;
	int iError = 0;
	FILE *file = NULL;
	int retval = 0;
	int recvCount = 0;
	CString strText;

	to.tv_sec = 1;
	to.tv_usec = 0;

	SOCKET connsock = clientItem->m_Socket;
	int value=64 * 1024;//64K
	int tmpCode=0;
	tmpCode=::setsockopt(connsock, SOL_SOCKET, SO_RCVBUF, (char*)&value, sizeof(value));

	memset(recvBuf,0,TCP_PACKET_SIZE);
	recvBufSize = 0;
	//等待服务器回应
	for(;;)
	{
		if(clientItem->bDisconnect)
		{
			closesocket(connsock);
			if(file)
			{
				fclose(file);
				file = NULL;
			}
			CoUninitialize(); // 结束对COM的使用。
			delete clientItem;
			return 0;
		}

		if(fileItem && fileItem->m_pFileDlg->m_bClose)
		{
			closesocket(connsock);
			if(file)
			{
				fclose(file);
				file = NULL;
			}
			CoUninitialize(); // 结束对COM的使用。
			delete clientItem;
			::PostMessage(fileItem->m_pFileDlg->GetSafeHwnd(),WM_CLOSE,0,0);//关闭对话框
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
				bytesRecv = 0;
				if((bytesRecv = recv(connsock, recvBuf + recvBufSize, sizeof(recvBuf) - recvBufSize, 0)) > 0)
				{
                     recvBufSize += bytesRecv;

					if(recvCount == 0)
					{
						clean_string(recvBuf);
						int textlen ;
						wchar_t * result;
						textlen = MultiByteToWideChar( CP_UTF8,0, recvBuf,-1, NULL,0 ); 
						result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
						memset(result,0,(textlen+1)*sizeof(wchar_t)); 
						MultiByteToWideChar(CP_UTF8, 0,recvBuf,-1,(LPWSTR)result,textlen);
	                   
						CString strType = getXmlType(result);
						

						if(strType == _T("FILE_SEND"))
						{
                            //回复FILE_RECV
							CString strFileID = resolveFileSendXml(result,clientItem);

							if(strFileID != _T(""))
							{
                                   //查找对应的FileItem
								 list<CFileItem* >::iterator it;
								for(it = clientItem->m_pMainWnd->m_fileList.begin(); it != clientItem->m_pMainWnd->m_fileList.end();it++)
								{
									   CFileItem* item = (CFileItem* ) *it;
									   if(item && item->strFileID == strFileID)
									   {  
										   fileItem = item;
										   file = _wfsopen(fileItem->strFilePath,L"wb",_SH_DENYNO);
										   break;
									   }
								}

							    
							}
						}
						free(result);
					}
					else
					{
                          if(file && fileItem)
						{
							int n;
							n = fwrite(recvBuf, 1, recvBufSize, file);
							recvBufSize = 0;

							fileItem->m_pFileDlg->bytesSent += n;
							::PostMessage(fileItem->m_pFileDlg->GetSafeHwnd(),WM_FILEPROGRESS_MSG,fileItem->m_pFileDlg->bytesSent,0);

							if (fileItem->m_pFileDlg->bytesSent >= fileItem->longFileLength)
							{
								fclose(file);
								file = NULL;
								closesocket(connsock);
								::PostMessage(fileItem->m_pFileDlg->GetSafeHwnd(),WM_CLOSE,0,0);//关闭对话框
								CoUninitialize(); // 结束对COM的使用。
						        delete clientItem;
								return 0;
							}    
						}
					}
					recvCount++;
					recvBufSize = 0;
				}
				else
				{			
					closesocket(connsock);
					::PostMessage(fileItem->m_pFileDlg->GetSafeHwnd(),WM_CLOSE,0,0);//关闭对话框
					CoUninitialize(); // 结束对COM的使用。
					delete clientItem;
					return 0;
				}
			}
		}
	}

	CoUninitialize(); // 结束对COM的使用。
	delete clientItem;
	return 0;
}

//监听客户端连接
DWORD WINAPI ListenSenderThread(LPVOID pParam)
{
    CFileShareDlg *pFileDlg = (CFileShareDlg *)pParam;
	ASSERT(pFileDlg != NULL);

	pFileDlg->m_ListenSock = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
	if ( pFileDlg->m_ListenSock == INVALID_SOCKET ) {
		AfxMessageBox(_T("新建Socket失败！"));
		return FALSE;
	}

	int iPort = LISTEN_CLIENT_PORT;

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(iPort);

	if ( bind(pFileDlg->m_ListenSock, (sockaddr*)&service, sizeof(sockaddr_in)) == SOCKET_ERROR ) {
		AfxMessageBox(_T("绑定端口失败！"));
		goto __Error_End;
	}

	if( listen(pFileDlg->m_ListenSock, 5) == SOCKET_ERROR ) {
		AfxMessageBox(_T("监听失败！"));
		goto __Error_End;
	}

	while( TRUE && !(pFileDlg->bShutDown))
	{
		if ( SOCKET_Select(pFileDlg->m_ListenSock, 100, TRUE) ) 
		{
			sockaddr_in clientAddr;
			int iLen = sizeof(sockaddr_in);
			SOCKET accSock = accept(pFileDlg->m_ListenSock, (struct sockaddr *)&clientAddr , &iLen);
			if (accSock == INVALID_SOCKET) {
				continue;
			}
			CClientItem* tItem = new CClientItem;
			tItem->m_Socket = accSock;
			tItem->m_pMainWnd = pFileDlg;
			tItem->m_strIp = inet_ntoa(clientAddr.sin_addr);
			pFileDlg->m_clientList.push_back(tItem);
			tItem->hThread = CreateThread(NULL, 0, ClientThreadProc,tItem,0, NULL);
		}

		Sleep(100);
	}

__Error_End:
	closesocket(pFileDlg->m_ListenSock);
	return TRUE;
}

//监听文件传输请求
DWORD WINAPI ListenFileSenderThread(LPVOID pParam)
{
    CFileShareDlg *pFileDlg = (CFileShareDlg *)pParam;
	ASSERT(pFileDlg != NULL);

	pFileDlg->m_ListenFileSock = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
	if ( pFileDlg->m_ListenFileSock == INVALID_SOCKET ) {
		AfxMessageBox(_T("新建Socket失败！"));
		return FALSE;
	}

	int iPort = LISTEN_FILE_PORT;

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(iPort);

	if ( bind(pFileDlg->m_ListenFileSock, (sockaddr*)&service, sizeof(sockaddr_in)) == SOCKET_ERROR ) {
		AfxMessageBox(_T("绑定端口失败！"));
		goto __Error_End;
	}

	if( listen(pFileDlg->m_ListenFileSock, 5) == SOCKET_ERROR ) {
		AfxMessageBox(_T("监听失败！"));
		goto __Error_End;
	}

	while( TRUE && !(pFileDlg->bShutDown))
	{
		if ( SOCKET_Select(pFileDlg->m_ListenFileSock, 100, TRUE) ) 
		{
			sockaddr_in clientAddr;
			int iLen = sizeof(sockaddr_in);
			SOCKET accSock = accept(pFileDlg->m_ListenFileSock, (struct sockaddr *)&clientAddr , &iLen);
			if (accSock == INVALID_SOCKET) {
				continue;
			}

			//文件传输请求 创建文件接收线程 将对应的socket传进去
			CClientItem* tItem = new CClientItem;
			tItem->m_Socket = accSock;
			tItem->m_pMainWnd = pFileDlg;
			tItem->m_strIp = inet_ntoa(clientAddr.sin_addr);
			tItem->intConnectType = 1;//文件传输连接
		    HANDLE tHandle = CreateThread(NULL, 0, RecvFileThread,tItem,0, NULL);
			tItem->hThread = tHandle;
		}

		Sleep(100);
	}

__Error_End:
	closesocket(pFileDlg->m_ListenSock);
	return TRUE;
}

//解析连接消息
DWORD WINAPI ClientThreadProc(LPVOID lpParameter)
{
     CoInitialize(NULL);

	CClientItem* m_ClientItem = (CClientItem *) lpParameter;
	ASSERT(m_ClientItem != NULL);
	char szBuf[TCP_PACKET_SIZE] = {0};
	DWORD nRecvBytes = 0,nStoreBytes = 0;

	while( TRUE && !(m_ClientItem->bDisconnect) && !m_ClientItem->m_pMainWnd->bShutDown)
	{
		if ( SOCKET_Select(m_ClientItem->m_Socket, 100, TRUE) ) 
		{
			int iRet = recv(m_ClientItem->m_Socket, (char *)szBuf + nStoreBytes, MAX_BUF_SIZE, 0);
			if ( iRet > 0 )
			{
				clean_string(szBuf);
				int textlen ;
				wchar_t * result;
				textlen = MultiByteToWideChar( CP_UTF8,0, szBuf,-1, NULL,0 ); 
				result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
				memset(result,0,(textlen+1)*sizeof(wchar_t)); 
				MultiByteToWideChar(CP_UTF8, 0,szBuf,-1,(LPWSTR)result,textlen);

				//拆包
				CString strTemp = result;
				strTemp.Replace(L"\r\n",L"");
				splitRecvData(strTemp.GetBuffer(0),textlen,szBuf,&nStoreBytes,m_ClientItem);
				strTemp.ReleaseBuffer();
                
    //            if(resolveXml(result,m_ClientItem))
				//{

				//}
				//else//不合法的请求
				//{
    //                 ::PostMessage(m_ClientItem->m_pMainWnd->m_hWnd,WM_FINDCLIENT_MSG,1,(LPARAM)m_ClientItem);
				//}
				free(result);
			}
			else
			{//客户端离开了网络
                
				break;
			}
		}
		Sleep(500);
	}

	CFileShareDlg *pFileDlg = m_ClientItem->m_pMainWnd;
	pFileDlg->m_clientList.remove(m_ClientItem);
	//发送消息通知界面
	SendMessage(pFileDlg->GetSafeHwnd(),WM_REFLESH_LIST,0,( LPARAM)m_ClientItem);
    closesocket(m_ClientItem->m_Socket);
	delete m_ClientItem;
	
    CoUninitialize(); // 结束对COM的使用。
	TRACE(_T("ClientThreadProc 结束"));
	return TRUE;
}


