/*******************************************************************
   *	vIDCs.cpp
   *    DESCRIPTION:vIDC�������ʵ��
   *
   *    AUTHOR:yyc
   *	http://hi.baidu.com/yycblog/home
   *
   *******************************************************************/

#include "../../net4cpp21/include/sysconfig.h"
#include "../../net4cpp21/utils/utils.h"
#include "../../net4cpp21/include/cLogger.h"
#include "../../net4cpp21/include/httprsp.h"
#include "vidcs.h"

using namespace std;
using namespace net4cpp21;

mportTCP_vidcs :: mportTCP_vidcs(vidccSession *psession):m_psession(psession),mportTCP()
{
}
//����ӳ��Ӧ�÷���
socketTCP * mportTCP_vidcs :: connectAppsvr(char *strHost,socketTCP *psock)
{
	//��ȡһ�����йܵ�
	socketTCP *ppeer=m_psession->GetPipe();
	if( ppeer==NULL ) return NULL;

	std::pair<std::string,int> *p=GetAppsvr();
	if(p==NULL) return ppeer; //��ʱ��ʾ��ӳ��˿�Ϊ�����������˿ڣ���������ָ��������Ӧ�÷���
	char buf[128];//����һ����������
	//��չܵ�����ܲ���������
	int iret=ppeer->checkSocket(0,SOCKS_OP_READ);
	if(iret>0) while( (iret=ppeer->Receive(buf,128,0))==128) NULL;

	iret=sprintf(buf,"CONNECT %s:%d HTTP/1.1\r\n\r\n",p->first.c_str(),p->second);
	if( (iret=ppeer->Send(iret,buf,-1))>0 )
	{
		httpResponse httprsp;
		int rspcode=httprsp.recv_rspH(ppeer,VIDC_MAX_RESPTIMEOUT);
		if(rspcode==200){ //�ɹ�������Զ��Ӧ�÷���
			if(httprsp.lReceivedContent()>0) 
				psock->Send(httprsp.lReceivedContent(),httprsp.szReceivedContent(),-1);
			if(p->second!=80) 
				sprintf(strHost,"%s:%d",p->first.c_str(),p->second);
			else sprintf(strHost,"%s",p->first.c_str());
			return ppeer;
		}
	}//?if( (iret=ppeer->Send(iret,buf,-1))>0 )
//	if(ppeer->status()==SOCKS_CONNECTED) //���岻��
//		if(m_psession->AddPipe(ppeer) ) return NULL; //������ӵ��ܵ�������
	delete ppeer;  return NULL;
}

//********************************vidccSession****************************
vidccSession :: vidccSession(socketTCP *psock,int ver,const char *strname,const char *strdesc)
:m_psock_command(psock)
{
	if(strname) m_strName.assign(strname);
	if(strdesc) m_strDesc.assign(strdesc);
	m_vidccVer=ver;
	m_tmConnected=time(NULL);
}

void vidccSession :: Destroy()
{
	m_psock_command->Close();
	std::map<std::string,mportTCP_vidcs *>::iterator it=m_tcpsets.begin();
	for(;it!=m_tcpsets.end();it++){
		mportTCP_vidcs *ptr_mtcp=(*it).second;
		ptr_mtcp->Stop(); delete ptr_mtcp;
	}
	m_tcpsets.clear();
	//�ȴ��ܵ�����Ϊ�գ���Ϊ�ܵ����м����߳̽���ʱ��Ӵ�vidccSessionɾ���ܵ�
	//����˴����ȴ������п���vidccSession�ͷź�ܵ����м����̻߳����DelPipe����ɾ���ܵ�
	//����Ұָ����ʳ���
	while(m_pipes.size()>0) cUtils::usleep(200000); //��ʱ200ms
}

