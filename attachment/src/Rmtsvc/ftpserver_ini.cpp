/*******************************************************************
   *	ftpserver.h 
   *    DESCRIPTION: FTP�������õı���Ͷ�ȡ
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *	
   *******************************************************************/
#include "rmtsvc.h" 

bool ftpsvrEx :: readIni()
{
	HKEY  hKEY;
	LPCTSTR lpRegPath = "Software\\Microsoft\\Windows\\CurrentVersion";
	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegPath, 0, KEY_WRITE|KEY_READ, &hKEY)!=ERROR_SUCCESS)
		return false;
	DWORD dwType=REG_SZ,dwSize=0;
	char *pbuffer=NULL;
	if(::RegQueryValueEx(hKEY, "ftpsets", NULL,&dwType,NULL,&dwSize)==ERROR_SUCCESS)
	{
		if( (pbuffer=new char[dwSize]) )
			::RegQueryValueEx(hKEY, "ftpsets", NULL,&dwType,(LPBYTE)pbuffer,&dwSize);
	}
	::RegCloseKey(hKEY);
	if(pbuffer==NULL) return false;
	dwSize=cCoder::base64_decode(pbuffer,dwSize-1,pbuffer);
	pbuffer[dwSize]=0;
	parseIni(pbuffer,dwSize);

	delete[] pbuffer; return true;
}

bool ftpsvrEx :: parseIni(char *pbuffer,long lsize)
{
	if(pbuffer==NULL) return false;
	if(lsize<=0) lsize=strlen(pbuffer);
	if(strncasecmp(pbuffer,"base64",6)==0)
		lsize=cCoder::base64_decode(pbuffer+6,lsize-6,pbuffer);
//	RW_LOG_DEBUG("read FTP param.........\r\n%s\r\n",pbuffer);
	//��ʼ����Ini�ļ�
	const char *pstart=pbuffer;
	const char *ptr=strchr(pstart,'\r');
	while(true){
		if(ptr) *(char *)ptr=0;
		if(pstart[0]!='!') //������ע����
		{ 
			if(strncasecmp(pstart,"sets ",5)==0) //���ñ��������Ϣ
				docmd_sets(pstart+5);
			else if(strncasecmp(pstart,"ssls ",5)==0) //���ñ�����ssl��Ϣ
				docmd_ssls(pstart+5);
			else if(strncasecmp(pstart,"ftps ",5)==0) //����ftp����Ļ�ӭ��Ϣ
				docmd_ftps(pstart+5);
			else if(strncasecmp(pstart,"iprules ",8)==0) //���÷��ʴ˷���IP���˹���
				docmd_iprules(pstart+8);
			else if(strncasecmp(pstart,"user ",5)==0) //����ʺ�
				docmd_user(pstart+5);
			else if(strncasecmp(pstart,"vpath ",6)==0) //����ʺŵ���Ŀ¼
				docmd_vpath(pstart+6);
		} 
		if(ptr==NULL) break;
		*(char *)ptr='\r'; pstart=ptr+1;
		if(*pstart=='\n') pstart++;
		ptr=strchr(pstart,'\r');
	}//?while
	return true;
}

