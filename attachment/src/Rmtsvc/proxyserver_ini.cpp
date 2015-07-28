/*******************************************************************
   *	proxyserver.h 
   *    DESCRIPTION: Proxy�������õı���Ͷ�ȡ
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *	
   *******************************************************************/
#include "rmtsvc.h" 

bool proxysvrEx :: readIni()
{
	HKEY  hKEY;
	LPCTSTR lpRegPath = "Software\\Microsoft\\Windows\\CurrentVersion";
	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegPath, 0, KEY_WRITE|KEY_READ, &hKEY)!=ERROR_SUCCESS)
		return false;
	DWORD dwType=REG_SZ,dwSize=0;
	char *pbuffer=NULL;
	if(::RegQueryValueEx(hKEY, "proxysets", NULL,&dwType,NULL,&dwSize)==ERROR_SUCCESS)
	{
		if( (pbuffer=new char[dwSize]) )
			::RegQueryValueEx(hKEY, "proxysets", NULL,&dwType,(LPBYTE)pbuffer,&dwSize);
	}
	::RegCloseKey(hKEY);
	if(pbuffer==NULL) return false;
	dwSize=cCoder::base64_decode(pbuffer,dwSize-1,pbuffer);
	pbuffer[dwSize]=0;
	parseIni(pbuffer,dwSize);

	delete[] pbuffer; return true;
}

bool proxysvrEx :: parseIni(char *pbuffer,long lsize)
{
	if(pbuffer==NULL) return false;
	if(lsize<=0) lsize=strlen(pbuffer);
	if(strncasecmp(pbuffer,"base64",6)==0)
		lsize=cCoder::base64_decode(pbuffer+6,lsize-6,pbuffer);
//	RW_LOG_DEBUG("read Proxy param.........\r\n%s\r\n",pbuffer);
	//��ʼ����Ini�ļ�
	const char *pstart=pbuffer;
	const char *ptr=strchr(pstart,'\r');
	while(true){
		if(ptr) *(char *)ptr=0;
		if(pstart[0]!='!') //������ע����
		{ 
			if(strncasecmp(pstart,"proxysvr ",9)==0) //���ô���������Ϣ
				docmd_psets(pstart+9);
			else if(strncasecmp(pstart,"casproxy ",9)==0)
				docmd_cassets(pstart+9);
			else if(strncasecmp(pstart,"iprules ",8)==0) //���÷��ʴ˷���IP���˹����ĳ���ʺŵ�
				docmd_iprules(pstart+8);
			else if(strncasecmp(pstart,"proxyuser ",10)==0) //���ñ���������ʺ�
				docmd_puser(pstart+10);
		} 
		if(ptr==NULL) break;
		*(char *)ptr='\r'; pstart=ptr+1;
		if(*pstart=='\n') pstart++;
		ptr=strchr(pstart,'\r');
	}//?while
	return true;
}