bool vidccSession :: AddPipe(socketTCP *pipe)
{
	if(!isConnected()) return false;
	m_mutex.lock();
	m_pipes.push_back(pipe);
	m_mutex.unlock();
	return true;
}
//����û�а󶨵Ĺܵ�������ر�����ô�����ɾ��
bool vidccSession :: DelPipe(socketTCP *pipe)
{
	bool bret=false;
	m_mutex.lock();
	std::vector<socketTCP *>::iterator it=m_pipes.begin();
	for(;it!=m_pipes.end();it++){
		if(*it==pipe){ m_pipes.erase(it); bret=true; break; }
	}
	m_mutex.unlock();
	return bret;
}

socketTCP * vidccSession :: GetPipe()
{
	//ͨ����vidcc���������ȡ�ܵ�
	if( m_psock_command->Send(7,"PIPE \r\n",-1)<=0 ) return NULL;
	time_t t=time(NULL);
	while(m_pipes.size()<=0) //�ȴ��µĹܵ�
	{
		if((time(NULL)-t)>VIDC_MAX_RESPTIMEOUT) return NULL;
		if(m_psock_command->status()!=SOCKS_CONNECTED) return NULL;
		cUtils::usleep(200000); //��ʱ200ms
	}
	socketTCP * pipe=NULL;
	m_mutex.lock();
	std::vector<socketTCP *>::iterator it=m_pipes.begin();
	//��ȡ���ӹܵ��������Ƴ�
	if(it!=m_pipes.end()){ pipe=*it; m_pipes.erase(it); }
	m_mutex.unlock(); return pipe;
}

void vidccSession :: parseCommand(const char *ptrCommand)
{
	RW_LOG_DEBUG("[vidcsvr] c--->s:\r\n\t%s\r\n",ptrCommand);
	if(strncmp(ptrCommand,"BIND ",5)==0)
		docmd_bind(ptrCommand+5);
	else if(strncmp(ptrCommand,"UBND ",5)==0)
		docmd_unbind(ptrCommand+5);
	else if(strncmp(ptrCommand,"vNOP ",5)==0)
		docmd_vnop(ptrCommand+5);
	else if(strncmp(ptrCommand,"ADDR ",5)==0) //��ȡvIDCs������IP��ַ�б�
		docmd_addr(ptrCommand+5);
	else if(strncmp(ptrCommand,"FILT ",5)==0) //����ĳ��ӳ������IP���˹���
		docmd_ipfilter(ptrCommand+5);
	else if(strncmp(ptrCommand,"HRSP ",5)==0)
		docmd_mdhrsp(ptrCommand+5);
	else if(strncmp(ptrCommand,"HREQ ",5)==0)
		docmd_mdhreq(ptrCommand+5);
	else docmd_unknowed(ptrCommand);
	return;
}

