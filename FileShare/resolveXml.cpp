
#include "stdafx.h"
#include "Inc.h"
#include "FileShareDlg.h"
#include "SendFileDlg.h"


BOOL resolveXml(wchar_t* strXml,CClientItem* clientItem)
{
    CComPtr<MSXML6::IXMLDOMDocument> pkXMLDOM;
	CComPtr<MSXML6::IXMLDOMElement> pkXMLRoot;
	CComPtr<MSXML6::IXMLDOMNode> pkXMLNode;
    CString strType;

     HRESULT hr=pkXMLDOM.CoCreateInstance (_uuidof(DOMDocument));
	 if(SUCCEEDED(hr))
	 {
		VARIANT_BOOL bSuccess = pkXMLDOM->loadXML(strXml);
		if (bSuccess == VARIANT_FALSE)
		{
			DWORD dError = GetLastError();
			return FALSE;
		}

		hr = pkXMLDOM->get_documentElement(&pkXMLRoot);
		if(SUCCEEDED(hr))
		{
             variant_t varData;
			 varData = pkXMLRoot->getAttribute(_bstr_t(_T("Type")));
             strType =  (wchar_t*)(_bstr_t)varData;

			 if(strType == _T("HELLO"))
			 {
				 BSTR clientName;
				 pkXMLNode = pkXMLRoot->selectSingleNode(_T("ClientName"));
				 pkXMLNode->get_text(&clientName);

				 if(clientItem)
				 {
					 clientItem->m_strClientName = clientName;
					 clientItem->bOnline = TRUE;
				 }
                 
                  //发送请求
                 CString strHello;
				 strHello.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"HELLO_REPLY\"><ClientName>%s</ClientName></XinZongMsg>\r\n"),
					 clientItem->m_pMainWnd->m_strPCName);
				 sendBytes(clientItem->m_Socket,strHello.GetBuffer(0));
				 strHello.ReleaseBuffer();

				 ::PostMessage(clientItem->m_pMainWnd->m_hWnd,WM_FINDCLIENT_MSG,0,(LPARAM)clientItem);

				 return TRUE;
			 }
			 else if(strType == _T("HELLO_REPLY"))
			 {
                 BSTR clientName;
				 pkXMLNode = pkXMLRoot->selectSingleNode(_T("ClientName"));
				 pkXMLNode->get_text(&clientName);

				 if(clientItem)
				 {
					 clientItem->m_strClientName = clientName;
					  clientItem->bOnline = TRUE;
				 }
                 
                 ::PostMessage(clientItem->m_pMainWnd->m_hWnd,WM_FINDCLIENT_MSG,0,(LPARAM)clientItem);
			 }
			 else if(strType == _T("FILE_REQUEST"))
			 {//收到请求，发送消息到主界面，弹出文件接收窗口
                 BSTR fileID,fileName,fileSize;
				 pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileTransferID"));
				 pkXMLNode->get_text(&fileID);

                 pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileName"));
				 pkXMLNode->get_text(&fileName);

                 pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileSize"));
				 pkXMLNode->get_text(&fileSize);

				 CFileItem* fileItem = new CFileItem();
				 fileItem->longFileLength = _wtoi(fileSize);
				 fileItem->strFileName = fileName;
				 fileItem->strFileID = fileID;

				 CClientItem_FileItem* lparam = new CClientItem_FileItem();
				 lparam->clientItem = clientItem;
				 lparam->fileItem = fileItem;

				 ::PostMessage(clientItem->m_pMainWnd->GetSafeHwnd(),WM_RECVFILEREQUEST_MSG,0,(LPARAM)lparam);
			 }
			 else if(strType == _T("FILE_ACCEPT"))
			 {//收到文件回复请求，发送消息至住界面
                 BSTR fileID;
				 pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileTransferID"));
				 pkXMLNode->get_text(&fileID);
                 TCHAR* pstrFileID = new TCHAR[MAX_PATH];
				 memset(pstrFileID,0,MAX_PATH * sizeof(TCHAR));
				 _tcscpy(pstrFileID,fileID);

                 ::PostMessage(clientItem->m_pMainWnd->GetSafeHwnd(),WM_RECVFILEACCEPT_MSG,0,(LPARAM)pstrFileID);
			 }
			 else if(strType == _T("FILE_REFUSED"))
			 {
                 BSTR fileID;
				 pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileTransferID"));
				 pkXMLNode->get_text(&fileID);
                 TCHAR* pstrFileID = new TCHAR[MAX_PATH];
				 memset(pstrFileID,0,MAX_PATH * sizeof(TCHAR));
				 _tcscpy(pstrFileID,fileID);

                 ::PostMessage(clientItem->m_pMainWnd->GetSafeHwnd(),WM_FILE_REFUSED,0,(LPARAM)pstrFileID);
			 }
			 else if(strType == _T("FILE_CANCEL"))
			 {
                  BSTR fileID;
				  pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileTransferID"));
				  pkXMLNode->get_text(&fileID);
                  TCHAR* pstrFileID = new TCHAR[MAX_PATH];
				  memset(pstrFileID,0,MAX_PATH * sizeof(TCHAR));
				  _tcscpy(pstrFileID,fileID);

                  ::PostMessage(clientItem->m_pMainWnd->GetSafeHwnd(),WM_FILE_CANCEL,0,(LPARAM)pstrFileID);
			 }
			 //else if(strType == _T("FILE_RECV"))
			 //{
    //             BSTR fileID;
				// pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileTransferID"));
				// pkXMLNode->get_text(&fileID);
    //             TCHAR* pstrFileID = new TCHAR[MAX_PATH];
				// memset(pstrFileID,0,MAX_PATH * sizeof(TCHAR));
				// _tcscpy(pstrFileID,fileID);
    //           
				// clientItem->bDisconnect = TRUE;
    //             //将socket 传递出去
    //             //查找对应的FileItem
    //             list<CFileItem* >::iterator it;
				//for(it = clientItem->m_pMainWnd->m_fileList.begin(); it != clientItem->m_pMainWnd->m_fileList.end();it++)
				//{
    //                   CFileItem* fileItem = (CFileItem* ) *it;
				//	   if(fileItem->strFileID == fileID)
				//	   {
				//			  ::PostMessage(fileItem->m_pFileDlg->GetSafeHwnd(),WM_BEGINSENDFILE_MSG,0,(LPARAM)clientItem);
    //                          break;
				//	   }
				//}
				// 
			 //}
			 //else if(strType == _T("FILE_SEND"))
			 //{
    //             BSTR fileID;
				// pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileTransferID"));
				// pkXMLNode->get_text(&fileID);
    //             TCHAR* pstrFileID = new TCHAR[MAX_PATH];
				// memset(pstrFileID,0,MAX_PATH * sizeof(TCHAR));
				// _tcscpy(pstrFileID,fileID);

    //             //将socket 传递出去
    //             //查找对应的FileItem
    //             list<CFileItem* >::iterator it;
				//for(it = clientItem->m_pMainWnd->m_fileList.begin(); it != clientItem->m_pMainWnd->m_fileList.end();it++)
				//{
    //                   CFileItem* fileItem = (CFileItem* ) *it;
				//	   if(fileItem && fileItem->strFileID == fileID)
				//	   {
    //                         //回复FILE_RECV
				//            CString strMsg;
				//            strMsg.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"FILE_RECV\">\
				//					 <FileTransferID>%s</FileTransferID><FileName>%s</FileName><FileSize>%d</FileSize></XinZongMsg >"),
				//					 fileItem->strFileID,fileItem->strFileName,fileItem->longFileLength);
				//			sendBytes(clientItem->m_Socket,strMsg.GetBuffer(0));
				//            strMsg.ReleaseBuffer();
    //                        break;
				//	   }
				//}
			 //}
			 else
			 {
                 return FALSE;
			 }
		}
	 }

	 return TRUE;
}

