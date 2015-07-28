/*******************************************************************
   *	cCoder.cpp
   *    DESCRIPTION:�ַ�����빤�߼�
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	net4cpp 2.0
   *******************************************************************/
   
#include "../include/sysconfig.h"  
#include "../include/cCoder.h"
   
using namespace net4cpp21;
   
//����ʱ���ÿ�еĳ���
unsigned int cCoder::m_LineWidth = 76; //80; yyc modify. smtp��Ĭ���г���Ϊ76

//Base64�����
const char cCoder::BASE64_ENCODE_TABLE[64] = {
	 65,  66,  67,  68,  69,  70,  71,  72,  // 00 - 07 A - H
	 73,  74,  75,  76,  77,  78,  79,  80,  // 08 - 15 I - P
	 81,  82,  83,  84,  85,  86,  87,  88,  // 16 - 23 Q - X
	 89,  90,  97,  98,  99, 100, 101, 102,  // 24 - 31 Y - f
	103, 104, 105, 106, 107, 108, 109, 110,  // 32 - 39 j - n
	111, 112, 113, 114, 115, 116, 117, 118,  // 40 - 47 o - v
	119, 120, 121, 122,  48,  49,  50,  51,  // 48 - 55 w - 3
	 52,  53,  54,  55,  56,  57,  43,  47 };// 56 - 63 4 - /
	
	//Base64�����
const unsigned int cCoder::BASE64_DECODE_TABLE[256] = {
	255, 255, 255, 255, 255, 255, 255, 255, //  00 -  07
	255, 255, 255, 255, 255, 255, 255, 255, //  08 -  15
	255, 255, 255, 255, 255, 255, 255, 255, //  16 -  23
	255, 255, 255, 255, 255, 255, 255, 255, //  24 -  31
	255, 255, 255, 255, 255, 255, 255, 255, //  32 -  39
	255, 255, 255,  62, 255, 255, 255,  63, //  40 -  47
	 52,  53,  54,  55,  56,  57,  58,  59, //  48 -  55
	 60,  61, 255, 255, 255, 255, 255, 255, //  56 -  63
	255,   0,   1,   2,   3,   4,   5,   6, //  64 -  71
	  7,   8,   9,  10,  11,  12,  13,  14, //  72 -  79
	 15,  16,  17,  18,  19,  20,  21,  22, //  80 -  87
	 23,  24,  25, 255, 255, 255, 255, 255, //  88 -  95
	255,  26,  27,  28,  29,  30,  31,  32, //  96 - 103
	 33,  34,  35,  36,  37,  38,  39,  40, // 104 - 111
	 41,  42,  43,  44,  45,  46,  47,  48, // 112 - 119
	 49,  50,  51, 255, 255, 255, 255, 255, // 120 - 127
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255 };
//------------------------------------------------------------------------------
		
//Quoted�����
const unsigned char cCoder::QUOTED_ENCODE_TABLE[256] = {
	255, 255, 255, 255, 255, 255, 255, 255, // 00 -  07
	255, 255,  10, 255, 255,  13, 255, 255, // 08 - 15
	255, 255, 255, 255, 255, 255, 255, 255, // 16 - 23
	255, 255, 255, 255, 255, 255, 255, 255, // 24 - 31
	255,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60, 255,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255 };
//------------------------------------------------------------------------------
	
//��ȡBase64���볤��
//nLineWidth: ==-1�����ÿ�г��Ȳ��ޣ�==0�����ÿ�г���Ϊm_LineWidth
int cCoder::Base64EncodeSize(int iSize,unsigned int nLineWidth)
{
	if(nLineWidth==0) nLineWidth=m_LineWidth;
	int nSize, nCR;
	nSize = (iSize + 2) / 3 * 4 ;
	nCR = nSize / nLineWidth; //����س�����
	nSize+= nCR * 2;
	return nSize;
}
	
//------------------------------------------------------------------------------
	
//��ȡBase64���볤��
int cCoder::Base64DecodeSize(int iSize)
{
	return (iSize + 3) / 4 * 3;
}

//------------------------------------------------------------------------------

//��ȡUUCode���볤��
int cCoder::UUEncodeSize(int iSize)
{
	int nSize, nCR;
	nSize = (iSize + 2) / 3 * 4 ;
	nCR = nSize / m_LineWidth + 2; //����س�����
	nSize+= nCR * 3 + 2;
	return nSize;
}
			
