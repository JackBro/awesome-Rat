/*******************************************************************
   *	proxysvr.cp
   *    DESCRIPTION:��������ʵ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2006-08-24
   *
   *	net4cpp 2.1
   *	
   *******************************************************************/

#include "../include/sysconfig.h"
#include "../include/proxysvr.h"
#include "../utils/utils.h"
#include "../include/cLogger.h"

using namespace std;
using namespace net4cpp21;

cProxysvr :: cProxysvr()
{
	m_proxytype=PROXY_HTTPS|PROXY_SOCKS4|PROXY_SOCKS5;
	m_bProxyAuthentication=false;

	//����������ز���
	m_bCascade=false; //Ĭ�ϲ�֧�ֶ�������
//	m_casProxysvr="";
//	m_casProxyport=0; 
	m_casProxytype=PROXY_HTTPS|PROXY_SOCKS4|PROXY_SOCKS5; //��������֧�ֵ�����
	m_casProxyAuthentication=false; //���������Ƿ���Ҫ��֤
	m_bLogdatafile=false;
}


SOCKSRESULT cProxysvr :: delAccount(const char *struser)
{
	if((long)struser==-1){ m_accounts.clear(); return SOCKSERR_OK;}
	if(struser==NULL || struser[0]==0) return SOCKSERR_PROXY_USER;
	::strlwr((char *)struser);
	std::map<std::string,PROXYACCOUNT>::iterator it=m_accounts.find(struser);
	if(it==m_accounts.end()) return SOCKSERR_PROXY_USER;
	if((*it).second.m_loginusers>0) return SOCKSERR_PROXY_DENY;
	m_accounts.erase(it);
	return SOCKSERR_OK;
}

//��ȡ�ʺ���Ϣ������ָ�����ʺŶ���
PROXYACCOUNT * cProxysvr :: getAccount(const char *struser)
{
	if(struser==NULL || struser[0]==0) return NULL;
	::strlwr((char *)struser);
	std::map<std::string,PROXYACCOUNT>::iterator it=m_accounts.find(struser);
	if(it!=m_accounts.end()) return &(*it).second;
	return NULL;
}

//������ʺ���Ϣ
PROXYACCOUNT * cProxysvr :: newAccount(const char *struser)
{
	if(struser==NULL || struser[0]==0) return NULL;
	PROXYACCOUNT proa;
	proa.m_username.assign(struser);
	proa.m_maxratio=0;
	proa.m_loginusers=0;
	proa.m_maxLoginusers=0;
	proa.m_limitedTime=0;

	::strlwr((char *)struser);
	std::map<std::string,PROXYACCOUNT>::iterator it=m_accounts.find(struser);
	if(it!=m_accounts.end()){
		if((*it).second.m_loginusers>0) return NULL;
		m_accounts.erase(it);
	}
	m_accounts[struser]=proa;
	it=m_accounts.find(struser);
	if(it==m_accounts.end()) return NULL;
	return &(*it).second;
}

//���ö���������ز��� m_vecCassvr
bool cProxysvr::setCascade(const char *casHost,int casPort,int type,const char *user,const char *pswd)
{
	m_vecCassvr.clear(); //��ն�������
	if(casHost && casHost[0]!=0)
	{
		const char *ptrEnd,*ptrBegin=casHost;
		while(true)
		{
			while(*ptrBegin==' ') ptrBegin++;
			ptrEnd=strchr(ptrBegin,',');
			if(ptrEnd) *(char *)ptrEnd=0;

			const char *ptr=strchr(ptrBegin,':');
			int hostport=casPort;
			if(ptr){ *(char *)ptr=0; hostport=atoi(ptr+1);}
			if(hostport>0 && ptrBegin[0]!=0)
			{ //������������ַ�Ͷ˿���Ч
				std::pair<std::string,int> p(ptrBegin,hostport);
				m_vecCassvr.push_back(p);
			}
			if(ptr) *(char *)ptr=':';

			if(ptrEnd) *(char *)ptrEnd=',';
			else break;
			ptrBegin=ptrEnd+1;
		}//?while
	}//?if(casHost && casHost[0]!=0) 
	m_bCascade=(m_vecCassvr.size()>0)?true:false; //���������֧�ֶ�������
/*	if(casHost==NULL || casHost[0]==0 || casPort<=0)
	{//��֧�ֶ�������
		m_bCascade=false;
	}else{
		m_bCascade=true;
		m_casProxysvr.assign(casHost);
		m_casProxyport=casPort;
	} */

	m_casProxytype=type;
	if(user && user[0]!=0)
		m_casProxyAuthentication=true;
	else
		m_casProxyAuthentication=false;
	if(user) m_casAccessAuth.first.assign(user);
	if(pswd) m_casAccessAuth.second.assign(pswd);
	return m_bCascade;
}


