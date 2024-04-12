#include <reg52.h>
#include "pcf8591.h"

 #define FOSC 11059200L //�������ã�Ĭ��ʹ��11.0592M Hz
//#define FOSC 12000000L //�������ã�ʹ��12M Hz
//#define FOSC 24000000L //�������ã�ʹ��24M Hz

#define DAY_NIGHT  100.0 //�涨��������� ȡֵ0~250��250Ϊ�
#define NO_PROTUES  0  //proteus������ӻ���1Ϊ�����棻0ʱ���棬���ڷ����п������Ե�led_test(P1^7)��������������

//IO�ӿڶ���
#define LED_PORT P0
sbit wela_1 = P2^4;					   //�����


//ȫ�ֱ�������
unsigned char num;

//LED��ʾ��ģ 0-F ����ģʽ
unsigned code table[]= {0Xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};

//��������
void delay(unsigned int z); 
void delay_us(unsigned int aa); 
void Delay(unsigned int xms);
void Delayms(unsigned int ms);
/*====================================
 Ӳ���ӿ�λ����
====================================*/
sbit HW  = P3^2;     //��������������ݽӿ�	�ⲿ�ж�O�����
sbit led1 = P1^0;
sbit led2 = P1^1;
sbit led3 = P1^2;
sbit led4 = P1^3;
sbit beep = P1^6;
sbit led_test = P1^7;
unsigned char HWtime; 		//������ߵ�ƽ����ʱ�䣨����
unsigned char HWcord[4];    //���������ڴ�����������4���ֽڵ����ݣ��û���2���ֽ�+��ֵ��2���ֽڣ�
unsigned char HWdata[33];   //���������ڴ�������33λ���ݣ���һλΪ�������û���16+��ֵ��16��
bit HW1,HW2;  //��һ�����ں������4���ֽ���ϡ�HWok��Ϊ����������

void init()	   //��ʼ����ʱ��0 ���ⲿ�ж�0
{
	TMOD = 0x02; //��ʱ��0������ʽ2��8λ�Զ���װ
	TH0 = 0x00;  //��8λװ��0��ô��ʱ�����һ�ε�ʱ����256����������
	TL0 = 0x00;
	EA = 1;      //���ж�
	ET0 = 1;	   //��ʱ��0�ж�
	TR0 = 1;     //������ʱ��0

	IT0 = 1;	   //�����ⲿ�ж�0Ϊ���ش�����ʽ����һ���½��ش���һ��
	EX0 = 1;	   //�����ⲿ�ж�0
}

void time0() interrupt 1   //���嶨ʱ��0
{
	HWtime++; 			   //�������1��Ϊ278us
}

void int0() interrupt 0	  		//�����ⲿ�ж�0
{
	static unsigned char i;	 			//	������̬�������������������ڻ���ִ�е�ʱ�򲻻ᶪʧ��ֵ��i���ڰ�33�θߵ�ƽ�ĳ���ʱ�����IRdata
	static bit startflag;		//��ʼ���������־λ
	if(startflag)	 			//��ʼ����������
	{
		if( (HWtime < 53) && (HWtime >= 32) ) /*�ж��Ƿ��������룬�׵�ƽ9000us+��4500us	
		����Լ�����������11.0592������NECЭ����������8000-10000+��4000-5000 
		����Ѿ���������������ôi���ᱻ��0�ͻῪʼ���δ�������*/
		i = 0;				 //�������������ôִ��i=0�����浽IRdata�ĵ�һ��λ
		HWdata[i] = HWtime;  		 //��T0������������������������ʱ��浽�������浽�����ж�
		HWtime = 0;				 //�������㣬��һ���½��ص�ʱ���ڴ�������
		i++; 					 //�����������Ĵ���
		if(i == 33) 				 //�������34�� ������±��Ǵ�0��ʼi����33��ʾִ����34��
		{
		 	HW2 = 1;				 //��ô��ʾ���������
			i = 0; 				 //�������������׼���´δ���
		}
	}
	else		  
	{
		HWtime = 0; 				 //�����뿪ʼ���������������㿪ʼ����
		startflag = 1;			 //��ʼ�����־λ��1
	}
}

