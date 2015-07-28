/*******************************************************************
   *	proxyclnt.h
   *    DESCRIPTION:����Э��ͻ�������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-20
   *
   *	net4cpp 2.1
   *	֧��HTTP��socks4��socks5
   *******************************************************************/

#ifndef __YY_PROXY_CLINET_H__
#define __YY_PROXY_CLINET_H__

#include "proxydef.h"
#include "socketTcp.h"
#include "cThread.h"

namespace net4cpp21
{
	class socketProxy : public socketTCP
	{
	public:
		socketProxy();
		virtual ~socketProxy();
		//����ָ����TCP���񡣳ɹ����ر���socket�˿ڣ����򷵻ش���
		virtual SOCKSRESULT Connect(const char *host,int port,time_t lWaitout=-1);
		//����socks BIND����ָ����socks�����Ͽ�һ�������˿�
		//�ɹ����ؿ����Ķ˿�>0 ����ʧ��
		//[out] svrIP/svrPort����socks���񿪵Ķ˿ں�IP��ַ
		//[in] svrIP,����socks���񿪵Ķ˿ڵȴ��������Ǹ�ԴIP���ӣ�svrPort������
		SOCKSRESULT Bind(std::string &svrIP,int &svrPort,time_t lWaitout=-1);
		bool WaitForBinded(time_t lWaitout,bool BindedEvent);
		//����UDP����Э������ (����socks5����Э��֧��UDP����)
		//�ɹ����ؿ�����UDP�˿�>0 ����ʧ��
		//[out] svrIP/svrPort����socks���񿪵Ķ˿ں�IP��ַ
		SOCKSRESULT UdpAssociate(std::string &svrIP,int &svrPort,time_t lWaitout=-1);
		PROXYTYPE proxyType() const { return m_proxytype; }
		bool setProxy(PROXYTYPE proxytype,const char *proxyhost,int proxyport,const char *user,const char *pwd);
		void setProxy(socketProxy &socks);

	private: //��ֹcopy�͸�ֵ
		socketProxy(socketProxy &sockProxy){ return; }
		socketProxy & operator = (socketProxy &sockProxy) { return *this; }

	private:
		//���ʹ����������󣬳ɹ�����SOCKSERR_OK(0)
		SOCKSRESULT sendReq_Connect(const char *host,int port,time_t lWaitout);
		//���ʹ���BIND���󣬳ɹ�����SOCKSERR_OK(0)
		SOCKSRESULT sendReq_Bind(std::string &svrIP,int &svrPort,time_t lWaitout);
		//����UDP����Э�����󣬳ɹ�����SOCKSERR_OK(0) (����socks5����Э��֧��UDP����)
		SOCKSRESULT sendReq_UdpAssociate(std::string &svrIP,int &svrPort,time_t lWaitout);
		//socks5Э����֤,�ɹ���������򷵻ؼ�
		bool socks5_Negotiation(time_t lWaitout);
	private:
		PROXYTYPE m_proxytype;//��������
		std::string m_proxyhost;//�������������
		int m_proxyport;//����������˿�
		std::string m_proxyuser;//���Ӵ�����������ʺ�
		std::string m_proxypwd;//���Ӵ��������������
		int m_dnsType;//����������ʽ 0:����˽��� 1�����ض˽���  2:�ȳ��Ա��ض˽�����Ȼ���ڳ��Է���˽���
			//����socks4ֻ�ܱ��ض˽���,https����ʽ���Ƿ������˽���

		cThread m_thread; //Bind����ȴ��ڶ�����Ӧ�߳�
	};
};//namespace net4cpp21

#endif