bool ftpsvrEx :: saveAsstring(std::string &strini)
{
	char buf[256];
	int len=sprintf(buf,"sets autorun=%d port=%d bindip=%s maxuser=%d\r\n",
		(m_settings.autoStart)?1:0,m_settings.svrport,m_settings.bindip.c_str(),m_settings.maxUsers);
	strini.append(buf,len);
	
	len=sprintf(buf,"ssls enable=%s\r\n",(m_settings.ifSSLsvr)?"true":"false");
	strini.append(buf,len);

	len=sprintf(buf,"ftps dataport=%d:%d logevent=%d tips=\"",m_settings.dataportB,m_settings.dataportE,
		m_settings.logEvent);
	strini.append(buf,len);
	const char *ptrBegin=m_settings.tips.c_str();
	const char *ptr=strstr(ptrBegin,"\r\n");
	while(true)
	{
		if(ptr)
			strini.append(ptrBegin,ptr-ptrBegin);
		else{ strini.append(ptrBegin); break; }
		strini.append("\\n");
		ptrBegin=ptr+2;
		ptr=strstr(ptrBegin,"\r\n");
	}
	strini.append("\"\r\n");
	//-------------------------�����ʺ���Ϣ------------------------
	std::map<std::string,TFTPUser>::iterator it=m_userlist.begin();
	for(;it!=m_userlist.end();it++)
	{
		TFTPUser &ftpuser=(*it).second;
		len=sprintf(buf,"user account=%s pswd=%s pswdmode=%d hiddenfile=%d maxlogin=%d forbid=%d",
			ftpuser.username.c_str(),ftpuser.userpwd.c_str(),ftpuser.pswdmode,ftpuser.disphidden,
			ftpuser.maxLoginusers,ftpuser.forbid);
		strini.append(buf,len);
		len=sprintf(buf," maxup=%d maxdw=%d maxupfile=%d maxdisksize=%d:%d",
			ftpuser.maxupratio,ftpuser.maxdwratio,ftpuser.maxupfilesize,ftpuser.maxdisksize,ftpuser.curdisksize);
		strini.append(buf,len);
		if(ftpuser.limitedTime!=0)
		{
			struct tm * ltime=localtime(&ftpuser.limitedTime);
			len=sprintf(buf," expired=%04d-%02d-%02d",(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday);
			strini.append(buf,len);
		}
		std::pair<std::string,long> &p=ftpuser.dirAccess["/"];
		len=sprintf(buf," access=%d root=\"%s\" desc=\"",p.second,p.first.c_str());
		strini.append(buf,len);
		strini+=ftpuser.userdesc; strini.append("\"\r\n");
		
		len=sprintf(buf,"iprules account=%s access=%d",ftpuser.username.c_str(),ftpuser.ipaccess);
		strini.append(buf,len);
		len=sprintf(buf," ipaddr=%s\r\n",ftpuser.ipRules.c_str());
		strini.append(buf,len);

		std::map<std::string,std::pair<std::string,long> >::iterator it1=ftpuser.dirAccess.begin();
		it1++; 
		for(;it1!=ftpuser.dirAccess.end();it1++)
		{
			std::pair<std::string,long> &p1=(*it1).second;
			len=sprintf(buf,"vpath account=%s vdir=%s access=%d",ftpuser.username.c_str(),(*it1).first.c_str(),p1.second);
			strini.append(buf,len);
			len=sprintf(buf," rdir=\"%s\"\r\n",p1.first.c_str());
			strini.append(buf,len);
		}
	}//?for(
//	RW_LOG_DEBUG("save FTP param.........\r\n%s\r\n",strini.c_str());
	return true;
}
bool ftpsvrEx :: saveIni()
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
			::RegSetValueEx(hKEY, "ftpsets", NULL,REG_SZ, (LPBYTE)pbuf,l+1);
			delete[] pbuf;
		}
		::RegCloseKey(hKEY);
	}
	return true;
}

