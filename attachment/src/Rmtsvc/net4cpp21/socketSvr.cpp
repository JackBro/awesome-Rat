/*******************************************************************
   *	socketSvr.cpp
   *    DESCRIPTION:TCP �첽���������ʵ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-01
   *	
   *	net4cpp 2.1
   *******************************************************************/

#include "include/sysconfig.h"
#include "include/socketSvr.h"
#include "utils/utils.h"
#include "include/cLogger.h"

using namespace std;
using namespace net4cpp21;

socketSvr :: socketSvr()
{
	m_strSvrname="Listen-server";//��������
	m_curConnection=0;
	m_maxConnection=0;
	m_lAcceptTimeOut=500; //500ms �첽Accept��ʱʱ��,����Ĭ��accept��ʱ�ȴ�ʱ��
	//==-1�������ȴ�ֱ��һ�����ӣ�ʵ�ʲ��Է����������Ϊ-1,����Listenning socket��һ��
	//�Է�δ�رյ����ӣ���ʱ��ʹ�ر���Listen Socket��accept���ǻ�һֱ������
	//���m_lAcceptTimeOut��ò�Ҫ����Ϊ-1
	m_bReuseAddr=FALSE;
}

socketSvr :: ~socketSvr()
{
	Close();
	m_threadpool.join();//����Closeʱ�ȴ��߳̽�������ֹ�ڻص��е���Close
}

//��������
SOCKSRESULT socketSvr::Listen(int port,BOOL bReuseAddr/*=FALSE*/,const char *bindIP/*=NULL*/)
{
	m_bReuseAddr=bReuseAddr;
	SOCKSRESULT sr=socketTCP::ListenX(port,m_bReuseAddr,bindIP);
	if(sr<=0){
		RW_LOG_DEBUG("Failed to Listen at %d,error=%d\r\n",port,sr);
		return sr;
	}
	//����˿������ɹ�����ʼ����Accept�����߳�
	if( m_threadpool.addTask((THREAD_CALLBACK *)&listenThread,(void *)this,0)!=0 )
		return sr;
	
	Close();//�����߳�����ʧ��
	return SOCKSERR_THREAD;
}

//��������
SOCKSRESULT socketSvr::Listen(int startport,int endport,BOOL bReuseAddr/*=FALSE*/,const char *bindIP/*=NULL*/)
{
	m_bReuseAddr=bReuseAddr;
	SOCKSRESULT sr=(startport==endport)?
				socketTCP::ListenX(startport,m_bReuseAddr,bindIP):
				socketTCP::ListenX(startport,endport,m_bReuseAddr,bindIP);
	if(sr<=0){
		RW_LOG_DEBUG("Failed to Listen at %d-%d,error=%d\r\n",startport,endport,sr);
		return sr;
	}
	//����˿������ɹ�����ʼ����Accept�����߳�
	if( m_threadpool.addTask((THREAD_CALLBACK *)&listenThread,(void *)this,0)!=0 )
		return sr;
	
	Close();//�����߳�����ʧ��
	return SOCKSERR_THREAD;
}

//�����߳�
void socketSvr :: listenThread(socketSvr *psvr)
{
	if(psvr==NULL) return;
	int svrPort=psvr->getLocalPort();
#ifdef _SURPPORT_OPENSSL_
	RW_LOG_DEBUG("[socketSvr] %s%s has been started,port=%d\r\n",
				psvr->m_strSvrname.c_str(),(psvr->ifSSL())?"(ssl)":"",svrPort);
#else
	RW_LOG_DEBUG("[socketSvr] %s has been started,port=%d\r\n",
				psvr->m_strSvrname.c_str(),svrPort);
#endif
	//yyc add 2007-03-29 ���������󶨶˿�ʱָ���˿������Ѱ󶨶˿�
	//���ұ��������ĳIP,��û�а�127.0.0.1����ô�Բ��ɷ��ʵķ�����
	//�ض����ϵķ��񣬼�127.0.0.1:<���˿�>
	bool bReirectOldSvr=( psvr->getLocalip()!=INADDR_ANY && psvr->m_bReuseAddr==SO_REUSEADDR)?true:false;
	socketTCP *psock=NULL;
	while(psvr->m_sockstatus==SOCKS_LISTEN)
	{
		if(psock==NULL)
		{
			if( (psock=new socketTCP())==NULL) 
			{
				RW_LOG_PRINT(LOGLEVEL_ERROR,0,"[listenThread] failed to new socketTCP()\r\n");
				break;
			}
		}
		SOCKSRESULT sr=psvr->Accept(psvr->m_lAcceptTimeOut,psock);
		if(sr==SOCKSERR_TIMEOUT){ psvr->onIdle(); continue; } //��ʱ
		if(sr<0) break; //��������

		RW_LOG_DEBUG("[socketSvr] one connection (%s:%d) coming in\r\n",psock->getRemoteIP(),psock->getRemotePort());
		if(psvr->m_srcRules.check(psock->getRemoteip(),psock->getRemotePort(),RULETYPE_TCP))
		{
			if( psvr->m_threadpool.addTask((THREAD_CALLBACK *)&doacceptTask,(void *)psock,THREADLIVETIME)!=0 )
			{	psock=NULL; continue; }
		}
		else if(bReirectOldSvr) //��ָ�����������¶������õ÷�����
		{
			if( psvr->m_threadpool.addTask((THREAD_CALLBACK *)&doRedirectTask,(void *)psock,THREADLIVETIME)!=0 )
			{	psock=NULL; continue; }
		}
		//yyc add 2007-03-29 ---------------------------------------
		psock->Close();
	}//?while
	
	if(psock) delete psock;
	RW_LOG_DEBUG("[socketSvr] %s has been ended,port=%d!\r\n",psvr->m_strSvrname.c_str(),svrPort);
}

