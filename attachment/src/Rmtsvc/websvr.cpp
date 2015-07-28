/*******************************************************************
   *	websvr.cpp 
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

webServer :: webServer():m_svrport(7778)
{
	m_bPowerOff=false;
#ifdef _SURPPORT_OPENSSL_
		setCacert(NULL,NULL,NULL,true); //Ĭ�ϼ������õ�֤��
#endif
		setRoot(NULL,HTTP_ACCESS_NONE,NULL);
		
		m_quality=30;
		m_dwImgSize=0;
		m_bGetFileFromRes=true;

		m_defaultPage="index.htm";

		m_bSSLenabled=false;
		m_bSSLverify=false;
		m_bAnonymous=true;
}


//��������
bool webServer :: Start() 
{
	if(m_svrport==0) return true; //����������
#ifdef _SURPPORT_OPENSSL_
	if(m_bSSLenabled) //����SSL����
	{
		if(g_strMyCert=="" || g_strMyKey=="")
			setCacert(NULL,NULL,NULL,true,NULL,NULL); //ʹ�����õ�֤��
		else if(m_bSSLverify)
			setCacert(g_strMyCert.c_str(),g_strMyKey.c_str(),g_strKeyPswd.c_str(),false,
			g_strCaCert.c_str(),g_strCaCRL.c_str()); //ʹ���û�ָ����֤���CRL
		else setCacert(g_strMyCert.c_str(),g_strMyKey.c_str(),g_strKeyPswd.c_str(),false,NULL,NULL);
		this->initSSL(true,NULL);
	}
#endif
	
	const char *ip=(m_bindip=="")?NULL:m_bindip.c_str();
	BOOL bReuseAddr=(ip)?SO_REUSEADDR:FALSE;//����IP������˿�����
	SOCKSRESULT sr=Listen( ((m_svrport<0)?0:m_svrport) ,bReuseAddr,ip);
	return (sr>0)?true:false;
}

void webServer :: Stop()
{ 
	Close();
#ifdef _SURPPORT_OPENSSL_
	freeSSL();
#endif
	return;
}

void webServer :: setRoot(const char *rpath,long lAccess,const char *defaultPage)
{
	std::string spath; if(rpath) spath.assign(rpath);
	if(spath!="/"){
		getAbsolutfilepath(spath);
		if(spath[spath.length()-1]!='\\') spath.append("\\");
	}else spath="";
	this->setvpath("/",spath.c_str(),lAccess);
	if(defaultPage && defaultPage[0]!=0) m_defaultPage.assign(defaultPage);
	return;
}

//���onHttpReq������������������Ѿ������ˣ�����httpsvr�����ٴ���
//����ת��������httpsvr��������
bool webServer :: onHttpReq(socketTCP *psock,httpRequest &httpreq,httpSession &session,
			std::map<std::string,std::string>& application,httpResponse &httprsp)
{
	if(m_bPowerOff && strcasecmp(httpreq.url().c_str(),"/command")==0)
	{//����Ȩ���κ��˶���Զ��ִ�йػ���������
		const char *ptr_cmd=httpreq.Request("cmd");
		httprsp_command(psock,httprsp,ptr_cmd);
		return true;
	}
	if(strcasecmp(httpreq.url().c_str(),"/checkcode")==0)
	{//��������У����
		httprsp_checkcode(psock,httprsp,session);
		return true;
	}
	if(httpreq.url()=="/login") //�û����͵���֤����
	{
		if(httprsp_login(psock,httpreq,httprsp,session)) return true;
	}//?if(httpreq.url()=="/login")
	//ͨ�����������urlֱ�������ļ������������ع��������� //yyc add 2007-11-06
	if(strncasecmp(httpreq.url().c_str(),"/dwfiles/",9)==0)
	{
		const char *ptr=strchr(httpreq.url().c_str()+9,'/');
		if(ptr==NULL) return true; else *(char *)ptr=0;
		httpSession *psession=this->GetSession(httpreq.url().c_str()+9);
		if(psession==NULL) return true; else ptr++;//ptr ָ���ļ���
		long lqx=atol( (*psession)["lAccess"].c_str() );
		if((lqx & RMTSVC_ACCESS_FILE_VIEW)==0) return true;
		
		const char *ptr_path=httpreq.Request("path");
		if(ptr_path==NULL) return true;
		string filepath(ptr_path);
		if(filepath[filepath.length()-1]!='\\') filepath.append("\\");
		filepath.append(ptr);

		long lstartpos,lendpos;//��ȡ�ļ��ķ�Χ
		int iRangeNums=httpreq.get_requestRange(&lstartpos,&lendpos,0);
		if(iRangeNums>1){
			long *lppos=new long[iRangeNums*2];
			if(lppos==NULL) return true;
			for(int i=0;i<iRangeNums;i++) httpreq.get_requestRange(&lppos[i],&lppos[iRangeNums+i],i);
			httprsp.sendfile(psock,filepath.c_str(),MIMETYPE_UNKNOWED,&lppos[0],&lppos[iRangeNums],iRangeNums);
			delete[] lppos; return true;
		}else if(iRangeNums==0) { lstartpos=0; lendpos=-1; }
		httprsp.sendfile(psock,filepath.c_str(),MIMETYPE_UNKNOWED,lstartpos,lendpos);
		return true;
	}
	
	//�ж��û��Ƿ񾭹���֤������ȡȨ��
	long lAccess=atol(session["lAccess"].c_str());
	if(lAccess==RMTSVC_ACCESS_NONE){
		if(m_bAnonymous){//�����ǰ��������½ģʽ
			session["user"]=string("Anonymous");
			session["lAccess"]=string("-1");
		}else if(strcasecmp(httpreq.url().c_str(),"/login.htm")!=0)
		{//���û�з���Ȩ���ҵ�ǰ���ʵĲ��ǵ�½ҳ������ת����½ҳ��
			this->httprsp_Redirect(psock,httprsp,"/login.htm"); 
			return true;
		}
	}//û�з���Ȩ��
	
	if(httpreq.url()=="/"){ //��ת��Ĭ��ҳ��
		httpreq.url()+=m_defaultPage;
		this->httprsp_Redirect(psock,httprsp,httpreq.url().c_str());
		return true;
	}
//----------------------------------------------------------------------------
//-----------------------URL ����---------------------------------------------
	if(strcasecmp(httpreq.url().c_str(),"/capSetting")==0)
	{
		httprsp_capSetting(psock,httpreq,httprsp,session,((lAccess & RMTSVC_ACCESS_SCREEN_ALL)!=0) );
		return true;
	}
	else if(strcasecmp(httpreq.url().c_str(),"/version")==0)
	{
		httprsp_version(psock,httprsp);
		return true;
	}
	else if(strcasecmp(httpreq.url().c_str(),"/cmdpage")==0)
	{
		const char *ptr_cmd=httpreq.Request("cmd");
		httprsp_cmdpage(psock,httprsp,ptr_cmd);
		return true;
	}
	//����û�������Ļ����鿴Ȩ��
	if((lAccess & RMTSVC_ACCESS_SCREEN_ALL)!=0)
	{
		if(strcasecmp(httpreq.url().c_str(),"/capWindow")==0)
		{//�����Ƿ��������ָ������
			httprsp_capWindow(psock,httpreq,httprsp,session);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/getPassword")==0)
		{
			httprsp_getpswdfromwnd(psock,httpreq,httprsp,session);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/capDesktop")==0)
		{
			httprsp_capDesktop(psock,httprsp,session);
			return true;
		}else if((lAccess & RMTSVC_ACCESS_SCREEN_ALL)==RMTSVC_ACCESS_SCREEN_ALL)
		{//����û�������ȫ����Ȩ��
			if(strcasecmp(httpreq.url().c_str(),"/getclipboard")==0)
			{
				httprsp_GetClipBoard(psock,httprsp);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/setclipboard")==0)
			{
				const char *ptr_val=httpreq.Request("val");
				httprsp_SetClipBoard(psock,httprsp,ptr_val);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/msevent")==0)
			{
				httprsp_msevent(psock,httpreq,httprsp,session);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/keyevent")==0)
			{
				httprsp_keyevent(psock,httpreq,httprsp);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/command")==0)
			{
				const char *ptr_cmd=httpreq.Request("cmd");
				httprsp_command(psock,httprsp,ptr_cmd);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/sysinfo")==0)
			{
				httprsp_sysinfo(psock,httprsp);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/usageimage")==0)
			{
				httprsp_usageimage(psock,httprsp);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/fport")==0)
			{
				httprsp_fport(psock,httprsp);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/plist")==0)
			{
				httprsp_plist(psock,httprsp);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/pkill")==0)
			{
				const char *strpid=httpreq.Request("pid");
				DWORD pid=(strpid)?((DWORD)atol(strpid)):0;
				httprsp_pkill(psock,httprsp,pid);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/mlist")==0)
			{
				const char *strpid=httpreq.Request("pid");
				DWORD pid=(strpid)?((DWORD)atol(strpid)):0;
				httprsp_mlist(psock,httprsp,pid);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/mdattach")==0)
			{
				const char *ptr=httpreq.Request("pid");
				DWORD pid=(ptr)?((DWORD)atol(ptr)):0;
				ptr=httpreq.Request("hmdl");
				HMODULE hmdl=(ptr)?((HMODULE)atol(ptr)):0;
				ptr=httpreq.Request("count");
				long lCount=(ptr)?atol(ptr):1;
				if(pid && hmdl) httprsp_mdattach(psock,httprsp,pid,hmdl,lCount);
				return true;
			}
		}
	}//?if((lAccess & RMTSVC_ACCESS_SCREEN_ALL)!=0)
	//����û�����ע������Ȩ��
	if((lAccess & RMTSVC_ACCESS_REGIST_ALL)!=0)
	{
		if(strcasecmp(httpreq.url().c_str(),"/reglist")==0)
		{
			int listWhat=1;
			const char *ptr_key=httpreq.Request("listwhat");
			if(ptr_key) listWhat=atoi(ptr_key);
			ptr_key=httpreq.Request("rkey");
			httprsp_reglist(psock,httprsp,ptr_key,listWhat);
			return true;
		}
		else if( (lAccess & RMTSVC_ACCESS_REGIST_ALL)==RMTSVC_ACCESS_REGIST_ALL)
		{
			if(strcasecmp(httpreq.url().c_str(),"/regkey_del")==0)
			{
				const char *ptr_path=httpreq.Request("rpath");
				const char *ptr_key=httpreq.Request("rkey");
				httprsp_regkey_del(psock,httprsp,ptr_path,ptr_key);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/regkey_add")==0)
			{
				const char *ptr_path=httpreq.Request("rpath");
				const char *ptr_key=httpreq.Request("rkey");
				httprsp_regkey_add(psock,httprsp,ptr_path,ptr_key);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/regitem_del")==0)
			{
				const char *ptr_path=httpreq.Request("rpath");
				const char *ptr_name=httpreq.Request("rname");
				httprsp_regitem_del(psock,httprsp,ptr_path,ptr_name);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/regitem_add")==0)
			{
				const char *ptr_path=httpreq.Request("rpath");
				const char *ptr_type=httpreq.Request("rtype");
				const char *ptr_name=httpreq.Request("rname");
				const char *ptr_value=httpreq.Request("rdata");
				httprsp_regitem_add(psock,httprsp,ptr_path,ptr_type,ptr_name,ptr_value);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/regitem_md")==0)
			{
				const char *ptr_path=httpreq.Request("rpath");
				const char *ptr_type=httpreq.Request("rtype");
				const char *ptr_name=httpreq.Request("rname");
				const char *ptr_value=httpreq.Request("rdata");
				httprsp_regitem_md(psock,httprsp,ptr_path,ptr_type,ptr_name,ptr_value);
				return true;
			}
		}//?else if( (lAccess & RMTSVC_ACCESS_REGIST_ALL)==RMTSVC_ACCESS_REGIST_ALL)
	}//?if((lAccess & RMTSVC_ACCESS_REGIST_ALL)!=0)
	//����û����з������Ȩ��
	if((lAccess & RMTSVC_ACCESS_SERVICE_ALL)!=0)
	{
		if(strcasecmp(httpreq.url().c_str(),"/slist")==0)
		{
			const char *ptr_sname=httpreq.Request("sname");
			if(ptr_sname && //?�Է�����ж�������
			  (lAccess & RMTSVC_ACCESS_SERVICE_ALL)==RMTSVC_ACCESS_SERVICE_ALL ) 
				sevent(ptr_sname,httpreq.Request("cmd"));
			httprsp_slist(psock,httprsp);
			return true;
		}
	}//?if((lAccess & RMTSVC_ACCESS_SERVICE_ALL)!=0) 
	//����û�����telnetȨ��
	if((lAccess & RMTSVC_ACCESS_TELNET_ALL)==RMTSVC_ACCESS_TELNET_ALL)
	{
		if(strcasecmp(httpreq.url().c_str(),"/telnet")==0)
		{
			httprsp_telnet(psock,httprsp,lAccess);
			return true;
		}
	}//?if((lAccess & RMTSVC_ACCESS_TELNET_ALL)==RMTSVC_ACCESS_TELNET_ALL)
	//����û�����FTP��������Ȩ��
	if((lAccess & RMTSVC_ACCESS_FTP_ADMIN)==RMTSVC_ACCESS_FTP_ADMIN)
	{
		if(strcasecmp(httpreq.url().c_str(),"/ftpsets")==0)
		{
			httprsp_ftpsets(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/ftpusers")==0)
		{
			httprsp_ftpusers(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/ftpini")==0)
		{
			httprsp_ftpini(psock,httpreq,httprsp);
			return true;
		}
	}//?if((lAccess & RMTSVC_ACCESS_FTP_ADMIN)==RMTSVC_ACCESS_FTP_ADMIN)
	//����û������ļ�����Ȩ��
	if((lAccess & RMTSVC_ACCESS_FILE_ALL)!=0)
	{
		if(strcasecmp(httpreq.url().c_str(),"/filelist")==0)
		{
			int listWhat=1; bool bdsphide=false;
			const char *ptr_path=httpreq.Request("listwhat");
			if(ptr_path) listWhat=atoi(ptr_path);
			ptr_path=httpreq.Request("bdsph");
			if(ptr_path && strcmp(ptr_path,"true")==0) bdsphide=true;
			ptr_path=httpreq.Request("path");
			httprsp_filelist(psock,httprsp,ptr_path,listWhat,bdsphide);
			return true; 
		}
		else if(strcasecmp(httpreq.url().c_str(),"/profile_ver")==0)
		{
			const char *ptr_path=httpreq.Request("path");
			httprsp_profile_verinfo(psock,httprsp,ptr_path);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/profile")==0)
		{
			const char *ptr_prof=httpreq.Request("prof");
			const char *ptr_path=httpreq.Request("path");
			httprsp_profile(psock,httprsp,ptr_path,ptr_prof);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/profolder")==0)
		{
			const char *ptr_prof=httpreq.Request("prof");
			const char *ptr_path=httpreq.Request("path");
			httprsp_profolder(psock,httprsp,ptr_path,ptr_prof);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/prodrive")==0)
		{
			const char *ptr_volu=httpreq.Request("volu");
			const char *ptr_path=httpreq.Request("path");
			httprsp_prodrive(psock,httprsp,ptr_path,ptr_volu);
			return true;
		}
		else if(strncasecmp(httpreq.url().c_str(),"/download/",10)==0)
		{//����ָ�����ļ�
			const char *ptr_path=httpreq.Request("path");
			if(ptr_path==NULL) return true;
			string filepath(ptr_path);
			if(filepath[filepath.length()-1]!='\\') filepath.append("\\");
			filepath.append(httpreq.url().c_str()+10);
			httprsp.sendfile(psock,filepath.c_str());
			return true;
		}
		else if( (lAccess & RMTSVC_ACCESS_FILE_ALL)==RMTSVC_ACCESS_FILE_ALL)
		{
			if(strcasecmp(httpreq.url().c_str(),"/folder_del")==0)
			{
				bool bdsphide=false; const char *ptr_name=NULL;
				const char *ptr_path=httpreq.Request("bdsph");
				if(ptr_path && strcmp(ptr_path,"true")==0) bdsphide=true;
				ptr_path=httpreq.Request("path");
				ptr_name=httpreq.Request("fname");
				httprsp_folder_del(psock,httprsp,ptr_path,ptr_name,bdsphide);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/folder_new")==0)
			{
				bool bdsphide=false; const char *ptr_name=NULL;
				const char *ptr_path=httpreq.Request("bdsph");
				if(ptr_path && strcmp(ptr_path,"true")==0) bdsphide=true;
				ptr_path=httpreq.Request("path");
				ptr_name=httpreq.Request("fname");
				httprsp_folder_new(psock,httprsp,ptr_path,ptr_name,bdsphide);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/folder_ren")==0)
			{
				bool bdsphide=false; const char *ptr_name=NULL,*ptr_newname=NULL;
				const char *ptr_path=httpreq.Request("bdsph");
				if(ptr_path && strcmp(ptr_path,"true")==0) bdsphide=true;
				ptr_path=httpreq.Request("path");
				ptr_name=httpreq.Request("fname");
				ptr_newname=httpreq.Request("nname");
				httprsp_folder_ren(psock,httprsp,ptr_path,ptr_name,ptr_newname,bdsphide);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/file_del")==0)
			{
				bool bdsphide=false; const char *ptr_name=NULL;
				const char *ptr_path=httpreq.Request("bdsph");
				if(ptr_path && strcmp(ptr_path,"true")==0) bdsphide=true;
				ptr_path=httpreq.Request("path");
				ptr_name=httpreq.Request("fname");
				httprsp_file_del(psock,httprsp,ptr_path,ptr_name,bdsphide);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/file_ren")==0)
			{
				bool bdsphide=false; const char *ptr_name=NULL,*ptr_newname=NULL;
				const char *ptr_path=httpreq.Request("bdsph");
				if(ptr_path && strcmp(ptr_path,"true")==0) bdsphide=true;
				ptr_path=httpreq.Request("path");
				ptr_name=httpreq.Request("fname");
				ptr_newname=httpreq.Request("nname");
				httprsp_file_ren(psock,httprsp,ptr_path,ptr_name,ptr_newname,bdsphide);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/file_run")==0)
			{
				const char *ptr_path=httpreq.Request("path");
				httprsp_docommandEx(psock,httprsp,ptr_path);
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/upload")==0)
			{
				HTTPREQ_CONTENT_TYPE rt=httpreq.get_contentType(NULL);
				if(rt==HTTP_CONTENT_TEXTXML) //����XML�ļ�
					httprsp_upload(psock,httpreq,httprsp,session);
				else return false;
				return true;
			}
			else if(strcasecmp(httpreq.url().c_str(),"/get_upratio")==0)
			{
				httprsp_get_upratio(psock,httprsp,session);
				return true;
			} 
		}//?else if( (lAccess & RMTSVC_ACCESS_FILE_ALL)==RMTSVC_ACCESS_REGIST_ALL)
	}//?if((lAccess & RMTSVC_ACCESS_FILE_ALL)!=0)
	//-----------------------------------Proxy------------------------------------
	//����û����д����������Ȩ��
	if((lAccess & RMTSVC_ACCESS_VIDC_ADMIN)==RMTSVC_ACCESS_VIDC_ADMIN)
	{
		if(strcasecmp(httpreq.url().c_str(),"/proxysets")==0)
		{
			httprsp_proxysets(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/proxyusers")==0)
		{
			httprsp_proxyusers(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/proxyini")==0)
		{
			httprsp_proxyini(psock,httpreq,httprsp);
			return true;
		}
	}//?if((lAccess & RMTSVC_ACCESS_FTP_ADMIN)==RMTSVC_ACCESS_FTP_ADMIN)
	//------------------------------------vIDC-------------------------------------
	//����û�����VIDC��������Ȩ��
	if((lAccess & RMTSVC_ACCESS_VIDC_ADMIN)==RMTSVC_ACCESS_VIDC_ADMIN)
	{
		if(strcasecmp(httpreq.url().c_str(),"/mportL")==0)
		{
			httprsp_mportl(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/mportR")==0)
		{
			httprsp_mportr(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/vidcini")==0)
		{
			httprsp_vidcini(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/vidcsvr")==0)
		{
			httprsp_vidcsvr(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/vidccs")==0)
		{
			httprsp_vidccs(psock,httpreq,httprsp);
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/upnp")==0)
		{
			httprsp_upnp(psock,httpreq,httprsp); 
			return true;
		}
		else if(strcasecmp(httpreq.url().c_str(),"/upnpxml")==0)
		{
			httprsp_upnpxml(psock,httpreq,httprsp);
			return true;
		}
	}//?if((lAccess & RMTSVC_ACCESS_VIDC_ADMIN)==RMTSVC_ACCESS_VIDC_ADMIN)

	DWORD dwFileLen=0;
	const char *ptrfiledata=(!m_bGetFileFromRes)?NULL: //�����Ƿ����Դ�л�ȡ�ļ�
							GetFileFromRes(httpreq.url().c_str()+1,dwFileLen);
	if(ptrfiledata)
	{
		if(setLastModify(psock,httpreq,httprsp))
		{//���������ļ���http�����е�ʱ������˵�����޸Ĺ������ͷ����Ӧ�ļ�δ���޸Ĺ�
			MIMETYPE_ENUM mt=httpResponse::MimeType(httpreq.url().c_str());
			httprsp.set_mimetype(mt); //����MIME����
			httprsp.lContentLength(dwFileLen); //������Ӧ���ݳ���
			httprsp.send_rspH(psock,200,"OK");
			psock->Send(dwFileLen,ptrfiledata,-1);
		}
		return true;
	}
	return false;
}

//����Դ�л�ȡָ�����ļ�,��Դ�ļ���ʽ:
//����Դ�ļ��в���ָ�����ļ�����Դ�ļ�ͷ���ֽ�ΪYY�����������ֽ�Ϊ�ļ�������
//12�ֽڵı���,Ȼ�����n���ļ������ṹ,Ȼ������ļ�����
typedef struct tagFILERESOURCE
{
	DWORD filesize;
	char  filepath[28];
}FILERESOURCE;
#include "resource.h"
const char * webServer :: GetFileFromRes(const char *filepath,DWORD &flength)
{
	static const char *ptrResource=NULL; //ָ����Դ��ָ��
	if(ptrResource==NULL) //��ȡ��Դָ��
	{
		HRSRC hrs=::FindResource(NULL,MAKEINTRESOURCE(IDR_HTMLPAGE),RT_HTML);
		if(hrs!=NULL)
		{
			HGLOBAL hg=::LoadResource(NULL,hrs);
			if(hg!=NULL)
				ptrResource=(const char *)::LockResource(hg);
		}
	}//?if(ptrResource==NULL)
	if(ptrResource==NULL) return NULL;
	if(*(short *)ptrResource!=0x5959) return NULL;
	//��ȡ�ļ�����
	short filenums=*(short *)(ptrResource+2);
	FILERESOURCE *ptr_fileres=(FILERESOURCE *)(ptrResource+16);
	const char *ptr_filedata=ptrResource+16+filenums*sizeof(FILERESOURCE);
	for(short i=0;i<filenums;i++)
	{
		if(strcasecmp(ptr_fileres->filepath,filepath)==0)
		{
			flength=ptr_fileres->filesize;
			return ptr_filedata;
		}
		ptr_filedata+=ptr_fileres->filesize;
		ptr_fileres++; 
	} 
	return NULL;
}

//������Դ�ļ�������޸����� 
#include "net4cpp21/utils/cTime.h"
bool webServer :: setLastModify(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp)
{
	char buf[64]; 
	sprintf(buf,"%s %s",__DATE__,__TIME__); //��ʽ:Sep 11 2006 12:58:20
	cTime ct; ct.parseDate(buf);  //ct.parseDate1(buf);
	//�ж��ļ��Ƿ��޸�-------------- start---------------------
	cTime ct0; time_t t0=0,t1=1;
	const char *p=httpreq.Header("If-Modified-Since");
	if(p && ct0.parseDate(p) ){
		t0=ct0.Gettime();
		t1=ct.Gettime()+_timezone;//����ʱ������
	}//?if(p)
	if(t1<=t0) //�ļ�û�б��޸Ĺ�
	{
		this->httprsp_NotModify(psock,httprsp);
		return false;
	}
	//�ж��ļ��Ƿ��޸�--------------  end ---------------------
	ct.FormatGmt(buf,64,"%a, %d %b %Y %H:%M:%S GMT");
	httprsp.Header()["Last-Modified"]=string(buf);
	ct=cTime::GetCurrentTime();
	ct.FormatGmt(buf,64,"%a, %d %b %Y %H:%M:%S GMT");
	httprsp.Header()["Date"]=string(buf);
	return true;
}
/* 
//����ָ����HTML��Դ��ʽ�ļ�
DWORD createHTMLFILERESOURCE(std::string &htmlpath)
{
	WIN32_FIND_DATA finddata;
	HANDLE hd=INVALID_HANDLE_VALUE;
	std::string htmlpage; 
	getAbsolutfilepath(htmlpath);
	if(htmlpath[htmlpath.length()-1]=='\\'){ htmlpage=htmlpath+string("htmlpage"); htmlpath.append("*"); }
	else{ htmlpage=htmlpath+string("\\htmlpage"); htmlpath.append("\\*"); }
	FILE *fp=::fopen(htmlpage.c_str(),"w+b");
	if(fp==NULL) return 0;

	DWORD dwret=0; short filenums=0; FILERESOURCE fres;
	filenums=0x5959; ::fwrite(&filenums,sizeof(filenums),1,fp);
	filenums=0x0; ::fwrite(&filenums,sizeof(filenums),1,fp);
	for(int j=0;j<6;j++) ::fwrite(&filenums,sizeof(filenums),1,fp);
	dwret=16; //����12�ֽ�,ͷ�ܹ�16�ֽ�
	hd=::FindFirstFile(htmlpath.c_str(), &finddata);
	if(hd==INVALID_HANDLE_VALUE){ ::fclose(fp); return 0; }
	do{
		if(!(finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			const char *ptr=strrchr(finddata.cFileName,'.');
			if(ptr && (strcasecmp(ptr,".htm")==0 || strcasecmp(ptr,".js")==0 || strcasecmp(ptr,".css")==0) )
			{ 
				fres.filesize=finddata.nFileSizeLow;
				::memset(fres.filepath,0,sizeof(fres.filepath));
				strcpy(fres.filepath,finddata.cFileName);
				::fwrite(&fres,sizeof(fres),1,fp);
				filenums++; dwret+=sizeof(fres);
			}
		}
	}while(::FindNextFile(hd,&finddata));
	::FindClose(hd); htmlpath.erase(htmlpath.length()-1); //ɾ������"*"
	printf("[createHRES] total %d files in %s\r\n",filenums,htmlpath.c_str());
	::fseek(fp,2,SEEK_SET); ::fwrite(&filenums,sizeof(filenums),1,fp);
	for(short i=0;i<filenums;i++)
	{
		::fseek(fp,16+i*sizeof(fres),SEEK_SET);
		::fread(&fres,sizeof(fres),1,fp);
		printf("[createHRES] %d reading %s, filesize=%d\r\n,",i+1,fres.filepath,fres.filesize);
		char *pbuf=new char[fres.filesize];
		if(pbuf==NULL){ printf("[createHRES] failed to memory alloc\r\n"); break; }
		htmlpage=htmlpath+string(fres.filepath);
		FILE *fp_read=::fopen(htmlpage.c_str(),"rb");
		if(fp_read==NULL){ 
			printf("[createHRES] failed to open %s\r\n",htmlpage.c_str());
			delete pbuf; break; }
		::fread(pbuf,sizeof(char),fres.filesize,fp_read);
		::fseek(fp,0,SEEK_END);
		::fwrite(pbuf,sizeof(char),fres.filesize,fp);
		::fclose(fp_read); delete pbuf; dwret+=fres.filesize;
	}
	::fclose(fp); return dwret;
}
*/

