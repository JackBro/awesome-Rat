/*******************************************************************
   *	cCoder.h
   *    DESCRIPTION:�ַ�����빤�߼�
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	net4cpp 2.0
   *******************************************************************/
   
#ifndef __YY_CCODER_H__
#define __YY_CCODER_H__

namespace net4cpp21
{
	class cCoder
	{
		static unsigned char DecToHex(unsigned char B);		//ΪQuoted������������ַ�ת��
		static unsigned char HexToDec(unsigned char C);		//ΪQuoted������������ַ�ת��
		
		static const char BASE64_ENCODE_TABLE[64];		//Base64�����
		static const unsigned int BASE64_DECODE_TABLE[256];	//Base64�����
		static const unsigned char QUOTED_ENCODE_TABLE[256];	//Quoted�����
		
	public:
		static unsigned int m_LineWidth;			//ָ�������ÿ�еĳ��ȣ�ȱʡ��76
		//�����ļ�ʵ�ʳ��Ȼ�ȡ����Base64��ĳ���,-1�����ÿ�г��Ȳ��ޣ�0�����ÿ�г���Ϊm_LineWidth
		static int Base64EncodeSize(int iSize,unsigned int nLineWidth=0);  
		static int Base64DecodeSize(int iSize);		//�����ѱ����ļ����Ȼ�ȡBase64�Ľ��볤��
		static int UUEncodeSize(int iSize);			//�����ļ�ʵ�ʳ��Ȼ�ȡUUCode�����ĳ���
		static int UUDecodeSize(int iSize);			//�����ѱ����ļ����Ȼ�ȡUUCode�����ĳ���
		static int QuotedEncodeSize(int iSize);		//����ʵ���ļ��ĳ��Ȼ�ȡQuoted����
		
		
		/*
		*  ��һ��Buffer����Base64����
		*
		*	 	pSrc	����Buffer
		*		nSize	Buffer����
		*		pDest	�������
		*
		*	 ע: ���Buffer�ĳ��ȿ���ʹ�� Base64EncodeSize(int) ����ȡ��
		*/
		static int base64_encode(char *pSrc, unsigned int nSize, char *pDest,
			unsigned int nLineWidth=0); //��ָ����LineWidth���CRLF
		//==-1�����ÿ�г��Ȳ��ޣ�==0�����ÿ�г���Ϊm_LineWidth
		/*
		*  ��һ��Buffer����Base64����
		*	
		*	 	pSrc	����Buffer
		*		nSize	Buffer����
		*		pDest	�������
		*		return	������ʵ�ʳ���
		*
		*	 ע: ���Buffer�ĳ��ȿ���ʹ�� Base64DecodeSize(int) ����ȡ��
		*/
		static int  base64_decode(char *pSrc, unsigned int nSize, char *pDest);
		/*
		*  ��һ��Buffer����UUCODE����
		*
		*	 	pSrc	����Buffer
		*		nSize	Buffer����
		*		pDest	�������
		*
		*	 ע: ���Buffer�ĳ��ȿ���ʹ�� UUEncodeSize(int) ����ȡ��
		*/
		static int UU_encode(char *pSrc, unsigned int nSize, char *pDest);
		
		/*
		*  ��һ��Buffer����UUCODE����
		*
		*	 	pSrc	����Buffer
		*		nSize	Buffer����
		*		pDest	�������
		*
		*	 ע: ���Buffer�ĳ��ȿ���ʹ�� UUDecodeSize(int) ����ȡ��
		*/
		static int UU_decode(char *pSrc, unsigned int nSize, char *pDest);
		/*
		*  ��һ��Buffer����Quoted-Printble����
		*
		*	 	pSrc	����Buffer
		*		nSize	Buffer����
		*		pDest	�������
		*		return	������ʵ�ʳ���
		*
		*	 ע: ���Buffer�ĳ��ȿ���ʹ�� QuotedDecodeSize(int) ����ȡ��
		*/
		static int quoted_encode(char *pSrc, unsigned int nSize, char *pDest);
		
		/*
		*  ��һ��Buffer����Quoted-Printble����
		*
		*	 	pSrc	����Buffer
		*		nSize	Buffer����
		*		pDest	�������
		*		return	������ʵ�ʳ���
		*
		*	 ע������û���ṩ������볤�ȵķ��� ֱ��ʹ������Buffer�����Buffer�Ϳ�����
		*/
		static int quoted_decode(char *pSrc, unsigned int nSize, char *pDest);
		
		//url����� ��&amp; &lt; &gt; �����Ϊ & < >
		static int url_decode(const char *pSrc,int nSize,char *pDest);
		static int url_encode(const char *pSrc,int nSize,char *pDest);
		static int MimeEncodeSize(int iSize){ return iSize *3;}
		/*
		*  ��ָ�����ַ�������Mime����
		*
		*	 	pSrc	�����ַ���
		*		nSize	�ַ�������
		*		pDest	�������
		*		return	������ʵ�ʳ���
		*
		*	 ע���˺������Ժ��ֽ��б���
		*/
		static int mime_encode(const char *pSrc,unsigned int nSize,char *pDest);
		//����Ҳ���б���
		static int mime_encodeEx(const char *pSrc,unsigned int nSize,char *pDest);
		static int mime_encodeURL(const char *pSrc,unsigned int nSize,char *pDest);
		/*
		*  ��ָ�����ַ�������Mime����
		*
		*	 	pSrc	�����ַ���
		*		nSize	�ַ�������
		*		pDest	�������
		*		return	��������ʵ�ʳ���
		*
		*	 ע������û���ṩ������볤�ȵķ��� ֱ��ʹ������Buffer�����Buffer�Ϳ�����
		*/
		static int mime_decode(const char *pSrc,unsigned int nSize,char *pDest);
		
		//UTF-8 - ASCII ���ݵĶ��ֽ�(1~3)�ֽ� Unicode ����
		//ʵ�ʵ�utf8������ֽ�Ϊ1~6�ֽڣ�������һ��Ҳ����˫�ֽڵ��ַ�������������õ�3�ֽ�
		//��Ϊ0x00000800 - 0x0000FFFF�ַ�ת��Ϊutf8��Ϊ3�ֽ�
		static int Utf8EncodeSize(int iSize){ return iSize *3;}
		/*
		*  ��ָ����Buffer����utf8����
		*
		*	 	pSrc	����Buffer
		*		nSize	Buffer����
		*		pDest	�������
		*		return	������ʵ�ʳ���
		*
		*/
		static int utf8_encode(const char *pSrc,unsigned int nSize,char *pDest);
		static int utf8_encodeW(const unsigned short *pSrc,unsigned int nSize,char *pDest);
		/*
		*  ��ָ����Buffer����Utf8����
		*
		*	 	pSrc	�����ַ���
		*		nSize	�ַ�������
		*		pDest	�������
		*		return	��������ʵ�ʳ���
		*
		*	 ע������û���ṩ������볤�ȵķ��� ֱ��ʹ������Buffer�����Buffer�Ϳ�����
		*/
		static int utf8_decode(const char *pSrc,unsigned int nSize,char *pDest);
		static int utf8_decodeW(const char *pSrc,unsigned int nSize,unsigned short *pDest);

		//��16�����ַ���תΪ��ֵ
		static unsigned long hex_atol(const char *str);

		//=?charset?encoding-type?data?= ���ݸ�ʽ����
		static int eml_decode(const char *pSrc,unsigned int nSize,char *pDest);
		static int eml_encode(const char *pSrc,unsigned int nSize,char *pDest);
		static int EmlEncodeSize(int iSize);
	};
}//?namespace net4cpp21

#endif