//�����ӵ��ﴦ������
void socketSvr :: doacceptTask(socketTCP *psock)
{
	if(psock==NULL) return;
	socketSvr *psvr=(socketSvr *)psock->parent();
#ifdef _SURPPORT_OPENSSL_
	if(psock->ifSSL()) psock->SSL_Associate();
#endif
	if(psvr){
		psvr->m_curConnection++;
		if(psvr->m_maxConnection!=0 &&  psvr->m_curConnection > psvr->m_maxConnection)
			psvr->onTooMany(psock); 
		else
			psvr->onAccept(psock);
		psvr->m_curConnection--;
	}
	delete psock; return;
}

//yyc add 2007-03-29 �����ϵͳ��ĳ������˿ڽ�������
#define MAX_TRANSFER_BUFFER 2048
#define MAX_CONNECT_TIMEOUT 5
void socketSvr :: doRedirectTask(socketTCP *psock)
{
	if(psock==NULL) return;
	socketSvr *psvr=(socketSvr *)psock->parent();
	//�����ر����õķ���˿ڵ�ַ:127.0.0.1
	socketTCP *ppeer=new socketTCP;
	if(ppeer==NULL) { delete psock; return; }
	if( ppeer->Connect("127.0.0.1",psvr->getLocalPort(),MAX_CONNECT_TIMEOUT)>0)
		ppeer->setParent(psock);
	else { delete psock; delete ppeer; return; }

	RW_LOG_DEBUG("[socketSvr] Redirect old server(127.0.0.1:%d)\r\n",psvr->getLocalPort());

	if( psvr->m_threadpool.addTask((THREAD_CALLBACK *)&transThread,(void *)ppeer,THREADLIVETIME)==0 )
	{
		delete psock; delete ppeer;
		return;
	}

	char buf[MAX_TRANSFER_BUFFER]; //��ʼת��
	while( psock->status()==SOCKS_CONNECTED )
	{
		int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
		if(iret<0) break; 
		if(iret==0) continue;
		//���ͻ��˷��͵�����
		iret=psock->Receive(buf,MAX_TRANSFER_BUFFER-1,-1);
		if(iret<0) break; //==0������������������������
		if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
		iret=ppeer->Send(iret,buf,-1);
		if(iret<0) break;
	}//?while
	ppeer->Close(); //�ȴ�transThread�߳̽���
	while(ppeer->parent()!=NULL) cUtils::usleep(SCHECKTIMEOUT);
	
	delete psock; delete ppeer;
	return;
}

//ת���߳�
void socketSvr :: transThread(socketTCP *psock)
{
	if(psock==NULL) return;
	socketTCP *ppeer=(socketTCP *)psock->parent();
	if(ppeer==NULL) return;
//	socketSvr *psvr=(socketSvr *)ppeer->parent();

	char buf[MAX_TRANSFER_BUFFER]; //��ʼת��
	while( psock->status()==SOCKS_CONNECTED )
	{
		int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
		if(iret<0) break; 
		if(iret==0) continue;
		//���ͻ��˷��͵�����
		iret=psock->Receive(buf,MAX_TRANSFER_BUFFER-1,-1);
		if(iret<0) break; //==0������������������������
		if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
		iret=ppeer->Send(iret,buf,-1);
		if(iret<0) break;
	}//?while
	ppeer->Close(); 
	psock->setParent(NULL); //���ڴ��ж�ת���߳��Ƿ����
	return;
}