void HWcordpro()   				 //��ȡ����33������������ݽ���
{
	unsigned char i, j, k, cord, value;	/*i���ڴ���4���ֽڣ�j���ڴ���һ���ֽ���ÿһλ��k����33�������е���һλ
	cord����ȡ�������ʱ���ж��Ƿ����1������ʱ��*/
	k = 1; 						//�ӵ�һλ����ʼȡ����������������
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 8; j++)
		{
			cord = HWdata[k];	    //���������cord
			if(cord > 5)	 		//������������11.0592��t0�����ΪԼ278us*5=1390��ô�ж�Ϊ1
			value = value | 0x80;	/*���յ�ʱ�����Ƚ������λ��
			�����λ�ȷŵ�value�����λ�ں�0x08��λ��һ��
			��������ı�valua������λ����ֵֻ���������λΪ1*/
			if(j < 7)
			{
				value = value >> 1;	//valueλ�������ν���8λ���ݡ�
			}
			k++;				//ÿִ��һ������λ��1
		}
		HWcord[i] = value;	   //ÿ������һ���ֽڰ�������IRcord�����С�
		value = 0; 			   //����value�����´��ڴ�������
	}
	HW1 = 1;				   //������4���ֽں�IRpro ok��1��ʾ����������	
}

/*******************������**************************/ 
void main() 
{
	//����pcf8591����ֵ���������������ղ������������ֵԽС������Խ��
	unsigned char ad_sampling = 0,
		      ad_sampling_cnt = 0;
	              
	//����ֵ�˲����ӣ���μ����ܺͣ�����������м�ֵ����
	float ad_filter = 0, ad = 0, ad_result = 0;
	//�ж����ȱ�־λ��1Ϊ��ڣ�0Ϊ����
	bit day_flag = 0;
	
	init();	//ִ�г�ʼ����ʱ��0���ⲿ�ж�0
	/*wela_1 = 0;	  //������ܹ����� */

	while(1)
	{
		ad_sampling = PCF8591_adc(0x48, 0);  //��pcf8591���в�����0x48Ϊ������ַ��0Ϊͨ��AIN0
		ad = ad_sampling * 1.0 * 500 / 255;  //����*250��ʾ��·�в����ο���ѹ��2.5v������255��ʾ����������8λ��оƬ�����������ԣ�
		ad_filter += ad;                     //�����ۼ�
		ad_sampling_cnt++;                   //���������ۼ�
		if (ad_sampling_cnt > 7) {
			ad_sampling_cnt = 0;
			ad_result = ad_filter / 8;   //���ۼ����ݳ���8��ȡ��ƽ��ֵ�����ͷ��������ݶ���ʵֵ��Ӱ��
			ad_filter = 0;
			
#if NO_PROTUES          /*****������ִ�г����޲��Ե�*****/			
			if (ad_result < DAY_NIGHT) { //�ж��������ݣ��������ȣ��Ƿ�ﵽ�涨��������ȣ����С����Ϊ����
				day_flag = 0;
				P1 = P1 | 0x0F;
			} else {
				day_flag = 1;
			}
			
#else			/*****proteus����ִ�г���*****/
			if (ad_result < DAY_NIGHT) {
				day_flag = 0;
				led_test = 1;  //���������ԵƲ���
			} else {
				day_flag = 1;
				led_test = 0;
			}
#endif
		}

#if NO_PROTUES  /*****�����棬��ִ������ң��������*****/
		if(HW2 && day_flag)    //�ж������Ƿ������  ������Ϊ����day_flag=1��
		{   
			HWcordpro();//������������4���ֽڵ�����
			HW2 = 0;	//���µȴ�������
			if(HW1) //�ж��Ƿ�������  
			{	
				wela_1 = 0;
				switch(HWcord[2])
		   		{		
				     case 0x45: led2=led3=led4=1;led1 = 0;	beep=0;Delay(50);beep=1; LED_PORT=0xf9;		 break;  //��"1"����
				     case 0x46: led3=led4=1;led1=led2 = 0;	beep=0;Delay(100);beep=1; LED_PORT=0xa4;			break;  //
					 case 0x47: led4=1;	 led1=led2 =led3= 0; beep=0;Delay(150);beep=1;LED_PORT=0xb0;		break;
					 case 0x44: led1=led2=led3=led4 = 0; beep=0;Delay(200);beep=1;	LED_PORT=0x99;	 		break;
					 case 0x40: led1=led2=led3 =led4= 1; beep=0;Delay(50);beep=1;	wela_1 = 1;		break;
					   default:break;
		   		}
				HW1 = 0;
			}
		}
#endif
	}

} 


/******************z ����ʱ����*************************/ 
void delay(unsigned int z) 
{ 
	unsigned int x,y; 
	for(x=z;x>0;x--) 
		for(y=120;y>0;y--); 
} 
/****************΢����ʱ******************************/ 
void delay_us(unsigned int aa) 
{ 
	while(aa--); 
}

//��ʱxms��������,xΪ���ټ�Ϊ��ʱ����ms�������ڴ���ʱ
void    Delays(unsigned int xms)
{
    unsigned int i, j;
    for (i = xms; i > 0; i--)
            for (j = 110; j > 0; j--);

}


