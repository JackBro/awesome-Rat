/*******************************************************************
   *	cTelnet.h
   *    DESCRIPTION:Telnet for windows��Ķ���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	net4cpp 2.1
   *******************************************************************/
   
#ifndef __YY_CTELNET_H__
#define __YY_CTELNET_H__

#include "socketSvr.h"
	
namespace net4cpp21
{
	class cTelnet //telnet������
	{
		std::string m_telUser;
		std::string m_telPwd;
		bool m_bTelAuthentication;//��telnet�����Ƿ���Ҫ��֤
		long m_telClntnums;//��ǰ���ӵ�telnet�ͻ��˸���
	protected:
		std::string m_telHello;//���пͻ��������Ϻ����ʾ��Ϣ
		std::string m_telTip;//������������ʾ��
		char m_cmd_prefix; //��չ����ǰ׺

		virtual void onCommand(const char *strcmd,socketTCP *psock){ return ; }//�յ��û���������
		virtual bool onLogin(){ return false; }//��һ���û�telnet��¼�ɹ�,��������ֱ�Ӵ���cmd shell
		void onConnect(socketTCP *psock);//��һ���û���������
	public:
		cTelnet();
		virtual ~cTelnet(){}
		//����telnet�ķ����ʺ�,���user==NULL���������Ȩ����,������Ҫ��Ȩ����
		void setTelAccount(const char *user,const char *pwd);
		const char *getTelAccount() { return m_telUser.c_str(); }
		const char *getTelPassword() { return m_telPwd.c_str(); }
		bool bTelAuthentication() { return m_bTelAuthentication;}
		int telClntnums() { return m_telClntnums; }
		void setTelHello(const char *strHello){
			if(strHello) m_telHello.assign(strHello);
			return;
		}
		void setTelTip(const char *strTips){
			if(strTips) m_telTip.assign(strTips);
			return;
		}
	private:
		//���û�д������򷵻���
		bool getInput(socketTCP *psock,std::string &strRet,int bEcho,int timeout);
	};
	
	class telServer : public socketSvr,cTelnet
	{
	public:
		telServer();
		virtual ~telServer();
	protected:
		//����һ���µĿͻ����Ӵ˷��񴥷��˺���
		virtual void onAccept(socketTCP *psock){ cTelnet::onConnect(psock); }
		virtual bool onLogin(){ return true; }
	};

}//namespace net4cpp21

#endif