//------------------------------------------------------------------------------

//��ȡUUCode���볤��
int cCoder::UUDecodeSize(int iSize)
{
	return (iSize + 3) / 4 * 3;
}
			
//------------------------------------------------------------------------------

//��ȡQuoted���볤��
int cCoder::QuotedEncodeSize(int iSize)
{
	int nSize = iSize * 3 + 1;
	int nCR = nSize / m_LineWidth;
	nSize+= nCR * 3;
	return nSize;
}

//------------------------------------------------------------------------------

//Base64����
//nLineWidth: ==-1�����ÿ�г��Ȳ��ޣ�==0�����ÿ�г���Ϊm_LineWidth
int cCoder::base64_encode(char *pSrc, unsigned int nSize, char *pDest,
						  unsigned int nLineWidth)
{
	if ((pSrc == NULL) || (nSize <= 0)) return 0;
	if(nLineWidth==0) nLineWidth=m_LineWidth;	
	unsigned int iB, iInMax3, Len;
	char *pInPtr, *pInLimitPtr;
	char *OutPtr ;
			
	pInPtr = pSrc;
	iInMax3 = nSize / 3 * 3;
	OutPtr = pDest;
	pInLimitPtr = pInPtr + iInMax3;
				
	while (pInPtr != pInLimitPtr)
	{
		Len = 0;
		while ((Len < nLineWidth) && (pInPtr != pInLimitPtr))
		{
			iB = (unsigned char) *pInPtr++;
			iB = iB << 8;
				
			iB = iB | (unsigned char) *pInPtr++;
			iB = iB << 8;
						
			iB = iB | (unsigned char) *pInPtr++;
						
			//��4 byte����д���������
			OutPtr[3] = BASE64_ENCODE_TABLE[iB & 0x3F];
			iB = iB >> 6;
			OutPtr[2] = BASE64_ENCODE_TABLE[iB & 0x3F];
			iB = iB >> 6;
			OutPtr[1] = BASE64_ENCODE_TABLE[iB & 0x3F];
			iB = iB >> 6;
			OutPtr[0] = BASE64_ENCODE_TABLE[iB];
			OutPtr+=4;
			Len+=4;
		}
		if (Len >= nLineWidth)
		{
			*OutPtr++ = '\r'; //���ϻس����з�
			*OutPtr++ = '\n';
		}
	}
	//����β��
	switch (nSize - iInMax3)
	{
		case 1:
			iB = (unsigned char) *pInPtr;
			iB = iB << 4;
			OutPtr[1] = BASE64_ENCODE_TABLE[iB & 0x3F];
			iB = iB >> 6;
			OutPtr[0] = BASE64_ENCODE_TABLE[iB];
			OutPtr[2] = '='; //��'='Ҳ����64�����ʣ�ಿ��
			OutPtr[3] = '=';
			OutPtr+=4;
			break;
		case 2:
			iB = (unsigned char) *pInPtr++;
			iB = iB << 8;
			iB = iB | (unsigned char) *pInPtr;
			iB = iB << 2;
			OutPtr[2] = BASE64_ENCODE_TABLE[iB & 0x3F];
			iB = iB >> 6;
			OutPtr[1] = BASE64_ENCODE_TABLE[iB & 0x3F];
			iB = iB >> 6;
			OutPtr[0] = BASE64_ENCODE_TABLE[iB];
			OutPtr[3] = '='; // Fill remaining byte.
			OutPtr+=4;
			break;
	}
	return (unsigned int) (OutPtr - pDest);
}
			
//------------------------------------------------------------------------------
//Base64����

