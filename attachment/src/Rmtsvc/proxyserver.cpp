/*******************************************************************
   *	proxyserver.h 
   *    DESCRIPTION: Proxy����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *	
   *******************************************************************/

#include "rmtsvc.h" 

proxysvrEx :: proxysvrEx()
{
	initSetting();
}

void proxysvrEx ::initSetting()
{
	m_settings.svrport=8088;
	m_settings.svrtype=PROXY_HTTPS|PROXY_SOCKS4|PROXY_SOCKS5;
	m_settings.bAuth=false;
	m_settings.autorun=false;
	m_settings.bCascade=false;
	m_settings.cassvrip="";
	m_settings.castype=PROXY_HTTPS|PROXY_SOCKS4|PROXY_SOCKS5;
	m_settings.casAuth=false;
	m_settings.casuser="";
	m_settings.caspswd="";
	m_settings.ipaccess=1;
	m_settings.ipRules="";
	m_settings.bLogdatafile=false;
}

//��������
bool proxysvrEx :: Start() 
{
	if(this->status()==SOCKS_LISTEN) return true;
	this->setIfLogdata(m_settings.bLogdatafile);
	//���ô������Ĳ���
	this->setProxyType(m_settings.svrtype);
	const char *ptr_casuser=(m_settings.casAuth)?m_settings.casuser.c_str():NULL;
	//���ö���������Ϣ
	if(m_settings.bCascade)
		this->setCascade(m_settings.cassvrip.c_str(),0,m_settings.castype,ptr_casuser,m_settings.caspswd.c_str());
	else 
		this->setCascade(NULL,0,m_settings.castype,ptr_casuser,m_settings.caspswd.c_str());

	//����IP���˹���
	this->rules().addRules_new(RULETYPE_TCP,m_settings.ipaccess,m_settings.ipRules.c_str());
	//����proxy�����ʺ���Ϣ
	this->setProxyAuth(m_settings.bAuth);
	this->delAccount((const char *)-1); //��������ʺ���Ϣ
	std::map<std::string,TProxyUser>::iterator it=m_userlist.begin();
	for(;it!=m_userlist.end();it++)
	{
		TProxyUser &proxyuser=(*it).second;
		if(proxyuser.forbid==0) modiUser(proxyuser);
	}

	//�����������
	const char *ip=(m_settings.bindip=="")?NULL:m_settings.bindip.c_str();
	BOOL bReuseAddr=(ip)?SO_REUSEADDR:FALSE;//����IP������˿�����
	SOCKSRESULT sr=this->Listen( m_settings.svrport ,bReuseAddr,ip);

	return (sr>0)?true:false;
}

void proxysvrEx :: Stop()
{ 
	Close();
	return;
}
//ɾ��ָ�����û�,����0�ɹ�
//����1��Ч���ʺ�,����2ɾ��ʧ��
int proxysvrEx::deleUser(const char *ptr_user)
{
	std::map<std::string,TProxyUser>::iterator it=m_userlist.end();
	if(ptr_user) it=m_userlist.find(ptr_user);
	if(it==m_userlist.end()) return 1;
	if(!this->delAccount(ptr_user)) return 2;
	m_userlist.erase(it);
	return 0;
}
//���/�޸��û�
bool proxysvrEx::modiUser(TProxyUser &proxyuser)
{
	PROXYACCOUNT *ptr_account=this->getAccount(proxyuser.username.c_str());
	if(ptr_account==NULL) ptr_account=this->newAccount(proxyuser.username.c_str());
	if(ptr_account==NULL) return false;
	ptr_account->m_userpwd=proxyuser.userpwd;
	ptr_account->m_maxratio=proxyuser.maxratio;
	ptr_account->m_maxLoginusers=proxyuser.maxLoginusers;
	ptr_account->m_limitedTime=proxyuser.limitedTime;
	ptr_account->m_ipRules.addRules(NULL); //������й��˹���
	ptr_account->m_ipRules.addRules_new(RULETYPE_TCP,proxyuser.ipaccess,proxyuser.ipRules.c_str());

	ptr_account->m_dstRules.addRules(NULL); //�������Ŀ�Ĺ��˹���
	ptr_account->m_dstRules.addRules_new(RULETYPE_TCP,proxyuser.bAccessDest,proxyuser.strAccessDest.c_str());

	return true;
}

