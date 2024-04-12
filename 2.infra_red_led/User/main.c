#include <reg52.h>
#include "pcf8591.h"

 #define FOSC 11059200L //晶振设置，默认使用11.0592M Hz
//#define FOSC 12000000L //晶振设置，使用12M Hz
//#define FOSC 24000000L //晶振设置，使用24M Hz

#define DAY_NIGHT  100.0 //规定的天黑亮度 取值0~250，250为最暗
#define NO_PROTUES  0  //proteus仿真可视化，1为不仿真；0时仿真，可在仿真中看到测试灯led_test(P1^7)随光敏电阻而亮灭

//IO接口定义
#define LED_PORT P0
sbit wela_1 = P2^4;					   //数码管


//全局变量定义
unsigned char num;

//LED显示字模 0-F 共阳模式
unsigned code table[]= {0Xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};

//函数定义
void delay(unsigned int z); 
void delay_us(unsigned int aa); 
void Delay(unsigned int xms);
void Delayms(unsigned int ms);
/*====================================
 硬件接口位声明
====================================*/
sbit HW  = P3^2;     //定义红外脉冲数据接口	外部中断O输入口
sbit led1 = P1^0;
sbit led2 = P1^1;
sbit led3 = P1^2;
sbit led4 = P1^3;
sbit beep = P1^6;
sbit led_test = P1^7;
unsigned char HWtime; 		//检测红外高电平持续时间（脉宽）
unsigned char HWcord[4];    //此数组用于储存分离出来的4个字节的数据（用户码2个字节+键值码2个字节）
unsigned char HWdata[33];   //此数组用于储存红外的33位数据（第一位为引导码用户码16+键值码16）
bit HW1,HW2;  //第一个用于红外接收4个字节完毕。HWok用为检测脉宽完毕

void init()	   //初始化定时器0 和外部中断0
{
	TMOD = 0x02; //定时器0工作方式2，8位自动重装
	TH0 = 0x00;  //高8位装入0那么定时器溢出一次的时间是256个机器周期
	TL0 = 0x00;
	EA = 1;      //总中断
	ET0 = 1;	   //定时器0中断
	TR0 = 1;     //启动定时器0

	IT0 = 1;	   //设置外部中断0为跳沿触发方式，来一个下降沿触发一次
	EX0 = 1;	   //启动外部中断0
}

void time0() interrupt 1   //定义定时器0
{
	HWtime++; 			   //检测脉宽，1次为278us
}

void int0() interrupt 0	  		//定义外部中断0
{
	static unsigned char i;	 			//	声明静态变量（在跳出函数后在回来执行的时候不会丢失数值）i用于把33次高电平的持续时间存入IRdata
	static bit startflag;		//开始储存脉宽标志位
	if(startflag)	 			//开始接收脉宽检测
	{
		if( (HWtime < 53) && (HWtime >= 32) ) /*判断是否是引导码，底电平9000us+高4500us	
		这个自己可以算我以11.0592来算了NEC协议的引导码低8000-10000+高4000-5000 
		如果已经接收了引导码那么i不会被置0就会开始依次存入脉宽*/
		i = 0;				 //如果是引导码那么执行i=0把他存到IRdata的第一个位
		HWdata[i] = HWtime;  		 //以T0的溢出次数来计算脉宽，把这个时间存到数组里面到后面判断
		HWtime = 0;				 //计数清零，下一个下降沿的时候在存入脉宽
		i++; 					 //计数脉宽存入的次数
		if(i == 33) 				 //如果存入34次 数组的下标是从0开始i等于33表示执行了34次
		{
		 	HW2 = 1;				 //那么表示脉宽检测完毕
			i = 0; 				 //把脉宽计数清零准备下次存入
		}
	}
	else		  
	{
		HWtime = 0; 				 //引导码开始进入把脉宽计数清零开始计数
		startflag = 1;			 //开始处理标志位置1
	}
}