int cCoder::base64_decode(char *pSrc, unsigned int nSize, char *pDest)
{
	if ((pSrc == NULL) || (pDest == NULL) || (nSize <= 0)) return 0;
			
	unsigned int lByteBuffer, lByteBufferSpace;
	unsigned int C; //��ʱ�Ķ�����
	int reallen;
	char *InPtr, *InLimitPtr;
	char *OutPtr;
				
	lByteBuffer = 0; lByteBufferSpace = 4;
				
	InPtr = pSrc;
	InLimitPtr= InPtr + nSize;
	OutPtr = pDest;
				
	while (InPtr != InLimitPtr)
	{
		C = BASE64_DECODE_TABLE[*InPtr]; // Read from InputBuffer.
		InPtr++;
		if (C == 0xFF) continue; //����255�Ƿ��ַ�
		lByteBuffer = lByteBuffer << 6 ;
		lByteBuffer = lByteBuffer | C ;
		lByteBufferSpace--;
		if (lByteBufferSpace != 0) continue; //һ�ζ���4���ֽ�
		//����д��3���ֽڵ�����
		OutPtr[2] = lByteBuffer;
		lByteBuffer = lByteBuffer >> 8;
		OutPtr[1] = lByteBuffer;
		lByteBuffer = lByteBuffer >> 8;
		OutPtr[0] = lByteBuffer;
		//׼��д���3λ
		OutPtr+= 3; lByteBuffer = 0; lByteBufferSpace = 4;
	}
	reallen = (unsigned int)OutPtr - (unsigned int)pDest;
	//����β�� ����ʵ�ʳ���
	switch (lByteBufferSpace)
	{
		case 1:
			lByteBuffer = lByteBuffer >> 2;
			OutPtr[1] = lByteBuffer;
			lByteBuffer = lByteBuffer >> 8;
			OutPtr[0] = lByteBuffer;
			return reallen + 2;
		case 2:
			lByteBuffer = lByteBuffer >> 4;
			OutPtr[0] = lByteBuffer;
			return reallen + 1;
		default:
			return reallen;
	}
}

//------------------------------------------------------------------------------

//UUCode����
int cCoder::UU_encode(char *pSrc, unsigned int nSize, char *pDest)
{
	if ((pSrc == NULL) || (pDest == NULL) || (nSize <= 0)) return 0;
				
	unsigned int lByteBufferSpace, Len;
	unsigned char B[3]; //��ʱ�Ķ�����
	char *InPtr, *InLimitPtr;
	char *OutPtr;
			
	InPtr = pSrc;
	InLimitPtr= InPtr + nSize;
	OutPtr = pDest;
				
	while (InPtr < InLimitPtr)
	{
		Len = 0;
					
		while ((InPtr < InLimitPtr) && (Len < m_LineWidth))
		{
			lByteBufferSpace = (unsigned int) InLimitPtr - (unsigned int) InPtr;
			if (lByteBufferSpace > 3) lByteBufferSpace = 3; //���ò���
			//ȡֵ
			for (unsigned int i = 0; i < lByteBufferSpace; i++ )
			{
				B[i] = *InPtr++;
			}
			if (Len == 0)
			{
				*OutPtr++ = 'M';
				Len++;
			}
			//��������
			OutPtr[0] = B[0] >> 2;
			OutPtr[1] = (unsigned int) (B[0] << 4 & 0x30) + (unsigned int) (B[1] >> 4 & 0x0F);
			OutPtr[2] = (unsigned int) (B[1] << 2 & 0x3C) + (unsigned int) (B[2] >> 6 & 0x03);
			OutPtr[3] = B[2] & 0x3F;
			for (i = 0; i < 4; i++)
			{
				if (OutPtr[i] == NULL)
					OutPtr[i] = '`';//(unsigned char) (OutPtr[i] + 0x60);
				else 
					OutPtr[i] = ' ' + OutPtr[i];//(unsigned char) (OutPtr[i] + 0x20);
			}	
			OutPtr+=4;
			Len+=4;
		}
					
		*OutPtr++ = '\r'; //���ûس�
		*OutPtr++ = '\n';
	}
	return (unsigned int) (OutPtr - pDest);
}

//------------------------------------------------------------------------------

