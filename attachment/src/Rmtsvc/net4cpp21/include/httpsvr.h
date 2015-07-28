/*******************************************************************
   *	httpsvr.h
   *    DESCRIPTION:HTTPЭ����������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:
   *
   *	net4cpp 2.1
   *	HTTP/1.1����Э��
   *******************************************************************/

#ifndef __YY_HTTP_SERVER_H__
#define __YY_HTTP_SERVER_H__

#include "httpdef.h"
#include "httpreq.h"
#include "httprsp.h"
#include "socketSvr.h"

namespace net4cpp21
{
	
	class httpSession
	{
		char m_sessionID[24];
		std::map<std::string,std::string> m_maps;
	public:
		time_t m_startTime;//����session����ʼʱ��
		time_t m_lastTime;//���һ�η��ʴ�sessionʱ��
	public:
		static const long SESSION_VALIDTIME;
		static const char SESSION_IDNAME[];
		httpSession();
		virtual ~httpSession(){}
		bool isValid(time_t tNow) { return (tNow-m_lastTime)<SESSION_VALIDTIME; }
		const char *sessionID() { return m_sessionID;}
		bool SetSessionID(const char *strID);
		std::string& operator[](const std::string& key) { return m_maps[key]; }
	};

	class httpServer : public socketSvr
	{
	public:
		httpServer();
		virtual ~httpServer();
		//����web�������Ŀ¼�Լ���Ŀ¼
		bool setvpath(const char *vpath,const char *rpath,long lAccess);
	protected:
		virtual bool onHttpReq(socketTCP *psock,httpRequest &httpreq,httpSession &session,
			std::map<std::string,std::string>& application,httpResponse &httprsp){ return false; }

		//����һ���µĿͻ����Ӵ˷��񴥷��˺���
		virtual void onAccept(socketTCP *psock);
		//�����ǰ���������ڵ�ǰ�趨������������򴥷����¼�
		virtual void onTooMany(socketTCP *psock);
		virtual void onIdle(void);
	protected:
		void httprsp_fileNoFind(socketTCP *psock,httpResponse &httprsp);
		void httprsp_listDenied(socketTCP *psock,httpResponse &httprsp);
		void httprsp_accessDenied(socketTCP *psock,httpResponse &httprsp);
		void httprsp_listDir(socketTCP *psock,std::string &strPath,httpRequest &httpreq,httpResponse &httprsp);

		void httprsp_Redirect(socketTCP *psock,httpResponse &httprsp,const char *url);
		void httprsp_Redirect(socketTCP *psock,httpResponse &httprsp,const char *url,int iSeconds);
		void httprsp_NotModify(socketTCP *psock,httpResponse &httprsp);
		httpSession *GetSession(const char *sessionID)
		{ 
			if(sessionID==NULL) return NULL;
			std::map<std::string,httpSession *>::iterator it=m_sessions.find(sessionID);
			return (it!=m_sessions.end())?(*it).second:NULL;
		}
	private:
		long cvtVPath2RPath(std::string &vpath);

		cMutex m_mutex;
		std::map<std::string,httpSession *> m_sessions;
		std::map<std::string,std::string> m_application;

		std::map<std::string,std::pair<std::string,long> > m_dirAccess;//Ŀ¼����Ȩ��
			//first --- string : http����Ŀ¼·��������/ �� /aa/����Ŀ¼�����ִ�Сдȫ��ת��ΪСд
			//second --- pair : ��http��Ŀ¼��Ӧ��ʵ��Ŀ¼��Ŀ¼�ķ���Ȩ�ޣ�ʵ��Ŀ¼����Ϊ\��β(winƽ̨)
	};
}//?namespace net4cpp21

#endif