//-------------------------------------------------------------
//����������
bool webServer::httprsp_proxysets(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp)
{
	MyService *ptrService=MyService::GetService();
	proxysvrEx &proxysvr=ptrService->m_proxysvr;
	PROXYSETTINGS &settings=proxysvr.m_settings;
	const char *ptr,*ptr_cmd=httpreq.Request("cmd");
	cBuffer buffer(1024);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	
	if(strcasecmp(ptr_cmd,"run")==0) //����ftp����
		proxysvr.Start();
	else if(strcasecmp(ptr_cmd,"stop")==0)
		proxysvr.Stop();
	else if(strcasecmp(ptr_cmd,"setting")==0) 
	{//�������������
		if( (ptr=httpreq.Request("svrport")) )
		{ 
			if( (settings.svrport=atoi(ptr))<=0 ) settings.svrport=0;
		}
		ptr=httpreq.Request("bindip");
		if(ptr) settings.bindip.assign(ptr);
		ptr=httpreq.Request("svrtype");
		if(ptr) settings.svrtype=atoi(ptr);
		ptr=httpreq.Request("autorun");
		if(ptr) settings.autorun=(atoi(ptr)==1)?true:false;
		ptr=httpreq.Request("bauth");
		if(ptr) settings.bAuth=(atoi(ptr)==1)?true:false;
		
		ptr=httpreq.Request("cascade");
		if(ptr) settings.bCascade=(atoi(ptr)==1)?true:false;
		ptr=httpreq.Request("cassvrip");
		if(ptr) settings.cassvrip.assign(ptr);
		ptr=httpreq.Request("castype");
		if(ptr) settings.castype=atoi(ptr);
		ptr=httpreq.Request("casauth");
		if(ptr) settings.casAuth=(atoi(ptr)==1)?true:false;
		ptr=httpreq.Request("casuser");
		if(ptr) settings.casuser.assign(ptr);
		ptr=httpreq.Request("caspswd");
		if(ptr) settings.caspswd.assign(ptr);
		ptr=httpreq.Request("ipaccess");
		if(ptr) settings.ipaccess=atoi(ptr);
		ptr=httpreq.Request("ipaddr");
		if(ptr) settings.ipRules.assign(ptr);
		ptr=httpreq.Request("blogd");
		if(ptr) settings.bLogdatafile =(atoi(ptr)==1)?true:false;

		proxysvr.saveIni(); //�������ò���
	}//?else if(strcasecmp(ptr_cmd,"setting")==0) 

	//��ȡ�������Ĳ������ú�״̬----start---------------------------------------------
	struct tm * ltime=NULL; time_t t;
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<proxy_status>");
	if(proxysvr.status()==SOCKS_LISTEN) //����������
	{
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>%d</status>",proxysvr.getLocalPort());
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<connected>%d</connected>",proxysvr.curConnection());
		t=proxysvr.getStartTime(); ltime=localtime(&t);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<starttime>%04d��%02d��%02d�� %02d:%02d:%02d</starttime>",
			(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday,ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
	}
	else
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<status>0</status>");
	t=time(NULL); ltime=localtime(&t);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<curtime>%04d��%02d��%02d�� %02d:%02d:%02d</curtime>",
			(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday,ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
	
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<svrport>%d</svrport>",settings.svrport);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<svrtype>%d</svrtype>",settings.svrtype);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<autorun>%d</autorun>",(settings.autorun)?1:0);
	//�󶨱���IP
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<bindip>%s</bindip><localip>",settings.bindip.c_str());
	std::vector<std::string> vec;//�õ�����IP�����صõ�����IP�ĸ���
	long n=socketBase::getLocalHostIP(vec);
	for(int i=0;i<n;i++) buffer.len()+=sprintf(buffer.str()+buffer.len(),"%s ",vec[i].c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"</localip>");
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<bauth>%d</bauth>",(settings.bAuth)?1:0);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<blogd>%d</blogd>",(settings.bLogdatafile)?1:0);

	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<cascade>%d</cascade>",(settings.bCascade)?1:0);
	if( (ptr=strchr(settings.cassvrip.c_str(),',')) ){
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<cassvrip>%s</cassvrip><cassvrport></cassvrport>",settings.cassvrip.c_str());
	}else if( (ptr=strchr(settings.cassvrip.c_str(),':')) )
	{
		*(char *)ptr=0;
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<cassvrip>%s</cassvrip>",settings.cassvrip.c_str());
		*(char *)ptr=':';
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<cassvrport>%s</cassvrport>",ptr+1);
	}else buffer.len()+=sprintf(buffer.str()+buffer.len(),"<cassvrip>%s</cassvrip><cassvrport></cassvrport>",settings.cassvrip.c_str());

	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<castype>%d</castype>",settings.castype);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<casauth>%d</casauth>",(settings.casAuth)?1:0);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<casuser>%s</casuser>",settings.casuser.c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<caspswd>%s</caspswd>",settings.caspswd.c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipfilter>");
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<access>%d</access>",settings.ipaccess);
	if(buffer.Space()<(settings.ipRules.length()+64)) buffer.Resize(buffer.size()+(settings.ipRules.length()+64));
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipaddr>%s</ipaddr>",settings.ipRules.c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"</ipfilter>");
	//��ȡ�������Ĳ������ú�״̬---- end ---------------------------------------------

	if(buffer.Space()<32) buffer.Resize(buffer.size()+32);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</proxy_status></xmlroot>");
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}