//UUCode����
int cCoder::UU_decode(char *pSrc, unsigned int nSize, char *pDest)
{
	char C[4]; //��ʱ�Ķ�����
	char Tmp;
	int CurrIndex, Index;
	char *InPtr, *InLimitPtr;
	char *OutPtr;
		
	if ((pSrc == NULL) || (pDest == NULL) || (nSize <= 0)) return 0;
		
	CurrIndex = 0;
				
	InPtr = pSrc;
	InLimitPtr= InPtr + nSize;
	OutPtr = pDest;
				
	while (InPtr != InLimitPtr) //��ȡ4���ַ�
	{
		memset(C, 0, sizeof(C));
		Index = 0;
		do
		{
			Tmp = *InPtr++;
			if (Tmp == 0x60) 
			{
				Tmp = 0x20;       //Ϊ�˼���OutLook Express
			}
			else if (Tmp =='\r')  //�׸���ĸ������
			{
				InPtr++;
				CurrIndex = 0;
			}
			
			if ((Tmp > 0x20) || ( CurrIndex > 0 ))
			{
				Tmp = Tmp - 0x20;
				if (CurrIndex == 0) 
				{
					CurrIndex = Tmp;
				}
				else
				{
					C[Index] = Tmp;  // ��������׷���ַ�
					Index++;
				}
			}
		}
		while ((InPtr < InLimitPtr) && (Index < sizeof(C)));
					
		OutPtr[0] = (char) ((C[0] << 2) + (C[1] >> 4));
		OutPtr[1] = (char) ((C[1] << 4) + (C[2] >> 2));
		OutPtr[2] = (char) ((C[2] << 6) + C[3]);
		OutPtr+=3; //������ʼλ��
	}
	return (unsigned int)OutPtr - (unsigned int)pDest;
}

//------------------------------------------------------------------------------

//Quoted����
int cCoder::quoted_encode(char *pSrc, unsigned int nSize, char *pDest)
{
	unsigned int Len;
	unsigned char B; //��ʱ�Ķ�����
	char *InPtr, *InLimitPtr;
	char *OutPtr;
			
	if ((pSrc == NULL) || (pDest == NULL) || (nSize <= 0)) return 0;
			
	InPtr = pSrc;
	InLimitPtr= InPtr + nSize;
	OutPtr = pDest;
				
	while (InPtr < InLimitPtr)
	{
		Len = 0;
		while ((InPtr < InLimitPtr) && (Len < m_LineWidth))
		{
			B = *InPtr++;
			if (QUOTED_ENCODE_TABLE[ B ]  == 255)
			{
				*OutPtr++ = '=';
				*OutPtr++ = DecToHex(B >> 4);
				*OutPtr++ = DecToHex(B & 0x0f);
				Len+=3;
			}
			else
			{
				*OutPtr++ = B;
				Len++;
			}
		}
		if (Len >= m_LineWidth)
		{
			*OutPtr++ = '=';
			*OutPtr++ = '\r'; //��������
			*OutPtr++ = '\n';
		}
	}
	*OutPtr = '\0';
	return (unsigned int) (OutPtr - pDest);
}

//------------------------------------------------------------------------------

