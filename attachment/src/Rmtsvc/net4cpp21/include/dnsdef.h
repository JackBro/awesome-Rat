/*******************************************************************
   *	dnsdef.h
   *    DESCRIPTION:����dnsЭ�����õ��ĳ������ṹ�Լ�enum�Ķ���
   *				
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-02
   *	
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_DNSPDEF_H__
#define __YY_DNSPDEF_H__

#define SOCKSERR_DNS_SERVER -101 //��Ч��IP��ַ
#define SOCKSERR_DNS_IP SOCKSERR_DNS_SERVER-1 //��Ч��dns�����ַ
#define SOCKSERR_DNS_NAMES SOCKSERR_DNS_SERVER-2 //��Ч��Ҫ����������

#define DNS_SERVER_PORT	53 //Ĭ��DNS����Ķ˿�
#define DNS_MAX_PACKAGE_SIZE 512 //һ��DNS��ѯ����С

//----------------�˴�����������LITTER_BYTE�����ϵͳDNS_HEADER.flagsλ����ĺ���-----
#define DNS_FLAGS_QR 0x80 //�����һ�ֽڵĵ�7λ
		//0�����ѯ��1����DNS�ظ�
#define DNS_FLAGS_OPCODE 0x78 //�����һ�ֽڵĵ�6,5,4,3λ
		//ָʾ��ѯ���ࣺ0:��׼��ѯ��1:�����ѯ��2:������״̬��ѯ��3-15:δʹ�á�
#define DNS_FLAGS_AA 0x04 //�����һ�ֽڵĵ�2λ
		//�Ƿ�Ȩ���ظ�
#define DNS_FLAGS_TC 0x02 //�����һ�ֽڵĵ�1λ
		//��Ϊһ��UDP����Ϊ512�ֽڣ����Ը�λָʾ�Ƿ�ص������Ĳ���
#define DNS_FLAGS_RD 0x01 //�����һ�ֽڵĵ�0λ
		//��λ�ڲ�ѯ��ָ�����ظ�ʱ��ͬ������Ϊ1ָʾ���������еݹ��ѯ
#define DNS_FLAGS_RA 0x8000 //����ڶ��ֽڵĵ�7λ
		//��DNS�ظ�����ָ����˵��DNS�������Ƿ�֧�ֵݹ��ѯ
#define DNS_FLAGS_Z 0x7000 //����ڶ��ֽڵĵ�6,5,4λ
		//�����ֶΣ���������Ϊ0
#define DNS_FLAGS_RCODE 0x0f00 //����ڶ��ֽڵĵ�3,2,1,0λ
		//�ɻظ�ʱָ���ķ����룺0:�޲��1:��ʽ��2:DNS����3:���������ڣ�4:DNS��֧�������ѯ��5:DNS�ܾ���ѯ��6-15:�����ֶ�

#define DNS_OPCODE_QUERY 0x0 //��׼��ѯ
#define DNS_OPCODE_IQUERY 0x08 //�����ѯ
#define DNS_OPCODE_STATUS 0x10 //״̬��ѯ

#define DNS_RCODE_ERR_OK 0x0 //�޲��
#define DNS_RCODE_ERR_FORMAT 0x0100 //��ʽ��
#define DNS_RCODE_ERR_DNS 0x0200 //DNS����
#define DNS_RCODE_ERR_EXIST 0x0300 //����������
#define DNS_RCODE_ERR_SURPPORT 0x0400 //DNS��֧�������ѯ
#define DNS_RCODE_ERR_REJECT 0x0500 //DNS�ܾ���ѯ

//--------------�˴�����������BIG_BYTE�����ϵͳDNS_HEADER.flagsλ����ĺ���--------
//#define DNS_FLAGS_QR 0x8000 //�����һ�ֽڵĵ�7λ
//#define DNS_FLAGS_OPCODE 0x7800 //�����һ�ֽڵĵ�6,5,4,3λ
//#define DNS_FLAGS_AA 0x0400 //�����һ�ֽڵĵ�2λ
//#define DNS_FLAGS_TC 0x0200 //�����һ�ֽڵĵ�1λ
//#define DNS_FLAGS_RD 0x0100 //�����һ�ֽڵĵ�0λ
//#define DNS_FLAGS_RA 0x0080 //����ڶ��ֽڵĵ�7λ
//#define DNS_FLAGS_Z 0x0070 //����ڶ��ֽڵĵ�6,5,4λ
//#define DNS_FLAGS_RCODE 0x000f //����ڶ��ֽڵĵ�3,2,1,0λ

//#define DNS_OPCODE_QUERY 0x0 //��׼��ѯ
//#define DNS_OPCODE_IQUERY 0x0800 //�����ѯ
//#define DNS_OPCODE_STATUS 0x1000 //״̬��ѯ

//#define DNS_RCODE_ERR_OK 0x0 //�޲��
//#define DNS_RCODE_ERR_FORMAT 0x01 //��ʽ��
//#define DNS_RCODE_ERR_DNS 0x02 //DNS����
//#define DNS_RCODE_ERR_EXIST 0x03 //����������
//#define DNS_RCODE_ERR_SURPPORT 0x04 //DNS��֧�������ѯ
//#define DNS_RCODE_ERR_REJECT 0x05 //DNS�ܾ���ѯ
//-----------------------------------------------------------------------
/* ��ѯ���� Q_type
 * Type values for resources and queries
 */