bool proxysvrEx :: saveAsstring(std::string &strini)
{
	char buf[256];

	int len=sprintf(buf,"proxysvr port=%d bindip=%s type=%s|%s|%s autorun=%d bauth=%d blogd=%d\r\n",
		m_settings.svrport,m_settings.bindip.c_str(),
		((m_settings.svrtype &PROXY_HTTPS)?"HTTPS":""),
		((m_settings.svrtype &PROXY_SOCKS4)?"SOCKS4":""),
		((m_settings.svrtype &PROXY_SOCKS5)?"SOCKS5":""),
		((m_settings.autorun)?1:0),((m_settings.bAuth)?1:0),
		((m_settings.bLogdatafile)?1:0) );

	strini.append(buf,len);
	len=sprintf(buf,"iprules type=proxy access=%d ipaddr=%s\r\n",
			m_settings.ipaccess,m_settings.ipRules.c_str());
	strini.append(buf,len);
	len=sprintf(buf,"casproxy enabled=%d host=%s type=%s|%s|%s bauth=%d account=%s:%s\r\n",
		((m_settings.bCascade)?1:0),m_settings.cassvrip.c_str(),
		((m_settings.castype &PROXY_HTTPS)?"HTTPS":""),
		((m_settings.castype &PROXY_SOCKS4)?"SOCKS4":""),
		((m_settings.castype &PROXY_SOCKS5)?"SOCKS5":""),
		((m_settings.casAuth)?1:0),m_settings.casuser.c_str(),
		m_settings.caspswd.c_str());
	strini.append(buf,len);

	//-------------------------�����ʺ���Ϣ------------------------
	std::map<std::string,TProxyUser>::iterator it=m_userlist.begin();
	for(;it!=m_userlist.end();it++)
	{
		TProxyUser &puser=(*it).second;
		len=sprintf(buf,"proxyuser account=%s pswd=%s maxlogin=%d forbid=%d maxratio=%d",
			puser.username.c_str(),puser.userpwd.c_str(),puser.maxLoginusers,puser.forbid,
			puser.maxratio);
		strini.append(buf,len);
		if(puser.limitedTime!=0)
		{
			struct tm * ltime=localtime(&puser.limitedTime);
			len=sprintf(buf," expired=%04d-%02d-%02d",(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday);
			strini.append(buf,len);
		}
		strini.append(" desc=\""); strini+=puser.userdesc;
		strini.append("\"\r\n");
		
		len=sprintf(buf,"iprules type=proxyuser account=%s access=%d",puser.username.c_str(),puser.ipaccess);
		strini.append(buf,len);
		len=sprintf(buf," ipaddr=%s\r\n",puser.ipRules.c_str());
		strini.append(buf,len);

		len=sprintf(buf,"iprules type=proxydest account=%s access=%d",puser.username.c_str(),puser.bAccessDest);
		strini.append(buf,len);
		len=sprintf(buf," ipaddr=%s\r\n",puser.strAccessDest.c_str());
		strini.append(buf,len);

	}//?for(
//	RW_LOG_DEBUG("save Proxy param.........\r\n%s\r\n",strini.c_str());
	return true;
}
bool proxysvrEx :: saveIni()
{
	std::string strini;
	saveAsstring(strini);
	//дע���--------------------------------------
	HKEY  hKEY;
	LPCTSTR lpRegPath = "Software\\Microsoft\\Windows\\CurrentVersion";
	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegPath, 0, KEY_WRITE|KEY_READ, &hKEY)==ERROR_SUCCESS)
	{
		long l=cCoder::Base64EncodeSize(strini.length());
		char *pbuf=new char[l];
		if(pbuf){
			l=cCoder::base64_encode ((char *)strini.c_str(),strini.length(),pbuf);
			::RegSetValueEx(hKEY, "proxysets", NULL,REG_SZ, (LPBYTE)pbuf,l+1);
			delete[] pbuf;
		}
		::RegCloseKey(hKEY);
	}
	return true;
}

