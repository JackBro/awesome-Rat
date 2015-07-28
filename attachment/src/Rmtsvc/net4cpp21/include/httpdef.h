/*******************************************************************
   *	httpdef.h
   *    DESCRIPTION:����httpЭ�����õ��ĳ������ṹ�Լ�enum�Ķ���
   *				
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2006-02-08
   *	
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_HTTPDEF_H__
#define __YY_HTTPDEF_H__


//HTTP��������
#define HTTP_SERVER_PORT	80 //Ĭ��HTTP����Ķ˿�
#define HTTPS_SERVER_PORT	443 //Ĭ��HTTP SSL����Ķ˿�
#define HTTP_MAX_RESPTIMEOUT 10 //http���ݽ�����Ӧ����ӳ�ʱ��

//HTTP���ؽ����������
#define SOCKSERR_HTTP_RESP -301 //������Ӧ����
#define SOCKSERR_HTTP_SENDREQ SOCKSERR_HTTP_RESP-1 //���뷢��http����ʧ��


//����HTTP��������
typedef enum
{
	HTTP_REQ_UNKNOWN   = 0,
	HTTP_REQ_POST      = 1,
	HTTP_REQ_GET       = 2,
	HTTP_REQ_HEAD      = 3,
	HTTP_REQ_PUT       = 4,
	HTTP_REQ_LINK      = 5,
	HTTP_REQ_DELETE    = 6,
	HTTP_REQ_UNLINK    = 7,	
	HTTP_REQ_CONNECT   = 8,
}HTTPREQ_TYPE;

//����HTTP�����content������
typedef enum
{
	HTTP_CONTENT_APPLICATION=0, //form�ύ����
	HTTP_CONTENT_TEXTXML=1, //xml����
	HTTP_CONTENT_MULTIPART=2, //
	HTTP_CONTENT_UNKNOWED=3
}HTTPREQ_CONTENT_TYPE;

//http��֤����
typedef enum HttpAuthorization
{
	HTTP_AUTHORIZATION_ANONYMOUS = 0, //��������
	HTTP_AUTHORIZATION_PLAINTEXT = 1,
}HTTPAUTH_TYPE;  

//http��Ӧ��MIME����
typedef enum 
{
MIMETYPE_HTML=0,
MIMETYPE_XML,
MIMETYPE_TEXT,
MIMETYPE_CSS,
MIMETYPE_ZIP,
MIMETYPE_WORD,
MIMETYPE_OCTET,
MIMETYPE_ICON,
MIMETYPE_BMP,
MIMETYPE_GIF,
MIMETYPE_PNG,
MIMETYPE_JPG,
MIMETYPE_AVI,
MIMETYPE_ASF,
MIMETYPE_MPEG,
MIMETYPE_PDF,
MIMETYPE_MHT,
MIMETYPE_UNKNOWED,
MIMETYPE_NONE		//sendfileʱ�����趨Content-Type,���û��Լ��趨
}MIMETYPE_ENUM;

//HTTP������Ŀ¼Ȩ�޳�������
#define HTTP_ACCESS_READ 1 //��ȡ
#define HTTP_ACCESS_WRITE 2 //д��
#define HTTP_ACCESS_EXEC 4 //��ִ��
#define HTTP_ACCESS_LIST 8 //Ŀ¼���
#define HTTP_ACCESS_VHIDE 64 //��ʾ�����ļ���Ŀ¼
#define HTTP_ACCESS_SUBDIR_INHERIT 128 //��Ŀ¼�̳н�ֹ

#define HTTP_ACCESS_ALL 0x7f
#define HTTP_ACCESS_NONE 0x0
#endif

/*
100 Continue����ʼ�������Ѿ����ܣ��ͻ�Ӧ������������������ಿ�֡� 
����101tching Protocols������������ӿͻ�������ת��������һ��Э�� 
����200 OK��һ����������GET��POST�����Ӧ���ĵ����ں��档�������SetStatus����״̬���룬ServletĬ��ʹ��202״̬���롣 
����201 Created �������Ѿ��������ĵ���Locationͷ����������URL�� 
����202 Accepted���Ѿ��������󣬵�������δ��ɡ� 
����203 Non-Authoritative Information���ĵ��Ѿ������ط��أ���һЩӦ��ͷ���ܲ���ȷ����Ϊʹ�õ����ĵ��Ŀ����� 
����204 No Content��û�����ĵ��������Ӧ�ü�����ʾԭ�����ĵ�������û����ڵ�ˢ��ҳ�棬��Servlet����ȷ���û��ĵ��㹻�£����״̬�����Ǻ����õġ� 
����205 Reset Content��û���µ����ݣ��������Ӧ������������ʾ�����ݡ�����ǿ�������������������ݡ� 
����206 Partial Content���ͻ�������һ������Rangeͷ��GET���󣬷�������������� 
����300 Multiple Choices���ͻ�������ĵ������ڶ��λ���ҵ�����Щλ���Ѿ��ڷ��ص��ĵ����г������������Ҫ�������ѡ����Ӧ����LocationӦ��ͷָ���� 
����301 Moved Permanently���ͻ�������ĵ��������ط����µ�URL��Locationͷ�и����������Ӧ���Զ��ط����µ�URL�� 
����302 Found��������301�����µ�URLӦ�ñ���Ϊ��ʱ�Ե�����������������Եġ�ע�⣬��HTTP1.0�ж�Ӧ��״̬��Ϣ�ǡ�Moved Temporatily������HttpServletResponse����Ӧ�ĳ�����SC_MOVED_TEMPORARILY��������SC_FOUND�����ָ�״̬����ʱ��������ܹ��Զ������µ�URL���������һ�������õ�״̬���롣Ϊ�ˣ�Servlet�ṩ��һ��ר�õķ�������sendRedirect��ʹ��response.sendRedirect(url)��ʹ��response.setStatus(response.SC_MOVED_TEMPORARILY)��response.setHeader("Location",url)���á�ע�����״̬������ʱ����Ժ�301�滻ʹ�á����� 
����303 See Other��������301/302����֮ͬ�����ڣ����ԭ����������POST��Locationͷָ�����ض���Ŀ���ĵ�Ӧ��ͨ��GET��ȡ�� 
����304 Not Modified���ͻ����л�����ĵ���������һ�������Ե�����һ�����ṩIf-Modified-Sinceͷ��ʾ�ͻ�ֻ���ָ�����ڸ��µ��ĵ��������������߿ͻ���ԭ��������ĵ������Լ���ʹ�á� 
����305 Use Proxy���ͻ�������ĵ�Ӧ��ͨ��Locationͷ��ָ���Ĵ����������ȡ�� 
����307 Temporary Redirect����302��Found����ͬ������������������Ӧ302Ӧ������ض��򣬼�ʹԭ����������POST����ʹ��ʵ����ֻ����POST�����Ӧ����303ʱ�����ض����������ԭ��HTTP 1.1������307���Ա������������ּ���״̬���룺������303Ӧ��ʱ����������Ը����ض����GET��POST���������307Ӧ���������ֻ�ܸ����GET������ض���ע�⣺HttpServletResponse��û��Ϊ��״̬�����ṩ��Ӧ�ĳ����� 
����400 Bad Request����������﷨���� 
����401 Unauthorized���ͻ���ͼδ����Ȩ���������뱣����ҳ�档Ӧ���л����һ��WWW-Authenticateͷ��������ݴ���ʾ�û�����/����Ի���Ȼ������д���ʵ�Authorizationͷ���ٴη������� 
����403 Forbidden����Դ�����á����������ͻ������󣬵��ܾ���������ͨ�����ڷ��������ļ���Ŀ¼��Ȩ�����õ��¡� 
����404 Not Found���޷��ҵ�ָ��λ�õ���Դ����Ҳ��һ�����õ�Ӧ��HttpServletResponseר���ṩ����Ӧ�ķ�����sendError(message)�� 
����405 Method Not Allowed�����󷽷���GET��POST��HEAD��DELETE��PUT��TRACE�ȣ���ָ������Դ�����á� 
����406 Not Acceptable��ָ������Դ�Ѿ��ҵ���������MIME���ͺͿͻ���Accpetͷ����ָ���Ĳ����ݡ� 
����407 Proxy Authentication Required��������401����ʾ�ͻ������Ⱦ����������������Ȩ�� 
����408 Request Timeout���ڷ�������ɵĵȴ�ʱ���ڣ��ͻ�һֱû�з����κ����󡣿ͻ��������Ժ��ظ�ͬһ���� 
����409 Conflict��ͨ����PUT�����йء������������Դ�ĵ�ǰ״̬���ͻ����������ܳɹ��� 
����410 Gone����������ĵ��Ѿ����ٿ��ã����ҷ�������֪��Ӧ���ض�����һ����ַ������404�Ĳ�ͬ���ڣ�����407��ʾ�ĵ����õ��뿪��ָ����λ�ã���404��ʾ����δ֪��ԭ���ĵ������á� 
����411 Length Required�����������ܴ������󣬳��ǿͻ�����һ��Content-Lengthͷ�� 
����412 Precondition Failed������ͷ��ָ����һЩǰ������ʧ�ܡ� 
����413 Request Entity Too Large��Ŀ���ĵ��Ĵ�С������������ǰԸ�⴦��Ĵ�С�������������Ϊ�Լ��ܹ��Ժ��ٴ����������Ӧ���ṩһ��Retry-Afterͷ�� 
����414 Request URI Too Long��URI̫���� 
����416 Requested Range Not Satisfiable����������������ͻ���������ָ����Rangeͷ�� 
����500 Internal Server Error�����������������ϲ����������������ɿͻ������� 
����501 Not Implemented����������֧��ʵ����������Ҫ�Ĺ��ܡ����磬�ͻ�������һ����������֧�ֵ�PUT���� 
����502 Bad Gateway����������Ϊ���ػ��ߴ���ʱ��Ϊ��������������һ�������������÷����������˷Ƿ���Ӧ�� 
����503 Service Unavailable������������ά�����߸��ع���δ��Ӧ�����磬Servlet���������ݿ����ӳ�����������·���503������������503ʱ�����ṩһ��Retry-Afterͷ�� 
����504 Gateway Timeout������Ϊ��������صķ�����ʹ�ã���ʾ���ܼ�ʱ�ش�Զ�̷��������Ӧ�� 
����505 HTTP Version Not Supported����������֧����������ָ����HTTP�汾��
*/