PROXYACCOUNT * cProxysvr::ifAccess(socketTCP *psock,const char *user,const char *pwd,int * perrCode)
{
	if(perrCode) *perrCode=SOCKSERR_PROXY_USER;
	if(user==NULL || psock==NULL) return NULL;
	while(*user==' ') user++;//ɾ��ǰ���ո�
	::_strlwr((char *)user);//���ʺ�ת��ΪСд
	std::map<std::string,PROXYACCOUNT>::iterator it=m_accounts.find(user);
	if(it==m_accounts.end()) return NULL;
	
	PROXYACCOUNT &proa=(*it).second;
	std::string strPwd;
	if(pwd) strPwd.assign(pwd);
	if(proa.m_userpwd!="" && proa.m_userpwd!=strPwd)
		return NULL; //������� //������ɿ�������������֤
	
	if(proa.m_limitedTime>0 && time(NULL)>proa.m_limitedTime ){
		if(perrCode) *perrCode=SOCKSERR_PROXY_EXPIRED;
		return NULL; //�ʺŹ���
	}
	if(!proa.m_ipRules.check(psock->getRemoteip(),psock->getRemotePort(),RULETYPE_TCP) ){
		if(perrCode) *perrCode=SOCKSERR_PROXY_DENY;
		return NULL;
	}
	if(proa.m_maxLoginusers>0 && proa.m_loginusers>proa.m_maxLoginusers){
		if(perrCode) *perrCode=SOCKSERR_PROXY_CONNECTIONS;
		return NULL;
	}

	if(perrCode) *perrCode=SOCKSERR_OK;
	proa.m_loginusers++; //��������ͨ������������
	psock->setSpeedRatio(proa.m_maxratio*1024,proa.m_maxratio*1024);
	return &proa;
}


void cProxysvr::onConnect(socketTCP *psock)
{
	char ctype; int iret;
	if((m_proxytype & 0x7)==0) return;
	iret=psock->Peek(&ctype,1,PROXY_MAX_RESPTIMEOUT);
	if(iret<=0) return;
	
	//�жϴ�����������
	if(ctype==0x04) //�����յ�socks4��������
	{
		if((m_proxytype & PROXY_SOCKS4)==0) return; //������֧��socks4����Э��
		doSock4req(psock);
	}
	else if(ctype==0x05) //�����յ�socks5��������
	{
		if((m_proxytype & PROXY_SOCKS5)==0) return; //������֧��socks5����Э��
		doSock5req(psock);
	}
	else if((m_proxytype & PROXY_HTTPS)!=0) //������֧��HTTPS����Э��
		doHttpsreq(psock);
	return;
}

typedef struct _TransThread_Param
{
	socketTCP * psock;
	FILE *fp;
	cProxysvr *psvr;
}TransThread_Param;

