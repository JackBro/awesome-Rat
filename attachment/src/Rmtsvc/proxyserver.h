/*******************************************************************
   *	proxyserver.h 
   *    DESCRIPTION: proxy����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *	
   *******************************************************************/

#include "net4cpp21/include/sysconfig.h"
#include "net4cpp21/include/cCoder.h"
#include "net4cpp21/include/cLogger.h"

#include "net4cpp21/include//proxysvr.h"

using namespace std;
using namespace net4cpp21;

//����������ò�����Ϣ
typedef struct _TPROXYSETTINGS
{
	int svrport; //����˿�
	std::string bindip; //��������IP
	int svrtype; //�������֧�ֵĴ�������
	bool bAuth; //��������Ƿ���Ҫ��֤
	bool autorun;  //���������Ƿ��Զ������������
	bool bCascade; //�Ƿ����ö�������
	std::string cassvrip; //������������ַ�Ͷ˿�
	int castype; //��������֧�ֵĴ�������
	bool casAuth; //���������Ƿ���Ҫ��֤
	std::string casuser; //���ʶ���������ʺź�����
	std::string caspswd;
	long ipaccess;   //���ʱ���������IP���˹���
	std::string ipRules;
	bool bLogdatafile; //�Ƿ��¼����ת������
}PROXYSETTINGS;


typedef struct _TProxyUser
{
	std::string username;//�ʺ�,�ʺŲ����ִ�Сд(�ʺ�ת��ΪСд)
	std::string userpwd;//����,�������==""������������֤
	std::string userdesc;
	long ipaccess;
	std::string ipRules;//ip���ʹ���
	unsigned long maxratio;//������ K/s,���=0����
	long maxLoginusers;//���ƴ��ʺŵ����ͬʱ��¼�û���,<=0������ 
	time_t limitedTime;//���ƴ��ʺ�ֻ��ĳ������֮ǰ��Ч��==0������
	long forbid; //�Ƿ���ô��ʺ�

	std::string strAccessDest;//������ֹ���ʵ�Ŀ��
	int bAccessDest; //����ָ����Ŀ���ǽ�ֹ�������� 0��ֹ��������
}TProxyUser;

class proxysvrEx : public proxyServer
{
public:
	proxysvrEx();
	virtual ~proxysvrEx(){}
	bool Start();
	void Stop();
	int deleUser(const char *ptr_user);
	bool modiUser(TProxyUser &puser);
	bool readIni();
	bool saveIni();
	void initSetting();
	bool parseIni(char *pbuffer,long lsize);
	bool saveAsstring(std::string &strini);

	PROXYSETTINGS m_settings;
	std::map<std::string,TProxyUser> m_userlist;
protected:
	void docmd_psets(const char *strParam);
	void docmd_cassets(const char *strParam);
	void docmd_puser(const char *strParam);
	void docmd_iprules(const char *strParam);
private:
	
};