//Quoted����
int cCoder::quoted_decode(char *pSrc, unsigned int nSize, char *pDest)
{
	if ((pSrc == NULL) || (pDest == NULL) || (nSize <= 0)) return 0;
				
	unsigned char nA, nB;
	char C[2]; //��ʱ�Ķ�����
				
	char *InLimitPtr= pSrc + nSize;
	char *pDestOrg = pDest;
				
	while (pSrc < InLimitPtr)
	{
		C[0] = *pSrc++; //��ȡ��һ�ַ�
		if (C[0] == '=') //��������Ǿ��������
		{
			C[0] = *pSrc++;  //ȡ�����������ַ�
			C[1] = *pSrc++;

			if (C[0] != '\r')
			{
				nA = HexToDec(C[0]); //����任
				nB = HexToDec(C[1]);
				*pDest++ = (nA << 4) + nB;
			}
		}
		else //����ֱ�����
		{
			*pDest++ = C[0];
		}
	}
	return (unsigned int)(pDest - pDestOrg);
}
//url����� ��&amp; &lt; &gt; &quot; &mdash; �����Ϊ & < > " -
int cCoder::url_decode(const char *pSrc,int nSize,char *pDest)
{
	if(nSize<=0) nSize=(pSrc)?strlen(pSrc):0;
	if(pSrc==NULL || nSize<=0) return 0;
	char *pOld=pDest;
	while(nSize>0){
		if(*pSrc=='&'){
			if(nSize>=5 && strncmp(pSrc,"&amp;",5)==0)
			{	*pDest++='&'; pSrc+=5; nSize-=5; }
			else if(nSize>=4 && strncmp(pSrc,"&lt;",4)==0)
			{	*pDest++='<'; pSrc+=4; nSize-=4; }
			else if(nSize>=4 && strncmp(pSrc,"&gt;",4)==0)
			{	*pDest++='>'; pSrc+=4; nSize-=4; }
			else if(nSize>=6 && strncmp(pSrc,"&quot;",6)==0)
			{	*pDest++='"'; pSrc+=6; nSize-=6; }
			else if(nSize>=7 && strncmp(pSrc,"&mdash;",7)==0)
			{	*pDest++='-'; pSrc+=7; nSize-=7; }
			else{ *pDest++=*pSrc++; nSize--; }
		}else{ *pDest++=*pSrc++; nSize--; }
	}//?while
	*pDest='\0'; return pDest-pOld;
}
int cCoder::url_encode(const char *pSrc,int nSize,char *pDest)
{
	if(nSize<=0) nSize=(pSrc)?strlen(pSrc):0;
	if(pSrc==NULL || nSize<=0) return 0;
	int nPos=0;
	while(nSize>0){
		if(*pSrc=='&'){
			pDest[nPos++]='&';
			pDest[nPos++]='a';
			pDest[nPos++]='m';
			pDest[nPos++]='p';
			pDest[nPos++]=';';
		}else if(*pSrc=='<'){
			pDest[nPos++]='&';
			pDest[nPos++]='l';
			pDest[nPos++]='t';
			pDest[nPos++]=';';
		}else if(*pSrc=='>'){
			pDest[nPos++]='&';
			pDest[nPos++]='g';
			pDest[nPos++]='t';
			pDest[nPos++]=';';
		}else pDest[nPos++]=*pSrc;
		pSrc++; nSize--;
	}//?while
	pDest[nPos]=0; return nPos;
}
//MIME����
int cCoder::mime_encode(const char *pSrc,unsigned int nSize,char *pDest)
{
	int pos=0;
	for(unsigned int i=0;i<nSize;i++)
	{
		if(pSrc[i]=='@' || pSrc[i]==':' || pSrc[i]=='/' || 
		   pSrc[i]=='<' || pSrc[i]=='=' || pSrc[i]=='\"' || 
		   pSrc[i]=='>' || pSrc[i]=='+' || pSrc[i]==' ')
		{
			//sprintf(pDest+pos,"%%%02X",pSrc[i]); pos+=3;
			pDest[pos++]='%';
			pDest[pos++]=cCoder::DecToHex( ((unsigned char)pSrc[i])/16 );
			pDest[pos++]=cCoder::DecToHex( ((unsigned char)pSrc[i])%16 );
		}
		else
			pDest[pos++]=pSrc[i];
	}//?for(int i=0;...
	pDest[pos]=0;
	return pos;
}
//MIME����
int cCoder::mime_encodeEx(const char *pSrc,unsigned int nSize,char *pDest)
{
	int pos=0;
	for(unsigned int i=0;i<nSize;i++)
	{
		if(pSrc[i]<0 || pSrc[i]=='@' || pSrc[i]==':' || pSrc[i]=='/' || 
		   pSrc[i]=='<' || pSrc[i]=='=' || pSrc[i]=='\"' || 
		   pSrc[i]=='>' || pSrc[i]=='+' || pSrc[i]==' ' )
		{
			//sprintf(pDest+pos,"%%%02X",pSrc[i]); pos+=3;
			pDest[pos++]='%';
			pDest[pos++]=cCoder::DecToHex( ((unsigned char)pSrc[i])/16 );
			pDest[pos++]=cCoder::DecToHex( ((unsigned char)pSrc[i])%16 );
		}
		else
			pDest[pos++]=pSrc[i];
	}//?for(int i=0;...
	pDest[pos]=0;
	return pos;
}
//MIME����URL ��mime_encodeEx��Ƚ���������/
int cCoder::mime_encodeURL(const char *pSrc,unsigned int nSize,char *pDest)
{
	int pos=0;
	for(unsigned int i=0;i<nSize;i++)
	{
		if(pSrc[i]<0 || pSrc[i]=='@' || pSrc[i]==':' ||  
		   pSrc[i]=='<' || pSrc[i]=='=' || pSrc[i]=='\"' || 
		   pSrc[i]=='>' || pSrc[i]=='+' || pSrc[i]==' ' )
		{
			//sprintf(pDest+pos,"%%%02X",pSrc[i]); pos+=3;
			pDest[pos++]='%';
			pDest[pos++]=cCoder::DecToHex( ((unsigned char)pSrc[i])/16 );
			pDest[pos++]=cCoder::DecToHex( ((unsigned char)pSrc[i])%16 );
		}
		else
			pDest[pos++]=pSrc[i];
	}//?for(int i=0;...
	pDest[pos]=0;
	return pos;
}
//MIME����
int cCoder::mime_decode(const char *pSrc,unsigned int nSize,char *pDest)
{
	unsigned int i=0;
	int pos=0;
	while(nSize>1 && i<(nSize-2))
	{
		if(pSrc[i]=='%')
		{
			unsigned char c=HexToDec(pSrc[i+1])*16+HexToDec(pSrc[i+2]);
			pDest[pos++]=c;
			i+=3;
		}
		else
			pDest[pos++]=pSrc[i++];
	}//?while(i<nSize)
	while(i<nSize) pDest[pos++]=pSrc[i++];
	pDest[pos]=0;
	return pos;
}

