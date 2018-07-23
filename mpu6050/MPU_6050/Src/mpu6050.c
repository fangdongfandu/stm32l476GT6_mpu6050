#include "mpu6050.h"
#include "i2c.h"
#include "usart.h"

unsigned char MPU6050_Init(void)
{ 
	unsigned char res;
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//��λMPU6050
  HAL_Delay(100);
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//����MPU6050 
	MPU_Set_Gyro_Fsr(3);					//�����Ǵ�����,��2000dps
	MPU_Set_Accel_Fsr(0);					//���ٶȴ�����,��2g
	MPU_Set_Rate(50);						//���ò�����50Hz
	MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//�ر������ж�
	MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C��ģʽ�ر�
	MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);	//�ر�FIFO
	MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);	//INT���ŵ͵�ƽ��Ч
	res=MPU_Read_Byte(MPU_DEVICE_ID_REG);
	if(res==MPU_ADDR)//����ID��ȷ
	{
		MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);	//����CLKSEL,PLL X��Ϊ�ο�
		MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);	//���ٶ��������Ƕ�����
		MPU_Set_Rate(50);						//���ò�����Ϊ50Hz
 	}else return 1;
	return 0;
}

//����MPU6050�����Ǵ����������̷�Χ
//fsr:0,��250dps;1,��500dps;2,��1000dps;3,��2000dps
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
unsigned char MPU_Set_Gyro_Fsr(unsigned char fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG,fsr<<3);//���������������̷�Χ  
}

//����MPU6050���ٶȴ����������̷�Χ
//fsr:0,��2g;1,��4g;2,��8g;3,��16g
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
unsigned char MPU_Set_Accel_Fsr(unsigned char fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);//���ü��ٶȴ����������̷�Χ  
}

//����MPU6050�����ֵ�ͨ�˲���
//lpf:���ֵ�ͨ�˲�Ƶ��(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
unsigned char MPU_Set_LPF(unsigned short lpf)
{
	unsigned char data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return MPU_Write_Byte(MPU_CFG_REG,data);//�������ֵ�ͨ�˲���  
}

//����MPU6050�Ĳ�����(�ٶ�Fs=1KHz)
//rate:4~1000(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
unsigned char MPU_Set_Rate(unsigned short rate)
{
	unsigned char data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	//�������ֵ�ͨ�˲���
 	return MPU_Set_LPF(rate/2);	//�Զ�����LPFΪ�����ʵ�һ��
}

//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��)
short MPU_Get_Temperature(void)
{
    unsigned char buf[2]; 
    short raw;
	float temp;
	MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((unsigned short)buf[0]<<8)|buf[1];  
    temp=36.53+((double)raw)/340;  
    return temp*100;;
}

//�õ�������ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
unsigned char MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
{
    unsigned char buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		*gx=((unsigned short)buf[0]<<8)|buf[1];  
		*gy=((unsigned short)buf[2]<<8)|buf[3];  
		*gz=((unsigned short)buf[4]<<8)|buf[5];
	} 	
    return res;;
}

//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
unsigned char MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
    unsigned char buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		*ax=((unsigned short)buf[0]<<8)|buf[1];  
		*ay=((unsigned short)buf[2]<<8)|buf[3];  
		*az=((unsigned short)buf[4]<<8)|buf[5];
	} 	
    return res;;
}

//IIC����д
//addr:������ַ 
//reg:�Ĵ�����ַ
//len:д�볤��
//buf:������
//����ֵ:0,����
//    ����,�������
unsigned char MPU_Write_Len(unsigned char addr,unsigned char reg,unsigned char len,unsigned char *buf)
{
    if(HAL_I2C_Mem_Write(&hi2c1,(addr << 1),(uint16_t)reg,sizeof(uint8_t),buf,len,I2C_TIMEOUT) != HAL_OK)
        {
            log_printf("I2C1 memery write len failed!");
        }
		return 0;
} 

//IIC������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
unsigned char MPU_Read_Len(unsigned char addr,unsigned char reg,unsigned char len,unsigned char *buf)
{ 
    uint8_t tmp = 0;
    if(HAL_I2C_Mem_Read(&hi2c1,((addr << 1) + 1),(uint16_t)reg,sizeof(uint8_t),buf,len,I2C_TIMEOUT) != HAL_OK)
        {
            log_printf("I2C1 memery write len failed!");
        }
    return tmp;
}

//IICдһ���ֽ� 
//reg:�Ĵ�����ַ
//data:����
//����ֵ:0,����
//    ����,�������
unsigned char MPU_Write_Byte(unsigned char reg,unsigned char data) 				 
{ 
    if(HAL_I2C_Mem_Write(&hi2c1,(MPU_ADDR<<1),(uint16_t)reg,sizeof(uint8_t),&data,sizeof(uint8_t),I2C_TIMEOUT) != HAL_OK)
    {
        log_printf("I2C1 memery write failed!");
    }
		return 0;
}

//IIC��һ���ֽ� 
//reg:�Ĵ�����ַ 
//����ֵ:����������
unsigned char MPU_Read_Byte(unsigned char reg)
{
    uint8_t tmp = 0;
    if(HAL_I2C_Mem_Read(&hi2c1,((MPU_ADDR<<1) + 1),(uint16_t)reg,sizeof(uint8_t),&tmp,sizeof(uint8_t),I2C_TIMEOUT) != HAL_OK)
        {
            log_printf("I2C1 memery write failed!");
        }
    return tmp;	
}


