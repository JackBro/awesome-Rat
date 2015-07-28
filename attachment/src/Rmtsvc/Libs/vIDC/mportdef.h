/*******************************************************************
   *	mportdef.h
   *    DESCRIPTION:����˿�ӳ�����õ��ĳ������ṹ�Լ�enum�Ķ���
   *				
   *    AUTHOR:yyc
   *	http://hi.baidu.com/yycblog/home
   *	
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_MPORTDEF_H__
#define __YY_MPORTDEF_H__

#define MAX_TRANSFER_BUFFER 4096
#define MAX_CONNECT_TIMEOUT 15
#define MAX_RECVREQ_TIMEOUT 10
 
typedef enum //�˿�ӳ�����Ͷ���
{
	MPORTTYPE_UNKNOW=0,//δ֪
	MPORTTYPE_UDP,
	MPORTTYPE_TCP,
	MPORTTYPE_WWW,
	MPORTTYPE_FTP
}MPORTTYPE; 

typedef enum
{
	TCPSVR_TCPSVR=0, //�����з���ת��
	TCPSVR_SSLSVR,   //����ͨTCP����ת��ΪSSL���ܷ���
	SSLSVR_TCPSVR    //��SSL���ܷ���ת��Ϊ��ͨTCP����
}SSLTYPE;

typedef enum 
{
	VIDC_MAPTYPE_TCP=0,		//TCPӳ��
	VIDC_MAPTYPE_UDP,		//UDPӳ��
	VIDC_MAPTYPE_PROXY		//��������ӳ��
}VIDC_MAPTYPE;


#endif