extern int splitString(const char *str,char delm,std::map<std::string,std::string> &maps);
//vIDCc���͵�Զ��ӳ������ //2.5�°�����
//	BIND type=[TCP|UDP] name=<XXX> appsvr=<Ҫӳ���Ӧ�÷���> mport=<ӳ��˿�>[+|-ssl] [sslverify=0|1] [bindip=<������󶨵ı���IP>] [apptype=FTP|WWW|TCP|UNKNOW] [appdesc=<����>]
//	BIND type=PROXY name=<XXX> mport=<ӳ��˿�> [bindip=<������󶨵ı���IP>] [appdesc=<����>]
void vidccSession :: docmd_bind(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	
	std::map<std::string,std::string>::iterator it;
	if( (it=maps.find("name"))==maps.end() || (*it).second=="" )
	{
		m_psock_command->Send(sizeof(msg_err_501)-1,msg_err_501,-1);
		return;
	}
	const char *mapname=(*it).second.c_str();
	::strlwr((char *)mapname); //ת��ΪСд
	
	VIDC_MAPTYPE maptype=VIDC_MAPTYPE_TCP;
	if( (it=maps.find("type"))!=maps.end())
	{
		if((*it).second=="PROXY")    maptype=VIDC_MAPTYPE_PROXY;
		else if((*it).second=="UDP") maptype=VIDC_MAPTYPE_UDP;
	}
	if(maptype==VIDC_MAPTYPE_UDP)
	{
		m_psock_command->Send(sizeof(msg_err_503)-1,msg_err_503,-1);
		return;
	}

	const char *ptr,*ptr_appsvr,*ptr_appdesc,*ptr_bindip;
	int mportBegin=0,mportEnd=0;
	SSLTYPE ssltype=TCPSVR_TCPSVR;
	bool bSSLVerify=false;
	MPORTTYPE apptype=MPORTTYPE_UNKNOW;
	
	
	if( (it=maps.find("apptype"))!=maps.end())
	{
		if((*it).second=="FTP")      apptype=MPORTTYPE_FTP;
		else if((*it).second=="WWW") apptype=MPORTTYPE_WWW;
		else if((*it).second=="TCP") apptype=MPORTTYPE_TCP;
	}
	it=maps.find("mport");
	if(it!=maps.end()){
		if( (ptr=strstr((*it).second.c_str(),"+ssl")) ){ ssltype=TCPSVR_SSLSVR; *(char *)ptr=0; }
		else if( (ptr=strstr((*it).second.c_str(),"-ssl")) ){ ssltype=SSLSVR_TCPSVR; *(char *)ptr=0; }
		//����ӳ��˿�
		if( (ptr=strchr((*it).second.c_str(),'-')) ){
			mportBegin=atoi((*it).second.c_str());
			mportEnd=atoi(ptr+1);
		}else{ mportBegin=atoi((*it).second.c_str()); mportEnd=mportBegin; }	
	}//?�����˿�ӳ�����

	it=maps.find("bindip");
	ptr_bindip=(it!=maps.end())?(*it).second.c_str():NULL;
	it=maps.find("appsvr");
	ptr_appsvr=(it!=maps.end())?(*it).second.c_str():NULL;
	it=maps.find("appdesc");
	ptr_appdesc=(it!=maps.end())?(*it).second.c_str():NULL;

	if(maptype==VIDC_MAPTYPE_PROXY){ apptype=MPORTTYPE_TCP; ssltype=TCPSVR_TCPSVR; }

	it=maps.find("sslverify");
	if(ssltype==TCPSVR_SSLSVR && it!=maps.end() && (*it).second=="1") bSSLVerify=true;

	mportTCP_vidcs *ptr_mtcp=new mportTCP_vidcs(this);
	if(ptr_mtcp==NULL){
		m_psock_command->Send(sizeof(msg_err_502)-1,msg_err_502,-1);
		return;
	}

	long ltmp; it=maps.find("maxratio"); //���ƴ���
	if(it!=maps.end()) ltmp=atol((*it).second.c_str()); else ltmp=0;
	ptr_mtcp->setMaxRatio( ((ltmp<0)?0:ltmp) );
	it=maps.find("maxconn"); //�������������
	if(it!=maps.end()) ltmp=atol((*it).second.c_str()); else ltmp=0;
	ptr_mtcp->maxConnection( ((ltmp<0)?0:ltmp) );

	ptr_mtcp->setMapping(mportBegin,mportEnd,ptr_bindip);
	ptr_mtcp->setSSLType(ssltype,bSSLVerify);
	ptr_mtcp->setAppsvr(ptr_appsvr,0,ptr_appdesc,apptype);
	//��vIDCs�����ȡSSL֤��������Ϣ
	socketTCP * ptr_vidcsSocket=(socketTCP *)m_psock_command->parent();
	if(ptr_vidcsSocket) ptr_mtcp->setCacert(ptr_vidcsSocket,((bSSLVerify)?false:true) );
	SOCKSRESULT sr=ptr_mtcp->StartX(); //����ӳ�����
	if(sr<=0){ //����ӳ�����ʧ��
		m_psock_command->Send(msg_err_504,sr);
		delete ptr_mtcp; return;
	}
	m_tcpsets[mapname]=ptr_mtcp;
	m_psock_command->Send(msg_err_ok,sr,((ptr_mtcp->ifSSLVerify())?"sslv=1":"") );
	return;
}
//ָ��ӳ��˿ڵĿͻ�����֤֤�� //2.5�°�����
//��ʽ: SSLC name=<XXX> certlen=<֤���ֽ�> keylen=<˽Կ�ֽ�> pwdlen=<���볤��>\r\n�����ֽ�\r\n
//certlen,keylen,pwdlen�ֽڳ��Ȱ�������'\0'
long vidccSession :: docmd_sslc(const char *strSSLC,const char *received,long receivedByte)
{
	long lret=0;
	std::map<std::string,std::string> maps;
	if(splitString(strSSLC,' ',maps)<=0){
		m_psock_command->Send(sizeof(msg_err_500)-1,msg_err_500,-1);
		return lret;
	}

	std::map<std::string,std::string>::iterator it;
	long certlen=0,keylen=0,pwdlen=0,totalByte=0;
	char *lpCertBuf=NULL; const char *mapname=NULL;
	if( (it=maps.find("certlen"))!=maps.end() ) certlen=atol((*it).second.c_str());
	if( (it=maps.find("keylen"))!=maps.end() ) keylen=atol((*it).second.c_str());
	if( (it=maps.find("pwdlen"))!=maps.end() ) pwdlen=atol((*it).second.c_str());
	totalByte=certlen+keylen+pwdlen+2;//��������\r\n
	lret=receivedByte;
	if(receivedByte>=totalByte) //�����Ѿ��������
	{
		lret=totalByte;
		lpCertBuf=(char *)received;
	}else if( (lpCertBuf=new char[totalByte]) ) {
		::memcpy(lpCertBuf,received,receivedByte);
		while(totalByte>receivedByte) //���Ž���ʣ�������
		{
			int sr=m_psock_command->Receive(lpCertBuf+receivedByte,totalByte-receivedByte,VIDC_MAX_RESPTIMEOUT);
			if(sr>0) receivedByte+=sr;
			else {delete[] lpCertBuf; lpCertBuf=NULL; break; }
		}//?while
	}//?else if
	if(lpCertBuf==NULL){ //���յ����������
		m_psock_command->Send(sizeof(msg_err_500)-1,msg_err_500,-1);
		return lret;
	}

	if( (it=maps.find("name"))!=maps.end() ) mapname=(*it).second.c_str();
	if(mapname) ::strlwr((char *)mapname); //ת��ΪСд
	std::map<std::string,mportTCP_vidcs *>::iterator it_mtcp=(mapname)?m_tcpsets.find(mapname):m_tcpsets.end();
	if( it_mtcp != m_tcpsets.end() && (*it_mtcp).second )
	{
		const char *ptr_cert=lpCertBuf;
		const char *ptr_key=ptr_cert+certlen;
		const char *ptr_keypswd=ptr_key+keylen;
		mportTCP_vidcs *ptr_mtcp=(*it_mtcp).second;
		if(ptr_mtcp->getSSLType()==SSLSVR_TCPSVR)
			ptr_mtcp->setCacert(ptr_cert,ptr_key,ptr_keypswd,true,NULL,NULL);
		m_psock_command->Send(sizeof(msg_ok_200)-1,msg_ok_200,-1); 
	}else m_psock_command->Send(sizeof(msg_err_501)-1,msg_err_501,-1);
	if(lpCertBuf!=received) delete[] lpCertBuf;
	return lret;
}
////ָ��ӳ��˿ڵĸ���http��Ӧͷ����
//�����ʽ:
//	HRSP name=<ӳ���������> cond=<http��Ӧ����> header=<��Ӧͷ����>
//							  pattern=<ƥ��ģʽ> replto=<�滻�ַ���>
//cond=<http��Ӧ����> - ȷ������HTTP��Ӧ����Ϊָ�������ͷ
//pattern=<ƥ��ģʽ>  - ���ƥ��ģʽΪ����ֱ����repltoָ�����ַ����滻
//replto=<�滻�ַ���> - ���repltoΪ����ֱ��ɾ����ͷ
void vidccSession :: docmd_mdhrsp(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	
	std::map<std::string,std::string>::iterator it;
	const char *mapname=NULL;
	if( (it=maps.find("name"))!=maps.end() ) mapname=(*it).second.c_str();
	if(mapname) ::strlwr((char *)mapname); //ת��ΪСд
	std::map<std::string,mportTCP_vidcs *>::iterator it_mtcp=(mapname)?m_tcpsets.find(mapname):m_tcpsets.end();
	mportTCP_vidcs *ptr_mtcp=(it_mtcp == m_tcpsets.end())?NULL:(*it_mtcp).second;

	int rspcode; std::string strHeader,strPattern,strReplto;
	if( (it=maps.find("cond"))!=maps.end())
		rspcode=atoi((*it).second.c_str());
	else rspcode=0;
	if( (it=maps.find("header"))!=maps.end())
		strHeader=(*it).second;
	else strHeader="";
	if( (it=maps.find("pattern"))!=maps.end())
		strPattern=(*it).second;
	else strPattern="";
	if( (it=maps.find("replto"))!=maps.end())
		strReplto=(*it).second;
	else strReplto="";

	if(ptr_mtcp && strHeader!="")
	{
		ptr_mtcp->addRegCond(rspcode,strHeader.c_str(),strPattern.c_str(),strReplto.c_str());
		m_psock_command->Send(sizeof(msg_ok_200)-1,msg_ok_200,-1); 
	}else m_psock_command->Send(sizeof(msg_err_501)-1,msg_err_501,-1);
}
////ָ��ӳ��˿ڵĸ���http����ͷ����
//�����ʽ:
//	HREQ name=<ӳ���������> cond=<http����url> header=<��Ӧͷ����>
//							  pattern=<ƥ��ģʽ> replto=<�滻�ַ���>
//cond=<http����url>  - ���ķ���������HTTP����ͷ�������Url��ָ����cond����ƥ��
//pattern=<ƥ��ģʽ>  - ���ƥ��ģʽΪ����ֱ����repltoָ�����ַ����滻
//replto=<�滻�ַ���> - ���repltoΪ����ֱ��ɾ����ͷ
void vidccSession :: docmd_mdhreq(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	
	std::map<std::string,std::string>::iterator it;
	const char *mapname=NULL;
	if( (it=maps.find("name"))!=maps.end() ) mapname=(*it).second.c_str();
	if(mapname) ::strlwr((char *)mapname); //ת��ΪСд
	std::map<std::string,mportTCP_vidcs *>::iterator it_mtcp=(mapname)?m_tcpsets.find(mapname):m_tcpsets.end();
	mportTCP_vidcs *ptr_mtcp=(it_mtcp == m_tcpsets.end())?NULL:(*it_mtcp).second;

	std::string strUrl,strHeader,strPattern,strReplto;
	if( (it=maps.find("cond"))!=maps.end())
		strUrl=(*it).second;
	if(strUrl=="") strUrl="/";
	if( (it=maps.find("header"))!=maps.end())
		strHeader=(*it).second;
	else strHeader="";
	if( (it=maps.find("pattern"))!=maps.end())
		strPattern=(*it).second;
	else strPattern="";
	if( (it=maps.find("replto"))!=maps.end())
		strReplto=(*it).second;
	else strReplto="";

	if(ptr_mtcp && strHeader!="")
	{
		ptr_mtcp->addRegCond(strUrl.c_str(),strHeader.c_str(),strPattern.c_str(),strReplto.c_str());
		m_psock_command->Send(sizeof(msg_ok_200)-1,msg_ok_200,-1); 
	}else m_psock_command->Send(sizeof(msg_err_501)-1,msg_err_501,-1);
}
//ָ��ӳ��˿ڵ�ip���˹��� //2.5�°�����
//��ʽ: FILT name=<XXX> access=<0|1> ipaddr=
void vidccSession :: docmd_ipfilter(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	
	std::map<std::string,std::string>::iterator it;
	const char *mapname=NULL;
	if( (it=maps.find("name"))!=maps.end() ) mapname=(*it).second.c_str();
	if(mapname) ::strlwr((char *)mapname); //ת��ΪСд
	std::map<std::string,mportTCP_vidcs *>::iterator it_mtcp=(mapname)?m_tcpsets.find(mapname):m_tcpsets.end();
	if( it_mtcp == m_tcpsets.end())
	{
		m_psock_command->Send(sizeof(msg_err_501)-1,msg_err_501,-1);
		return;
	}
	mportTCP_vidcs *ptr_mtcp=(*it_mtcp).second;
	std::string ipRules; int ipaccess=1;
	if( (it=maps.find("ipaddr"))!=maps.end() ) ipRules=(*it).second;
	if( (it=maps.find("access"))!=maps.end() ) ipaccess=atoi((*it).second.c_str());
	ptr_mtcp->rules().addRules_new(RULETYPE_TCP,ipaccess,ipRules.c_str());
	m_psock_command->Send(sizeof(msg_ok_200)-1,msg_ok_200,-1); 
}