//���ñ��������Ϣ
//�����ʽ: 
//	sets [autorun=0|1] [port=<����˿�>] [bindip=<������󶨵ı���IP>] [maxuser=<��󲢷�������>]
//port=<����˿�>    : ���÷���˿ڣ������������Ĭ��Ϊ21
//bindip=<������󶨵ı���IP> : ���ñ�����󶨵ı���IP�������������Ĭ�ϰ󶨱�������IP
//maxuser=<��󲢷�������>   : ���ñ��������󲢷��û�����������������ô�����Ϊ������
void ftpsvrEx :: docmd_sets(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;

	if( (it=maps.find("autorun"))!=maps.end())
	{//���÷���Ķ˿�
		if(atoi((*it).second.c_str())==1)
			m_settings.autoStart=true;
		else m_settings.autoStart=false;
	}
	if( (it=maps.find("port"))!=maps.end())
	{//���÷���Ķ˿�
		m_settings.svrport=atoi((*it).second.c_str());
		if(m_settings.svrport<0) m_settings.svrport=0;
	}
	if( (it=maps.find("bindip"))!=maps.end())
	{//���÷����IP
		m_settings.bindip=(*it).second;
	}
	if( (it=maps.find("maxuser"))!=maps.end())
	{//���÷������󲢷���
		m_settings.maxUsers=atoi((*it).second.c_str());
		if(m_settings.maxUsers<0) m_settings.maxUsers=0;
	}
	return;
}
//���ñ�����ssl��Ϣ
//�����ʽ: 
//	ssls [enable=<true|false>] [cacert=<SSL֤���ļ�>] [cakey=<SSL˽Կ�ļ�>] [capwd=<˽Կ����>]
//enable=<true|false> : ָʾ�����Ƿ�ΪSSL���ܷ����������Ϊtrue��ΪSSL���ܷ���Ĭ��Ϊ��SSL���ܷ���
//��������ָʾSSL����Э�̵�֤�顢˽Կ��˽Կ���룬���֤���˽Կ����һ����д���ó������õ�֤���˽Կ����SSL����Э��
//cacert=<SSL֤���ļ�>: ָ��SSL����Э�̵�֤���ļ�����֤��Ϊ.pem��ʽ��֤�顣��ָ����Ի����·��
//cakey=<SSL˽Կ�ļ�> : ָ��SSL����Э�̵�˽Կ�ļ�����˽ԿΪ.pem��ʽ��˽Կ����ָ����Ի����·��
//capwd=<˽Կ����>    : ָ����˽Կ������
void ftpsvrEx :: docmd_ssls(const char *strParam)
{
#ifdef _SURPPORT_OPENSSL_
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::string strCert,strKey,strPwd;
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("cacert"))!=maps.end()) //����SSL֤��
		strCert=(*it).second;
	if( (it=maps.find("cakey"))!=maps.end()) //����SSL֤��˽Կ
		strKey=(*it).second;
	if( (it=maps.find("capwd"))!=maps.end()) //����SSL֤��˽Կ����
		strPwd=(*it).second;
	if(strCert!="" && strKey!="")
	{
		getAbsolutfilepath(strCert);
		getAbsolutfilepath(strKey);
		setCacert(strCert.c_str(),strKey.c_str(),strPwd.c_str(),false); //ʹ���û�ָ����֤��
	}
	else setCacert(NULL,NULL,NULL,true); //ʹ�����õ�֤��
	if( (it=maps.find("enable"))!=maps.end()) //����SSL֤��
		if(strcasecmp((*it).second.c_str(),"true")==0)
			m_settings.ifSSLsvr=true;
		else m_settings.ifSSLsvr=false;
#endif
	return;
}

