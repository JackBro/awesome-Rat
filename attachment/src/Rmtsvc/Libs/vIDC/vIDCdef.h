/*******************************************************************
   *	vidcdef.h
   *    DESCRIPTION:��������ͷ�ļ�
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-06-03
   *	
   *******************************************************************/
#ifndef __YY_VIDCDEF_H__
#define __YY_VIDCDEF_H__

#define VIDCS_VERSION 26 //vIDC ����˰汾
#define VIDCC_VERSION 26 //vIDC �ͻ��˰汾
#define VIDCC_MIN_VERSION 25 //֧�ֵ�vidcc��С�汾

#define VIDC_SERVER_PORT 8080 //Ĭ��vIDC����˿�
#define VIDC_MAX_RESPTIMEOUT 10 //s ���ȴ���ʱʱ��
#define VIDC_PIPE_ALIVETIME 20 //s �ܵ�����ʱ�䣬�����ָ����ʱ�仹û�б������ͷ�
#define VIDC_MAX_COMMAND_SIZE 256 //vIDC��������ֽڳ���
#define VIDC_MAX_CLIENT_TIMEOUT 180 //������û���յ�vIDC�ͻ��˵��������Ϣ����Ϊ�Ͽ���
#define VIDC_NOP_INTERVAL 60  //vIDCc���������ļ��s
//����VIDC�Ĵ��󷵻���Ϣ
#define SOCKSERR_VIDC_VER -301 //�汾��ƥ��
#define SOCKSERR_VIDC_PSWD -302 //���벻��ȷ
#define SOCKSERR_VIDC_RESP -303 //��ʱ����Ӧ����
#define SOCKSERR_VIDC_NAME -304 //��Ч������
#define SOCKSERR_VIDC_MEMO -305 //�ڴ����ʧ��
#define SOCKSERR_VIDC_MAP  -306 //ӳ��ʧ��
#define SOCKSERR_VIDC_SURPPORT -307 //��ʱ��֧�ִ˹���
#endif

/***********************vIDC��������******************************************************

vidcc                              vidcs
           <connect>
	------------------------------->
	       HELO ������֤
	------------------------------->
	       200 ... OK
	<-------------------------------
	       ADDR ...
	------------------------------->
		   200 ... OK
	<-------------------------------
	       <��ʼ���������>
	       .................
		   <close>
	<------------------------------->


------------------vIDC Server ��������-----------------------------------------
1����������
2������һ�����ӽ�������������
3���ȴ������������ݣ����VIDC_MAX_RESPTIMEOUT��û���յ��κ�������رմ�����
4���ж������Ƿ�ΪHELO��PIPE�����������ر�����
5�������HELO�������пͻ���������֤�������ͨ���򷵻ش��󣬹ر�����
6��������һ��session�ػ���ѭ���ȴ�����vIDC��������سɹ���Ϣ
7�������PIPE���
------------------vIDC Client ��������-----------------------------------------
*****************************************************************************************/