/*******************************************************************
   *	msndef.h
   *    DESCRIPTION:��������ͷ�ļ�
   *
   *    AUTHOR:yyc
   *
   *    HISTORY:
   *
   *    DATE:2005-06-03
   *	
   *******************************************************************/
#ifndef __YY_CMSNDEF_H__
#define __YY_CMSNDEF_H__

#include "../../include/proxyclnt.h"
#include "../../include/Buffer.h"
#include "../../utils/cCmdShell.h"
#include "msncx.h"

#define MSN_SERVER_HOST "messenger.hotmail.com"
#define MSN_SERVER_PORT 1863 //Ĭ��MSN����˿�
#define MSN_MAX_RESPTIMEOUT 15 //s ���ȴ���ʱʱ��
#define MSN_MAX_NS_COMMAND_SIZE 1024 //MSN��������ֽڳ���
#define MSN_MAX_SS_COMMAND_SIZE 4096

#define MSN_ERR_OK SOCKSERR_OK
#define SOCKSERR_MSN_RESP -101 //����ȷ����Ϣ����
#define SOCKSERR_MSN_EMAIL SOCKSERR_MSN_RESP-1 //��Ч���ʺ�
#define SOCKSERR_MSN_DSCONN SOCKSERR_MSN_RESP-2 //�޷�����DS������
#define SOCKSERR_MSN_NSCONN SOCKSERR_MSN_RESP-3 //�޷�����NS������
#define SOCKSERR_MSN_AUTH SOCKSERR_MSN_RESP-4
#define SOCKSERR_MSN_SIGNIN SOCKSERR_MSN_RESP-5 //�ͻ���û�е�¼
#define SOCKSERR_MSN_GETNS SOCKSERR_MSN_RESP-6 //��ȡNS��������Ϣʧ��
#define SOCKSERR_MSN_GETHASHKEY SOCKSERR_MSN_RESP-7
#define SOCKSERR_MSN_STATUS SOCKSERR_MSN_RESP-8 //��ǰ��ϵ�˵�״̬��Ч,����ִ����Ӧ�Ĳ���
#define SOCKSERR_MSN_XFR SOCKSERR_MSN_RESP-9
#define SOCKSERR_MSN_NULL SOCKSERR_MSN_RESP-10
#define SOCKSERR_MSN_UNKNOWED SOCKSERR_MSN_RESP-11

typedef long HCHATSESSION; //��������Ự�����ʵ�ʴ˾��ָ��cContactor����
	
typedef enum
{
	MSN_CHATSESSION_CREATE,
	MSN_CHATSESSION_DESTROY, 
	MSN_CHATSESSION_JOIN,
	MSN_CHATSESSION_BYE,
	MSN_CHATSESSION_TYPING,
	MSN_CHATSESSION_CHAT
}MSN_CHATEVENT_TYPE;
	
typedef enum
{ 
	MSNINVITE_TYPE_UNKNOW=0,
	MSNINVITE_TYPE_PICTURE, //ͷ������ only for msnc1
	MSNINVITE_TYPE_FILE,//�ļ���������
	MSNINVITE_TYPE_CAM=4, //��������ͷ,only for msnc1
	MSNINVITE_TYPE_NETMEET, //only for msnc0
	MSNINVITE_TYPE_AUDIO, //��Ƶ���죬only for msnc0
	MSNINVITE_TYPE_ROBOT=99995225  //����������
}MSN_INVITE_TYPE;
	
typedef enum
{
	MSNINVITE_CMD_INVITE=0,
	MSNINVITE_CMD_ACCEPT,
	MSNINVITE_CMD_REJECT,
	MSNINVITE_CMD_COMPLETED //yyc add 2006-05-19 ����/�������
}MSN_INVITE_CMD;
 
namespace net4cpp21
{
	class cContactor //��ϵ����Ϣ�ṹ
	{
	public:
		unsigned long m_clientID;//��ʶ�û��ͻ�����Ϣ��clientID����clientID�����˵��
		std::string m_strMsnObj;//���յ�msnboj�ַ��������ڱ��ʺţ�ָ���ʺŵ�ͷ�����
							//����MIME������ַ�����δ����MIME����
		std::string m_email;//account;
		std::wstring m_nick;//�û��ǳ� unicode����
		std::string m_nick_char;//�û��ǳ�
		std::string m_uid;//������û�Ψһ��ʶ��������FL�е���ϵ����uid
		std::string m_gid;//�û�group ID��ʶ
		int m_flags;//λ��־ 1=FL, 2=AL, 4=BL ,8=RL

		std::string m_bpr_phh;//�û���ͥ�绰
		std::string m_bpr_phw;//�û��칫�ҵ绰
		std::string m_bpr_phm;//�û����ƶ��绰
		char m_bpr_mob; //�����Ƿ�ɽ����ƶ���Ϣ��Ĭ��ΪN
		char m_bpr_hsb;//�Ƿ���msn�ռ�Ĭ��Ϊ0û�� 1-��
		std::string m_status;//״̬ "NLN","FLN","IDL","BSY","AWY","BRB","PHN","LUN","HDN"
	//						��Ӧ��״̬����Ϊ"online","offline","idle","busy","away","brb","phone","lunch","invisible"
		
		socketProxy m_chatSock;//msn�ͻ��˺�SS�������ĶԵ�ǰ��ϵ�˵�����ͨ����
						//���ڵ�¼�ʺţ����socketΪ��NS����������������ͨ��
		std::map<std::string,cMsncx *> m_msncxMaps; //msncx�����б�
		std::map<long,cMsnc1 *> m_msnc1Maps;//msnc1����Ự״̬�б�,m_msncxMaps�б����m_msnc1Maps
		cBuffer m_buffer; //��ʱ������only for msnc1. ���ڽ���������msnslp��Ϣ
		cCmdShellAsyn m_shell;
		//�����־λ������msnMessager�������¼��ϵ�˵���������
		unsigned long m_lflagEx;
		
		//yyc add 2006-03-13 //�����Ựsession����ʱ��������͵���Ϣ
		//��Ϊ�Ựsession�����󣬿��ܻ�û���յ�IRO��JOI��Ϣ����ʱsession�л�û���������ʱ����
		//���ܻᵼ�·������˹ر�session������
		long m_chat_contacts;//����˴ν�̸�ĻỰ����ϵ�˸���
		std::vector<std::pair<char *,long> > m_vecMessage;

		cContactor():m_flags(0),m_clientID(0),m_bpr_mob('N'),m_chat_contacts(0),
			m_status("FLN"),m_bpr_hsb(0),m_lflagEx(0){}
//		{ m_chatSock.ForbidAutoLinger(); } //yyc remove 2007-08-22
		~cContactor(){m_buffer.Resize(0);m_shell.destroy();}
	};
}//?namespace net4cpp21

inline int wchar2chars(const wchar_t *wstr,char *pdest,int destLen)
{
	return 	WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK|WC_DISCARDNS|WC_SEPCHARS|WC_DEFAULTCHAR,wstr,-1,pdest,destLen,NULL,NULL);
}
#endif
