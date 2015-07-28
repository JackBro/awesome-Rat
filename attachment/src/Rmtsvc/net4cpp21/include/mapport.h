/*******************************************************************
   *	mapport.h
   *    DESCRIPTION:�˿�ӳ�����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:
   *
   *	net4cpp 2.1
   *	
   *******************************************************************/

#ifndef __YY_MAPPORT_H__
#define __YY_MAPPORT_H__

#include "socketSvr.h"

namespace net4cpp21
{
	
	class mapServer : public socketSvr
	{
	public:
		mapServer();
		virtual ~mapServer();
		//����ӳ���Ӧ�÷���
		bool mapped(const char *appsvr,int appport,int apptype);

	protected:
		//����ӳ��Ӧ�÷���
		socketTCP * connect_mapped(std::pair<std::string,int>* &p);

		//����һ���µĿͻ����Ӵ˷��񴥷��˺���
		virtual void onAccept(socketTCP *psock);
		//�յ�ת�����ݣ��������ݷ�������
		virtual void onData(char *buf,long len,socketTCP *from,socketTCP *to)
		{ return; }
	private:
		static void transThread(socketTCP *psock);
		//��ӳ���Ӧ�÷���
		std::vector<std::pair<std::string,int> > m_mappedApp;
	};
}//?namespace net4cpp21

#endif