/*
httpЭ��ͨ�������ķ����кܶ࣬����ֻ�г���http/1.1Э�鶨���п����ķ�����get��post��head��options��put��delete��trace��connect��

get�������ڻ�ȡURI��Դ������Ϊ���õ�һ�ַ�����

post����������ָ��URI�ύ���ݣ�����������Ӧ����Ϊ���÷���Ҳ��Ϊ���á�

head������URI�������󣬽���ֻ��Ҫ�����Ӧ��Э��ͷ��

put����������URI����������URI�����ڣ���Ҫ��������˸������󴴽���Դ����URI����ʱ���������˱�������������ݣ�������ΪURI��Դ���޸ĺ�汾��

delete��������ɾ��URI��ʶ��ָ����Դ��

trace�������ڼ���������˶������ѭ��������������Ϊhttp��Ӧ���������ݱ�����ؿͻ��ˡ�

connect����ͨ��������ʹ�ô������ӡ�
*/
/* hhtp ��Ӧͷ����
HTTP/1.1 200 OK
Server: Microsoft-IIS/5.0
Date: Thu, 12 Jul 2007 07:10:25 GMT
X-Powered-By: ASP.NET
X-AspNet-Version: 1.1.4322
Cache-Control: no-cache
Pragma: no-cache
Expires: -1
Content-Type: text/html; charset=gb2312
Content-Length: 3381
*/