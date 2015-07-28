/*******************************************************************
   *	vidcsdef.h
   *    DESCRIPTION:��������ͷ�ļ�
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-06-03
   *	
   *******************************************************************/
#ifndef __YY_VIDCCDEF_H__
#define __YY_VIDCCDEF_H__

#include "mportdef.h"

typedef struct _VIDCSINFO
{
	std::string m_vidcsHost;
	int m_vidcsPort;
	std::string m_vidcsPswd; //����vIDCs������
	bool m_bAutoConn;

	long m_vidccID; //������vDCs��vIDCs�����ID��ʶ
	std::string m_vidcsIPList; //vIDCs���ص�vidcs������IP��ַ�б�
	int m_vidcsVer; //vIDCs�İ汾
}VIDCSINFO;

class mapInfo
{
public:
	VIDC_MAPTYPE m_mapType;
	std::string m_appsvr;//Ӧ�÷����ַ�Ͷ˿ڣ���,�ָ�����
	std::string m_appdesc;
	SSLTYPE m_ssltype;
	bool m_sslverify;  //�Ƿ���Ҫ���пͻ���֤����֤������ssltype==TCPSVR_SSLSVR��Ч
	int m_proxyType; //֧�ֵĴ�������
	std::string m_proxyuser;
	std::string m_proxypswd;
	bool m_proxyauth; //�����Ƿ���Ҫ��֤
	MPORTTYPE m_apptype;
	int m_mportBegin;  //Ҫ��ӳ��˿ڷ�Χ
	int m_mportEnd;
	char m_bindLocalIP[16]; //Ҫ��󶨵ı���IP
	bool m_bAutoMap; //��������vIDCs���Ƿ��Զ�ӳ��
	long m_ipaccess;
	std::string m_ipRules;//ip���ʹ��� 
	std::string m_clicert; //�ͻ���֤֤����Ϣ
	std::string m_clikey;
	std::string m_clikeypswd;
	int m_mappedPort; //ʵ��ӳ��Ķ˿�
	bool m_mappedSSLv; //ӳ���ķ���˿��Ƿ���Ҫ���пͻ�֤����֤
	
	unsigned long m_maxconn; //����������ӣ����������� kb/s
	unsigned long m_maxratio;
	std::vector<std::string> m_hrspRegCond;
	std::vector<std::string> m_hreqRegCond;
	mapInfo(){
		m_mapType=VIDC_MAPTYPE_TCP;
		m_appsvr="";
		m_appdesc="";
		m_ssltype=TCPSVR_TCPSVR;
		m_sslverify=false;
		m_proxyType=7;
		m_proxyuser="";
		m_proxypswd="";
		m_proxyauth=false;
		m_apptype=MPORTTYPE_UNKNOW;
		m_mportBegin=m_mportEnd=0;
		m_bindLocalIP[0]=0;
		m_bAutoMap=false;
		m_ipaccess=1;
		m_ipRules="";
		m_clicert="";
		m_clikey="";
		m_clikeypswd="";
		m_mappedPort=0;
		m_mappedSSLv=false;

		m_maxconn=0;
		m_maxratio=0;
	}
	~mapInfo(){}
};


#endif
