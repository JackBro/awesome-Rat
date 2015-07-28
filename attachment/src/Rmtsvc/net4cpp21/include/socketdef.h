/*******************************************************************
   *	socketdef.h
   *    DESCRIPTION:socket�����õ��ĳ������ṹ�Լ�enum�Ķ���
   *				
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	Last modify: 2005-09-01
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_CSOCKETDEF_H__
#define __YY_CSOCKETDEF_H__

//#define IPPROTO_IPV6

#ifdef WIN32 //windowsϵͳƽ̨
	//socket errno
	#define        ENOTSOCK        WSAEOPNOTSUPP
	#define        ECONNRESET      WSAECONNRESET
	#define        ENOTCONN        WSAENOTCONN
	#define	       EINPROGRESS	WSAEINPROGRESS
	//#define      EBADF           WSAENOTSOCK
	//#define      EPIPE            WSAESHUTDOWN
	//#define	       MSG_NOSIGNAL    0  //windows��û�д˶���  //��sysconfig.h�ж���
	#define        SOCK_M_GETERROR WSAGetLastError() //get wrong code
			
	#define socklen_t int
	#pragma comment(lib,"ws2_32") //��̬����ws2_32.dll��̬���lib��
#elif defined MAC //��ʱ��֧��
	//....
#else  //unix/linuxƽ̨
	extern "C"
	{
		//����socketͨѶ�⺯��
		#include <unistd.h>  //::close(fd) --- close socket
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <arpa/inet.h>
		#include <netdb.h>
	}//?extern "C"
	//��Sun OS5.8�±���ᱨ�Ҳ���INADDR_NONE�Ķ���(δ������Ӧͷ�ļ�)������ֹ�����
	#ifndef INADDR_NONE
		#define INADDR_NONE             ((unsigned long int) 0xffffffff)
	#endif
	
	#define closesocket close
	#define ioctlsocket ioctl	
	#define SOCK_M_GETERROR errno //get wrong code
	#define WSAEACCES     EACCES		
	#define WSADATA long
#endif

#ifndef SD_SEND
#define SD_RECEIVE 0x00		//����������������� 
#define SD_SEND 0x01		//����������������� 
#define SD_BOTH 0x02
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef SO_EXCLUSIVEADDRUSE
#define SO_EXCLUSIVEADDRUSE   (u_int)(~SO_REUSEADDR)
#endif

#define SSENDBUFFERSIZE 1460 //��ѷ��ͻ����С
							 //��SOCK_STREAM��ʽ�£�������η������ݳ���1460��ϵͳ���ֳɶ�����ݱ����ͣ�
							//�ڶԷ����ܵ��Ľ���һ����������Ӧ�ó�����Ҫ���Ӷ�֡���жϡ���Ȼ���Բ����޸�
							//ע���ķ�ʽ�ı�1460�Ĵ�С����MicrcoSoft��Ϊ1460�����Ч�ʵĲ������������޸ġ� 

#define MAXRATIOTIMEOUT 600000 //us �����������ӳ� 600ms
#define SCHECKTIMEOUT 200000 //us,select�ĳ�ʱʱ�� 200ms

#define SOCKSRESULT int
#define SOCKSERR_OK 0
#define SOCKSERR_ERROR -1 //����ϵͳ����ͨ��getErrcode��ô������
#define SOCKSERR_INVALID -2 //��Ч��socket���
#define SOCKSERR_CLOSED -3 //�Է��Ѿ��ر�������
#define SOCKSERR_HOST -4 //��Ч�������������������޷�����
#define SOCKSERR_BIND -5 //�󶨴���
#define SOCKSERR_SETOPT -6 //����setsockopt��������
#define SOCKSERR_CONN -7 //����ʧ��
#define SOCKSERR_LISTEN -8 //����ʧ��
#define SOCKSERR_SNIFF -9 //snifferʧ��
#define SOCKSERR_ZEROLEN -10 //���ͻ�������ݷ���0

#define SOCKSERR_PARAM -11 //��Ч�Ĳ���
#define SOCKSERR_BUFFER -12 //buffer�������
#define SOCKSERR_TIMEOUT -13 //��ʱ
#define SOCKSERR_EACCES	-14 //ָ���ĵ�ַ��һ���㲥��ַ����û�����ù㲥��־
#define SOCKSERR_THREAD -15 //�����߳�ִ������ʧ��
#define SOCKSERR_NOTSURPPORT -16 //��֧�ִ˹���
#define SOCKSERR_MEMORY -17 //�ڴ�������
#define SOCKSERR_SSLASSCIATE -18 //SSLЭ��ʧ��
#define SOCKSERR_PARENT -19 //��socket�رջ����

#define SOCKS_TCP_IN 1    //ָʾTCP�����ӷ���

#include <vector>
#include <string>
#include <ctime>

namespace net4cpp21
{
	typedef enum //socket���������
	{
		SOCKS_NONE,//δ��������Ч��socket���
		SOCKS_TCP,
		SOCKS_UDP,
		SOCKS_RAW //ԭʼsocket
	}SOCKETTYPE;
	typedef enum //socket���������״̬
	{
		SOCKS_ERROR, //TCP�����쳣��������
		SOCKS_CLOSED, //�ѹر�
		SOCKS_LISTEN, //TCP ����״̬
		SOCKS_CONNECTED,//�ѽ���TCP����
		SOCKS_OPENED//udp��ԭʼ�׽����Ѵ�
	}SOCKETSTATUS;
	typedef enum
	{
		SOCKS_OP_PEEK,
		SOCKS_OP_READ,
		SOCKS_OP_WRITE,
		SOCKS_OP_ROOB,//��OOB����
		SOCKS_OP_WOOB //дOOB����
	}SOCKETOPMODE; //��socket�Ĳ���ģʽ
	typedef enum //SSL��ʼ������
	{
		SSL_INIT_NONE=0, //δ��ʼ��SSL
		SSL_INIT_SERV, //�����
		SSL_INIT_CLNT  //�ͻ���
	}SSL_INIT_TYPE;
	//��������ʱ���жϺ�������,����˺������ؼ����˳��ȴ�����ʱ
	typedef bool (FUNC_BLOCK_HANDLER)(void *);

}//?namespace net4cpp21

#endif
