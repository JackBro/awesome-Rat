/*******************************************************************
   *	cLogger.h
   *    DESCRIPTION:��־��¼����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	Last modify: 2005-09-01
   *	net4cpp 2.1
   *******************************************************************/
  
#ifndef __YY_CLOGGER_H__
#define __YY_CLOGGER_H__

#include <cstdio>
#include <string>
#include <map>

typedef bool (CB_LOG_OUTPUT)(const char *,long,long);
namespace net4cpp21
{
	typedef enum //���������Ϣ����
	{
		LOGLEVEL_NONE=-1,//���������Ϣ
		LOGLEVEL_DEBUG=0,//������Ϣ
		LOGLEVEL_WARN,//���������Ϣ
		LOGLEVEL_INFO,//����û���¼��Ϣ
		LOGLEVEL_ERROR,//���������Ϣ
		LOGLEVEL_FATAL //�������������Ϣ
	}LOGLEVEL;
	typedef enum //��־�������
	{
		LOGTYPE_NONE,
		LOGTYPE_STDOUT,//�������׼����豸��
		LOGTYPE_FILE,//������ļ�
		LOGTYPE_HWND,//�����һ��windows�Ĵ�����
		LOGTYPE_SOCKET,//�����һ��socket�ܵ�
	}LOGTYPE;
	class cLogger
	{
		LOGTYPE m_logtype;//��־�������
		LOGLEVEL m_loglevel;//��־����
		long m_hout;//������������LOGTYPE_FILE���Ͷ�ӦFILE *,LOGTYPE_HWND���Ͷ�ӦHWND��LOGTYPE_SOCKET���Ͷ�Ӧsocket���
		TCHAR m_fileopenType[4]; //Ĭ��δ���Ǵ򿪷�ʽ"w"
		bool m_bOutStdout;//�Ƿ�ͬ�����������̨
		bool m_bOutputTime;//�Ƿ��ӡ���ʱ��
		CB_LOG_OUTPUT *m_pcallback;
		long	m_lcbparam; //�ص������û�����
		static cLogger *plogger;//��̬��־��ʵ��ָ��
		cLogger(); //������������ʵ��
		
		void _dump(LPCTSTR buf,size_t len);
	public:
		~cLogger();
		static cLogger & getInstance(){ return *plogger; }
		//�����Ƿ��ӡʱ��
		void setPrintTime(bool bPrint){ m_bOutputTime=bPrint; return; }
		void setOutStdout(bool b) { m_bOutStdout=b; return; }
		LOGLEVEL setLogLevel(LOGLEVEL ll){ 
			LOGLEVEL lOld=m_loglevel; m_loglevel=ll; return lOld; }
		//������־�ļ��򿪷�ʽ
		void setOpenfileType(TCHAR c) { m_fileopenType[0]=c; return; }
		void setCallback(CB_LOG_OUTPUT *pfunc,long param)
		{
			m_pcallback=pfunc; m_lcbparam=param;
		}
		//�Ƿ��������ָ���������־
		bool ifOutPutLog(LOGLEVEL ll) { return ( (unsigned int)m_loglevel<=(unsigned int)ll ); }
		LOGTYPE LogType() { return m_logtype; }
		LOGTYPE setLogType(LOGTYPE lt,long lParam);
		void flush(){ 
			if(m_logtype==LOGTYPE_FILE && m_hout)
				::fflush((FILE *)m_hout); return;}
		void dump(LOGLEVEL ll,LPCTSTR fmt,...);
		void dump(LOGLEVEL ll,size_t len,LPCTSTR buf);
		//���DEBUG�������־
		void debug(LPCTSTR fmt,...);
		void debug(size_t len,LPCTSTR buf);
		void dumpBinary(const char *buf,size_t len);
		void dumpMaps(std::map<std::string,std::string> &maps,const char *desc);
		void printTime(); //��ӡ��ǰʱ��
	};
}//?namespace net4cpp21

//�����ڴ�ӡ��Ϣǰ�ȴ�ӡʱ��
#define RW_LOG_SETPRINTTIME(b) \
{ \
	cLogger::getInstance().setPrintTime(b); \
}
//������־�������
#define RW_LOG_SETLOGLEVEL(ll) cLogger::getInstance().setLogLevel(ll);
//#define RW_LOG_SETLOGLEVEL(ll) \
//{ \
//	cLogger::getInstance().setLogLevel(ll); \
//}
//������־����ص�
#define RW_LOG_CALLBACK(pfunc,param) \
{ \
	cLogger::getInstance().setCallback(pfunc,param); \
}
//������־�����ָ�����ļ�
#define RW_LOG_SETFILE(filename) \
{ \
	cLogger::getInstance().setLogType(LOGTYPE_FILE,filename); \
}
//�����ļ���־Ϊ׷�ӷ�ʽ�������Ǹ��Ƿ�ʽ
#define RW_LOG_OPENFILE_APPEND() \
{ \
	cLogger::getInstance().setOpenfileType('a'); \
}
//������־�����ָ���Ĵ���
#define RW_LOG_SETHWND(hWnd) \
{ \
	cLogger::getInstance().setLogType(LOGTYPE_HWND,hWnd); \
}
//������־�����ָ����tcp socket
#define RW_LOG_SETSOCK(sockfd) \
{ \
	cLogger::getInstance().setLogType(LOGTYPE_SOCKET,sockfd); \
}
//������־�������׼����豸stdout
#define RW_LOG_SETSTDOUT() \
{ \
	cLogger::getInstance().setLogType(LOGTYPE_STDOUT,0); \
}
//�������־
#define RW_LOG_SETNONE() \
{ \
	cLogger::getInstance().setLogType(LOGTYPE_NONE,0); \
}

#define RW_LOG_FFLUSH() \
{ \
	cLogger::getInstance().flush(); \
}

#define RW_LOG_LOGTYPE cLogger::getInstance().LogType
//����Ƿ��������ָ���������־
#define RW_LOG_CHECK cLogger::getInstance().ifOutPutLog
//��־���
#define RW_LOG_PRINT cLogger::getInstance().dump
#define RW_LOG_PRINTBINARY cLogger::getInstance().dumpBinary
#define RW_LOG_PRINTMAPS cLogger::getInstance().dumpMaps
//��ӡ��ǰʱ��
#define RW_LOG_PRINTTIME cLogger::getInstance().printTime
//�����Ƿ�ͬ�����������̨
#define RW_LOG_OUTSTDOUT cLogger::getInstance().setOutStdout

#define RW_LOG_DEBUG if(cLogger::getInstance().ifOutPutLog(LOGLEVEL_DEBUG)) cLogger::getInstance().debug
#endif