//utf8����
int cCoder::utf8_encodeW(const unsigned short *buf,unsigned int nSize,char *pDest)
{
	unsigned int i=0;
	int pos=0;
	if(nSize==0) nSize=stringlenW(buf);
	for(i=0;i<nSize;i++)
	{
		if(buf[i]<=0x7f)
		{
			pDest[pos++]=(unsigned char)buf[i];
		}
		else if(buf[i]>=0x80 && buf[i]<=0x7ff)
		{
			pDest[pos++]=(((unsigned char)(buf[i]>>6)) | 0xe0) & 0xdf;
			pDest[pos++]=(((unsigned char)buf[i]) | 0xc0) & 0xbf;
		}
		else if(buf[i]>=0x800 && buf[i]<=0xffff)
		{
			pDest[pos++]=(((unsigned char)(buf[i]>>12)) | 0xf0) & 0xef;
			pDest[pos++]=(((unsigned char)(buf[i]>>6)) | 0xc0) & 0xbf;
			pDest[pos++]=(((unsigned char)buf[i]) | 0xc0) & 0xbf;
		}
	}
	pDest[pos]=0; return pos;
}

int cCoder::utf8_encode(const char *pSrc,unsigned int nSize,char *pDest)
{
	unsigned int i=0;
	//�ж��Ƿ���Ҫ����
	for(i=0;i<nSize;i++)
	{
		if(((unsigned char)pSrc[i])>=0x80) break;
	}
	if(i==nSize){ strncpy(pDest,pSrc,nSize); pDest[nSize]=0; return nSize;} //�������
	//�����ֽڱ���ת��Ϊunicode˫�ֽڱ���
	unsigned short *buf=(unsigned short *)::malloc(nSize*sizeof(unsigned short));
	if(buf==NULL) return 0;
#ifdef WIN32
	nSize=MultiByteToWideChar(CP_ACP,0,pSrc,nSize,buf,nSize);
#else
	return 0;
#endif

	int pos=utf8_encodeW(buf,nSize,pDest);
	::free(buf); return pos;
}