//����ת��
void cProxysvr :: transData(socketTCP *psock,socketTCP *peer,const char *sending_buf,long sending_size)
{
//	ASSERT(psock);
//	ASSERT(peer);
	char buf[SSENDBUFFERSIZE];
	FILE *fp=NULL;
	if(m_bLogdatafile){ //�Ƿ��¼�������ݼ�¼
		char logfilename[256]; //��¼��־�ļ���
		int logfilenameLen=sprintf(logfilename,"%s(%d)-",psock->getRemoteIP(),psock->getRemotePort());
		logfilenameLen+=sprintf(logfilename+logfilenameLen,"%s(%d).log",peer->getRemoteIP(),peer->getRemotePort());
		logfilename[logfilenameLen]=0;
		if( (fp=::fopen(logfilename,"ab")) )
		{
			logfilenameLen=sprintf(logfilename,"C %s:%d <----> S ",psock->getRemoteIP(),psock->getRemotePort());
			::fwrite(logfilename,1,logfilenameLen,fp);
			logfilenameLen=sprintf(logfilename,"%s:%d\r\n",peer->getRemoteIP(),peer->getRemotePort());
			::fwrite(logfilename,1,logfilenameLen,fp);
		}
	}//?if(m_bLogdatafile)

	onData((char *)1,0,psock,peer); //���ڷ����������ݣ���ʼһ������
	if(sending_buf && sending_size>0)//�����͵�����
	{
		onData((char*)sending_buf,sending_size,psock,peer);
		peer->Send(sending_size,sending_buf,-1);
///#ifdef PROXYDATA_LOG //�Ƿ��¼�������ݼ�¼
		if(fp){
			::fwrite("\r\nC ---> S\r\n",1,12,fp);
			::fwrite(sending_buf,1,sending_size,fp);
		}
///#endif
	}//?if(sending_buf && sending_size>0)

	peer->setSpeedRatio(psock->getMaxSendRatio(),psock->getMaxRecvRatio());
	TransThread_Param ttParam; ttParam.psvr =this;
	ttParam.psock =peer; ttParam.fp =fp;
//	std::pair<socketTCP *,FILE *> p(peer,fp);
	if(onTransferTask((THREAD_CALLBACK *)&transThread,(void *)&ttParam))
	{
		while( psock->status()==SOCKS_CONNECTED )
		{
			int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
			if(iret<0) break; 
			if(iret==0) continue;
			//���ͻ��˷��͵�����
			iret=psock->Receive(buf,SSENDBUFFERSIZE,-1);
			if(iret<0) break; //==0������������������������
			if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
///#ifdef PROXYDATA_LOG //�Ƿ��¼�������ݼ�¼
			if(fp){
				::fwrite("\r\nC ---> S\r\n",1,12,fp);
				::fwrite(buf,1,iret,fp);
			}
///#endif
			onData(buf,iret,psock,peer);
			iret=peer->Send(iret,buf,-1);
			if(iret<0) break;
		}//?while
		peer->Close(); //�ȴ�ת���߳̽���
		while(peer->parent()!=NULL) cUtils::usleep(SCHECKTIMEOUT);
		onData(NULL,0,peer,psock); //����֪ͨЭ�������ӡ���������Ѿ��رգ����ͷ������Դ
	}else{//?if(onTransferTask
		peer->Close(); peer->setParent(NULL);
		RW_LOG_DEBUG(0,"Failed to create transfer-Thread\r\n");
	}

///#ifdef PROXYDATA_LOG //�Ƿ��¼�������ݼ�¼
	if(fp){
		::fwrite("\r\n***Proxy End*** \r\n",1,20,fp);
		::fclose(fp);
	}
///#endif
	return;
}

//ת���߳�
void cProxysvr :: transThread(void *pthreadParam)
{	
//	std::pair<socketTCP *,FILE *> &p=*(std::pair<socketTCP *,FILE *> *)pthreadParam;
//	FILE *fp=p.second; cProxysvr *psvr=NULL;
//	socketTCP *psock=p.first;
	
	TransThread_Param *ptParam=(TransThread_Param *)pthreadParam;
	FILE *fp=ptParam->fp; socketTCP *psock=ptParam->psock;
	cProxysvr *psvr=ptParam->psvr;

	if(psock==NULL) return;
	socketTCP *ppeer=(socketTCP *)psock->parent();
	if(ppeer==NULL) return;

	char buf[SSENDBUFFERSIZE]; //��ʼת��
	while( psock->status()==SOCKS_CONNECTED )
	{
		int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
		if(iret<0) break; 
		if(iret==0) continue;
		//���ͻ��˷��͵�����
		iret=psock->Receive(buf,SSENDBUFFERSIZE,-1);
		if(iret<0) break; //==0������������������������
		if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
///#ifdef PROXYDATA_LOG //�Ƿ��¼�������ݼ�¼
			if(fp){
				::fwrite("\r\nS ---> C\r\n",1,12,fp);
				::fwrite(buf,1,iret,fp);
			}
///#endif
		if(psvr) psvr->onData(buf,iret,psock,ppeer);
		iret=ppeer->Send(iret,buf,-1);
		if(iret<0) break;
	}//?while
	
	ppeer->Close(); 
	psock->setParent(NULL); //����onAccept�߳��ж�ת���߳��Ƿ����
	return;
}

//------------------------------------------------------------
proxyServer :: proxyServer()
{
	m_strSvrname="Proxy Server";
}
proxyServer :: ~proxyServer()
{
	 Close();
	 m_threadpool.join();
}
