/*******************************************************************
   *	dnsclnt.h
   *    DESCRIPTION:DNSЭ��ͻ�������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-02
   *
   *	net4cpp 2.1
   *	����ϵͳ��DNS����һ������TCP/IPӦ�ó���ķֲ�ʽ���ݿ⣬
   *	���ṩ�������ֺ�IP��ַ֮���ת����Ϣ��ͨ���������û�ͨ
   *	��UDPЭ���DNS����������ͨ�ţ������������ض���53�˿ڼ������������û�����������Ϣ��
   *******************************************************************/
#ifndef __YY_DNS_CLINET_H__
#define __YY_DNS_CLINET_H__

#include "dnsdef.h"
#include "socketUdp.h"

namespace net4cpp21
{
	
	//MX��response��rdata�����ݽṹ
	typedef struct dns_rdata_mx
	{
		unsigned short priority; //���ȼ���
		std::string mxname; //�������ʼ���������������
	}DNS_RDATA_MX,*PDNS_RDATA_MX;

	class dnsClient : public socketUdp
	{
	public:
		dnsClient();
		virtual ~dnsClient(){}
		void setTimeout(time_t s);
		//������ѯ ����--->IP,����DNS_RCODE_ERR_OK(0)�ɹ�
		SOCKSRESULT Query(const char *names,const char *dnssvr,int dnsport=DNS_SERVER_PORT);
		//������������,����DNS_RCODE_ERR_OK(0)�ɹ�
		SOCKSRESULT IQuery(const char *ip,const char *dnssvr,int dnsport=DNS_SERVER_PORT);
		//�ʼ���������ѯ,����DNS_RCODE_ERR_OK(0)�ɹ�
		//һ��MX��¼����һ��2�ֽڵ�ָʾ���ʼ������������ȼ�ֵ�����������ʼ�����������ɵ�
		SOCKSRESULT Query_MX(const char *names,const char *dnssvr,int dnsport=DNS_SERVER_PORT);
		//��ȡ��ѯ���ص�dnsͷ��Ϣ
		PDNS_HEADER resp_dnsh() { return &m_dnsh; }
		//��ȡ��ѯ���ص�dns ��index����ѯ��Ϣ
		PDNS_QUERY resp_dnsq(unsigned short index=0);

		//��ȡ��ѯ���صĵ�index���
		PDNS_RESPONSE resp_dnsr(unsigned short index=0);
		//����DNS_RESPONSE��rdata�������
		unsigned long parse_rdata_Q(PDNS_RESPONSE pdnsr=NULL);
		const char * parse_rdata_IQ(PDNS_RESPONSE pdnsr=NULL);
		const char * parse_rdata_MX(PDNS_RDATA_MX pmx,PDNS_RESPONSE pdnsr=NULL);

	private:
		unsigned short m_msgID; //��ϢID
		time_t m_lTimeout;//���ȴ���ʱ����s
		
		std::string m_strnames; //������ʱ���������names
		char m_buffer[DNS_MAX_PACKAGE_SIZE];
		DNS_HEADER m_dnsh;
		DNS_QUERY m_dnsq;
		DNS_RESPONSE m_dnsr;
	};

}//?namespace net4cpp21

#endif
