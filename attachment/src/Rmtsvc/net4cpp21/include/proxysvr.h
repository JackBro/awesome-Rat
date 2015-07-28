/*******************************************************************
   *	proxysvr.h
   *    DESCRIPTION:������������
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

#ifndef __YY_PROXY_SERVER_H__
#define __YY_PROXY_SERVER_H__

#include "proxydef.h"
#include "socketSvr.h"

namespace net4cpp21
{
	class cProxysvr
	{
	public:
		cProxysvr();
		virtual ~cProxysvr(){}
		//���ô������֧�ֵ�����
		void setProxyType(int itype) { m_proxytype=itype; }
		void setProxyAuth(bool bAuth) { m_bProxyAuthentication=bAuth; }
		bool setCascade(const char *casHost,int casPort,int type,const char *user,const char *pswd);
		bool getIfLogdata() const { return m_bLogdatafile; }
		void setIfLogdata(bool b){ m_bLogdatafile=b; }
	protected:
		//��ȡ�ʺ���Ϣ������ָ�����ʺŶ���
		PROXYACCOUNT *getAccount(const char *struser);
		//������ʺ���Ϣ
		PROXYACCOUNT *newAccount(const char *struser);
		SOCKSRESULT delAccount(const char *struser);
		void onConnect(socketTCP *psock);//��һ���û���������
		//����ת���������߳�
		virtual bool onTransferTask(THREAD_CALLBACK *pfunc,void *pargs)
		{
			return false;
		}
		//�յ�ת�����ݣ��������ݷ�������
		virtual void onData(char *buf,long len,socketTCP *from,socketTCP *to)
		{ return; }
	private:
		std::pair<std::string,int> * GetCassvr(){ //��ȡ������������
			std::pair<std::string,int> *p=NULL;
			int n=m_vecCassvr.size();
			if(n==1) p=&m_vecCassvr[0];
			else if(n>1){
				srand(clock());
				p=&m_vecCassvr[rand()%n]; //�����ȡһ��Ӧ�÷������Ϣ
			}
			return p; 
		}

		PROXYACCOUNT * ifAccess(socketTCP *psock,const char *user,const char *pwd,int * perrCode=NULL);
		void doSock4req(socketTCP *psock);
		void doSock5req(socketTCP *psock);
		void doHttpsreq(socketTCP *psock);
		void transData(socketTCP *psock,socketTCP *peer,const char *sending_buf,long sending_size);
		static void transThread(void *pthreadParam);
	private:
		int m_proxytype;//���������֧�ֵĴ�������
		bool m_bProxyAuthentication;//�������Ƿ���Ҫ��֤
		//�˴��������ʺ���Ϣ
		std::map<std::string,PROXYACCOUNT> m_accounts;
		//����������ز���
		bool m_bCascade; //�Ƿ�֧�ֶ�������,֧�ֶ��������������������ѡ��
		std::vector<std::pair<std::string,int> > m_vecCassvr;
//		std::string m_casProxysvr; //�����������˿�
//		int m_casProxyport; 
		int m_casProxytype; //��������֧�ֵ�����
		bool m_casProxyAuthentication; //���������Ƿ���Ҫ��֤
		std::pair<std::string,std::string> m_casAccessAuth;
		bool m_bLogdatafile; //�Ƿ��¼�������ת�������ݵ���־�ļ�
	};

	class proxyServer : public socketSvr,public cProxysvr
	{
	public:
		proxyServer();
		virtual ~proxyServer();
	private:
		//����һ���µĿͻ����Ӵ˷��񴥷��˺���
		virtual void onAccept(socketTCP *psock)
		{
			cProxysvr::onConnect(psock);
			return;
		}
		//����ת���������߳�
		virtual bool onTransferTask(THREAD_CALLBACK *pfunc,void *pargs)
		{
			return (m_threadpool.addTask(pfunc,pargs,THREADLIVETIME)!=0);
		}
	};
}//?namespace net4cpp21

#endif

