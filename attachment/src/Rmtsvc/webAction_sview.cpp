/*******************************************************************
   *	webAction_sview.cpp web������ - �������
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

//�г��������з���
//buffer - ���ص�xml�ĵ�,��ʽ:
//<?xml version="1.0" encoding="gb2312" ?>
//<xmlroot>
//<service>
//<id>���</id>
//<sname>������</sname>
//<status>����״̬</status>
//<rtype>��������</rtype>
//<stype>��������</stype>
//<sdisp>��ʾ����</sdisp>
//<sdesc>��������</sdesc>
//<spath>����ģ��·��</spath>
//</service>
//...
//</xmlroot>
DWORD serviceList(cBuffer &buffer);
bool webServer::httprsp_slist(socketTCP *psock,httpResponse &httprsp)
{
	cBuffer buffer(1024);
	serviceList(buffer);
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len()); 
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}

bool webServer::sevent(const char *sname,const char *cmd)
{
	if(sname==NULL || sname[0]==0) return false;
	//��SCM���SCM���
	SC_HANDLE schSCManager =	OpenSCManager(
			0,						// machine (NULL == local)
			0,						// database (NULL == default)
			SC_MANAGER_ALL_ACCESS	// access required
		);
	if( schSCManager==NULL ) return false;
	SC_HANDLE hService=OpenService(schSCManager,sname,SERVICE_ALL_ACCESS);
	if(hService==NULL){ ::CloseServiceHandle(schSCManager); return false; }
	
	SERVICE_STATUS	ssStatus; 
	if(strcmp(cmd,"run")==0) //����ָ���ķ���
	{
		if( ::StartService(hService, 0, 0) ) Sleep(1000);
	}
	else if(strcmp(cmd,"stop")==0) //ָֹͣ���ķ���
	{
		if( ::ControlService(hService, SERVICE_CONTROL_STOP, &ssStatus) )
		{
			Sleep(1000); long lcount=5;
			while(--lcount>0 && QueryServiceStatus(hService, &ssStatus) ) 
			{
				if( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
					Sleep( 1000 );
				else break;
			}//?while
		}
	}
	else if(strcmp(cmd,"delete")==0) //ɾ��ж�ط���
	{
		if( ::ControlService(hService, SERVICE_CONTROL_STOP, &ssStatus) )
		{
			Sleep(1000); long lcount=5;
			while(--lcount>0 && QueryServiceStatus(hService, &ssStatus) ) 
			{
				if( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
					Sleep( 1000 );
				else break;
			}//?while
		}
		DeleteService(hService);
	}
	else if(strcmp(cmd,"forbid")==0) //���÷���
	{
		ChangeServiceConfig(hService,SERVICE_NO_CHANGE,SERVICE_DISABLED,SERVICE_NO_CHANGE,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
	}
	else if(strcmp(cmd,"auto")==0) 
	{
		ChangeServiceConfig(hService,SERVICE_NO_CHANGE,SERVICE_AUTO_START,SERVICE_NO_CHANGE,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
	}
	else if(strcmp(cmd,"manual")==0)
	{
		ChangeServiceConfig(hService,SERVICE_NO_CHANGE,SERVICE_DEMAND_START,SERVICE_NO_CHANGE,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
	}
	::CloseServiceHandle(hService);
	::CloseServiceHandle(schSCManager);
	return true;
}

DWORD serviceList(cBuffer &buffer)
{
	//��SCM���SCM���
	SC_HANDLE schSCManager =	OpenSCManager(
			0,						// machine (NULL == local)
			0,						// database (NULL == default)
			SC_MANAGER_ALL_ACCESS	// access required
		);
	if( schSCManager==NULL ) return 0;
	ENUM_SERVICE_STATUS service,*lpservice;
	DWORD bytesNeeded,servicesReturned,resumeHandle=0;
	BOOL rc=::EnumServicesStatus(schSCManager,SERVICE_WIN32,SERVICE_STATE_ALL,&service,
		sizeof(service),&bytesNeeded,&servicesReturned,&resumeHandle);

	if( rc==FALSE && ::GetLastError()!=ERROR_MORE_DATA ){ ::CloseServiceHandle(schSCManager); return 0; }

	LPBYTE lpqsconfig_buffer=NULL;
	DWORD bytes=bytesNeeded+sizeof(ENUM_SERVICE_STATUS);
	if( (lpservice=(ENUM_SERVICE_STATUS *)::malloc(bytes))==NULL ){ ::CloseServiceHandle(schSCManager); return 0; }

	::EnumServicesStatus(schSCManager,SERVICE_WIN32,SERVICE_STATE_ALL,lpservice,
		bytes,&bytesNeeded,&servicesReturned,&resumeHandle);

	DWORD dwret=0;
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	while( servicesReturned-- >0 ) //lpservice->lpServiceName)
	{
		if(buffer.Space()<280) buffer.Resize(buffer.size()+280);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),
			"<service><id>%d</id><sname>%s</sname>",++dwret,lpservice->lpServiceName);
		switch(lpservice->ServiceStatus.dwCurrentState)
		{
			case SERVICE_RUNNING:
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>������</status>");
				break;
			case SERVICE_STOPPED:
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>��ֹͣ</status>");
				break;
			case SERVICE_PAUSED:
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>����ͣ</status>");
				break;
			case SERVICE_STOP_PENDING:
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>��ֹͣ</status>");
				break;
			case SERVICE_CONTINUE_PENDING:
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>�ѹ���</status>");
				break;
			case SERVICE_PAUSE_PENDING:
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>�ѹ���</status>");
				break;
			default:
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>---</status>");
				break;
		}//?switch
		SC_HANDLE hService=OpenService(schSCManager,lpservice->lpServiceName,SERVICE_ALL_ACCESS);
		if(hService==NULL){ buffer.len()+=sprintf(buffer.str()+buffer.len(),"</service>"); continue; }
		
		bytesNeeded=0;//��һ����ȡ��Ϣ
		QueryServiceConfig( hService, NULL, 0, &bytesNeeded);
		DWORD lpqscBuf_Size=bytesNeeded;
		LPQUERY_SERVICE_CONFIG lpqscBuf=(LPQUERY_SERVICE_CONFIG)::malloc(lpqscBuf_Size);
		if(lpqscBuf && QueryServiceConfig( hService, lpqscBuf, lpqscBuf_Size,&bytesNeeded))
		{
			if(buffer.Space()<(bytesNeeded+100)) buffer.Resize(buffer.size()+(bytesNeeded+100));
			if( lpqscBuf->dwStartType==SERVICE_AUTO_START)
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<rtype>�Զ�</rtype>");
			else if( lpqscBuf->dwStartType==SERVICE_DEMAND_START)
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<rtype>�ֶ�</rtype>");
			else if( lpqscBuf->dwStartType==SERVICE_DISABLED)
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<rtype>����</rtype>");
			else buffer.len()+=sprintf(buffer.str()+buffer.len(),"<rtype>---</rtype>");
			
			if( lpqscBuf->dwServiceType & SERVICE_WIN32_OWN_PROCESS)
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<stype>�������̷���%s</stype>",
				(lpqscBuf->dwServiceType & SERVICE_INTERACTIVE_PROCESS)?",�ɽ���":"");
			else if( lpqscBuf->dwServiceType & SERVICE_WIN32_SHARE_PROCESS)
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<stype>������̷���%s</stype>",
				(lpqscBuf->dwServiceType & SERVICE_INTERACTIVE_PROCESS)?",�ɽ���":"");
			else if( lpqscBuf->dwServiceType & SERVICE_FILE_SYSTEM_DRIVER)
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<stype>�ļ�ϵͳ����</stype>");
			else if( lpqscBuf->dwServiceType & SERVICE_KERNEL_DRIVER)
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<stype>ϵͳ�ں�����</stype>");
			else buffer.len()+=sprintf(buffer.str()+buffer.len(),"<stype>---</stype>");
			
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<sdisp>%s</sdisp>",lpqscBuf->lpDisplayName);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<spath>%s</spath>",lpqscBuf->lpBinaryPathName);
			
			QueryServiceConfig2(hService,SERVICE_CONFIG_DESCRIPTION,NULL, 0, &bytesNeeded);
			if(bytesNeeded>lpqscBuf_Size)
			{
				::free(lpqscBuf);
				lpqscBuf=(LPQUERY_SERVICE_CONFIG)::malloc(bytesNeeded);
			} 
			lpqscBuf_Size=bytesNeeded;
			if(lpqscBuf)
			{
				QueryServiceConfig2(hService,SERVICE_CONFIG_DESCRIPTION,(LPBYTE)lpqscBuf, lpqscBuf_Size, &bytesNeeded);
				if(buffer.Space()<(lpqscBuf_Size+48)) buffer.Resize(buffer.size()+(lpqscBuf_Size+48));
				SERVICE_DESCRIPTION *p=(SERVICE_DESCRIPTION *)lpqscBuf;
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<sdesc><![CDATA[\r\n%s\r\n]]></sdesc>",p->lpDescription);
			}		
		}//?if(QueryServiceConfig
		if(lpqscBuf) ::free(lpqscBuf);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</service>");
		::CloseServiceHandle(hService); lpservice++;
	}//?while

	::CloseServiceHandle(schSCManager);
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	::free(lpservice); return dwret;
}
