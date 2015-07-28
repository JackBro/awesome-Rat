/*******************************************************************
   *	socketIcmp.h
   *    DESCRIPTION:ICMP socket ��Ķ���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-12-10
   *	
   *	net4cpp 2.1
   *******************************************************************/
#ifndef __YY_SOCKET_ICMP_H__
#define __YY_SOCKET_ICMP_H__

#include "socketRaw.h"
#include "icmpdef.h"

namespace net4cpp21
{
	
	class socketIcmp : public socketRaw
	{
	public:
		socketIcmp();
		virtual ~socketIcmp();
		//����Icmp Echo���ݰ�
		SOCKSRESULT sendIcmp_echo(unsigned long ipDest);
		//����Icmp Echo��Ӧ���ݰ�
		SOCKSRESULT sendIcmp_reply(unsigned long ipDest,unsigned short usId, 
										unsigned short usSeq,unsigned long ulData);
	protected:
		//����Icmp���ݰ�
		SOCKSRESULT sendIcmpPackage(unsigned long ipDest,IcmpHeader &icmph);
	private:
		unsigned short m_echo_seq;
		unsigned short m_echo_id;
	}; 
}//?namespace net4cpp21

#endif