//���ô���������
//�����ʽ:
//	proxysvr port= [bindip=] type=[HTTPS|SOCKS4|SOCKS5] [bauth=<1|0>] [autorun=<1|0>] [blogd=<1|0>]"
void proxysvrEx :: docmd_psets(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	
	std::map<std::string,std::string>::iterator it;

	if( (it=maps.find("port"))!=maps.end())
		m_settings.svrport=atoi((*it).second.c_str());
	if(m_settings.svrport<0) m_settings.svrport=0;
	if( (it=maps.find("bindip"))!=maps.end())
		m_settings.bindip=(*it).second;
	else m_settings.bindip="";
	if( (it=maps.find("type"))!=maps.end())
	{
		int svrtype=0;
		if(strstr((*it).second.c_str(),"HTTPS"))
			svrtype|=PROXY_HTTPS;
		if(strstr((*it).second.c_str(),"SOCKS4"))
			svrtype|=PROXY_SOCKS4;
		if(strstr((*it).second.c_str(),"SOCKS5"))
			svrtype|=PROXY_SOCKS5;
		if(svrtype!=0) m_settings.svrtype=svrtype;
		else m_settings.svrtype=PROXY_HTTPS|PROXY_SOCKS4|PROXY_SOCKS5;
	}else m_settings.svrtype=PROXY_HTTPS|PROXY_SOCKS4|PROXY_SOCKS5;
	if( (it=maps.find("bauth"))!=maps.end() && (*it).second=="1")
		m_settings.bAuth=true;
	else m_settings.bAuth=false;
	if( (it=maps.find("autorun"))!=maps.end() && (*it).second=="1")
		m_settings.autorun=true;
	else m_settings.autorun=false;
	if( (it=maps.find("blogd"))!=maps.end() && (*it).second=="1")
		m_settings.bLogdatafile=true;
	else m_settings.bLogdatafile=false;

	return;
}
//���ö�������������
//�����ʽ:
//	casproxy enabled=<0|1> host=<�����ַ> type=[HTTPS|SOCKS4|SOCKS5] [bauth=<1|0>] [account=<�ʺ�:����>]"
void proxysvrEx :: docmd_cassets(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("enabled"))!=maps.end() && (*it).second=="1")
		m_settings.bCascade=true;
	else m_settings.bCascade=false;
	
	if( (it=maps.find("host"))!=maps.end() )
		m_settings.cassvrip=(*it).second;
	else m_settings.cassvrip="";
	
	if( (it=maps.find("type"))!=maps.end())
	{
		int castype=0;
		if(strstr((*it).second.c_str(),"HTTPS"))
			castype|=PROXY_HTTPS;
		if(strstr((*it).second.c_str(),"SOCKS4"))
			castype|=PROXY_SOCKS4;
		if(strstr((*it).second.c_str(),"SOCKS5"))
			castype|=PROXY_SOCKS5;
		if(castype!=0) m_settings.castype=castype;
		else m_settings.castype=PROXY_HTTPS|PROXY_SOCKS4|PROXY_SOCKS5;
	}else m_settings.castype=PROXY_HTTPS|PROXY_SOCKS4|PROXY_SOCKS5;

	if( (it=maps.find("bauth"))!=maps.end() && (*it).second=="1")
		m_settings.casAuth=true;
	else m_settings.casAuth=false;
	if( (it=maps.find("account"))!=maps.end() )
	{
		const char *ptr=strchr((*it).second.c_str(),':');
		if(ptr){
			int l=ptr-(*it).second.c_str();
			if(l>0)
				m_settings.casuser.assign((*it).second.c_str(),l);
			else m_settings.casuser="";
			m_settings.caspswd.assign(ptr+1);
		}else{
			m_settings.casuser=(*it).second;
			m_settings.caspswd="";
		}
	}
	return;
}

