/*******************************************************************
   *	proxydef.h
   *    DESCRIPTION:����proxyЭ�����õ��ĳ������ṹ�Լ�enum�Ķ���
   *				
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-20
   *	
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_PROXYDEF_H__
#define __YY_PROXYDEF_H__

#include <string>
#include "IPRules.h"

#define SOCKSERR_PROXY_AUTH -201 //Э����֤ʧ��
#define SOCKSERR_PROXY_REJECT SOCKSERR_PROXY_AUTH-1
#define SOCKSERR_PROXY_ATYP SOCKSERR_PROXY_AUTH-2 //��֧�ֵĵ�ַ����
#define SOCKSERR_PROXY_USER SOCKSERR_PROXY_AUTH-3
#define SOCKSERR_PROXY_DENY SOCKSERR_PROXY_AUTH-4
#define SOCKSERR_PROXY_EXPIRED SOCKSERR_PROXY_AUTH-5
#define SOCKSERR_PROXY_CONNECTIONS SOCKSERR_PROXY_AUTH-6 //������̫��

#define PROXY_MAX_RESPTIMEOUT 10 //s ��Ӧ�����ʱ
#define PROXY_SERVER_PORT	1080 //Ĭ�ϴ������Ķ˿�

typedef enum //�������Ͷ���
{
	PROXY_NONE=0,//��ʹ�ô���
	PROXY_HTTPS=1,
	PROXY_SOCKS4=2,
	PROXY_SOCKS5=4 
}PROXYTYPE; 

typedef enum //�����������Ͷ���
{
	PROXYREQ_TCP,
	PROXYREQ_BIND,//
	PROXYREQ_UDP //����socks5����Э��֧��UDP����
}PROXYREQTYPE;

typedef struct _proxyaccount //proxy�ʺ���Ϣ
{
	std::string m_username;//�ʺ�,�ʺŲ����ִ�Сд(�ʺ�ת��ΪСд)
	std::string m_userpwd;//����,�������==""������������֤
	net4cpp21::iprules m_ipRules;//Դip���ʹ���

	unsigned long m_maxratio;//������ K/s,���=0����
	long m_loginusers;//��ǰ�Դ��ʺŵ�¼proxy������û�����,ֻ��û���û�����ʱ����ɾ�����ʺ�
	long m_maxLoginusers;//���ƴ��ʺŵ����ͬʱ��¼�û���,<=0������ 
	time_t m_limitedTime;//���ƴ��ʺ�ֻ��ĳ������֮ǰ��Ч��==0������
	
	net4cpp21::iprules m_dstRules;//Ŀ��ip���ʹ���
}PROXYACCOUNT;

//�趨1�ֽڶ��뷽ʽ
//#ifdef WIN32
//	#pragma pack(push)
//	#pragma pack(1)
//#endif

	//sock4����Ӧ��ṹ
	typedef struct _sock4req
	{
	char VN; //version ,must be 4
	char CD; //socks4 command, 1--connect 2--bind
	unsigned short Port;
	unsigned long IPAddr; //socks4����Э�鲻֧�ַ������������
	char other[1];
	}sock4req;
	typedef struct _sock4ans
	{
	char VN; //should be 0
	char CD; //reply code
			//90: request granted
			//91: request rejected or failed
			//92: request rejected becasue SOCKS server cannot connect to
			//	identd on the client
			//93: request rejected because the client program and identd
			//	report different user-ids
	unsigned short Port; 
	unsigned long IPAddr;
	}sock4ans;
	//CONNECT��Ӧ����ֻ��VN��CD�ֶ������壬DSTPORT��DSTIP�ֶα�����
	//******************************
	//sock5��������Ӧ��ṹ
	typedef struct _sock5req
	{
	char Ver;
	char nMethods;
	char Methods[255];
	}sock5req;
	typedef struct _sock5ans
	{
	char Ver;
	char Method;
	}sock5ans;
	typedef struct _sock5req1
	{
	char Ver;
	char Cmd;// 1=CONNECT, 2=TCP BIND, 3=UDP BIND
	char Rsv;// must be 0
	char Atyp;// 1=4 byte IP address, 3=domain name, 4=6 byte IP address
	char other[1]; // depends on address type�� x�ֽ�
	//unsigned short port; // should be in BIG ENDIAN format!
	}sock5req1;
	typedef struct _sock5ans1
	{
	char Ver;
	char Rep;
	char Rsv;// must be 0
	char Atyp;// 1=4 byte IP address, 3=domain name, 4=6 byte IP address
	//char other[1]; // depends on address type�� x�ֽ�
	//unsigned short port; // should be in BIG ENDIAN format!
	//yyc modify 2003-01-17,only support IP address 
	unsigned long IPAddr;
	unsigned short Port;
	//	unsigned long IPAddr end**************
	}sock5ans1;
	typedef struct _authreq
	{
	char Ver;//ver=1, authentication version not proxy version 
	unsigned char Ulen;// 1..255,�û�������
	char Name[255];//�û���,// not NULL terminated!
	unsigned char PLen;//1..255,���볤��
	char Pass[255];// not NULL terminated!
	}authreq;
	typedef struct _authans
	{
	char Ver;
	char Status;
	}authans;

	//UDP������ṹ
	typedef struct _socks5udp
	{
		unsigned short Rsv;  //must be 0
		char Frag; //�Ƿ����ݱ��ֶ������־
		char Atyp;// 1=4 byte IP address, 3=domain name, 4=6 byte IP address
		unsigned long IPAddr;
		unsigned short Port;
	}socks5udp;
//#ifdef WIN32
//	#pragma pack(pop)
//#endif

#endif

/*   RFC SOCKS4
SOCKSЭ�������David Koblas��ƣ���Ying-Da Lee�Ľ���SOCKS 4��

SOCKS 4ֻ֧��TCPת����

�����ĸ�ʽ����:

+----+----+----+----+----+----+----+----+----+----+...+----+
| VN | CD | DSTPORT |      DSTIP        | USERID      |NULL|
+----+----+----+----+----+----+----+----+----+----+...+----+
   1    1      2              4           variable       1

VN      SOCKSЭ��汾�ţ�Ӧ����0x04

CD      SOCKS�����ȡ����ֵ:

        0x01    CONNECT
        0x02    BIND

DSTPORT CD��صĶ˿���Ϣ

DSTIP   CD��صĵ�ַ��Ϣ

USERID  �ͻ�����USERID

NULL    0x00

��Ӧ���ĸ�ʽ����:

+----+----+----+----+----+----+----+----+
| VN | CD | DSTPORT |      DSTIP        |
+----+----+----+----+----+----+----+----+
   1    1      2              4

VN      Ӧ��Ϊ0x00������0x04

CD      ��ȡ����ֵ:

        0x5A    ����ת��
        0x5B    �ܾ�ת����һ����ʧ��
        0x5C    �ܾ�ת����SOCKS 4 Server�޷����ӵ�SOCS 4 Client����������
                IDENT����
        0x5D    �ܾ�ת�����������е�USERID��IDENT���񷵻�ֵ�����

DSTPORT CD��صĶ˿���Ϣ

DSTIP   CD��صĵ�ַ��Ϣ

1) CONNECT����

����CONNECT����DSTIP/DSTPORTָ��ת��Ŀ�ĵء�

SOCKS 4 Server����ԴIP��DSTPORT��DSTIP��USERID�Լ��ɴ�SOCS 4 Client������
����IDENT����(RFC 1413)��ȡ����Ϣ�����ۺ��������Ծ���������Ӧ���ӻ��Ǿܾ�
ת����

����CONNECT��������SOCKS 4 Server��ͼ������ת��Ŀ�ĵص�TCP���ӣ�Ȼ����
SOCKS 4 Client������Ӧ���ģ�ָ���Ƿ�ɹ�����ת�����ӡ�

���CONNECT���󱻾ܾ���SOCKS 4 ServerҲ��SOCKS 4 Client������Ӧ���ģ����
�����ر����ӡ�

CONNECT��Ӧ����ֻ��VN��CD�ֶ������壬DSTPORT��DSTIP�ֶα����ԡ����CD����
0x5A����ʾ�ɹ�����ת�����ӣ�֮��SOCKS 4 Clientֱ���ڵ�ǰTCP�����Ϸ��ʹ�ת
�����ݡ�

2) BIND����

FTPЭ����ĳЩ�����Ҫ��FTP Server����������FTP Client�����ӣ���FTP��������

FTP Client - SOCKS 4 Client - SOCKS 4 Server - FTP Server

a. FTP Client��ͼ����FTP��������SOCKS 4 Client��SOCKS 4 Server����CONNECT
   ���󣬺�����Ӧ��������FTP������������

   CONNECT�������ָ��FTPSERVER.ADDR/FTPSERVER.PORT��

b. FTP Client��ͼ����FTP��������SOCKS 4 Client�����µĵ�SOCKS 4 Server��
   TCP���ӣ������µ�TCP�����Ϸ���BIND����

   BIND���������Ȼָ��FTPSERVER.ADDR/FTPSERVER.PORT��

   SOCKS 4 Server�յ�BIND���󣬸�����������Ϣ�Լ�USERID��BIND�������������
   �������׽��֣�������AddrA/PortA�ϣ�����SOCKS 4 Client���͵�һ��BIND��Ӧ
   ����

   BIND��Ӧ����CD������0x5Aʱ��ʾʧ�ܣ�����DSTPORT��DSTIP�ֶα����ԡ�

   BIND��Ӧ����CD����0x5Aʱ������DSTIP/DSTPORT��ӦAddrA/PortA�����DSTIP��
   ��0(INADDR_ANY)��SOCKS 4 ClientӦ�����滻��SOCKS 4 Server��IP����SOCKS
   4 Server�Ƕ�Ŀ(multi-homed)����ʱ�Ϳ��ܳ������������

c. SOCKS 4 Client�յ���һ��BIND��Ӧ����

   FTP Client����getsockname(����getpeername)��ȡAddrA/PortA��ͨ��FTP����
   ����FTP Server����PORT���֪ͨFTP ServerӦ������������AddrA/PortA��
   TCP���ӡ�

d. FTP Server�յ�PORT�������������AddrA/PortA��TCP���ӣ�����TCP�������
   ��Ԫ����:

   AddrB��PortB��AddrA��PortA

e. SOCKS 4 Server�յ�����FTP Server��TCP�������󣬼�����������ӵ�ԴIP(
   AddrB)�Ƿ���FTPSERVER.ADDRƥ�䣬Ȼ����SOCKS 4 Client���͵ڶ���BIND��Ӧ
   ����

   ԴIP��ƥ��ʱ�ڶ���BIND��Ӧ����CD�ֶ���Ϊ0x5B��Ȼ��SOCKS 4 Server�ر���
   �����ڷ��͵ڶ���BIND��Ӧ����TCP���ӣ�ͬʱ�ر���FTP Server֮���TCP���ӣ�
   ����TCP����(��CONNECT������ص�����TCP����)���������С�

   ԴIPƥ��ʱCD�ֶ���Ϊ0x5A��Ȼ��SOCKS 4 Server��ʼת��FTP��������

   ������Σ��ڶ���BIND��Ӧ����DSTPORT��DSTIP�ֶα����ԡ�

����CONNECT��BIND����SOCKS 4 Server��һ����ʱ��(��ǰCSTCʵ�ֲ���������)��
���趨ʱ����ʱ����SOCKS 4 Server��Application Server֮���TCP����(�����ӻ�
������)��δ������SOCKS 4 Server���ر���SOCKS 4 Client֮����Ӧ��TCP���Ӳ���
����Ӧ��ת����
*/

