/*******************************************************************
   *	upnp.h
   *    DESCRIPTION:upnp �ඨ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_UPNP_H__
#define __YY_UPNP_H__

#include "socketUdp.h"
#include "upnpdef.h"

namespace net4cpp21
{
	class upnp : public socketUdpAnsy
	{
	public:
		upnp();
		virtual ~upnp();
		bool bFound() { return m_bFound; } //�Ƿ�Ѱ�ҵ����õ�UPnP�豸
		const char *name() { return m_friendlyName.c_str(); }
		const char *manufacturer() { return m_manufacturer.c_str(); }
		const char *controlURL() { return m_control_url.c_str(); }
		std::vector<UPnPInfo *> &upnpinfo() { return m_upnpsets; }
		bool Search(); //����֧��upnp��NAT�豸
		
		bool AddPortMapping(bool bTCP,const char *internalIP,int internalPort,int externPort,const char *desc);
		bool DeletePortMapping(bool bTCP,int externPort);
		bool GetWanIP(std::string &strRet); //��ȡ����IP��ַ
		bool GetDevXML(std::string &strXml);
		void Clear();
	protected:
		//�����ݵ���
		virtual void onData();
		bool AddPortMapping(UPnPInfo &info);
		bool DeletePortMapping(UPnPInfo &info);
		bool invoke_command(std::string &strCmd,std::map<std::string,std::string> &strArgs);
		bool invoke_property(std::string &reqName,std::string &rspName);

	private:
		std::string m_friendlyName;
		std::string m_manufacturer;
		std::string m_targetName;
		std::string m_control_url;
		std::string m_strLocation;
		bool m_bFound;

		std::vector<UPnPInfo *> m_upnpsets; //UPnPӳ����Ϣ����
	};
}//?namespace net4cpp21

#endif
