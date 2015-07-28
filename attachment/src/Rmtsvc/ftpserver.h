/*******************************************************************
   *	ftpserver.h 
   *    DESCRIPTION: FTP����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *	
   *******************************************************************/

#include "net4cpp21/include/sysconfig.h"
#include "net4cpp21/include/cCoder.h"
#include "net4cpp21/include/cLogger.h"

#include "net4cpp21/include/ftpsvr.h"

using namespace std;
using namespace net4cpp21;

//ftp��������ṹ
typedef struct _TFTPSetting
{
	bool autoStart;
	int svrport;
	bool ifSSLsvr;
	int dataportB;
	int dataportE;
	int maxUsers;
	int logEvent;
	std::string bindip;
	std::string tips; //��ӭ��
}TFTPSetting;

typedef struct _TFTPUser
{
	std::string username;//�ʺ�,�ʺŲ����ִ�Сд(�ʺ�ת��ΪСд)
	std::string userpwd;//����,�������==""������������֤
	std::string userdesc;
	unsigned long maxupratio;//����������� K/s,���=0����
	unsigned long maxdwratio;//����������� K/s,���=0����
	unsigned long maxupfilesize;//��������ļ���С KBytes ,0--����
	unsigned long maxdisksize;//����������ʹ�ÿռ� KBytes,0--����
	unsigned long curdisksize;//��ǰ��ʹ�ô��̿ռ� KBytes.
	std::map<std::string,std::pair<std::string,long> > dirAccess;//Ŀ¼����Ȩ��,Ŀ¼���ִ�Сд
			//first --- string : ftp����Ŀ¼·��,�����/����������/ �� /aa/��
			//second --- pair : ��ftp��Ŀ¼��Ӧ��ʵ��Ŀ¼��Ŀ¼�ķ���Ȩ�ޣ�ʵ��Ŀ¼����Ϊ\��β(winƽ̨)
	long ipaccess;
	std::string ipRules;//ip���ʹ���
	long maxLoginusers;//���ƴ��ʺŵ����ͬʱ��¼�û���,<=0������ 
	time_t limitedTime;//���ƴ��ʺ�ֻ��ĳ������֮ǰ��Ч��==0������
	long pswdmode;
	long disphidden; //�Ƿ���ʾ�����ļ�
	long forbid; //�Ƿ���ô��ʺ�
}TFTPUser;

class ftpsvrEx : public ftpServer
{
public:
	ftpsvrEx();
	virtual ~ftpsvrEx(){}
	bool Start();
	void Stop();
	int deleUser(const char *ptr_user);
	bool modiUser(TFTPUser &ftpuser);
	bool readIni();
	bool saveIni();
	void initSetting();
	bool parseIni(char *pbuffer,long lsize);
	bool saveAsstring(std::string &strini);

	TFTPSetting m_settings;
	std::map<std::string,TFTPUser> m_userlist;
protected:
	virtual void onLogEvent(long eventID,cFtpSession &session);
	void docmd_sets(const char *strParam);
	void docmd_ssls(const char *strParam);
	void docmd_ftps(const char *strParam);
	void docmd_user(const char *strParam);
	void docmd_vpath(const char *strParam);
	void docmd_iprules(const char *strParam);
private:
	
};