//�����û��б�XML
void listuser(cBuffer &buffer,std::map<std::string,TProxyUser> &userlist)
{
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<userlist>");
	std::map<std::string,TProxyUser>::iterator it=userlist.begin();
	for(;it!=userlist.end();it++)
	{
		if(buffer.Space()<((*it).first.length()+32)) buffer.Resize(buffer.size()+((*it).first.length()+32));
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<username>%s</username>",(*it).first.c_str());
	}//?for 
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"</userlist>");
	return;
}
bool webServer::httprsp_proxyusers(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp)
{
	MyService *ptrService=MyService::GetService();
	proxysvrEx *psvr=&ptrService->m_proxysvr;
	const char *ptr_cmd=httpreq.Request("cmd");
	cBuffer buffer(512);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	
	if(strcasecmp(ptr_cmd,"list")==0) //�г�����Proxy�ʺ�
		listuser(buffer,psvr->m_userlist);
	else if(strcasecmp(ptr_cmd,"dele")==0)
	{
		const char *ptr_user=httpreq.Request("user");
		int iret=psvr->deleUser(ptr_user);
		if(iret==1)
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>��Ч���ʺ�!</retmsg>");
		else if(iret==2)
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>��ʱ�޷�ɾ���ʺ�%s!</retmsg>",ptr_user);
		else{
			listuser(buffer,psvr->m_userlist);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>ɾ���ʺųɹ�!</retmsg>");
		}
	}//?else if(strcasecmp(ptr_cmd,"dele")==0)
	else if(strcasecmp(ptr_cmd,"save")==0) //��ӻ��޸��ʺ�
	{
		const char *ptr,*ptr_user=httpreq.Request("account");
		if(ptr_user && ptr_user[0]!=0)
		{
			std::map<std::string,TProxyUser>::iterator it=psvr->m_userlist.end();
			if(ptr_user) it=psvr->m_userlist.find(ptr_user);
			TProxyUser *ptr_puser=NULL;
			if(it!=psvr->m_userlist.end()) //�޸�ָ���ʺŵ���Ϣ
				ptr_puser=&(*it).second;
			else{ //���һ���ʺ�
				TProxyUser puser; 
				puser.username.assign(ptr_user);
				puser.ipaccess =1;
				puser.maxratio =0;
				puser.maxLoginusers =0;
				puser.limitedTime =0;
				puser.forbid=0;
				puser.bAccessDest=1;
				psvr->m_userlist[ptr_user]=puser;
				ptr_puser=&psvr->m_userlist[ptr_user];
			}
			ptr=httpreq.Request("pswd");
			if(ptr) ptr_puser->userpwd.assign(ptr);
			ptr=httpreq.Request("desc");
			if(ptr) ptr_puser->userdesc.assign(ptr);
			ptr=httpreq.Request("expired");
			if(ptr){
				if(ptr[0]==0) 
					ptr_puser->limitedTime=0;
				else{
					struct tm ltm; ::memset((void *)&ltm,0,sizeof(ltm));
					::sscanf(ptr,"%d-%d-%d",&ltm.tm_year,&ltm.tm_mon,&ltm.tm_mday);
					ltm.tm_year-=1900; //��ݴ�1900��ʼ����
					ltm.tm_mon-=1;//�·ݴ�0��ʼ����
					if(ltm.tm_year>100 && ltm.tm_year<200 && 
							ltm.tm_mon>=0 && ltm.tm_mon<=11 && 
							ltm.tm_mday>=1 && ltm.tm_mday<=31 )
					ptr_puser->limitedTime= mktime(&ltm);
				}
			}//?ptr=httpreq.Request("expired");
			ptr=httpreq.Request("maxsignin");
			if(ptr) ptr_puser->maxLoginusers=atoi(ptr);
			ptr=httpreq.Request("maxratio");
			if(ptr) ptr_puser->maxratio=atoi(ptr);
			ptr=httpreq.Request("forbid");
			if(ptr) ptr_puser->forbid=atoi(ptr);
			ptr=httpreq.Request("ipaccess");
			if(ptr) ptr_puser->ipaccess=atoi(ptr);
			ptr=httpreq.Request("ipaddr");
			if(ptr) ptr_puser->ipRules.assign(ptr);

			ptr=httpreq.Request("dstaccess");
			if(ptr) ptr_puser->bAccessDest=atoi(ptr);
			ptr=httpreq.Request("dstaddr");
			if(ptr) ptr_puser->strAccessDest.assign(ptr);

			psvr->modiUser(*ptr_puser);
			listuser(buffer,psvr->m_userlist);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����޸��ʺųɹ�!</retmsg>");
			psvr->saveIni();
		}//?if(ptr_user && ptr_user[0]!=0)
		else
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>��Ч���ʺ�!</retmsg>");
	}//?else if(strcasecmp(ptr_cmd,"save")==0)
	else if(strcasecmp(ptr_cmd,"info")==0)
	{
		const char *ptr_user=httpreq.Request("user");
		std::map<std::string,TProxyUser>::iterator it=psvr->m_userlist.end();
		if(ptr_user) it=psvr->m_userlist.find(ptr_user);
		if(it!=psvr->m_userlist.end())
		{
			TProxyUser &puser=(*it).second;
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<userinfo>");
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<account>%s</account>",puser.username.c_str());
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<password>%s</password>",puser.userpwd.c_str());
			if(puser.limitedTime==0)
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<expired></expired>");
			else{
				struct tm * ltime=localtime(&puser.limitedTime);
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<expired>%04d-%02d-%02d</expired>",
					(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday);
			}
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<maxsignin>%d</maxsignin>",puser.maxLoginusers);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<maxratio>%d</maxratio>",puser.maxratio);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<forbid>%d</forbid>",puser.forbid);
			
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipfilter>");
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<access>%d</access>",puser.ipaccess);
			if(buffer.Space()<(puser.ipRules.length()+64)) buffer.Resize(buffer.size()+(puser.ipRules.length()+64));
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipaddr>%s</ipaddr>",puser.ipRules.c_str());
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"</ipfilter>");
			
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<dstfilter>");
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<access>%d</access>",puser.bAccessDest);
			if(buffer.Space()<(puser.strAccessDest.length()+64)) buffer.Resize(buffer.size()+(puser.strAccessDest.length()+64));
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipaddr>%s</ipaddr>",puser.strAccessDest.c_str());
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"</dstfilter>");

			if(buffer.Space()<(puser.userdesc.length()+64)) buffer.Resize(buffer.size()+(puser.userdesc.length()+64));
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<description>%s</description>",puser.userdesc.c_str());

			buffer.len()+=sprintf(buffer.str()+buffer.len(),"</userinfo>");
		}//?if(it!=pftpsvr->m_userlist.end())
		else
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>��Ч���ʺ�!</retmsg>");
	}
	
	
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}
//Proxy�������õĵ��뵼��
bool webServer::httprsp_proxyini(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp)
{
	MyService *ptrService=MyService::GetService();
	proxysvrEx *psvr=&ptrService->m_proxysvr;
	const char *ptr_cmd=httpreq.Request("cmd");
	cBuffer buffer(512);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");

	if(strcasecmp(ptr_cmd,"out")==0) //����Proxy��������
	{
		std::string strini;
		psvr->saveAsstring(strini);
		if(buffer.Space()<(strini.length()+64)) buffer.Resize(buffer.size()+(strini.length()+64));
		if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<settings><![CDATA[%s]]></settings>",strini.c_str());
	}
	else if(strcasecmp(ptr_cmd,"in")==0) //����Proxy��������
	{
		const char *ptr=httpreq.Request("ini");
		if(ptr){
			psvr->initSetting();
			psvr->m_userlist.clear();
			psvr->parseIni((char *)ptr,0);
			psvr->saveIni(); //�������ò���
		}//?if(ptr)
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>�����������!</retmsg>");
	}//?else if(strcasecmp(ptr_cmd,"in")==0)

	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}
