/*******************************************************************
   *	msnlib.h
   *    DESCRIPTION:msnЭ�鴦���ඨ��
   *
   *    AUTHOR:yyc
   *
   *    HISTORY:
   *
   *    DATE:2005-06-03
   *	
   *******************************************************************/

#ifndef __YY_CMSNLIB_H__
#define __YY_CMSNLIB_H__

#include "msndef.h"

#define __SURPPORT_MSNPROXY__
namespace net4cpp21
{
	class msnMessager //msn�ͻ��˶���
	{
#ifdef __SURPPORT_MSNPROXY__
	public:
		void *proxy_createConnection(HCHATSESSION hchat,socketTCP *psock,const char *host);
//				std::vector<std::pair<HCHATSESSION,msnMessager *> > *pipes);
		static void proxy_transferData(void *param,const char *otherdata,long datalen);
		static void proxy_transThread(void *param){ proxy_transferData(param,NULL,0); }
	private:
		unsigned long proxy_senddata(HCHATSESSION hchat,const char *proxyCmd,const char *content,int len);
		void onProxyChat(HCHATSESSION hchat,const char *proxyCmd, char *undecode_szMsg,int szMsglen);
#endif
	
		friend cMsnc1; friend cMsnc0;
		cMutex m_mutex;
		//���ڵȴ���Ϣ�ظ���Ӧ��������, key--trID(m_msgID)
		std::map<unsigned long,cCond *> m_conds;
		
		std::string m_photofile;//���ʺŵ�ͷ���ļ�����
		unsigned long m_msgID;//��ϢID
		char m_Connectivity;//�Ƿ���ֱ�����ӱ��ͻ���(�����ͻ����Ƿ�ͨ��NAT������ȥ��)
						//Ĭ��'N' ,'Y' : ����ֱ������
		std::string m_clientIP; //���ͻ������ӳ�ȥ��IP(����IP)

		//����ỰĬ�Ϸ���������������
		std::string m_fontName;//��������
		std::string m_encodeFontname;//��������������������ַ���
		DWORD m_fontColor;//������ɫ
		std::string m_fontEF;//����Ч��,B���� �ճ��� Iб��  Sɾ���� U�»���.  �����

		bool sendcmd_VER();
		bool sendcmd_CVR(const char *strAccount);
		//���������ȡNS�������ĵ�ַ
		int sendcmd_USR(const char *strAccount,std::string &strNShost,int &iNSport);
		//���������ȡhttps hashkey
		bool sendcmd_USR(const char *strAccount,std::string &hashkey);
		bool sendcmd_USR(std::string &hashkey);
		//����״̬�ı���Ϣ "NLN","FLN","IDL","BSY","AWY","BRB","PHN","LUN","HDN"
		bool sendcmd_CHG(const char *sta);
		bool sendcmd_UUX();
		bool sendcmd_PNG();
		//nick --- ����utf8��mime������ǳ��ַ���
		bool sendcmd_ADC(const char *email,const char *nick_mime,long waittimeout);
		bool sendcmd_ADC(const char *email,const char *strFlag);
		bool sendcmd_REM(const char *email,const char *strFlag);
		bool sendcmd_XFR(socketProxy &chatSock,const char *email);
		
		cContactor * _newContact(const char *email,const wchar_t *nick);
		bool connectSvr(socketProxy &sock,const char *strhost,int iport);