//vIDCc���͵�ȡ��ĳ��TCPӳ���������
//��ʽ: UBND <SP> <ӳ������> <CRLF>
//ע: vIDC 2.5����ǰû��ӳ������һ˵��������Ʒ���������ʵ�ʵ�ӳ��˿�
//2.5 ��Ϊ�˼�����ǰ�İ汾����ӳ��ʱû��ָ�����Ƶ���ӳ���Ķ˿ڴ�������
void vidccSession :: docmd_unbind(const char *param)
{
	const char *mapname=param;
	::strlwr((char *)mapname); //ת��ΪСд
	std::map<std::string,mportTCP_vidcs *>::iterator it=m_tcpsets.find(mapname);
	if(it==m_tcpsets.end())
	{
		m_psock_command->Send(sizeof(msg_err_501)-1,msg_err_501,-1);
		return;
	}
	mportTCP_vidcs *ptr_mtcp=(*it).second;
	m_tcpsets.erase(it);
	ptr_mtcp->Stop(); delete ptr_mtcp;
	m_psock_command->Send(sizeof(msg_ok_200)-1,msg_ok_200,-1);
}

//vIDCc���͵Ļ�ȡvIDCs������IP��ַ�б�����
//��ʽ: ADDR <SP> <CRLF>
void vidccSession :: docmd_addr(const char *param)
{
	std::vector<std::string> vec;//�õ�����IP�����صõ�����IP�ĸ���
	long n=socketBase::getLocalHostIP(vec);
	char buf[VIDC_MAX_COMMAND_SIZE]; 
	int buflen=sprintf(buf,"200 ");
	for(int i=0;i<n;i++){
		buflen+=sprintf(buf+buflen,"%s ",vec[i].c_str());
		if((buflen+20)>=VIDC_MAX_COMMAND_SIZE) break;
	}
	buf[buflen++]='\r'; buf[buflen++]='\n';
	m_psock_command->Send(buflen,buf,-1);
}