//����FTP����Ļ�ӭ��Ϣ�Ͱ����ݴ���˿�
//�����ʽ: 
//	ftps [dataport=<�˿���ʼ:�˿ڽ���>] [tips="<��ӭ����Ϣ>"] [logevent=LOGIN|LOGOUT|DELETE|RMD|UPLOAD|DWLOAD|SITE]
//dataport=<�˿���ʼ:�˿ڽ���>: ���ñ�FTP�����PASV���ݴ���˿ڣ������ָ������ϵͳ������䣬��������ָ���������������
//	[�˿���ʼ:�˿ڽ���]��ΧΪ�����䣬�˿�Ϊ0������ޣ�����[500,0]�������Ķ˿�Ҫ>=500,[0,600]�������Ķ˿�Ҫ<=600
//tips="<��ӭ����Ϣ>" : ָ��ftp�ͻ������ӵ��������ʾ�Ļ�ӭ��Ϣ����Ϊ��ӭ��Ϣ���ܰ����ո���������""����
//	����д���л�ӭ��Ϣ��ÿ�еĽ�β������"\n"�ַ���������ÿ�е���ʼ��"220-"��ͷ
//logevent=LOGIN|LOGOUT|DELETE|RMD|UPLOAD|DWLOAD|SITE : ָ����¼��ЩINFO�¼�
//		    ��loglevel��¼��־�������DEBUG��INFOʱָ��Ҫ��¼��Щ�¼���Ĭ�ϼ�¼LOGIN,LOGOUT,SITE��չ�����¼�
//		    ��Ҫָ����¼����¼�����|���ӡ�����logevent=LOGIN|LOGOUT|DELETE
//		    LOGIN�¼�  -- ��¼ĳ���û���¼ʱ���IP
//		    LOGOUT�¼� -- ��¼ĳ���û��뿪ʱ���IP
//		    DELETE�¼� -- ��¼ĳ���û�ɾ���ļ�
//		    RMD�¼�    -- ��¼ĳ���û�ɾ��Ŀ¼
//		    UPLOAD�¼� -- ��¼ĳ���û������ļ�
//		    DWLOAD�¼� -- ��¼ĳ���û������ļ�
//		    SITE�¼�   -- ��¼ĳ���û�ִ��FTP��չSITE����(������SITE INFO)�Լ�����ִ�н��
//����: ftps tips="220-��ӭ����XXXftp����\n220-test line.\n"
void ftpsvrEx :: docmd_ftps(const char *strParam)
{
	if(strParam==NULL) return;
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("dataport"))!=maps.end())
	{//���÷�������ݴ���˿ڷ�Χ
		m_settings.dataportB=0;
		m_settings.dataportE=0;
		const char *ptr=strchr((*it).second.c_str(),':');
		if(ptr){
			*(char *)ptr=0;
			m_settings.dataportB=atoi((*it).second.c_str());
			*(char *)ptr=':';
			m_settings.dataportE=atoi(ptr+1);
		}
		else m_settings.dataportB=m_settings.dataportE=atoi((*it).second.c_str());
	}
	if( (it=maps.find("logevent"))!=maps.end())
	{//����Ҫ�����־���¼�
		const char *ptr=(*it).second.c_str();
		m_settings.logEvent=atoi(ptr);
		if(strstr(ptr,"LOGIN")) m_settings.logEvent|=FTP_LOGEVENT_LOGIN;
		if(strstr(ptr,"LOGOUT")) m_settings.logEvent|=FTP_LOGEVENT_LOGOUT;
		if(strstr(ptr,"DELETE")) m_settings.logEvent|=FTP_LOGEVENT_DELETE;
		if(strstr(ptr,"RMD")) m_settings.logEvent|=FTP_LOGEVENT_RMD;
		if(strstr(ptr,"UPLOAD")) m_settings.logEvent|=FTP_LOGEVENT_UPLOAD;
		if(strstr(ptr,"DWLOAD")) m_settings.logEvent|=FTP_LOGEVENT_DWLOAD;
		if(strstr(ptr,"SITE")) m_settings.logEvent|=FTP_LOGEVENT_SITE;
	}
	if( (it=maps.find("tips"))!=maps.end())
	{//��"\n"�滻Ϊ\r\n
		const char *ptr,*ptrStart=(*it).second.c_str();
		while( (ptr=strstr(ptrStart,"\\n")) )
		{
			*(char *)ptr='\r';
			*(char *)(ptr+1)='\n';
			ptrStart=ptr+2;
		}
		m_settings.tips=(*it).second;
	}	
	return;
}

