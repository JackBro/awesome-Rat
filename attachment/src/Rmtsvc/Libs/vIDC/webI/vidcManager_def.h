/*******************************************************************
   *	vidcManager.h
   *    DESCRIPTION:vIDC���Ϲ�����
   *
   *    AUTHOR:yyc
   *	http://hi.baidu.com/yycblog/home
   *
   *******************************************************************/

#ifndef __YY_VIDC_MANAGERDEF_H__
#define __YY_VIDC_MANAGERDEF_H__

#include "../../../net4cpp21/include/proxysvr.h"
#include "../mportsvr.h"
#include "../vidcs.h"
#include "../vidcc.h"

#ifdef _DEBUG
#pragma comment( lib, "libs/bin/vidc_d" )
#else
#pragma comment( lib, "libs/bin/vidc" )
#endif

using namespace std;
using namespace net4cpp21;

extern std::string g_strMyCert;
extern std::string g_strMyKey;
extern std::string g_strKeyPswd;
extern std::string g_strCaCert;
extern std::string g_strCaCRL;

//���汾�ض˿�ӳ��Ķ��������Ϣ�Ľṹ
typedef struct _TMapParam
{
	bool bAutorun;
	long ipaccess;
	std::string ipRules;//ip���ʹ��� 
	//-ssl ����ͻ�����֤֤��
	std::string clicert;
	std::string clikey;
	std::string clikeypswd;
}TMapParam,*PMapParam;

class vidcServerEx : public vidcServer
{
public:
	vidcServerEx(){
		m_svrport=VIDC_SERVER_PORT;
		m_autorun=false;
		m_ipaccess=1; m_ipRules="";
	}
	virtual ~vidcServerEx(){}
	SOCKSRESULT Start()
	{
		//����vIDCs��SSL֤����Ϣ��Զ��ӳ��ķ�����Ҫ֤����Ϣ�����Ӵ˶����ȡ
		//���vIDCs�����޷�����SSL���ܣ���������˱ؽ�����֤���������Ҫô����Ҫ֤����֤��
		//Ҫô������֤����֤�ģ����򽫲��ܶ�̬����
		this->setCacert(g_strMyCert.c_str(),g_strMyKey.c_str(),g_strKeyPswd.c_str(),false
					,g_strCaCert.c_str(),g_strCaCRL.c_str());
		
		//����IP���˹���
		this->rules().addRules_new(RULETYPE_TCP,m_ipaccess,m_ipRules.c_str());
		const char *ip=(m_bindip=="")?NULL:m_bindip.c_str();
		BOOL bReuseAddr=(ip)?SO_REUSEADDR:FALSE;//����IP������˿�����
		return this->Listen(m_svrport,bReuseAddr,ip);
	}
	
	int m_svrport;
	std::string m_bindip;
	bool m_autorun;
	long m_ipaccess;   //���ʱ���������IP���˹���
	std::string m_ipRules;
};

#endif

