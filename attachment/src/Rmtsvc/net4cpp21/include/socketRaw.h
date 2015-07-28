/*******************************************************************
   *	socketRaw.h
   *    DESCRIPTION:Raw socket ��Ķ���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-12-10
   *	
   *	net4cpp 2.1
   *******************************************************************/
#ifndef __YY_SOCKET_RAW_H__
#define __YY_SOCKET_RAW_H__

#include "socketBase.h"
#include "ipdef.h"

namespace net4cpp21
{
	class socketRaw : public socketBase
	{
	public:
		socketRaw();
		virtual ~socketRaw(){}
		//Set the packet Time to live
		void SetTTL(unsigned char ucTTL) { m_TTL=ucTTL; return;}
		//Set source address for spoofing
		void SetSourceAddress(const char *lpSourceAddress)
		{
			if(lpSourceAddress) m_SourceAddress.assign(lpSourceAddress);
			return;
		}
		
		//����У���
		static unsigned short checksum(unsigned short *buffer, int size);
	private: //��ֹcopy�͸�ֵ
		socketRaw(socketRaw &sockRaw){ return; }
		socketRaw & operator = (socketRaw &sockRaw) { return *this; }

	protected:
		//����IP ��,�ɹ������û����ݵ���buf�е�ָ�룬�����ip���򷵻�NULL
		//buf --- �յ���IP���ֽ�����len --- �ֽ�������
		//�����ݰ�����ɹ��󣬰����������ݵ�ָ���д˺������أ����ݵĳ��ȿ���datalen��������
		char * decode_ipv4(char *buf,int len);
		unsigned short dataLen_ipv4();
		//��ָ�����û����ݱ���ΪIP���ֽ���,���ر������ֽ�����С
		//data --- �û�����ָ��, datalen --- �û����ݳ���
		//encodebuf --- ��ű�����ip���ֽ���
		int encode_ipv4(const char *data,int datalen,char *encodebuf);
		void ConstructIPV4Header(unsigned char  ucProtocol,unsigned char  ucHeaderLength);
		//bindip==NULL ������IP��==""�󶨱�����һ��ip
		//�����ָ����ip,����Raw socket
		bool create(int IPPROTO_type,const char *bindip);
		//����/���ָ��fd�Ļ���ģʽ
		//�ɹ�����SOCKSERR_OK(0)�������
		SOCKSRESULT Set_Promisc(bool b);
		
		//Time to live
		unsigned char m_TTL;
		std::string m_SourceAddress;//ԴIPĬ��Ϊ������һ��IP
		IpV4Header m_IpV4Header;//IP Header
		union
		{
			TcpHeader m_TcpHeader;
			UdpHeader m_UdpHeader;
		}m_ProtocolHeader; //Protocol Header

	}; 
}//?namespace net4cpp21

#endif

/*
		//�õ�IPͷ�ṹָ��
		LPIpV4Header get_ipHeaderPtr()
		{
			return &m_IpV4Header;
		}
		LPTcpHeader get_tcpHeaderPtr()
		{
			if(m_IpV4Header.Protocol==IPPROTO_TCP) return &m_ProtocolHeader.m_TcpHeader;
			return NULL;
		}
		LPUdpHeader get_udpHeaderPtr()
		{
			if(m_IpV4Header.Protocol==IPPROTO_UDP) return &m_ProtocolHeader.m_UdpHeader;
			return NULL;
		} 
*/