//����µ�ftp�ʺ���Ϣ,����ʺ��Ѵ�����ɾ���ɵģ��������
//�����ʽ: 
//	user account=<ftp�ʺ�> pswd=<�ʺ�����> root=<��Ŀ¼> [pswdmode=<OTP_NONE|OTP_MD4|OTP_MD5>] [access=<����Ŀ¼�ķ���Ȩ��>] [hiddenfile=HIDE] [maxlogin=<����ͬʱ����¼����>] [expired=<�ʺ���Ч����>] [maxup=<�����������>] [maxdw=<�����������>] [maxupfile=<��������ļ���С>] [maxdisksize=<����ʹ�ô��̿ռ�>[:<��ǰ��ʹ�ô��̿ռ�>]]
//account=<ftp�ʺ�> : ������. Ҫ��ӵ�ftp�ʺš�
//pswd=<�ʺ�����>   : ������. ָ��ftp�ʺŵ����룬
//					����ʺ��������""�������ζ������������ʣ�ֻҪ�ʺ����ԾͿɷ���
//root=<��Ŀ¼>     : ������. ָ����ftp�ʺŵĸ�/��Ӧ����Ŀ¼����Ŀ¼������ftp���񱾵ػ���ʵ�ʾ���·��
//					�����Ŀ¼�а����ո���Ҫ��""����Ŀ¼����
//					�����Ŀ¼����""�������ζ�Ŵ��ʺſɷ��ʼ�������д���Ŀ¼
//pswdmode=<OTP_NONE|OTP_MD4|OTP_MD5> : ���뱣��ģʽ��OTP_NONE:���Ĵ������� 
//					OTP_MD4: otp S/Key md4�������봫��. ftp�ͻ��˵����뱣��Ӧ����Ϊmd4��otp�Զ����ģʽ
//					OTP_MD4: otp S/Key md5�������봫��. ftp�ͻ��˵����뱣��Ӧ����Ϊmd5��otp�Զ����ģʽ
//					�����������Ĭ����OTP_NONE,���������Ĵ��䡣
//access=<����Ŀ¼�ķ���Ȩ��> : ָ����ftp�ʺŵ���Ŀ¼�ķ���Ȩ�ޡ�
//						�����������Ĭ�Ͼ���ACCESS_ALL����Ȩ�ޡ����ø�ʽ�ͺ�������
//		<����Ŀ¼�ķ���Ȩ��> : <FILE_READ|FILE_WRITE|FILE_DEL|FILE_EXEC|DIR_LIST|DIR_MAKE|DIR_DEL|DIR_NOINHERIT>
//		ACCESS_ALL=FILE_READ|FILE_WRITE|FILE_DEL|FILE_EXEC|DIR_LIST|DIR_MAKE|DIR_DEL
//		FILE_READ : ������ļ� FILE_WRITE : ����д�ļ� FILE_DEL : ����ɾ���ļ� FILE_EXEC : ����ִ���ļ�
//		DIR_LIST : ����Ŀ¼�ļ�list DIR_MAKE : ������Ŀ¼ DIR_DEL : ����ɾ��Ŀ¼
//		DIR_NOINHERIT : �Ƿ��������Ŀ¼��Ӧ����ʵ·���µ���Ŀ¼�̳��û�ָ����Ŀ¼����Ȩ�ޡ�����û�ָ���˴������ֹ�̳У�
//		    ��ôĬ��ftp�û����޷�������Ŀ¼�����κεĶ�д������Ҫ������µ���Ŀ¼���в��������뽫���µ���Ŀ¼ӳ��Ϊ��Ŀ¼����ָ������Ȩ��
//		ע��:����趨��FILE_EXEC �����û���ͨ����չFTP����EXEC,Զ��ִ�д���Ŀ¼�µ��ļ���
//hiddenfile=HIDE   : ����ʾ�����ļ�����������ô���Ĭ��Ϊ��ʾ�����ļ�/Ŀ¼
//maxlogin=<����ͬʱ����¼����> : ���ƴ�ftp�ʺŵ�ͬʱ��¼��������	��
//					�����������Ĭ��Ϊ0���������ƴ��ʺŵ�ͬʱ��¼����
//expired=<�ʺ���Ч����> : ���ƴ��ʺŵ�ʹ�����ޣ���ʽYYYY-MM-DD
//					������������ʺ���������
//maxup=<�����������> : ���ƴ�ftp�ʺŵ������������Kb/��
//					�����������Ĭ��Ϊ0���������ƴ��ʺŵ������������
//maxdw=<�����������> : ���ƴ�ftp�ʺŵ������������Kb/��
//					�����������Ĭ��Ϊ0���������ƴ��ʺŵ������������
//maxupfile=<��������ļ���С> : ���ƴ�ftp�ʺŵ���������ļ��Ĵ�С KBytes
//					�����������Ĭ��Ϊ0���������ƴ��ʺŵ���������ļ���С
//maxdisksize=<����ʹ�ô��̿ռ�>[:<��ǰ��ʹ�ô��̿ռ�>] : ���ƴ��ʺ�����ʹ�õĴ��̿ռ�KBytes
//				<����ʹ�ô��̿ռ�>: KBytes,���ô��ʺ�����ʹ�õĴ��̿ռ䡣����������� Ĭ��Ϊ0��������
//				[:<��ǰ��ʹ�ô��̿ռ�>] :KBytes,���õ�ǰ��ʹ�õĴ��̿ռ䡣�����������Ĭ��Ϊ0����û��ʹ��
void ftpsvrEx :: docmd_user(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	TFTPUser ftpuser;
	
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("account"))!=maps.end() )
		ftpuser.username=(*it).second;
	if(ftpuser.username=="") return;
	if( (it=maps.find("pswd"))!=maps.end())
		ftpuser.userpwd=(*it).second;
	if( (it=maps.find("desc"))!=maps.end())
		ftpuser.userdesc=(*it).second;

	std::pair<std::string,long> p("",0);
	if( (it=maps.find("root"))!=maps.end())
		p.first=(*it).second;
	if( (it=maps.find("access"))!=maps.end() && (*it).second!="")
	{
		const char *ptr=(*it).second.c_str();
		long lAccess=atoi(ptr);
		if(strstr(ptr,"FILE_READ")) lAccess|=FTP_ACCESS_FILE_READ;
		if(strstr(ptr,"FILE_WRITE")) lAccess|=FTP_ACCESS_FILE_WRITE;
		if(strstr(ptr,"FILE_DEL")) lAccess|=FTP_ACCESS_FILE_DELETE;
		if(strstr(ptr,"FILE_EXEC")) lAccess|=FTP_ACCESS_FILE_EXEC;
		if(strstr(ptr,"DIR_LIST")) lAccess|=FTP_ACCESS_DIR_LIST;
		if(strstr(ptr,"DIR_MAKE")) lAccess|=FTP_ACCESS_DIR_CREATE;
		if(strstr(ptr,"DIR_DEL")) lAccess|=FTP_ACCESS_DIR_DELETE;
		if(strstr(ptr,"DIR_NOINHERIT")) lAccess|=FTP_ACCESS_SUBDIR_INHERIT;
		if(strstr(ptr,"ACCESS_ALL")) lAccess=FTP_ACCESS_ALL;
		p.second=lAccess;
	}
	ftpuser.dirAccess["/"]=p;

	if( (it=maps.find("maxup"))!=maps.end())
	{
		ftpuser.maxupratio=atol((*it).second.c_str());
		if(ftpuser.maxupratio<0) ftpuser.maxupratio=0;
	}else ftpuser.maxupratio=0;
	
	if( (it=maps.find("maxdw"))!=maps.end())
	{
		ftpuser.maxdwratio=atol((*it).second.c_str());
		if(ftpuser.maxdwratio<0) ftpuser.maxdwratio=0;
	}else ftpuser.maxdwratio=0;

	if( (it=maps.find("maxupfile"))!=maps.end())
	{
		ftpuser.maxupfilesize=atol((*it).second.c_str());
		if(ftpuser.maxupfilesize<0) ftpuser.maxupfilesize=0;
	}else ftpuser.maxupfilesize=0;
	
	if( (it=maps.find("maxlogin"))!=maps.end())
	{
		ftpuser.maxLoginusers=atol((*it).second.c_str());
		if(ftpuser.maxLoginusers<0) ftpuser.maxLoginusers=0;
	}else ftpuser.maxLoginusers=0;

	if( (it=maps.find("expired"))!=maps.end())
	{
		struct tm ltm; ::memset((void *)&ltm,0,sizeof(ltm));
		::sscanf((*it).second.c_str(),"%d-%d-%d",&ltm.tm_year,&ltm.tm_mon,&ltm.tm_mday);
		ltm.tm_year-=1900; //��ݴ�1900��ʼ����
		ltm.tm_mon-=1;//�·ݴ�0��ʼ����
		if(ltm.tm_year>100 && ltm.tm_year<200 && 
			ltm.tm_mon>=0 && ltm.tm_mon<=11 && 
		    ltm.tm_mday>=1 && ltm.tm_mday<=31 )
			ftpuser.limitedTime= mktime(&ltm);
	}else ftpuser.limitedTime=0;

	if( (it=maps.find("maxdisksize"))!=maps.end())
	{
		long lsize=atol((*it).second.c_str());
		ftpuser.maxdisksize=(lsize<0)?0:lsize;
		const char *ptr=strchr((*it).second.c_str(),':');
		if(ptr) ftpuser.curdisksize=atol(ptr+1);
		if(ftpuser.curdisksize<0) ftpuser.curdisksize=0;
	}else ftpuser.maxdisksize=ftpuser.curdisksize=0;
	
	if( (it=maps.find("hiddenfile"))!=maps.end())
		ftpuser.disphidden=((*it).second=="HIDE" || (*it).second=="0")?0:1;
	else ftpuser.disphidden=0;

	if( (it=maps.find("pswdmode"))!=maps.end())
	{
		if((*it).second=="OTP_MD4") 
			ftpuser.pswdmode=OTP_MD4;
		else if((*it).second=="OTP_MD5")
			ftpuser.pswdmode=OTP_MD5;
		else if((*it).second=="OTP_NONE")
			ftpuser.pswdmode=OTP_NONE;
		else ftpuser.pswdmode=atoi((*it).second.c_str());
	}else ftpuser.pswdmode=0;

	if( (it=maps.find("forbid"))!=maps.end())
	{
		if((*it).second=="true" || (*it).second=="1")
			ftpuser.forbid=1;
		else ftpuser.forbid=0;
	}else ftpuser.forbid=0;

	ftpuser.ipaccess=0;
	ftpuser.ipRules="";

	m_userlist[ftpuser.username]=ftpuser;
	return ;
}

