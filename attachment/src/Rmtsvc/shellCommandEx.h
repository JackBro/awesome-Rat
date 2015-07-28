/*******************************************************************
   *	shellCommandEx.h 
   *    DESCRIPTION: CMD Shell��չ�����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *	
   *******************************************************************/

extern std::string g_savepath; //�����ļ�Ĭ�ϱ���·��
//ִ��ָ���Ŀ���̨�����dos���������
extern BOOL docmd_kill(const char *processName); //ɱ��ָ���Ľ��� //execCommand.cpp
extern BOOL docmd_exec(const char *strParam); //����ָ���Ľ���   //execCommand.cpp
extern BOOL docmd_exec2buf(std::string &strBuffer,bool ifHide,int iTimeout=-1); //execCommand.cpp
extern BOOL portList(std::string &strRet); //webAction_fport.cpp

BOOL doCommandEx(const char *strCmd,const char *strParam,std::string &strRet);
BOOL setSavePath(const char *spath,std::string &strRet);
BOOL SetCmdPath(const char *spath,std::string &strRet); //����cmd.exe��·��
BOOL sysStatus(std::string &strRet);
BOOL listProcess(const char *filter,std::string &strRet);
BOOL updateRV(const char *strParam,std::string &strRet);
//===============================other-----------------------------------
BOOL upnp_cmd(const char *strCmd,const char *strParam,std::string &strRet);
BOOL ExportSetting(std::string &strRet);
BOOL ImportSetting(const char *strParam,std::string &strRet);
BOOL docmd_mtcpr(const char *strParam,std::string &strRet);
BOOL docmd_telnet(const char *strParam,std::string &strRet);
BOOL docmd_ftpsvc(const char *strParam,std::string &strRet);
BOOL docmd_vidcs(const char *strParam,std::string &strRet);
BOOL docmd_proxysvc(const char *strParam,std::string &strRet);
BOOL docmd_websvc(const char *strParam,std::string &strRet,const char *strIP,const char *urlparam);

class clsOutput
{
public:
	clsOutput(){};
	virtual ~clsOutput(){}
	virtual int print(const char *buf,int len){return 0;};
	virtual bool bTag() { return true; } 
};
BOOL downfile_http(const char *httpurl,const char *strSaveas,clsOutput &sout);
BOOL downfile_ftp(const char *ftpurl,const char *strSaveas,clsOutput &sout);