//utf8����
int cCoder::utf8_decodeW(const char *pSrc,unsigned int nSize,unsigned short *pDest)
{
	unsigned short *buf=pDest;
	int pos=0; unsigned long lc=0;
	if(nSize==0) nSize=strlen(pSrc);
	for(unsigned int i=0;i<nSize;i++)
	{
		unsigned char c=pSrc[i];
		if((c & 0x80)==0)
			lc=c;
		else if( (c & 0xe0)==0xc0 )
		{
			if((nSize-i)<=1) 
				return 0; //��������
			lc=(((unsigned long)c & 0x1f)<<6) + ((unsigned long)pSrc[i+1] & 0x3f);
			i+=1;
		}
		else if( (c & 0xf0)==0xe0 )
		{
			if((nSize-i)<=2) 
				return 0; //��������

			lc=(((unsigned long)c & 0x0f)<<12) + (((unsigned long)pSrc[i+1] & 0x3f)<<6) +  
				(((unsigned long)pSrc[i+2] & 0x3f));
			i+=2;
		}
		else if( (c & 0xf8)==0xf0 )
		{
			if((nSize-i)<=3) 
				return 0; //��������
			lc=(((unsigned long)c & 0x07)<<18) + (((unsigned long)pSrc[i+1] & 0x3f)<<12) + 
				(((unsigned long)pSrc[i+2] & 0x3f)<<6) + ((unsigned long)pSrc[i+3] & 0x3f);
			i+=3;
		}
		else if( (c & 0xfc)==0xf8 )
		{
			if((nSize-i)<=4) 
				return 0; //��������
			lc=(((unsigned long)c & 0x03)<<24) + (((unsigned long)pSrc[i+1] & 0x3f)<<18) + 
				(((unsigned long)pSrc[i+2] & 0x3f)<<12) + (((unsigned long)pSrc[i+3] & 0x3f)<<6) + 
				(((unsigned long)pSrc[i+4] & 0x3f));
			i+=4;
		}
		else if( (c & 0xfe)==0xfc )
		{
			if((nSize-i)<=5) 
				return 0; //��������
			lc=(((unsigned long)c & 0x01)<<30) + (((unsigned long)pSrc[i+1] & 0x3f)<<24) + 
				(((unsigned long)pSrc[i+2] & 0x3f)<<18) + (((unsigned long)pSrc[i+3] & 0x3f)<<12) + 
				(((unsigned long)pSrc[i+4] & 0x3f)<<6) +((unsigned long)pSrc[i+5] & 0x3f);
			i+=5;
		}
		else lc=c; //yyc modify return 0; //��������
		
		buf[pos++]=(unsigned short)lc;
	}//?for(int i=0;...
	buf[pos]=0;
	return pos;
}

int cCoder::utf8_decode(const char *pSrc,unsigned int nSize,char *pDest)
{
	unsigned int i=0;
	//�ж��Ƿ���Ҫ����
	for(i=0;i<nSize;i++)
	{
		if(((unsigned char)pSrc[i])>=0x80) break;
	}
	if(i==nSize){ if(pSrc!=pDest) strncpy(pDest,pSrc,nSize); pDest[nSize]=0; return nSize;} //�������
	unsigned short *buf=(unsigned short *)::malloc((nSize+1)*sizeof(unsigned short));
	if(buf==NULL) return 0;
	int len=utf8_decodeW(pSrc,nSize,buf);
	if(len>0)
	{//��unicode����ת��Ϊ���ֽڱ����ַ���
#ifdef WIN32
		len=WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK|WC_DISCARDNS|WC_SEPCHARS|WC_DEFAULTCHAR,buf,-1,(char *)pDest,nSize,NULL,NULL);
		if(len>0) len--;//WideCharToMultiByte���صó��Ȱ�����β��null
#else
		len=0;
#endif
	}
	::free(buf); return len;
}
//=?charset?encoding-type?data?= ���ݸ�ʽ����
//charsetָ��data���ݵ��ַ���������utf-8
//encoding-type: B��Q�� base64��Quoted-Printble
int cCoder::eml_decode(const char *pSrc,unsigned int nSize,char *pDest)
{
	if(pSrc[0]=='=' && pSrc[1]=='?' && pSrc[nSize-1]=='=' && pSrc[nSize-2]=='?')
	{//�ж��Ƿ���Ҫ����
		const char *ptr=strchr(pSrc+2,'?');
		char cType=(ptr)?(*(ptr+1)):0;
		if(cType=='B' || cType=='Q'){//��ʶ��ı���
			char *buf=(char *)::malloc(nSize*sizeof(char));
			if(buf==NULL) return 0;
			::memcpy(buf,pSrc,nSize); buf[nSize-2]=0; //ȥ������?
			int npos=(ptr-pSrc); buf[npos]=0;
			char *lpCharset=buf+2;
			char *lpData=buf+npos+3;
			int iDataLen=strlen(lpData);
			//��ʼ����cType���н���
			if(cType=='B') //base64����
				 iDataLen=cCoder::base64_decode(lpData,iDataLen,lpData);
			else iDataLen=cCoder::quoted_decode(lpData,iDataLen,lpData);
			//��ʼ�ַ�����
			if(strcasecmp(lpCharset,"utf-8")==0)
				iDataLen=cCoder::utf8_decode(lpData,iDataLen,lpData);
			strncpy(pDest,lpData,iDataLen); pDest[iDataLen]=0; 
			::free(buf); return iDataLen;
		}//?if(cType=='B' || cType=='Q')
	}//?if(pSrc[0]=='=' && pSrc[1]=='?'
	//��������Ч��֧�ֵı����ʽ���������
	if(pSrc!=pDest) strncpy(pDest,pSrc,nSize); pDest[nSize]=0; return nSize;
}
//���ǽ���utf-8��base64����
int cCoder::eml_encode(const char *pSrc,unsigned int nSize,char *pDest)
{
	strcpy(pDest,"=?utf-8?B?"); int npos=10;
	nSize=cCoder::utf8_encode(pSrc,nSize,(pDest+npos));
	char *buf=(char *)::malloc(nSize+1);
	if(buf==NULL) return 0;
	::memcpy(buf,pDest+npos,nSize); buf[nSize]=0;
	npos+=cCoder::base64_encode(buf,nSize,(pDest+npos));
	pDest[npos++]='?'; pDest[npos++]='='; pDest[npos]=0;
	::free(buf); return npos;
}
int cCoder::EmlEncodeSize(int iSize)
{
	iSize=cCoder::Utf8EncodeSize(iSize);
	iSize=cCoder::Base64EncodeSize(iSize);
	return iSize+12;
}

