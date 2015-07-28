/*******************************************************************
   *	rmtsvc.h 
   *    DESCRIPTION: rmtsvcԶ�̿��ƹ���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-12-30
   *	
   *******************************************************************/

#include "net4cpp21/include/sysconfig.h"
#include "net4cpp21/include/cCoder.h"
#include "net4cpp21/include/cLogger.h"
#include "net4cpp21/utils/utils.h"

#include "net4cpp21/include/httpsvr.h"
#ifdef _DEBUG
#pragma comment( lib, "libs/bin/net4cpp21_d" )
#elif defined _SURPPORT_OPENSSL_
#pragma comment( lib, "libs/bin/net4cpp21" )
#else
#pragma comment( lib, "libs/bin/net4cpp21_nossl" )
#endif

//#include "msnbot/msnbot.h"	//yyc remove MSN function
#include "ftpserver.h"
#include "proxyserver.h"
#include "telnetserver.h"
//-----------------------------vIDC--------------------------
#include "libs/vidc/webI/vidcManager.h"

using namespace std;
using namespace net4cpp21;

#define RMTSVC_ACCESS_NONE			0x0000
#define RMTSVC_ACCESS_ALL			0xFFFFFFFF
#define RMTSVC_ACCESS_SCREEN_ALL	0x0003
#define RMTSVC_ACCESS_SCREEN_VIEW	0x0001
#define RMTSVC_ACCESS_REGIST_ALL	0x000c
#define RMTSVC_ACCESS_REGIST_VIEW	0x0004
#define RMTSVC_ACCESS_SERVICE_ALL	0x0030
#define RMTSVC_ACCESS_SERVICE_VIEW	0x0010
#define RMTSVC_ACCESS_TELNET_ALL	0x00c0
#define RMTSVC_ACCESS_TELNET_VIEW	0x0040
#define RMTSVC_ACCESS_FILE_ALL		0x0300
#define RMTSVC_ACCESS_FILE_VIEW		0x0100
#define RMTSVC_ACCESS_FTP_ADMIN		0x0c00 //FTP���ù���Ȩ��
#define RMTSVC_ACCESS_VIDC_ADMIN	0x3000 //vIDC/Proxy����Ȩ��

class webServer : public httpServer
{
public:
	webServer();
	virtual ~webServer(){}
	bool Start(); //��������
	void Stop();//ֹͣ����
	
	void setRoot(const char *rpath,long lAccess,const char *defaultPage);
	void docmd_webs(const char *strParam);
	void docmd_webiprules(const char *strParam);
	void docmd_user(const char *strParam);

	int m_svrport;
	std::string m_bindip;
	bool m_bPowerOff; //�Ƿ�����Ȩ�޾Ϳ�ִ�йػ���������
private:
	virtual bool onHttpReq(socketTCP *psock,httpRequest &httpreq,httpSession &session,
			std::map<std::string,std::string>& application,httpResponse &httprsp);