#define T_A			1		/* host address ָ������� IP ��ַ*/
#define T_NS		2		/* authoritative server ָ��������������� DNS ���Ʒ�����*/
#define T_MD		3		/* mail destination ָ���ʼ�����վ���������Ѿ���ʱ�ˣ�ʹ��MX���棩*/
#define T_MF		4		/* mail forwarder ָ���ʼ���תվ���������Ѿ���ʱ�ˣ�ʹ��MX���棩*/
#define T_CNAME		5		/* canonical name ָ�����ڱ����Ĺ淶����*/
#define T_SOA		6		/* start of authority zone ָ������ DNS ����ġ���ʼ��Ȩ������*/
#define T_MB		7		/* mailbox domain name ָ����������*/
#define T_MG		8		/* mail group member ָ���ʼ����Ա*/
#define T_MR		9		/* mail rename name ָ���ʼ�����������*/
#define T_NULL		10		/* null resource record ָ���յ���Դ��¼*/
#define T_WKS		11		/* well known service ������֪����*/
#define T_PTR		12		/* domain name pointer �����ѯ�� IP ��ַ����ָ���������������ָ��ָ��������Ϣ��ָ��*/
#define T_HINFO		13		/* host information ָ������� CPU �Լ�����ϵͳ����*/
#define T_MINFO		14		/* mailbox information ָ��������ʼ��б���Ϣ*/
#define T_MX		15		/* mail routing information ָ���ʼ�������*/
#define T_TXT		16		/* text strings ָ���ı���Ϣ*/
#define T_RP		17		/* responsible person */
#define T_AFSDB		18		/* AFS cell database */
#define T_X25		19		/* X_25 calling address */
#define T_ISDN		20		/* ISDN calling address */
#define T_RT		21		/* router */
#define T_NSAP		22		/* NSAP address */
#define T_NSAP_PTR	23		/* reverse NSAP lookup (deprecated) */
#define T_SIG		24		/* security signature */
#define T_KEY		25		/* security key */
#define T_PX		26		/* X.400 mail mapping */
#define T_GPOS		27		/* geographical position (withdrawn) */
#define T_AAAA		28		/* IP6 Address */
#define T_LOC		29		/* Location Information */
#define T_NXT		30		/* Next Valid Name in Zone */
#define T_EID		31		/* Endpoint identifier */
#define T_NIMLOC	32		/* Nimrod locator */
#define T_SRV		33		/* Server selection */
#define T_ATMA		34		/* ATM Address */
#define T_NAPTR		35		/* Naming Authority PoinTeR */
	/* non standard */
#define T_UINFO		100		/* user (finger) information ָ���û���Ϣ*/
#define T_UID		101		/* user ID ָ���û���ʶ��*/
#define T_GID		102		/* group ID ָ�����������ʶ��*/
#define T_UNSPEC	103		/* Unspecified format (binary data) */
	/* Query type values which do not appear in resource records */
#define	T_IXFR		251		/* incremental zone transfer */
#define T_AXFR		252		/* transfer zone of authority */
#define T_MAILB		253		/* transfer mailbox records */
#define T_MAILA		254		/* transfer mail agent records */
#define T_ANY		255		/* wildcard match ָ��������������*/
/* ��ѯclass Q_class
 * Values for class field
 */
