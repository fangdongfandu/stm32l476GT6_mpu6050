#include "main.h"
#include "oled.h"
#include "oledfont.h"
#include "spi.h"

uint8_t oled_gram[128][8];

void oled_refresh_gram(void)
{
    uint8_t i,n;
    for(i = 0; i < 8; i++)
    {
        oled_wr_byte(0xb0 + i,oled_cmd);
        oled_wr_byte(0x00,oled_cmd);
        oled_wr_byte(0x10,oled_cmd);
        for(n = 0; n < 128; n++)
        {
            oled_wr_byte(oled_gram[n][i],oled_data);
        }
    }
}

void oled_wr_byte(uint8_t dat,uint8_t cmd)
{
    uint8_t temp;
    uint8_t data;
    data = dat;
    if(cmd)
        oled_rs_set();
    else
        oled_rs_clr();
    if(HAL_SPI_Transmit(&hspi2,&data,sizeof(uint8_t),5000) != HAL_OK)
    {
        //log_printf("spi transmit fail!\r\n");
    }
    oled_rs_set();
}

//OLED设置位置
void oled_set_pos(uint8_t x, uint8_t y) 
{ 
	oled_wr_byte(0xb0+y,oled_cmd);
	oled_wr_byte(((x&0xf0)>>4)|0x10,oled_cmd);
	oled_wr_byte((x&0x0f)|0x01,oled_cmd); 
}  

void oled_display_on(void)
{
    oled_wr_byte(0x8D,oled_cmd);//SET DCDC命令
    oled_wr_byte(0x14,oled_cmd);//DCDC ON
    oled_wr_byte(0xAF,oled_cmd);//DISPLAY ON
}

void oled_display_off(void)
{
    oled_wr_byte(0x8D,oled_cmd);//SET DCDC命令
    oled_wr_byte(0x10,oled_cmd);//DCDC OFF
    oled_wr_byte(0xAE,oled_cmd);//DISPLAY OFF
}

void oled_clear(void)
{
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		oled_wr_byte (0xb0+i,oled_cmd);    //设置页地址（0~7）
		oled_wr_byte (0x00,oled_cmd);      //设置显示位置—列低地址
		oled_wr_byte (0x10,oled_cmd);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)
		oled_wr_byte(0,oled_data); 
	} //更新显示
}

//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 16/12 
void oled_showchar(uint8_t x,uint8_t y,uint8_t chr)
{      	
	uint8_t c=0,i=0;	
	c=chr-' ';//得到偏移后的值			
	if(x>Max_Column-1)
	{
		x=0;
		y=y+2;
	}
	if(SIZE ==16)
	{
		oled_set_pos(x,y);	
		for(i=0;i<8;i++)
			oled_wr_byte(F8X16[c*16+i],oled_data);
		oled_set_pos(x,y+1);
		for(i=0;i<8;i++)
			oled_wr_byte(F8X16[c*16+i+8],oled_data);
	}
	else 
	{	
		oled_set_pos(x,y+1);
		for(i=0;i<6;i++)
			oled_wr_byte(F6x8[c][i],oled_data);
	}
}
//m^n函数
uint32_t oled_pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}

//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);	 		  
void oled_shownum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				oled_showchar(x+(size/2)*t,y,' ');
				continue;
			}else enshow=1; 
		}
	 	oled_showchar(x+(size/2)*t,y,temp+'0'); 
	}
} 


//显示一个字符号串  自动换行
void oled_showstring(uint8_t x,uint8_t y,uint8_t *chr)
{
	uint8_t j=0;
	while (chr[j]!='\0')
	{		
		oled_showchar(x,y,chr[j]);
		x+=8;
		if(x>120)
		{
			x=0;
			y+=2;
		}
		j++;
	}
}

//显示一个汉字
void OLED_ShowCh(uint8_t x,uint8_t y,uint8_t *dst)
{      			    
	uint8_t t;
	oled_set_pos(x,y);
	
	for(t=0;t<16;t++)
	{
		oled_wr_byte(*(dst + t),oled_data);
	}	
	oled_set_pos(x,y+1);	
	for(t=16;t<32;t++)
	{	
		oled_wr_byte(*(dst + t),oled_data);
	}					
}

void OLED_Show_Bat(uint8_t x,uint8_t y,uint8_t *dst)
{
	uint8_t t;
	oled_set_pos(x,y);
	
	for(t=0;t<32;t++)
	{
		oled_wr_byte(*(dst + t),oled_data);
	}	
	oled_set_pos(x,y+1);	
	for(t=32;t<64;t++)
	{	
		oled_wr_byte(*(dst + t),oled_data);
	}
}

/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,uint8_t BMP[])
{ 	
	uint8_t x,y;
	for(y=y0;y <= y1;y++)
	{
		oled_set_pos(x0,y);
        for(x=x0;x <= x1;x++)
	    {      
	    	oled_wr_byte(BMP[y*128 + x],oled_data);	    	
	    }
	}
} 

void oled_init(void)
{
    //MX_SPI2_Init();
    oled_rst_clr();
    HAL_Delay(100);
	
    oled_rst_set();
		oled_wr_byte(0xAE,oled_cmd);//--turn off oled panel
		oled_wr_byte(0x00,oled_cmd);//---set low column address
		oled_wr_byte(0x10,oled_cmd);//---set high column address
		oled_wr_byte(0x40,oled_cmd);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
		oled_wr_byte(0x81,oled_cmd);//--set contrast control register
		oled_wr_byte(0xCF,oled_cmd); // Set SEG Output Current Brightness
		oled_wr_byte(0xA1,oled_cmd);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
		oled_wr_byte(0xC8,oled_cmd);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
		oled_wr_byte(0xA6,oled_cmd);//--set normal display
		oled_wr_byte(0xA8,oled_cmd);//--set multiplex ratio(1 to 64)
		oled_wr_byte(0x3f,oled_cmd);//--1/64 duty
		oled_wr_byte(0xD3,oled_cmd);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
		oled_wr_byte(0x00,oled_cmd);//-not offset
		oled_wr_byte(0xd5,oled_cmd);//--set display clock divide ratio/oscillator frequency
		oled_wr_byte(0x80,oled_cmd);//--set divide ratio, Set Clock as 100 Frames/Sec
		oled_wr_byte(0xD9,oled_cmd);//--set pre-charge period
		oled_wr_byte(0xF1,oled_cmd);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
		oled_wr_byte(0xDA,oled_cmd);//--set com pins hardware configuration
		oled_wr_byte(0x12,oled_cmd);
		oled_wr_byte(0xDB,oled_cmd);//--set vcomh
		oled_wr_byte(0x40,oled_cmd);//Set VCOM Deselect Level
		oled_wr_byte(0x20,oled_cmd);//-Set Page Addressing Mode (0x00/0x01/0x02)
		oled_wr_byte(0x02,oled_cmd);//
		oled_wr_byte(0x8D,oled_cmd);//--set Charge Pump enable/disable
		oled_wr_byte(0x14,oled_cmd);//--set(0x10) disable
		oled_wr_byte(0xA4,oled_cmd);// Disable Entire Display On (0xa4/0xa5)
		oled_wr_byte(0xA6,oled_cmd);// Disable Inverse Display On (0xa6/a7) 
		oled_wr_byte(0xAF,oled_cmd);//--turn on oled panel
		
		oled_wr_byte(0xAF,oled_cmd); /*display ON*/ 

		oled_clear();
		oled_set_pos(0,0); 
		
		OLED_Show_Bat(95,0,battery_logol_100);
		
}

