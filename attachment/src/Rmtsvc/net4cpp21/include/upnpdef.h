/*******************************************************************
   *	upnpdef.h
   *    DESCRIPTION:����UPnPЭ�����õ��ĳ������ṹ�Լ�enum�Ķ���
   *	UPnP - ���弴�ã�����Ѱ��֧��UPnP��NAT/·���豸���Ա��������Ͻ��ж˿�ӳ��
   *	UPnP �豸�������ಥ�˿ڡ�һ���յ��������󣬸��豸�ͼ�������������ȷ�������Ƿ�ƥ�䡣
   *	���ƥ�䣬һ������ SSDP��ͨ�� HTTPU����Ӧ�������͵��ÿ��Ƶ�	
   *	ͬ���������豸��������ʱ�����ᷢ���ಥ SSDP չʾ֪ͨ��Ϣ��֪ͨ����֧�ֵķ��� 
   *	
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *	
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_UPNPDEF_H__
#define __YY_UPNPDEF_H__

#define UPnP_MULTI_ADDR	"239.255.255.250" //UPnP�鲥��ַ�Ͷ˿�
#define UPnP_MULTI_PORT	1900 
#define UPnP_MAX_MESAGE_SIZE 4096

#define HTTP_STATUS_OK 200

typedef struct _TUPnPInfo
{
	bool budp; //�Ƿ�Ϊudp����
	int mapport;
	int appport;
	bool bsuccess; //ӳ���Ƿ�ɹ�
	std::string appsvr;
	std::string appdesc;
	std::string retmsg;  //������ɹ����صĴ�����Ϣ

}UPnPInfo,*PUPnPInfo;

#endif
/* �鲥��ַ�ķ�Χ�Ƕ��٣�
�鲥�ĵ�ַ�Ǳ�����D���ַ��224.0.0.0��239.255.255.255������һЩ��ַ���ض����ô��磬
224.0.0.0��244.0.0.255ֻ�����ھ�������·�����ǲ���ת���ģ�����224.0.0.1�����������ĵ�ַ��
224.0.0.2����·�����ĵ�ַ��224.0.0.5����ospf·�����ĵ�ַ��224.0.13��PIMv2·�����ĵ�ַ��
239.0.0.0��239.255.255.255��˽�е�ַ����192.168.x..x����224.0.1.0��238.255.255.255��������Internet�ϵġ�
*/
/*
[UPnP] Sended Search Packet(len=132), return 132
M-SEARCH * HTTP/1.1
HOST: 239.255.255.250:1900
MAN: "ssdp:discover"
MX: 6
ST: urn:schemas-upnp-org:service:WANIPConnection:1

[UPnP] Received response ,len=322
HTTP/1.1 200 OK
CACHE-CONTROL: max-age=100
DATE: Wed, 25 Jul 2007 00:42:39 GMT
EXT:
LOCATION: http://192.168.0.3:1900/igd.xml
SERVER: TP-LINK Router, UPnP/1.0
ST: urn:schemas-upnp-org:service:WANIPConnection:1
USN: uuid:upnp-WANConnectionDevice-192168035678900001::urn:schemas-upnp-org:service:WANIPConnection:1

[UPnP] Found Loaction: http://192.168.0.3:1900/igd.xml
[httpreq] Sending HTTP Request Header,len=207
GET /igd.xml HTTP/1.1
Accept: * /*
Accept-Language: zh-cn
User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)
Cache-Control: no-cache
Connection: close
Host: 192.168.0.3
Pragma: no-cache

[httprsp] Received HTTP Response Header
HTTP/1.1 200 OK
CONTENT-LENGTH: 2884
CONTENT-TYPE: text/xml
DATE: Wed, 25 Jul 2007 00:42:39 GMT
LAST-MODIFIED: Tue, 28 Oct 2003 08:46:08 GMT
SERVER: TP-LINK Router, UPnP/1.0
CONNECTION: close
[UPnP] Receive XML: 2884 / 2884


*/