#define C_IN		1		/* the arpa internet ָ�� Internet ���*/
#define C_CSNET		2		/* ָ�� CSNET ��𡣣��ѹ�ʱ��*/
#define C_CHAOS		3		/* for chaos net (MIT) ָ�� Chaos ���*/
#define C_HS		4		/* for Hesiod name server (MIT) (XXX) ָ�� MIT Athena Hesiod ���*/
	/* Query class values which do not appear in resource records ָ���κ���ǰ�г���ͨ���*/
#define C_ANY		255		/* wildcard match */

//Q_type�е�T_A,T_MX,T_CNAMEΪ���ã�Q_class�е�C_INΪ����

typedef struct dns_protocol_header //DNS���ݱ�ͷ
{
	unsigned short id;
	//��ʶ��ͨ�����ͻ��˿��Խ�DNS��������Ӧ����ƥ��
	unsigned short flags;
	//��־��[QR | opcode | AA| TC| RD| RA | zero | rcode ]
	unsigned short quests;
	//������Ŀ
	unsigned short answers;
	//��Դ��¼��Ŀ
	unsigned short author;
	//��Ȩ��Դ��¼��Ŀ
	unsigned short addition;
	//������Դ��¼��Ŀ
}DNS_HEADER,*PDNS_HEADER;
typedef struct dns_protocol_query //DNS��ѯ���ݱ�
{
	const char *name;
	//��ѯ������,����һ����С��0��63֮����ַ���
	unsigned short type;
	//��ѯ���ͣ���Q_type_array����
	unsigned short classes;
	//��ѯ��,��Q_class_array ͨ����A��Ȳ�ѯIP��ַ

}DNS_QUERY,*PDNS_QUERY;
typedef struct dns_protocol_response //DNS��Ӧ���ݱ�
{
	const char *name; //�ظ���ѯ��������������
	//��ѯ������
	unsigned short type; //�ظ������͡�2�ֽڣ����ѯͬ�塣ָʾRDATA�е���Դ��¼����
	//��ѯ����
	unsigned short classes; //�ظ����ࡣ2�ֽڣ����ѯͬ�塣ָʾRDATA�е���Դ��¼��
	//������
	unsigned long	ttl; //����ʱ�䡣4�ֽڣ�ָʾRDATA�е���Դ��¼�ڻ��������ʱ��
	//����ʱ��
	unsigned short length; //���ȡ�2�ֽڣ�ָʾRDATA��ĳ���(���������ֶε����ֽ�)
	//��Դ���ݳ���
	unsigned char *	rdata; //��Դ��¼�������壬��TYPE�Ĳ�ͬ���˼�¼�ĸ�ʾ��ͬ��
						  //ͨ��һ��MX��¼����һ��2�ֽڵ�ָʾ���ʼ������������ȼ�ֵ�����������ʼ�����������ɵ�
	//��Դ����
}DNS_RESPONSE,*PDNS_RESPONSE;

#endif

/*************************
���Ƶ������ʽ�������ɶ����ʶ������ɣ�ÿһ����ʶ���е����ֽ�˵���ñ�ʶ���ĳ��ȣ�
��������ASCII���ʾ�ַ����������֮�����ֽ�0��ʾ���ֽ�����
����ĳһ����ʶ���е����ַ��ĳ�������0xC0�Ļ�����ʾ��һ�ֽ�ָʾ���Ǳ�ʶ�����У�
����ָʾ���²����ڱ����հ��ڵ�ƫ��λ�ô�ʱ�����ֽ�0��ʾ���ֽ����� 
�������硡bbs.zzsy.com����.�ֿ�bbs��zzsy��com�������֡�ÿ�����ֵĳ���Ϊ3��4��3 
������DNS�����е���ʽ���� 3 b b s 4 z z s y 3 c o m 0 
���������ڰ��ڵĵ�12���ֽ�λ�ô����� 4 z z s y 3 c o m 0 �����������ˡ� 
�����Ǵ�ʱ�п���Ϊ��3 b b s 4 z z s y 0xC0 0x0C ��������ʽ��
**************************/