void HWcordpro()   				 //提取它的33次脉宽进行数据解码
{
	unsigned char i, j, k, cord, value;	/*i用于处理4个字节，j用于处理一个字节中每一位，k用于33次脉宽中的哪一位
	cord用于取出脉宽的时间判断是否符合1的脉宽时间*/
	k = 1; 						//从第一位脉宽开始取，丢弃引导码脉宽
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 8; j++)
		{
			cord = HWdata[k];	    //把脉宽存入cord
			if(cord > 5)	 		//如果脉宽大于我11.0592的t0溢出率为约278us*5=1390那么判断为1
			value = value | 0x80;	/*接收的时候是先接收最低位，
			把最低位先放到value的最高位在和0x08按位或一下
			这样不会改变valua的其他位的数值只会让他最高位为1*/
			if(j < 7)
			{
				value = value >> 1;	//value位左移依次接收8位数据。
			}
			k++;				//每执行一次脉宽位加1
		}
		HWcord[i] = value;	   //每处理完一个字节把它放入IRcord数组中。
		value = 0; 			   //清零value方便下次在存入数据
	}
	HW1 = 1;				   //接收完4个字节后IRpro ok置1表示红外解码完成	
}

/*******************主函数**************************/ 
void main() 
{
	//定义pcf8591采样值，采样次数，最终采样结果，采样值越小，亮度越高
	unsigned char ad_sampling = 0,
		      ad_sampling_cnt = 0;
	              
	//采样值滤波因子（多次计数总和）与采样计算中间值定义
	float ad_filter = 0, ad = 0, ad_result = 0;
	//判断亮度标志位，1为天黑，0为天亮
	bit day_flag = 0;
	
	init();	//执行初始化定时器0和外部中断0
	/*wela_1 = 0;	  //打开数码管共阳端 */

	while(1)
	{
		ad_sampling = PCF8591_adc(0x48, 0);  //从pcf8591进行采样，0x48为采样地址，0为通道AIN0
		ad = ad_sampling * 1.0 * 500 / 255;  //数据*250表示电路中采样参考电压是2.5v，除以255表示将数据右移8位（芯片采样数据特性）
		ad_filter += ad;                     //数据累加
		ad_sampling_cnt++;                   //采样次数累加
		if (ad_sampling_cnt > 7) {
			ad_sampling_cnt = 0;
			ad_result = ad_filter / 8;   //对累加数据除以8，取得平均值，降低非正常数据对真实值的影响
			ad_filter = 0;
			
#if NO_PROTUES          /*****不仿真执行程序，无测试灯*****/			
			if (ad_result < DAY_NIGHT) { //判定采样数据（环境亮度）是否达到规定的天黑亮度，如果小于则为天亮
				day_flag = 0;
				P1 = P1 | 0x0F;
			} else {
				day_flag = 1;
			}
			
#else			/*****proteus仿真执行程序*****/
			if (ad_result < DAY_NIGHT) {
				day_flag = 0;
				led_test = 1;  //天亮，测试灯不亮
			} else {
				day_flag = 1;
				led_test = 0;
			}
#endif
		}

#if NO_PROTUES  /*****不仿真，则执行正常遥控器代码*****/
		if(HW2 && day_flag)    //判断脉宽是否检测完毕  并且天为暗（day_flag=1）
		{   
			HWcordpro();//根据脉宽解码出4个字节的数据
			HW2 = 0;	//重新等待脉宽检测
			if(HW1) //判断是否解码完毕  
			{	
				wela_1 = 0;
				switch(HWcord[2])
		   		{		
				     case 0x45: led2=led3=led4=1;led1 = 0;	beep=0;Delay(50);beep=1; LED_PORT=0xf9;		 break;  //按"1"灯亮
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


/******************z 秒延时函数*************************/ 
void delay(unsigned int z) 
{ 
	unsigned int x,y; 
	for(x=z;x>0;x--) 
		for(y=120;y>0;y--); 
} 
/****************微妙延时******************************/ 
void delay_us(unsigned int aa) 
{ 
	while(aa--); 
}

//延时xms函数如下,x为多少即为延时多少ms，适用于大延时
void    Delays(unsigned int xms)
{
    unsigned int i, j;
    for (i = xms; i > 0; i--)
            for (j = 110; j > 0; j--);

}