//vIDCc���͵ı������ӵ���������
//��ʽ: vNOP <SP> <CRLF>
void vidccSession :: docmd_vnop(const char *param)
{
	m_psock_command->Send(7,"rNOP \r\n",-1);
}

//����ʶ�������
void vidccSession :: docmd_unknowed(const char *ptrCommand)
{
	m_psock_command->Send(sizeof(msg_err_500)-1,msg_err_500,-1);
}

void vidccSession :: setIfLogdata(bool b) //�����Ƿ��¼��־
{
	std::map<std::string,mportTCP_vidcs *>::iterator it=m_tcpsets.begin();
	for(;it!=m_tcpsets.end();it++)
	{
		mportTCP_vidcs *p=(*it).second;
		if(p==NULL) continue;
		p->setIfLogdata(b);
	}
	return;
}

//<maplist>
//</maplist>
void vidccSession :: xml_list_mapped(cBuffer &buffer)
{
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<maplist>");
	std::map<std::string,mportTCP_vidcs *>::iterator it=m_tcpsets.begin();
	for(;it!=m_tcpsets.end();it++)
	{
		mportTCP_vidcs *p=(*it).second;
		if(p==NULL) continue;
		if(buffer.Space()<48) buffer.Resize(buffer.size()+48);
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<mapinfo><iptype>TCP</iptype>");
		else break;
		p->xml_info_mtcp(buffer);
		if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"</mapinfo>");
		else break;
	}

	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</maplist>");
	return;
}

