/*******************************************************************
   *	ftpclnt.h
   *    DESCRIPTION:FTPЭ��ͻ�������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2006-01-23
   *
   *	net4cpp 2.1
   *	�ļ�����Э��
   *******************************************************************/

#ifndef __YY_FTP_CLIENT_H__
#define __YY_FTP_CLIENT_H__

#include "ftpdef.h"
#include "proxyclnt.h"

namespace net4cpp21
{
	class ftpClient : public socketProxy
	{
	public:
		ftpClient(){}
		virtual ~ftpClient(){}
		void setTimeout(time_t s);
		//���÷���ftp������ʺ�
		void setFTPAuth(const char *strAccount,const char *strPwd);
		//����ָ����ftp������
		SOCKSRESULT ConnectSvr(const char *ftpsvr,int ftpport);
		SOCKSRESULT ConnectSvr(const char *ftpurl);
		SOCKSRESULT GetDatasock(const char *ftppath,socketTCP &datasock,bool bRetr);
		SOCKSRESULT RetrFile(const char *ftppath,const char *savefile,
								  long startPoint=0,long lens=-1);
		SOCKSRESULT StorFile(const char *ftppath,const char *filename,
								  long startPoint=0);
		SOCKSRESULT FileSize(const char *ftppath); 
		SOCKSRESULT CWD(const char *ftppath);
		SOCKSRESULT RMD(const char *ftppath);
		SOCKSRESULT MKD(const char *ftppath);
		SOCKSRESULT Delete(const char *ftppath);
		SOCKSRESULT LIST(std::string &listbuf);
	private:
		
		SOCKSRESULT Auth_LOGIN();
		SOCKSRESULT sendPASV(char *buf,int MAXBUFSIZE);
		SOCKSRESULT sendLIST(const char *listcmd,std::string &listbuf);
		SOCKSRESULT sendRETR(const char *retr,FILE *fp,long receiveBytes);
		SOCKSRESULT sendSTOR(const char *destfile,FILE *fp);
		bool sendCommand(int response_expected,char *buf,int buflen,int maxbuflen);

	private:
		std::string m_strAccount;//LOGIN��֤���ʺź�����
		std::string m_strPwd;
		time_t m_lTimeout;//���ȴ���ʱ����s
		
	};
}//?namespace net4cpp21

#endif

