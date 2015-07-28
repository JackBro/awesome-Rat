/*******************************************************************
   *	vIDCs.h
   *    DESCRIPTION:vIDC������Ķ���
   *
   *    AUTHOR:yyc
   *	http://hi.baidu.com/yycblog/home
   *
   *******************************************************************/
   
#ifndef __YY_CVIDCS_H__
#define __YY_CVIDCS_H__

#include "../../net4cpp21/include/socketSvr.h"
#include "../../net4cpp21/include/buffer.h"
#include "vidcdef.h"
#include "vidcsdef.h"

namespace net4cpp21
{
	class vidcsvr
	{
	public:
		vidcsvr();
		virtual ~vidcsvr();
		bool bAuthentication() { return m_bAuthentication; }
		void bAuthentication(bool b) { m_bAuthentication=b; }
		std::string &accessPswd() { return m_strPswd; }
		bool DisConnect(long vidccID); //ǿ�ƶϿ�ĳ��vidccc������
		void setLogdatafile(long vidccID,bool b);//���ö�ĳ��vidccӳ��ķ��������־��¼

		void xml_list_vidcc(cBuffer &buffer);
		void xml_info_vidcc(cBuffer &buffer,long vidccID);

	protected:
		void onConnect(socketTCP *psock);//��һ���û���������
		void Destroy(); //�����ͷ�ǰ�������ͷ���Դ����
		vidccSession * docmd_helo(socketTCP *psock,const char *param);
		vidccSession * AddPipeFromVidcSession(socketTCP *pipe,long vidccID);
		bool DelPipeFromVidcSession(socketTCP *pipe,long vidccID);
	private:
		bool m_bAuthentication; //vIDCs�Ƿ���Ҫ��֤
		std::string m_strPswd; //vIDCs����֤����
		cMutex m_mutex;
		//key - vidccID
		std::map<long,vidccSession *> m_sessions; //��ǰ�����ӵ�vIDCc�ͻ��˼���
	};
//***********************************************************************************	
	class vidcServer : public socketSvr,public vidcsvr
	{
	public:
		vidcServer();
		virtual ~vidcServer(){}
		void Stop();
	private:
		//����һ���µĿͻ����Ӵ˷��񴥷��˺���
		virtual void onAccept(socketTCP *psock)
		{
			vidcsvr::onConnect(psock);
			return;
		}
	};
}//?namespace net4cpp21

#endif

