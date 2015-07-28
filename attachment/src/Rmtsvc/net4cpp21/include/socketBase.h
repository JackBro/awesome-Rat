/*******************************************************************
   *	socketBase.h
   *    DESCRIPTION:socket�����װ��Ķ���
   *				֧��IPV4��IPV6�����Ҫ֧��IPV6�趨��IPPROTO_IPV6
   *				Ҫ����IPV6 socket��������SetIpv6(true);
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	Last modify: 2005-09-01
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_CSOCKETBASE_H__
#define __YY_CSOCKETBASE_H__

#include "socketdef.h"

namespace net4cpp21
{
	class socketBase //socket�����Ļ���
	{
	public:
		socketBase();
		socketBase(socketBase &sockB);
		socketBase & operator = (socketBase &sockB);
		virtual ~socketBase();
		virtual void Close();
		socketBase *parent() const { return m_parent; }
		void setParent(socketBase * psock) { m_parent=psock; return; }
		long getErrcode() const { return m_errcode;}
		int  getFlag() const { return m_sockflag; }
		SOCKETTYPE type() const { return m_socktype; }
		SOCKETSTATUS status() const { return m_sockstatus; }
		int getLocalPort() const { return ntohs(m_localAddr.sin_port); }
		int getRemotePort() const { return ntohs(m_remoteAddr.sin_port); }
		const char *getLocalIP() const { return inet_ntoa(m_localAddr.sin_addr); }
		const char *getRemoteIP() const { return inet_ntoa(m_remoteAddr.sin_addr); }
		unsigned long getLocalip() const { return m_localAddr.sin_addr.S_un.S_addr; }
		unsigned long getRemoteip() const { return m_remoteAddr.sin_addr.S_un.S_addr; }
		time_t getStartTime() const { return m_tmOpened; } //����socket�򿪿�ʼʱ��
		SOCKSRESULT setLinger(bool bEnabled,time_t iTimeout=5); //s
		
		SOCKSRESULT setRemoteInfo(const char *host,int port);
		void SetRemoteInfo(unsigned long ipAddr,int port)
		{
			m_remoteAddr.sin_port=htons(port);
			m_remoteAddr.sin_addr.s_addr=ipAddr;
		}
		
		unsigned long getMaxSendRatio() const { return m_maxSendRatio; }
		unsigned long getMaxRecvRatio() const { return m_maxRecvRatio; }
		//���÷��ͻ���������ٶ� Bytes/s
		void setSpeedRatio(unsigned long sendRatio,unsigned long recvRatio)
		{
			m_maxSendRatio=sendRatio;
			m_maxRecvRatio=recvRatio;
			return;
		}
		//��鱾�Ƿ�ɶ�/��д��
		//����ɶ����д�򷵻�1����ʱ����0�����򷵻ش����������-1����ϵͳ����
		int checkSocket(time_t wait_usec,SOCKETOPMODE opmode)
		{
			if(m_sockfd==INVALID_SOCKET) return SOCKSERR_INVALID;
			if(m_parent && m_parent->m_sockstatus<=SOCKS_CLOSED)
				return SOCKSERR_PARENT;
			int fd=m_sockfd;
			int iret=checkSocket(&fd,1,wait_usec,opmode);
			if(iret==SOCKET_ERROR) m_errcode=SOCK_M_GETERROR;
			return iret;
		}
		//��������(TCP/UDP/RAW),���ؽ������ݵĳ���
		SOCKSRESULT Receive(char *buf,size_t buflen,time_t lWaitout)
		{
			return _Receive(buf,buflen,lWaitout,SOCKS_OP_READ);
		}
		SOCKSRESULT Peek(char *buf,size_t buflen,time_t lWaitout)
		{
			return _Receive(buf,buflen,lWaitout,SOCKS_OP_PEEK);
		}
		//���մ�������
		SOCKSRESULT RecvOOB(char *buf,size_t buflen,time_t lWaitout)
		{
			return _Receive(buf,buflen,lWaitout,SOCKS_OP_ROOB);
		}
		//��Ŀ�ķ�������,���ط������ݵĴ�С�����<0��������
		SOCKSRESULT Send(size_t buflen,const char *buf,time_t lWaitout=-1);
		SOCKSRESULT Send(LPCTSTR fmt,...);
		SOCKSRESULT SendOOB(size_t buflen,const char *buf);
		//�õ�����IP�����صõ�����IP�ĸ���
		static long getLocalHostIP(std::vector<std::string> &vec);
		static const char *getLocalHostIP();

		//����ָ�������� ,only for IPV4
		static unsigned long Host2IP(const char *host);
		static const char *IP2A(unsigned long ipAddr)
		{
			struct in_addr in;
			in.S_un.S_addr =ipAddr;
			return inet_ntoa(in);
		}

	protected:
		bool m_ipv6; //�Ƿ�֧��IPV6
		time_t m_tmOpened;//��socket�򿪵�ʱ�䣬������������

		socketBase *m_parent; //������socketָ��
		long m_errcode;//������� ϵͳ���������Զ���������
		int m_sockfd;//socket�ķ��ʾ��
		int m_sockflag; //����socks�Ķ����־��λ0ֻ��TCP�����ӷ���
		SOCKETTYPE m_socktype;//socket���������
		SOCKETSTATUS m_sockstatus;//socket�����״̬
		SOCKADDR_IN m_localAddr;//��socket�󶨵ı����˿ں�ip
		SOCKADDR_IN m_remoteAddr;//��socket���ӵ�Զ�˶˿ں�ip(only for tcp)
							//���ڷ�tcp���͵�socket���˴�������յ����ݵ�Զ�˶˿ں�ip
							//��Ҫ�������ݵ�Զ�˶˿ں�ip
		unsigned long m_recvBytes;//�ܽ����ֽ���
		unsigned long m_sendBytes;//�ܷ����ֽ���
		unsigned long m_maxSendRatio;//��������� Bytes/�룬0-��������
		unsigned long m_maxRecvRatio;//���������� Bytes/�룬0-��������

		int getAF();
		int getSocketInfo();
		static int checkSocket(int *sockfds,size_t len,time_t wait_usec,SOCKETOPMODE opmode);
	protected:

		SOCKSRESULT setNonblocking(bool bNb);
		bool create(SOCKETTYPE socktype);
		SOCKSRESULT Bind(int port,BOOL bReuseAddr,const char *bindip);
		SOCKSRESULT Bind(int startport,int endport,BOOL bReuseAddr,const char *bindip);
		SOCKSRESULT _Receive(char *buf,size_t buflen,time_t lWaitout,SOCKETOPMODE opmode);
		SOCKSRESULT _Send(const char *buf,size_t buflen,time_t lWaitout);
		virtual size_t v_read(char *buf,size_t buflen);
		virtual size_t v_peek(char *buf,size_t buflen);
		virtual size_t v_write(const char *buf,size_t buflen);
		size_t v_writeto(const char *buf,size_t buflen,SOCKADDR_IN &addr);
	};

	//��ʼ��windows�����绷��----------------------------
	class NetEnv
	{
		WSADATA m_wsadata;
		bool m_bState;
	public:
		NetEnv();
		~NetEnv();
		bool getState(){return m_bState;}
		static NetEnv &getInstance();
	};
	//��ʼ��windows�����绷��----------------------------
}//?namespace net4cpp21

#endif