//ʮ����-->ʮ������
inline unsigned char cCoder::DecToHex( unsigned char B)
{
	return B < 10 ? '0' + B:'A' + B - 10;
}

//------------------------------------------------------------------------------

//ʮ������-->ʮ����
inline unsigned char cCoder::HexToDec( unsigned char C )
{
	if(C<='9') return C-'0';
	return C>='a' ? (C-'a'+10) : (C-'A'+10);
//	return C <= '9' ? C - '0' : C - 'A' + 10; //yyc remove 2007-12-20
}

inline long power(long x,long z)
{
	long lret=1;
	while(--z>=0) lret *=x;
	return lret;
}
//��16�����ַ���(��д)תΪ��ֵ
unsigned long cCoder::hex_atol(const char *str)
{
	//����ȥ��ǰ���ո�
	while(*str==' ') str++;
	//�ж���Ч�ַ�����
	const char *ptr=str; int len=0;
	while( (*ptr>='0' && *ptr<='9') || (*ptr>='A' && *ptr<='F') ){ ptr++; if(++len>=8) break; }
	unsigned long ul=0; unsigned char c;
	while(len-->0){ 
		if( (c=cCoder::HexToDec(*str++))!=0 )
			ul+=c * power(16,len);
	}
	return ul;
}

/*UTF8�����ԭ�� 
��Ϊһ����ĸ����һЩ�����ϵķ��ż�����ֻ�ö�������λ�Ϳ��Ա�ʾ��������һ���ֽھ��ǰ�λ������UTF8����һ���ֽ�����ʽ��ĸ��һЩ�����ϵķ��š�Ȼ���������õ���������һ���ֽں���ô֪��������ɣ����п�����Ӣ����ĸ��һ���ֽڣ�Ҳ�п����Ǻ��ֵ������ֽ��е�һ���ֽڣ����ԣ�UTF8���б�־λ�ģ� 

��Ҫ��ʾ��������7λ��ʱ�����һ���ֽڣ�0******* ��һ��0Ϊ��־λ��ʣ�µĿռ����ÿ��Ա�ʾASCII 0��127 �����ݡ� 

��Ҫ��ʾ��������8��11λ��ʱ����������ֽڣ�110***** 10****** ��һ���ֽڵ�110�͵ڶ����ֽڵ�10Ϊ��־λ�� 

��Ҫ��ʾ��������12��16λ��ʱ����������ֽڣ�1110***** 10****** 10****** ������һ������һ���ֽڵ�1110�͵ڶ��������ֽڵ�10���Ǳ�־λ��ʣ�µĿռ����ÿ��Ա�ʾ���֡� 

�Դ����ƣ� 
�ĸ��ֽڣ�11110**** 10****** 10****** 10****** 
����ֽڣ�111110*** 10****** 10****** 10****** 10****** 
�����ֽڣ�1111110** 10****** 10****** 10****** 10****** 10****** 
*/