		//nscmd_xxx������Ϣ��ID��trID
		unsigned long nscmd_sbs(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_msg(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_chl(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_syn(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_lsg(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_lst(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_bpr(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_prp(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_ubx(socketTCP *psock,const char *email,const char *pdata);
		unsigned long nscmd_iln(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_fln(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_nln(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_rem(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_adc(socketTCP *psock,const char *pcmd);
		unsigned long nscmd_rng(socketTCP *psock,const char *pcmd);
		
		unsigned long sscmd_msg(cContactor *pcon,const char *msg_email,char *pcmd,int cmdlen);
		void msnc0_parse(cContactor *pcon,const char *msg_email,char *ptrmsg);
		void msnc1_parse(cContactor *pcon,const char *msg_email,unsigned char *pBinarystuff,
								char *ptrmsg,unsigned long lfooter);

		static void receiveThread(msnMessager *pmsnmessager); //�����ͻ��˺�NS�������������
		static void sessionThread(void *param); //�����ͻ��˺�SS�������������
		static void shellThread(void *param);
		static SOCKSRESULT passport_auth(std::string &strKey,const char *strAccount,const char *pwd);
		static bool MSNP11Challenge(std::string &strChallenge,const char *szClientID,const char *szClientCode);
		unsigned long msgID() { return ++m_msgID; }
		void eraseCond(unsigned long trID){
			m_mutex.lock(); m_conds.erase(trID); m_mutex.unlock();
			return; }

	protected:
		SOCKSRESULT m_lasterrorcode;
		cThreadPool m_threadpool;//�����̳߳�
		cContactor m_curAccount;//��ǰ��¼�ʺ���Ϣ
		std::map<std::string, cContactor *> m_contacts;//��ϵ���б� first:��ϵ��email second:��ϵ����Ϣ�ṹָ��
		std::map<std::string,std::wstring> m_groups;//���б� first:��ID��second:������
		
		void clear();
		bool sendcmd_SS_Typing(cContactor *pcon,const char *type_email);
		bool sendcmd_SS_chatMsg(cContactor *pcon,char *msgHeader,int headerlen,const char *chatMsg,int msglen);
		bool sendcmd_SS_chatMsg(std::vector<cContactor *> &vec,char *msgHeader,int headerlen,const char *chatMsg,int msglen);
		bool sendcmd_SS_chatMsgW(cContactor *pcon,char *msgHeader,int headerlen,const wchar_t *chatMsg,int msglen);
		bool sendcmd_SS_chatMsgW(std::vector<cContactor *> &vec,char *msgHeader,int headerlen,const wchar_t *chatMsg,int msglen);
		
		int encodeChatMsgHead(char *buffer,int buflen,const char *IMFont,const char *dspname);
		int encodeChatMsgHeadW(char *buffer,int buflen,const wchar_t *IMFont,const wchar_t *dspname);
		
		bool createShell(HCHATSESSION hchat);
		bool destroyShell(HCHATSESSION hchat);

		HCHATSESSION createChat(cContactor *pcon);
	public:
		msnMessager();
		virtual ~msnMessager();
		int getLastErrorCode() const { return m_lasterrorcode; }
		const char *thisEmail() { return m_curAccount.m_email.c_str(); }
		bool Connectivity() const { return (m_Connectivity=='Y'); }
		const char *clientIP() const { return m_clientIP.c_str(); }
		bool ifSigned() { 
			return (m_curAccount.m_chatSock.status()==SOCKS_CONNECTED); 
		}

		bool signin(const char *strAccount,const char *strPwd);
		virtual void signout();//�������¼��е��ô˷���
		//������������
		bool setChatFont(const char *fontName,const char *fontEF,long fontColor);
		bool changeNick(const char *nick);
		bool changeNickW(const wchar_t *nickW,int nicklen=0);
		bool changeStatus(const char *sta)
		{
			return sendcmd_CHG(sta);
		}
		//����ĳ���ʺţ����ʺŴ�AL��rem����ӵ�BL��
		bool blockEmail(const char *email);
		bool unblockEmail(const char *email);
		//ɾ��ĳ���ʺţ�ifBlock--�Ƿ���ֹ���ʺ�
		bool remEmail(const char *email,bool ifBlock);
		//�����ϵ��
		//���waittimeout��0�򲻵ȴ��������ķ���
		//>0�ȴ�ָ����ʱ�����ȴ�MSN_MAX_TIMEOUT
		bool addEmail(const char *email,long waittimeout);
		//��ĳ���û�����һ������,�ɹ�����HCHATSESSION�����򷵻�0
		//�ܷ���������û���������Ч���û�״̬������FLN�������ҵ���ϵ��FL������
		HCHATSESSION createChat(const char *email);
		//����ĳ�˽���һ������Ự
		bool inviteChat(HCHATSESSION hchat,const char *email);

		void destroyChat(HCHATSESSION hchat);
		//������������.���strMsg==NULL�������������Ϣ��ָ��������������������Ϣ
		bool sendChatMsg(HCHATSESSION hchat,const char *strMsg,const char *dspname=NULL);
		bool sendChatMsg(HCHATSESSION hchat,std::string &strMsg,const char *dspname=NULL);
		bool sendChatMsgW(HCHATSESSION hchat,const wchar_t *strMsg,const wchar_t *dspname=NULL);
		bool sendChatMsgW(HCHATSESSION hchat,std::wstring &strMsg,const wchar_t *dspname=NULL);
		//����/ȡ�����ʺŵ�ͷ��
		bool setPhoto(const char *imagefile);
		//��ȡĳ����ϵ�˵�ͷ��
		bool getPhoto(HCHATSESSION hchat,const char *filename);
		bool getPhoto(const char *email,const char *filename);
		bool sendFile(HCHATSESSION hchat,const char *filename);
//-----------------------virtual function--------------------------------------
		//��¼�ɹ����յ���������SYN����Ӧ�����ش��ʺŵ���ϵ�˸����������
		virtual void onSYN(long contactors,long groups) { return; }
		//�ӷ���˻��ĳ������Ϣ
		virtual void onLSG(const wchar_t *groupname,const char *groupid){ return; }
		//�ӷ���˻��ĳ����ϵ����Ϣ
		virtual void onLST(const char *email,const wchar_t *nick,int flags){ return; }
		//��¼�ɹ��󣬽�����������ϵ���б������¼�,��ʱ�û�Ӧ����������Ϣ. 
		virtual void onSIGN(){
			sendcmd_CHG("NLN");//����������Ϣ��UUX��Ϣ
			sendcmd_UUX(); return;
		}
		virtual void onSIGNOUT(){ return; }
		//ĳ���û�����/����
		virtual void onLine(HCHATSESSION hchat,const char *email) { return; }
		//ĳ���û�����
		virtual void offLine(HCHATSESSION hchat,const char *email) { return; }
		//ĳ���û�״̬/���Ƹı�. 1:״̬�ı� 2:�ǳƸı� 4:clientID�ı� 8:ͷ��ı�
		//������λΪ1��˵���յ�����ILN��Ϣ
		virtual void onNLN(HCHATSESSION hchat,const char *email,long flags){ return; }
		//ĳ���û�ɾ�����㴥�����¼�.����0��ʲôҲ����������
		//1 --- �����û�ɾ�� //2 --- �����û����� //3 --- �����û�ɾ��������
		virtual int onREM(HCHATSESSION hchat,const char *email){ return 1;}
		//ĳ���û�����˱��ʺţ���������գ�����ܾ����˿����Լ�
		virtual bool onADC(HCHATSESSION hchat,const char *email){ return true; }
		//���ʺ��������ĳ���û����ɹ����֪ͨ�¼�
		virtual void onADD(HCHATSESSION hchat,const char *email){ return; }
		//chattype --- ָʾ���¼�������
		//	CHATSESSION_CREATE : ������һ���µ�chat session. email---������chatsession����ϵ���ʺ�email. lparam������
		//	CHATSESSION_DESTROY: һ��chat session�����١� email/lparam��������
		//	CHATSESSION_JOIN: һ����ϵ�˼��������Ự.email ---��ϵ���ʺ�email. lparam:��ǰ�Ự�������߸���(long)
		//	CHATSESSION_BYE: һ����ϵ���˳��˴�����Ự��email ---��ϵ���ʺ�email. lparam:��ǰ�Ự�������߸���(long)
		//	CHATSESSION_TYPING: һ����ϵ������������Ϣ.email ---��ϵ���ʺ�email. lparam:������
		virtual void onChatSession(HCHATSESSION hchat,MSN_CHATEVENT_TYPE chattype,const char *email,long lParam)
		{
			return;
		}
		//�յ�һ��������Ϣ�¼�,char *��δ����mime��utf8������ַ��������������Լ�����
		//����������utf-8����ģ������ʽ�Ǿ���MIME��utf-8�����
		virtual void onChat(HCHATSESSION hchat,const char *email, char *undecode_szMsg,
			int &szMsglen,char *undecode_szFomat,char *undecode_szdspName) { return; }
	
		//���invitecmd==INVITE_CMD_INVITE,��˺���������Ϊ�������룬����ܾ�����
		//������������������
		virtual bool onInvite(HCHATSESSION hchat,int invitetype,
			MSN_INVITE_CMD invitecmd,cMsncx *pmsncx)
		{ 
			if(invitecmd==MSNINVITE_CMD_INVITE && 
				(invitetype==MSNINVITE_TYPE_FILE || invitetype==MSNINVITE_TYPE_PICTURE) )
				return true;
			return false; 
		}

		static bool SHA1File(FILE *fp,std::string &strRet);
		static bool SHA1Buf(const char *buf,long len,std::string &strRet);
		static bool MD5Buf(const char *buf,long len,std::string &strRet);
	};
}//?namespace net4cpp21

#endif