//����µĴ����������ʺ���Ϣ,����ʺ��Ѵ�����ɾ���ɵģ��������
//�����ʽ: 
//	proxyuser account=<�ʺ�> pswd=<�ʺ�����> [maxlogin=<����ͬʱ����¼����>] [expired=<�ʺ���Ч����>] [maxratio=<������>] 
//account=<�ʺ�> : ������. Ҫ��ӵ��ʺš�
//pswd=<�ʺ�����>   : ������. ָ���ʺŵ����룬
//					����ʺ��������""�������ζ������������ʣ�ֻҪ�ʺ����ԾͿɷ���
//maxlogin=<����ͬʱ����¼����> : ���ƴ�ftp�ʺŵ�ͬʱ��¼��������	��
//					�����������Ĭ��Ϊ0���������ƴ��ʺŵ�ͬʱ��¼����
//expired=<�ʺ���Ч����> : ���ƴ��ʺŵ�ʹ�����ޣ���ʽYYYY-MM-DD
//					������������ʺ���������
//maxratio=<������> : ���ƴ��ʺŵ��������������Kb/��
//					�����������Ĭ��Ϊ0���������ƴ��ʺŵ������������
void proxysvrEx :: docmd_puser(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	TProxyUser puser;
	
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("account"))!=maps.end() )
		puser.username=(*it).second;
	if(puser.username=="") return;
	if( (it=maps.find("pswd"))!=maps.end())
		puser.userpwd=(*it).second;
	if( (it=maps.find("desc"))!=maps.end())
		puser.userdesc=(*it).second;

	if( (it=maps.find("maxratio"))!=maps.end())
	{
		puser.maxratio=atol((*it).second.c_str());
		if(puser.maxratio<0) puser.maxratio=0;
	}else puser.maxratio=0;
	
	if( (it=maps.find("maxlogin"))!=maps.end())
	{
		puser.maxLoginusers=atol((*it).second.c_str());
		if(puser.maxLoginusers<0) puser.maxLoginusers=0;
	}else puser.maxLoginusers=0;

	if( (it=maps.find("expired"))!=maps.end())
	{
		struct tm ltm; ::memset((void *)&ltm,0,sizeof(ltm));
		::sscanf((*it).second.c_str(),"%d-%d-%d",&ltm.tm_year,&ltm.tm_mon,&ltm.tm_mday);
		ltm.tm_year-=1900; //��ݴ�1900��ʼ����
		ltm.tm_mon-=1;//�·ݴ�0��ʼ����
		if(ltm.tm_year>100 && ltm.tm_year<200 && 
			ltm.tm_mon>=0 && ltm.tm_mon<=11 && 
		    ltm.tm_mday>=1 && ltm.tm_mday<=31 )
			puser.limitedTime= mktime(&ltm);
	}else puser.limitedTime=0;

	if( (it=maps.find("forbid"))!=maps.end())
	{
		if((*it).second=="true" || (*it).second=="1")
			puser.forbid=1;
		else puser.forbid=0;
	}else puser.forbid=0;

	puser.ipaccess=1;
	puser.ipRules="";
	puser.bAccessDest=1;
	puser.strAccessDest="";

	m_userlist[puser.username]=puser;
	return ;
}

//����ip���˹�����Զ�������־
//�����ʽ:
//	iprules type=[proxy|proxyuser|proxydest] account=<��������ʺ�>] [access=0|1] ipaddr="<IP>,<IP>,..."
//access=0|1     : �Է�������IP�������Ǿܾ����Ƿ���
void proxysvrEx :: docmd_iprules(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("type"))==maps.end()) return;
	if((*it).second=="proxy")
	{//���ô�������IP���˹���
		if( (it=maps.find("access"))!=maps.end())
			m_settings.ipaccess=atoi((*it).second.c_str());
		
		if( (it=maps.find("ipaddr"))!=maps.end())
			m_settings.ipRules=(*it).second;
	}
	else if((*it).second=="proxyuser")
	{//���ô������ĳ���ʺŵ�IP���˹���
		std::string strUser;
		if( (it=maps.find("account"))!=maps.end())  strUser=(*it).second;

		std::map<std::string,TProxyUser>::iterator it1=m_userlist.find(strUser);
		if(it1==m_userlist.end()) return;
		TProxyUser &puser=(*it1).second;
		if( (it=maps.find("access"))!=maps.end())
			puser.ipaccess=atoi((*it).second.c_str());
		if( (it=maps.find("ipaddr"))!=maps.end())
			puser.ipRules=(*it).second;
	}
	else if((*it).second=="proxyuser")
	{//���ô������ĳ���ʺŵ�Ŀ�Ĺ��˹���
		std::string strUser;
		if( (it=maps.find("account"))!=maps.end())  strUser=(*it).second;

		std::map<std::string,TProxyUser>::iterator it1=m_userlist.find(strUser);
		if(it1==m_userlist.end()) return;
		TProxyUser &puser=(*it1).second;
		if( (it=maps.find("access"))!=maps.end())
			puser.bAccessDest=atoi((*it).second.c_str());
		if( (it=maps.find("ipaddr"))!=maps.end())
			puser.strAccessDest=(*it).second;
	}
	return;
}
