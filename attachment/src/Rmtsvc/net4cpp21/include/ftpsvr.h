/*******************************************************************
   *	ftpsvr.h
   *    DESCRIPTION:FTPЭ����������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-12-16
   *
   *	net4cpp 2.1
   *	�ļ�����Э��
   *******************************************************************/

#ifndef __YY_FTP_SERVER_H__
#define __YY_FTP_SERVER_H__

#include "ftpdef.h"
#include "socketSvr.h"

namespace net4cpp21
{

	class cFtpsvr
	{
		class cFtpSession //һ��ftp�ͻ��˻Ự��Ϣ����
		{
			cFtpsvr *m_psvr; //ָ��cFtpsvr��ָ��
		public:
			FTPACCOUNT *m_paccount;//��session�������ʺ���Ϣ�������¼�ɹ���m_paccount����ָ�����ʺ���Ϣ
			socketTCP * m_pcmdsock;//�����socket
			socketTCP m_datasock;//���ݴ���socket
			time_t m_tmLogin;//��¼ʱ��
			char m_dataMode;//���ݴ���ģʽ S-stream C-compressed B-Block
			char m_dataconnMode; //���ݴ���ģʽ FTP_DATACONN_PORT/FTP_DATACONN_PASV
			char m_dataType;//�������ݴ���ʹ�õ���������
					// A--ascII��E-EBCDIC�ı� I-IMAGE һϵ��8λ�ֽڱ�ʾ��ԭʼ����������
					// L-LOCAL ʹ�ÿɱ��ֽڴ�С��ԭʼ����������
			char m_sslMode;//SSL���ݴ������ģʽ 'C' -- ���������ݴ���ͨ�� 'P'�������ݴ���ͨ��
			char m_opMode;//��ǰ���ݲ������� 
						//L--LIST m_filenameָ��Ҫlist��Ŀ¼/�ļ�
						//S--STOR�ļ����� m_filenameָ�����ص��ļ���
						//R--RETR�ļ����� m_filenameָ���ļ����ص��ļ���
			std::string m_filename;//��ʱ�洢��ǰ�����ļ�/Ŀ¼��
			long m_startPoint;//��ʼ���ػ������ļ�����ʼ��
							//����LIST�����˲���ָ��ҪlistĿ¼��Ȩ��,���Ϊ0˵��m_filenameΪ��Ŀ¼
			
			explicit cFtpSession(socketTCP *psock,cFtpsvr *psvr);
			~cFtpSession(){}
			cFtpsvr *pserver() const { return m_psvr; }
			const char *getvpath() const {return m_relativePath.c_str();}

			SOCKSRESULT setvpath(const char *vpath);//���õ�ǰ��Ŀ¼
			SOCKSRESULT getRealPath(std::string &vpath);
			SOCKSRESULT ifvpath(std::string &vpath);//�Ƿ�Ϊ���õ���Ŀ¼
			void list();//List����Ŀ¼
		private:
			std::string m_relativePath;//��ǰ��Ŀ¼·��,!!!���һ���ַ�Ϊ/
			std::string m_realPath;//��ǰ��Ŀ¼·����Ӧ����ʵ·��
			long m_iAccess;//��ǰ·����Ӧ�Ĳ���Ȩ��

			const char *cvtRelative2Absolute(std::string &vpath);
			long cvtVPath2RPath(std::string &vpath);
		};

	public:
		cFtpsvr();
		virtual ~cFtpsvr(){}
		void setHelloTip(const char *strTip){
			if(strTip) m_helloTip.assign(strTip);
			return;
		}
		//ָ��FTP��������ݴ���˿ڵķ�Χ[startport,endport]
		//�����Ϊ[0,0]����ϵͳ�Զ��������˿�
		//�����Ϊ[startport,0],�����Ķ˿�>=startport
		//�����Ϊ[0,endport],�����Ķ˿�<=endport
		void setDataPort(int startport,int endport)
		{
			if( (m_dataport_start=startport)< 0) 
				m_dataport_start=0;
			if( (m_dataport_end=endport)< 0) 
				m_dataport_end=0;
			return;
		}
		
	protected:
		//��ȡ�ʺ���Ϣ������ָ�����ʺŶ���
		FTPACCOUNT *getAccount(const char *struser);
		//������ʺ���Ϣ
		FTPACCOUNT *newAccount(const char *struser);
		SOCKSRESULT delAccount(const char *struser);
		//��һ���µĿͻ����Ӵ˷���
		void onConnect(socketTCP *psock,time_t tmOpened,unsigned long curConnection,unsigned long maxConnection);
		//��ǰ���������ڵ�ǰ�趨�����������
		void onManyClient(socketTCP *psock);
		