CString getXmlType(wchar_t* strXml)
{
    CComPtr<MSXML6::IXMLDOMDocument> pkXMLDOM;
	CComPtr<MSXML6::IXMLDOMElement> pkXMLRoot;
	CComPtr<MSXML6::IXMLDOMNode> pkXMLNode;
    CString strType = _T("");

     HRESULT hr=pkXMLDOM.CoCreateInstance (_uuidof(DOMDocument));
	 if(SUCCEEDED(hr))
	 {
		VARIANT_BOOL bSuccess = pkXMLDOM->loadXML(strXml);
		if (bSuccess == VARIANT_FALSE)
		{
			DWORD dError = GetLastError();
			return strType;
		}

		hr = pkXMLDOM->get_documentElement(&pkXMLRoot);
		if(SUCCEEDED(hr))
		{
             variant_t varData;
			 varData = pkXMLRoot->getAttribute(_bstr_t(_T("Type")));
             strType =  (wchar_t*)(_bstr_t)varData;
		}
	 }

	 return strType;
}

CString resolveFileSendXml(wchar_t* strXml,CClientItem* clientItem)
{
    CString strFileID;

	 CComPtr<MSXML6::IXMLDOMDocument> pkXMLDOM;
	CComPtr<MSXML6::IXMLDOMElement> pkXMLRoot;
	CComPtr<MSXML6::IXMLDOMNode> pkXMLNode;
    CString strType;

     HRESULT hr=pkXMLDOM.CoCreateInstance (_uuidof(DOMDocument));
	 if(SUCCEEDED(hr))
	 {
		VARIANT_BOOL bSuccess = pkXMLDOM->loadXML(strXml);
		if (bSuccess == VARIANT_FALSE)
		{
			DWORD dError = GetLastError();
			return strFileID;
		}

		hr = pkXMLDOM->get_documentElement(&pkXMLRoot);
		if(SUCCEEDED(hr))
		{
             variant_t varData;
			 varData = pkXMLRoot->getAttribute(_bstr_t(_T("Type")));
             strType =  (wchar_t*)(_bstr_t)varData;

			 if(strType == _T("FILE_SEND"))
			 {
                 BSTR fileID;
				 pkXMLNode = pkXMLRoot->selectSingleNode(_T("FileTransferID"));
				 pkXMLNode->get_text(&fileID);
				 strFileID = fileID;

                 //将socket 传递出去
                 //查找对应的FileItem
                 list<CFileItem* >::iterator it;
				for(it = clientItem->m_pMainWnd->m_fileList.begin(); it != clientItem->m_pMainWnd->m_fileList.end();it++)
				{
                       CFileItem* fileItem = (CFileItem* ) *it;
					   if(fileItem && fileItem->strFileID == fileID)
					   {
                             //回复FILE_RECV
				            CString strMsg;
				            strMsg.Format(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?><XinZongMsg Version=\"1.0\" Type=\"FILE_RECV\"><FileTransferID>%s</FileTransferID><FileName>%s</FileName><FileSize>%d</FileSize></XinZongMsg>\r\n"),
									 fileItem->strFileID,fileItem->strFileName,fileItem->longFileLength);
							sendBytes(clientItem->m_Socket,strMsg.GetBuffer(0));
				            strMsg.ReleaseBuffer();
                            break;
					   }
				}
			 }
			 else
			 {
                 return FALSE;
			 }
		}
	 }
	//返回文件传输ID
	return strFileID;
}

/*
去掉字符串首尾的 \x20 \r \n 字符
by sincoder
*/
void clean_string(char *str)
{
    char *start = str - 1;
    char *end = str;
    char *p = str;
    while(*p)
    {
        switch(*p)
        {
        case ' ':
        case '\r':
        case '\n':
            {
                if(start + 1==p)
                    start = p;
            }
            break;
        default:
            break;
        }
        ++p;
    }
    //现在来到了字符串的尾部 反向向前
    --p;
    ++start;
    if(*start == 0)
    {
        //已经到字符串的末尾了 
        *str = 0 ;
        return;
    }
    end = p + 1;
    while(p > start)
    {
        switch(*p)
        {
        case ' ':
        case '\r':
        case '\n':
            {
                if(end - 1 == p)
                    end = p;
            }
            break;
        default:
            break;
        }
        --p;
    }
    memmove(str,start,end-start);
    *(str + (int)end - (int)start) = 0;
}