	bool setLastModify(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool httprsp_version(socketTCP *psock,httpResponse &httprsp);
	bool httprsp_docommandEx(socketTCP *psock,httpResponse &httprsp,const char *strCommand);
	bool httprsp_telnet(socketTCP *psock,httpResponse &httprsp,long lAccess);
	bool httprsp_checkcode(socketTCP *psock,httpResponse &httprsp,httpSession &session);
	bool httprsp_login(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session);
	bool httprsp_capSetting(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session,bool bSetting);
	//���ý�����ȡָ�����ڵ���Ļ
	bool httprsp_capWindow(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session);
	bool httprsp_getpswdfromwnd(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session);
	bool httprsp_GetClipBoard(socketTCP *psock,httpResponse &httprsp);
	bool httprsp_SetClipBoard(socketTCP *psock,httpResponse &httprsp,const char *strval);
	bool httprsp_msevent(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session);
	bool httprsp_keyevent(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool httprsp_command(socketTCP *psock,httpResponse &httprsp,const char *ptrCmd);
	bool httprsp_cmdpage(socketTCP *psock,httpResponse &httprsp,const char *ptrCmd);
	bool httprsp_capDesktop(socketTCP *psock,httpResponse &httprsp,httpSession &session);
	bool httprsp_sysinfo(socketTCP *psock,httpResponse &httprsp);
	bool httprsp_usageimage(socketTCP *psock,httpResponse &httprsp);
	bool httprsp_fport(socketTCP *psock,httpResponse &httprsp);
	bool httprsp_plist(socketTCP *psock,httpResponse &httprsp);
	bool httprsp_mlist(socketTCP *psock,httpResponse &httprsp,DWORD pid);
	bool httprsp_pkill(socketTCP *psock,httpResponse &httprsp,DWORD pid);
	bool httprsp_mdattach(socketTCP *psock,httpResponse &httprsp,DWORD pid,HMODULE hmdl,long count);
	bool httprsp_slist(socketTCP *psock,httpResponse &httprsp);
	bool sevent(const char *sname,const char *cmd);
	bool httprsp_reglist(socketTCP *psock,httpResponse &httprsp,const char *skey,int listWhat);
	bool httprsp_regkey_del(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *skey);
	bool httprsp_regkey_add(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *skey);
	bool httprsp_regitem_del(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *sname);
	bool httprsp_regitem_add(socketTCP *psock,httpResponse &httprsp,const char *spath,
							const char *stype,const char *sname,const char *svalue);
	bool httprsp_regitem_md(socketTCP *psock,httpResponse &httprsp,const char *spath,
							const char *stype,const char *sname,const char *svalue);

	bool  httprsp_filelist(socketTCP *psock,httpResponse &httprsp,const char *spath,int listWhat,bool bdsphide);
	bool  httprsp_folder_del(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *fname,bool bdsphide);
	bool  httprsp_folder_new(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *fname,bool bdsphide);
	bool  httprsp_folder_ren(socketTCP *psock,httpResponse &httprsp,const char *spath,
									 const char *fname,const char *newname,bool bdsphide);
	bool  httprsp_file_del(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *fname,bool bdsphide);
	bool  httprsp_file_ren(socketTCP *psock,httpResponse &httprsp,const char *spath,
									 const char *fname,const char *newname,bool bdsphide);
	bool  httprsp_file_run(socketTCP *psock,httpResponse &httprsp,const char *spath);
	
	bool  httprsp_profile_verinfo(socketTCP *psock,httpResponse &httprsp,const char *spath);
	bool  httprsp_profile(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *prof);
	bool  httprsp_profolder(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *prof);
	bool  httprsp_prodrive(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *svolu);
	bool  httprsp_get_upratio(socketTCP *psock,httpResponse &httprsp,httpSession &session);
	bool  httprsp_upload(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session);
	
	bool  httprsp_ftpsets(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool  httprsp_ftpusers(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool  httprsp_ftpini(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	
	bool  httprsp_proxysets(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool  httprsp_proxyusers(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool  httprsp_proxyini(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	//--------------------------vIDC--------------------------------------------------
	bool  httprsp_mportl(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool  httprsp_mportr(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool  httprsp_vidcini(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool  httprsp_vidcsvr(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool  httprsp_vidccs(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	
	bool httprsp_upnp(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	bool httprsp_upnpxml(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp);
	//����Դ�л�ȡָ�����ļ�
	const char *GetFileFromRes(const char *filepath,DWORD &flength);

	std::string m_defaultPage; //Ĭ���ĵ�
	int m_quality;//��������ͼ�������
	DWORD m_dwImgSize;//��������ͼ��Ĵ�С 0-�����С������ָ����СHWORD=h,WWORD=w
	bool m_bGetFileFromRes;

	//first,�������ʺ� - �����ִ�Сд������ʱһ��ΪСд
	//second.first - �������ʺ� second.second - ������Ȩ��
	std::map<std::string,std::pair<std::string,long> > m_mapUsers;

	bool m_bSSLenabled; //����SSL����
	bool m_bSSLverify; //�Ƿ���пͻ���֤����֤
	//yyc add 2010-02-23 �����ini����ʾ���������˺���֧��������ʽ
	bool m_bAnonymous; //�Ƿ������������ʣ�Ĭ��Ϊ��
};

//---------------------------------------------------------------
//---------------------------------------------------------------
typedef struct _TaskTimer //��ʱ����ṹ
{
	long h,m;//��ʱ��ʱ��,����Ƕ�ʱ���������hָʾ��ʱ���(��)
	long type; //��ʱ���� 't'-��ѯ���(s)ִ�� 'd'ÿ�춨ʱִ��
	long flag; //Ĭ��0
	std::string strTask; //��ʱ����
}TaskTimer;
class sockEvent : public socketBase
{
public:
	sockEvent(){ m_sockstatus=SOCKS_OPENED; }
	virtual ~sockEvent(){}
	virtual void Close(){ m_sockstatus=SOCKS_CLOSED;}
}; //���ڳ������ʱ��ʱ�ر����е�����socket
#include "NTService.h"
class MyService : public CNTService 
{
	sockEvent m_hSockEvent; //���ڳ������ʱ��ʱ�ر����е�����socket
	HANDLE m_hStop; //����ֹͣEvent������
	HANDLE m_hStopEvent; //�Ƿ�����ͨ��SCM�����ֹ̨ͣ�����¼�
						//���������ֹͣ���룬�����ֹͣ���봴�������¼�
	bool m_bSpyself;//�Ƿ���������쳣�˳�
	bool m_bFaceless; //Ĭ��˫�����г����Ƿ񲻴�����̨����
	std::vector<TaskTimer> m_tasklist; //��ʱִ�������б�
	bool CreateTaskTime(const char *ptrAt,const char *strTask);
	void parseCommand(const char *strCommand);
	void docmd_sets(const char *strParam);
//*********************�û��������� statrt ****************************************
public:
	webServer m_websvr;
//	msnShell m_msnbot;	//yyc remove MSN function
	ftpsvrEx m_ftpsvr;
	proxysvrEx m_proxysvr;
	telServerEx m_telsvr;
	//---------------------------vIDC------------------------------
	vidcManager m_vidcManager; //vidc���Ϲ�����
	std::string m_preCmdpage; //cmdpage������ҳ�����ǰ׺
//*********************�û���������  end  ****************************************
public:
	static const char *ServiceVers;
	static MyService *GetService() { return (MyService *)AfxGetService(); }
public:
	explicit MyService(LPCTSTR ServiceName, LPCTSTR ServiceDesc = 0);
	virtual ~MyService(){}
	
	void SetStopEvent(const char *stop_pswd);
	BOOL AutoSpy(const char *commandline);//�����Զ�����
	socketBase *GetSockEvent(){ return &m_hSockEvent; }
private:
	//��������ֹͣ���뱣���¼�
	void CreateStopEvent(const char *stop_pswd);
	//���غ���
	virtual void	Run(DWORD argc, LPTSTR *argv); //����/����������
	virtual void	Stop();//����ֹͣ����
	virtual void	Stop_Request();
	virtual void	Shutdown(); //ϵͳ�ػ�����

};
//��һ�����·����ת��Ϊһ������·����
extern void getAbsolutfilepath(std::string &spath);
extern int splitString(const char *str,char delm,std::map<std::string,std::string> &maps);
extern int splitString(const char *str,char delm,std::vector<std::string> &vec,int maxSplit);