		virtual bool onNewTask(THREAD_CALLBACK *pfunc,void *pargs)
		{
			return false;
		}
		//��չ�����,������������ǿ�ʶ�����չ�������Ϊ����ʶ�����չ����
		virtual bool onCommandEx(socketTCP *psock,const char *strCommand
			,cFtpSession &clientSession)
		{
			return false;
		}
		virtual void onLogEvent(long eventID,cFtpSession &session)
		{
			return;
		}

	private:
		void parseCommand(cFtpSession &clientSession,socketTCP *psock
									  ,const char *ptrCommand);
		bool docmd_user(socketTCP *psock,const char *strUser,cFtpSession &clientSession);
		void docmd_type(socketTCP *psock,const char *strType,cFtpSession &clientSession);
		void docmd_cwd(socketTCP *psock,const char *strDir,cFtpSession &clientSession);
		void docmd_mkd(socketTCP *psock,const char *strDir,cFtpSession &clientSession);
		unsigned long docmd_rmd(socketTCP *psock,const char *strDir,cFtpSession &clientSession);
		unsigned long  docmd_dele(socketTCP *psock,const char *strfile,cFtpSession &clientSession);
		void docmd_rnfr(socketTCP *psock,const char *strfile,cFtpSession &clientSession);
		void docmd_rnto(socketTCP *psock,const char *strfile,cFtpSession &clientSession);
		void docmd_size(socketTCP *psock,const char *strfile,cFtpSession &clientSession);
		void docmd_rest(socketTCP *psock,const char *strRest,cFtpSession &clientSession);
		void docmd_prot(socketTCP *psock,const char *strParam,cFtpSession &clientSession);
		void docmd_list(socketTCP *psock,const char *strfile,cFtpSession &clientSession);
		void docmd_retr(socketTCP *psock,const char *strfile,cFtpSession &clientSession);
		void docmd_stor(socketTCP *psock,const char *strfile,cFtpSession &clientSession);
		void docmd_pswd(socketTCP *psock,const char *strpwd,cFtpSession &clientSession);
		void docmd_port(socketTCP *psock,char *strParam,cFtpSession &clientSession);
		
		void docmd_pasv(socketTCP *psock,cFtpSession &clientSession);
		void docmd_pwd(socketTCP *psock,cFtpSession &clientSession);
		void docmd_abor(socketTCP *psock,cFtpSession &clientSession);
		void docmd_cdup(socketTCP *psock,cFtpSession &clientSession);
		void docmd_rein(socketTCP *psock,cFtpSession &clientSession);
		void docmd_sitelist(socketTCP *psock,cFtpSession &clientSession);
		void docmd_authssl(socketTCP *psock);
		void docmd_feat(socketTCP *psock);
		void docmd_quit(socketTCP *psock);

		void resp_noLogin(socketTCP *psock);
		void resp_OK(socketTCP *psock);
		void resp_noImplement(socketTCP *psock);
		void resp_unknowed(socketTCP *psock);

	private:
		std::string m_helloTip;
		int m_dataport_start; //ָ��FTP��������ݴ���˿ڵķ�Χ
		int m_dataport_end;	//���[0,0]��������䣬������ָ�����������˿�
		//��FTP������ʺ���Ϣ
		std::map<std::string,FTPACCOUNT> m_accounts;

		static void dataTask(cFtpSession *psession);
	};

	class ftpServer : public socketSvr,public cFtpsvr
	{
	public:
		ftpServer();
		virtual ~ftpServer();
	private:
		//����һ���µĿͻ����Ӵ˷��񴥷��˺���
		virtual void onAccept(socketTCP *psock)
		{
			cFtpsvr::onConnect(psock,m_tmOpened,curConnection(),maxConnection());
			return;
		}
		//�����ǰ���������ڵ�ǰ�趨������������򴥷����¼�
		virtual void onTooMany(socketTCP *psock)
		{
			cFtpsvr::onManyClient(psock);
			return;
		}
		virtual bool onNewTask(THREAD_CALLBACK *pfunc,void *pargs)
		{
			return (m_threadpool.addTask(pfunc,pargs,THREADLIVETIME)!=0);
		}
	};
}//?namespace net4cpp21

#endif