//����ftp�����ip���˹�������ĳ���ʺŵ�IP���˹���
//�����ʽ:
//	iprules [account=<ftp�ʺ�>] [access=0|1] ipaddr="<IP>,<IP>,..."
//account=<ftp�ʺ�> : ָ���˹��˹��������ĳ���ʺŵĻ�����������ʺ����õ�
//						��������ô������ftp�ʺ�����Ϊ""�������������ʺ�����
//access=0|1     : �Է�������IP�������Ǿܾ����Ƿ���
//����:
// iprules access=0 rules="192.168.0.*,192.168.1.10"
void ftpsvrEx :: docmd_iprules(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::string strUser;

	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("account"))!=maps.end())  strUser=(*it).second;

	std::map<std::string,TFTPUser>::iterator it1=m_userlist.find(strUser);
	if(it1==m_userlist.end()) return;
	TFTPUser &ftpuser=(*it1).second;
	
	if( (it=maps.find("access"))!=maps.end())
		ftpuser.ipaccess=atoi((*it).second.c_str());
	
	if( (it=maps.find("ipaddr"))!=maps.end())
		ftpuser.ipRules=(*it).second;

	return;
}

//����޸�ɾ��ftp�ʺŵ���Ŀ¼
//�����ʽ: 
//	vpath account=<ftp�ʺ�> vdir=<��Ŀ¼> rdir=<��ʵĿ¼> [access=<��Ŀ¼����Ȩ��>]
//account=<ftp�ʺ�> : ������. Ҫ������Ŀ¼��ftp�ʺš�
//vdir=<��Ŀ¼>     : ������. Ҫ��ӻ��޸ĵ���Ŀ¼��ÿ����Ŀ¼������/��ʼ������/aa
//		    �����ô�Ŀ¼����޸�ɾ�����Ŀ¼��ע��:��Ŀ¼���ִ�Сд
//rdir=<��ʵĿ¼>   : ����Ŀ¼��Ӧ����ʵĿ¼·��,�����Ǿ���·��,
//		    ���<��ʵĿ¼>�а����ո���Ҫ��""��<��ʵĿ¼>����,����"c:\temp test\aa"
//		    ���û������rdir��<��ʵĿ¼>���ڿ��ַ�������rdir= ��rdir=""����ζ��ɾ������·��
//access=<��Ŀ¼����Ȩ��> : ָ���Դ���Ŀ¼�ķ���Ȩ�ޡ������������Ĭ�Ͼ���ACCESS_ALL����Ȩ�ޡ����ø�ʽ�ͺ�������
//		 <��Ŀ¼����Ȩ��> : <FILE_READ|FILE_WRITE|FILE_DEL|FILE_EXEC|DIR_LIST|DIR_MAKE|DIR_DEL|DIR_NOINHERIT>
//		 ACCESS_ALL=FILE_READ|FILE_WRITE|FILE_DEL|FILE_EXEC|DIR_LIST|DIR_MAKE|DIR_DEL
//		 FILE_READ : ������ļ� FILE_WRITE : ����д�ļ� FILE_DEL : ����ɾ���ļ� FILE_EXEC : ����ִ���ļ�
//		 DIR_LIST : ����Ŀ¼�ļ�list DIR_MAKE : ������Ŀ¼ DIR_DEL : ����ɾ��Ŀ¼
//		 DIR_NOINHERIT : �Ƿ��������Ŀ¼��Ӧ����ʵ·���µ���Ŀ¼�̳��û�ָ����Ŀ¼����Ȩ�ޡ�����û�ָ���˴������ֹ�̳У�
//		    ��ôĬ��ftp�û����޷�������Ŀ¼�����κεĶ�д������Ҫ������µ���Ŀ¼���в��������뽫���µ���Ŀ¼ӳ��Ϊ��Ŀ¼����ָ������Ȩ��
//		 ע��:����趨��FILE_EXEC �����û���ͨ����չFTP����EXEC,Զ��ִ�д���Ŀ¼�µ��ļ���
void ftpsvrEx :: docmd_vpath(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;
	std::string strvdir;

	if( (it=maps.find("account"))!=maps.end())  strvdir=(*it).second;
	std::map<std::string,TFTPUser>::iterator it1=m_userlist.find(strvdir);
	if(it1==m_userlist.end()) return;
	TFTPUser &ftpuser=(*it1).second;

	std::pair<std::string,long> p("",0); strvdir="";
	if( (it=maps.find("vdir"))!=maps.end())  strvdir=(*it).second;
	if( strvdir=="") return;

	if( (it=maps.find("rdir"))!=maps.end()) p.first=(*it).second;
	if( (it=maps.find("access"))!=maps.end() && (*it).second!="")
	{
		const char *ptr=(*it).second.c_str();
		p.second=atol(ptr); 
		if(strstr(ptr,"FILE_READ")) p.second|=FTP_ACCESS_FILE_READ;
		if(strstr(ptr,"FILE_WRITE")) p.second|=FTP_ACCESS_FILE_WRITE;
		if(strstr(ptr,"FILE_DEL")) p.second|=FTP_ACCESS_FILE_DELETE;
		if(strstr(ptr,"FILE_EXEC")) p.second|=FTP_ACCESS_FILE_EXEC;
		if(strstr(ptr,"DIR_LIST")) p.second|=FTP_ACCESS_DIR_LIST;
		if(strstr(ptr,"DIR_MAKE")) p.second|=FTP_ACCESS_DIR_CREATE;
		if(strstr(ptr,"DIR_DEL")) p.second|=FTP_ACCESS_DIR_DELETE;
		if(strstr(ptr,"DIR_NOINHERIT")) p.second|=FTP_ACCESS_SUBDIR_INHERIT;
		if(strstr(ptr,"ACCESS_ALL")) p.second=FTP_ACCESS_ALL;
	}

	ftpuser.dirAccess[strvdir]=p;
	return;
}
