/*******************************************************************
   *	socketSvr.h
   *    DESCRIPTION:TCP �첽��������Ķ���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-01
   *	
   *	net4cpp 2.1
   *******************************************************************/
#ifndef __YY_SOCKET_SVR_H__
#define __YY_SOCKET_SVR_H__

#include "socketTcp.h"
#include "IPRules.h"
#include "cThread.h"

namespace net4cpp21
{
	class socketSvr : public socketTCP
	{
	public:
		socketSvr();
		virtual ~socketSvr();
		//��������
		SOCKSRESULT Listen(int port,BOOL bReuseAddr=FALSE,const char *bindIP=NULL);
		SOCKSRESULT Listen(int startport,int endport,BOOL bReuseAddr=FALSE,const char *bindIP=NULL);
		const char *svrname() { return m_strSvrname.c_str();} //���ط�������
		iprules &rules() { return m_srcRules;}
		void maxConnection(unsigned long ulMax) { m_maxConnection=ulMax; return; }
		unsigned long maxConnection() const { return m_maxConnection; }
		unsigned long curConnection() const { return m_curConnection; }
		BOOL GetReuseAddr() const { return m_bReuseAddr; } //��ȡ�˿ڸ���״̬
		
	private: //��ֹcopy�͸�ֵ
		socketSvr(socketSvr &sockSvr){ return; }
		socketSvr & operator = (socketSvr &sockSvr) { return *this; }	
	protected:
		cThreadPool m_threadpool;//�����̳߳�
		std::string m_strSvrname;//��������
		
		//����һ���µĿͻ����Ӵ˷��񴥷��˺���
		virtual void onAccept(socketTCP *psock){ return; }
		//�����ǰ���������ڵ�ǰ�趨������������򴥷����¼�
		virtual void onTooMany(socketTCP *psock) { return; }
		virtual void onIdle(void) { return; } //����������m_lAcceptTimeOut�첽��ʱʱ����д��¼�

		
	private:
		iprules m_srcRules;//Ĭ�Ϸ��ʹ��˹������,��ԴIP�Ĺ��˹���
		unsigned long m_curConnection; //��ǰ����������
		unsigned long m_maxConnection; //�������󲢷�������, 0����
		long	m_lAcceptTimeOut; //�첽Accept�ĳ�ʱʱ�� ==-1һֱ�ȴ�

		BOOL m_bReuseAddr; //����˿ڸ���״̬��ֵ:SO_REUSEADDR/SO_EXCLUSIVEADDRUSE/FALSE
		static void transThread(socketTCP *psock); //yyc add 2007-03-29
		static void doRedirectTask(socketTCP *psock); //yyc add 2007-03-29

		static void listenThread(socketSvr *psvr);
		static void doacceptTask(socketTCP *psock);
	};
}//?namespace net4cpp21

#endif