/*       RFC1928 socks5
SOCKSЭ��λ�ڴ����(TCP/UDP��)��Ӧ�ò�֮�䣬�����Ȼ��λ�������(IP)֮�ϡ�
����IP�㱨��ת����ICMPЭ��ȵȶ���̫�Ͳ����SOCKSЭ���޹ء�

SOCKS 4��֧����֤��UDPЭ���Լ�Զ�̽���FQDN��SOCKS 5֧�֡�

SOCKS Serverȱʡ������1080/TCP�ڡ�����SOCKS Client���ӵ�SOCKS Server֮��
�͵ĵ�һ������:

+----+----------+----------+
|VER | NMETHODS | METHODS  |
+----+----------+----------+
| 1  |    1     | 1 to 255 |
+----+----------+----------+

����SOCKS 5��VER�ֶ�Ϊ0x05���汾4��Ӧ0x04��NMETHODS�ֶ�ָ��METHODS����ֽ�
������֪NMETHODS����Ϊ0�񣬿���ͼ��ʾ����ȡֵ[1,255]��METHODS�ֶ��ж�����
��(���費�ظ�)������ζ��SOCKS Client֧�ֶ�������֤���ơ�

SOCKS Server��METHODS�ֶ���ѡ��һ���ֽ�(һ����֤����)������SOCKS Client��
����Ӧ����:

+----+--------+
|VER | METHOD |
+----+--------+
| 1  |   1    |
+----+--------+

Ŀǰ����METHODֵ��:

0x00        NO AUTHENTICATION REQUIRED(������֤)
0x01        GSSAPI
0x02        USERNAME/PASSWORD(�û���/������֤����)
0x03-0x7F   IANA ASSIGNED
0x80-0xFE   RESERVED FOR PRIVATE METHODS(˽����֤����)
0xFF        NO ACCEPTABLE METHODS(��ȫ������)

���SOCKS Server��Ӧ��0xFF����ʾSOCKS Server��SOCKS Client��ȫ�����ݣ�
SOCKS Client����ر�TCP���ӡ���֤����Э����ɺ�SOCKS Client��
SOCKS Server������֤������ص���Э�̣��ο������ĵ���Ϊ������㷺�����ԣ�
SOCKS Client��SOCKS Server����֧��0x01��ͬʱӦ��֧��0x02��

��֤������ص���Э����ɺ�SOCKS Client�ύת������:

+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+

VER         ���ڰ汾5������0x05

CMD         ��ȡ����ֵ:

            0x01    CONNECT
            0x02    BIND
            0x03    UDP ASSOCIATE

RSV         �����ֶΣ�����Ϊ0x00

ATYP        ����ָ��DST.ADDR������ͣ���ȡ����ֵ:

            0x01    IPv4��ַ
            0x03    FQDN(ȫ������)
            0x04    IPv6��ַ

DST.ADDR    CMD��صĵ�ַ��Ϣ����ҪΪDST���Ի�

            �����IPv4��ַ��������big-endian���4�ֽ�����

            �����FQDN������"www.nsfocus.net"�����ｫ��:

            0F 77 77 77 2E 6E 73 66 6F 63 75 73 2E 6E 65 74

            ע�⣬û�н�β��NUL�ַ�����ASCIZ������һ�ֽ��ǳ�����

            �����IPv6��ַ��������16�ֽ����ݡ�

DST.PORT    CMD��صĶ˿���Ϣ��big-endian���2�ֽ�����

SOCKS Server��������SOCKS Client��ת�����󲢷�����Ӧ����:

+----+-----+-------+------+----------+----------+
|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+

VER         ���ڰ汾5������0x05

REP         ��ȡ����ֵ:

            0x00        �ɹ�
            0x01        һ����ʧ��
            0x02        ��������ת��
            0x03        ���粻�ɴ�
            0x04        �������ɴ�
            0x05        ���Ӿܾ�
            0x06        TTL��ʱ
            0x07        ��֧��������е�CMD
            0x08        ��֧��������е�ATYP
            0x09-0xFF   unassigned

RSV         �����ֶΣ�����Ϊ0x00

ATYP        ����ָ��BND.ADDR�������

BND.ADDR    CMD��صĵ�ַ��Ϣ����ҪΪBND���Ի�

BND.PORT    CMD��صĶ˿���Ϣ��big-endian���2�ֽ�����

1) CONNECT����

����CMDΪCONNECT��SOCKS Client��SOCKS Server֮��ͨ�ŵ������Ԫ����:

SOCKSCLIENT.ADDR��SOCKSCLIENT.PORT��SOCKSSERVER.ADDR��SOCKSSERVER.PORT

һ��SOCKSSERVER.PORT��1080/TCP��

CONNECT������е�DST.ADDR/DST.PORTָ��ת��Ŀ�ĵء�SOCKS Server���Կ�
DST.ADDR��DST.PORT��SOCKSCLIENT.ADDR��SOCKSCLIENT.PORT�����������Ծ�������
��ת��Ŀ�ĵص�TCP���ӻ��Ǿܾ�ת����

�����������ת�����ҳɹ�������ת��Ŀ�ĵص�TCP���ӣ������Ԫ����:

BND.ADDR��BND.PORT��DST.ADDR��DST.PORT

��ʱSOCKS Server��SOCKS Client���͵�CONNECT��Ӧ���н�ָ��BND.ADDR/BND.PORT��
ע�⣬BND.ADDR���ܲ�ͬ��SOCKSSERVER.ADDR��SOCKS Server�������������Ƕ�Ŀ(
multi-homed)������

����ܾ�ת����δ�ܳɹ�������ת��Ŀ�ĵص�TCP���ӣ�CONNECT��Ӧ����REP�ֶν�
ָ������ԭ��

��Ӧ����REP����ʱ��ʾʧ�ܣ�SOCKS Server�����ڷ�����Ӧ���󲻾�(������10s)��
����SOCKS Client֮���TCP���ӡ�

��Ӧ����REPΪ��ʱ��ʾ�ɹ���֮��SOCKS Clientֱ���ڵ�ǰTCP�����Ϸ��ʹ�ת����
�ݡ�

2) BIND����

����CMDΪBIND���������FTPЭ�飬FTPЭ����ĳЩ�����Ҫ��FTP Server��������
��FTP Client�����ӣ���FTP��������

FTP Client - SOCKS Client - SOCKS Server - FTP Server

a. FTP Client��ͼ����FTP��������SOCKS Client��SOCKS Server����CONNECT����
   ������Ӧ��������FTP������������

   CONNECT�������ָ��FTPSERVER.ADDR/FTPSERVER.PORT��

b. FTP Client��ͼ����FTP��������SOCKS Client�����µĵ�SOCKS Server��TCP��
   �ӣ������µ�TCP�����Ϸ���BIND����

   BIND���������Ȼָ��FTPSERVER.ADDR/FTPSERVER.PORT��SOCKS ServerӦ�þݴ�
   ����������

   SOCKS Server�յ�BIND���󣬴������׽��֣�������AddrA/PortA�ϣ�����SOCKS
   Client���͵�һ��BIND��Ӧ��������BND.ADDR/BND.PORT��AddrA/PortA��

c. SOCKS Client�յ���һ��BIND��Ӧ����FTP Clientͨ��FTP��������FTP Server��
   ��PORT���֪ͨFTP ServerӦ������������AddrA/PortA��TCP���ӡ�

d. FTP Server�յ�PORT�������������AddrA/PortA��TCP���ӣ�����TCP�������
   ��Ԫ����:

   AddrB��PortB��AddrA��PortA

e. SOCKS Server�յ�����FTP Server��TCP����������SOCKS Client���͵ڶ���
   BIND��Ӧ��������BND.ADDR/BND.PORT��AddrB/PortB��Ȼ��SOCKS Server��ʼת
   ��FTP��������

������һЩ���ۼ�¼:

scz

Ϊʲô��Ҫ���͵ڶ���BIND��Ӧ����ָ��AddrB/PortB��������ڡ�

knightmare@apue

ָ��AddrB/PortB���������ڣ�FTP Client���ڰ�ȫ���ǣ�����FTP��������ԴIP��
Դ�˿ڣ�����FTP��������Դ��ֻ������FTPSERVER.ADDR/20��

scz

knightmare�Ĵ�����ȷ�ģ����ҵ��ɻ����Դ���Ҷ�SOCKSЭ��Ĵ�����⣬����
���һ��������������⡣��ʵ��Ӧ�ò鿴David Koblas��ԭʼ�ĵ������BIND����
��ȫ���̡�ǰ�����FTP������������������������������˿��������ʵ�Ե���ˡ�

3) UDP ASSOCIATE����

����CMDΪUDP ASSOCIATE����ʱDST.ADDR��DST.PORTָ������UDP����ʱ��ԴIP��Դ
�˿ڣ�������UDPת��Ŀ�ĵأ�SOCKS Server���Ծݴ˽��������Ծ����Ƿ����UDPת
�������SOCKS Client����UDP ASSOCIATE����ʱ�޷��ṩDST.ADDR��DST.PORT����
���뽫�����������㡣

������һЩ���ۼ�¼:

scz

ʲô�����SOCKS Client����UDP ASSOCIATE������޷��ṩDST.ADDR��DST.PORT��
����˵����ʲô���ǲ���Ҫ���⽫�����������㡣����ʵ���Ӵ�����

shixudong@163.com

�����������:

Application Client - SOCKS Client - NAT - SOCKS Server - Application Server

SOCKS Client��UDP ASSOCIATE������ָ��DST.ADDR/DST.PORT��SOCKS Server����Щ
��Ϣ�����Ƿ�ת��ĳ��UDP���ġ���ͼ��SOCKS Client��SOCKS Server֮����NAT��ǰ
���޷�Ԥ֪UDP���ľ���NAT��ԴIP��Դ�˿ڻ���ʲô�������϶���䣬���ǰ����
����ǰ��UDP ASSOCIATE������ָ��DST.ADDR/DST.PORT�����ǿ��ָ������ֵ������
���⵽��ת��UDP���ĵ�ԴIP��Դ�˿���DST.ADDR/DST.PORT��ƥ����ܾ�ת������
�����������RFC 1928����SOCKS Client��DST.ADDR/DST.PORT���㣬SOCKS Server
��ʱ���ټ���ת��UDP���ĵ�ԴIP��Դ�˿ڡ�

��һ��TCP������SOCKS Client��SOCKS Server������UDP ASSOCIATE�������UDP
ת��Ҫ���TCP���Ӽ���ά�֣���TCP���ӹر�ʱ��Ӧ��UDPת��Ҳ����ֹ�����仰˵��
UDPת����Ȼ������һ��TCP���ӣ��⽫���Ķ������Դ��

SOCKS Server��SOCKS Client����UDP ASSOCIATE��Ӧ����BND.ADDR/BND.PORTָ��
SOCKS ClientӦ�����﷢�ʹ�ת��UDP���ġ�

����UDPת����SOCKS Client���ͳ�ȥ��UDP����������:

+----+------+------+----------+----------+----------+
|RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
+----+------+------+----------+----------+----------+
| 2  |  1   |  1   | Variable |    2     | Variable |
+----+------+------+----------+----------+----------+

RSV         �����ֶΣ�����Ϊ0x0000

FRAG        Current fragment number

            0x00        ����һ������Ƭ��SOCKS UDP����
            0x01-0x7F   SOCKS��Ƭ���
            0x80-0xFF   ���λ��1��ʾ��Ƭ���н��������������һ��SOCKS��Ƭ

ATYP        ����ָ��DST.ADDR������ͣ���ȡ����ֵ:

            0x01    IPv4��ַ
            0x03    FQDN(ȫ������)
            0x04    IPv6��ַ

DST.ADDR    ת��Ŀ���ַ

DST.PORT    ת��Ŀ��˿�

DATA        ԭʼUDP������

SOCKS Server������ΪSOCKS Client����UDPת��������֪ͨ����ת����ɻ��Ǳ���
����

FRAG����֧��SOCKS��Ƭ��SOCKS��Ƭ���շ�һ��ʵ����������������鶨ʱ��������
���鶨ʱ����ʱ���ߵ���SOCKS��Ƭ���ڸ���SOCKS��Ƭ����������У���ʱ��������
������С����鶨ʱ������С��5�롣Ӧ�þ����ܵر������SOCKS��Ƭ��

�Ƿ�֧��SOCKS��Ƭ�ǿ�ѡ�ģ����һ��SOCKSʵ�ֲ�֧��SOCKS��Ƭ������붪����
�н��յ���SOCKS��Ƭ������ЩFRAG�ֶη����SOCKS UDP���ġ�

����SOCKSʵ����֧��UDPת��ʱ����ԭʼUDP������ǰ����һ��SOCKSЭ����ص�ͷ��
���ΪUDP����������ռ�ʱҪΪ���ͷ����ռ�:

ATYP    ͷռ���ֽ�  ԭ��
0x01    10          IPv4��ַռ4�ֽڣ�4+6=10
0x03    262         ��������һ���ֽڣ�������0xFF��1+255+6=262
0x04    20          �����һ����Ǳ���IPv6��ַռ16�ֽڣ�16+6=22

�һ���RFC 1928�����б���д��ѯ��mleech@bnr.ca��ietf-web@ietf.orgȥ�ˡ�
*/
/*     RFC 1929
����SOCKS V5 Client/ServerЭ�̲����û���/������֤����(0x02)�����ڿ�ʼ��Ӧ
��Э�̡�

�ͻ��˷������±���:

+----+------+----------+------+----------+
|VER | ULEN |  UNAME   | PLEN |  PASSWD  |
+----+------+----------+------+----------+
| 1  |  1   | 1 to 255 |  1   | 1 to 255 |
+----+------+----------+------+----------+

VER     ��Э�̵ĵ�ǰ�汾��Ŀǰ��0x01

ULEN    UNAME�ֶεĳ���

UNAME   �û���

PLEN    PASSWD�ֶεĳ���

PASSWD  ���ע�������Ĵ����

�������֤������Ӧ��������:

+----+--------+
|VER | STATUS |
+----+--------+
| 1  |   1    |
+----+--------+

VER     ��Э�̵ĵ�ǰ�汾��Ŀǰ��0x01

STATUS  ��ȡ����ֵ:

        0x00        �ɹ�
        0x01-0xFF   ʧ�ܣ����SOCKS Server����ر���SOCKS Client֮���TCP
                    ����
*/
