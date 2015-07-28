/*******************************************************************
   *	smtpclnt.h
   *    DESCRIPTION:smtpЭ��ͻ�������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-20
   *
   *	net4cpp 2.1
   *	���ʼ�����Э��(smtp)
   *******************************************************************/

#ifndef __YY_SMTP_CLINET_H__
#define __YY_SMTP_CLINET_H__

#include "smtpdef.h"
#include "proxyclnt.h"

namespace net4cpp21
{
	
	class smtpClient : public socketProxy
	{
	public:
		smtpClient(const char *ehlo=NULL);
		virtual ~smtpClient();
		void setTimeout(time_t s){ if((m_lTimeout=s)<1) m_lTimeout=SMTP_MAX_RESPTIMEOUT; }
		//����smtp�������֤���ͺ��ʺ�
		void setSMTPAuth(SMTPAUTH_TYPE authType,const char *strAccount,const char *strPwd);
		// �������ܣ�����smtp������ ������ָ���ʼ����ɹ�����SOCKSERR_OK
		SOCKSRESULT sendMail(mailMessage &mms,const char *smtpsvr,int smtpport);
		SOCKSRESULT sendMail_MX(mailMessage &mms,const char *dnssvr,int dnsport);
		//****************************************
		// �������ܣ��ʼ�ֱͶ���ɹ�����SOCKSERR_OK
		//emlfile : �ʼ���ʽ�ļ������ָ�ʽ�ļ�. �ʼ�����ǰ!��ͷ��Ϊע����
		//�����һ��ΪEmail body is base64 encoded���������smtpsvr���յ�Ҫת�����ʼ�
		//����Ϊ�û��༭Ҫ���͵��ʼ�,��ʽ:
		//FROM: <������>\r\n
		//TO: <�ռ���>,<�ռ���>,...\r\n
		//Attachs: <����>,<����>,...\r\n
		//Subject: <����>\r\n
		//Bodytype: <TEXT|HTML>\r\n
		//\r\n
		//...
		//****************************************
		SOCKSRESULT sendMail(const char *emlfile,const char *smtpsvr,int smtpport,const char *from);
		SOCKSRESULT sendMail_MX(const char *emlfile,const char *dnssvr,int dnsport);
		std::vector<std::string> &errors() { return m_errors; }
	private:
		//����ָ����smtp������
		SOCKSRESULT ConnectSvr(const char *smtpsvr,int smtpport);
		SOCKSRESULT Auth_LOGIN();
		SOCKSRESULT _sendMail(mailMessage &mms,const char *toemail);
		bool sendCommand(int response_expected,char *buf,int buflen,int maxbuflen);

	private:
		SMTPAUTH_TYPE m_authType;//smtp�����Ƿ���Ҫ��֤,Ŀǰ����֧��LOGIN��֤��ʽ
		std::string m_strAccount;//LOGIN��֤���ʺź�����
		std::string m_strPwd;
		time_t m_lTimeout;//���ȴ���ʱ����s
		std::vector<std::string> m_errors; //��¼����ʱ�Ĵ���
		std::string m_ehloName;
	};
}//?namespace net4cpp21

#endif

