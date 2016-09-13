/*********  For Study**************
 *	文件名				:		 mpu9250.c
 *	描述					: 	 mpu9250配置文件
 *********(C) COPYRIGHT************/

#include "STM32F4.h"

u8	mpu9250_buf[14];				//spi读取MPU9250后存放数据
u8 offset_flag = 0;				   //校准模式标志位，为0未进行校准，为1进行校准

S_INT16_XYZ	MPU9250_Acc_Offset	=	{0,0,0};		
S_INT16_XYZ	MPU9250_Gyro_Offset	=	{0,0,0};	
S_INT16_XYZ MPU9250_Acc = {0,0,0};
S_INT16_XYZ MPU9250_Gyro = {0,0,0};

float	mpu9250_tempreature = 0;
s16 mpu9250_tempreature_temp = 0;
s16 mpu9250_tempreature_Offset = 0;

/*
 * 函数名：MPU9250_Date_Offset
 * 描述  ：MPU9250数据校准
 * 输入  ：校准次数
 * 输出  ：无
 */ 
void MPU9250_Date_Offset(u16 cnt)
{
	static S_INT32_XYZ Temp_Gyro , Temp_Acc;
	int i = 0;
	
	Temp_Gyro.X =	0;
	Temp_Gyro.Y =	0;
	Temp_Gyro.Z =	0;
	
	Temp_Acc.X = 0;
	Temp_Acc.Y = 0;
	Temp_Acc.Z = 0;
	
	offset_flag = 1;  //进入MPU9250校准模式
	for(i = cnt; i > 0; i--)
	{
    MPU9250_ReadValue();
		
		Temp_Acc.X	+=	MPU9250_Acc.X;
		Temp_Acc.Y	+=	MPU9250_Acc.Y;		
		//Temp_Acc.Z	+=	MPU9250_Acc.Z;
		Temp_Gyro.X	+=	MPU9250_Gyro.X;
		Temp_Gyro.Y	+=	MPU9250_Gyro.Y;
		Temp_Gyro.Z	+=	MPU9250_Gyro.Z;

	}
	
	MPU9250_Acc_Offset.X 	=	Temp_Acc.X	/	cnt;
	MPU9250_Acc_Offset.Y 	=	Temp_Acc.Y	/	cnt;
	//MPU9250_Acc_Offset.Z  =	Temp_Acc.Z	/	cnt;	
	MPU9250_Gyro_Offset.X	= Temp_Gyro.X	/	cnt;
	MPU9250_Gyro_Offset.Y	= Temp_Gyro.Y	/	cnt;
	MPU9250_Gyro_Offset.Z =	Temp_Gyro.Z	/	cnt;

	offset_flag = 0;//退出MPU9250校准模式
}

/*
 * 函数名：MPU9250_Init
 * 描述  ：MPU9250初始化函数
 * 输入  ：无
 * 输出  ：0：初始化失败 1：初始化成功
 */ 
void MPU9250_Init(void)
{	
	u16 adress;
	adress=MPU9250_Read_Reg(WHO_AM_I);
	MPU9250_Write_Reg(PWR_MGMT_1, 0x00);	//解除休眠状态
	MPU9250_Write_Reg(CONFIG, 0x07);      //低通滤波频率，典型值：0x07(3600Hz)此寄存器内决定Internal_Sample_Rate==8K
	
/**********************Init SLV0 i2c**********************************/	
//Use SPI-bus read slave0
	MPU9250_Write_Reg(INT_PIN_CFG ,0x30);// INT Pin / Bypass Enable Configuration  
	MPU9250_Write_Reg(I2C_MST_CTRL,0x4d);//I2C MAster mode and Speed 400 kHz
	MPU9250_Write_Reg(USER_CTRL ,0x20); // I2C_MST _EN 
	MPU9250_Write_Reg(I2C_MST_DELAY_CTRL ,0x01);//延时使能I2C_SLV0 _DLY_ enable 	
	MPU9250_Write_Reg(I2C_SLV0_CTRL ,0x81); //enable IIC	and EXT_SENS_DATA==1 Byte
	
/*******************Init GYRO and ACCEL******************************/	
	MPU9250_Write_Reg(SMPLRT_DIV, 0x07);  //陀螺仪采样率，典型值：0x07(1kHz) (SAMPLE_RATE= Internal_Sample_Rate / (1 + SMPLRT_DIV) )
	MPU9250_Write_Reg(GYRO_CONFIG, 0x18); //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)   16.4LSB/(°/s)
	MPU9250_Write_Reg(ACCEL_CONFIG, 0x18);//加速计自检、测量范围及高通滤波频率，典型值：0x18(不自检，16G)  2048/ g
	MPU9250_Write_Reg(ACCEL_CONFIG_2, 0x08);//加速计高通滤波频率 典型值 ：0x08  （1.13kHz）	
		
/**********************Init MAG **********************************/
	i2c_Mag_write(AK8963_CNTL2_REG,AK8963_CNTL2_SRST); // Reset AK8963
	i2c_Mag_write(AK8963_CNTL1_REG,0x12); // use i2c to set AK8963 working on Continuous measurement mode1 & 16-bit output	
}


////************************加速度读取**************************/
//void READ_MPU9250_ACCEL(void)
//{ 

//   BUF[0]=MPU9250_Read_Reg(ACCEL_XOUT_L); 
//   BUF[1]=MPU9250_Read_Reg(ACCEL_XOUT_H);
//   mpu_value.Accel[0]=	(BUF[1]<<8)|BUF[0];
//   mpu_value.Accel[0]/=164; 						   //读取计算X轴数据
//   BUF[2]=MPU9250_Read_Reg(ACCEL_YOUT_L);
//   BUF[3]=MPU9250_Read_Reg(ACCEL_YOUT_H);
//   mpu_value.Accel[1]=	(BUF[3]<<8)|BUF[2];
//   mpu_value.Accel[1]/=164; 						   //读取计算Y轴数据
//   BUF[4]=MPU9250_Read_Reg(ACCEL_ZOUT_L); 
//   BUF[5]=MPU9250_Read_Reg(ACCEL_ZOUT_H);
//   mpu_value.Accel[2]=  (BUF[5]<<8)|BUF[4];
//   mpu_value.Accel[2]/=164; 					      //读取计算Z轴数据 
//	
//}
////**********************陀螺仪读取*****************************/
//void READ_MPU9250_GYRO(void)
//{ 

//   BUF[0]=MPU9250_Read_Reg(GYRO_XOUT_L); 
//   BUF[1]=MPU9250_Read_Reg(GYRO_XOUT_H);
//   mpu_value.Gyro[0]=	(BUF[1]<<8)|BUF[0];
//   mpu_value.Gyro[0]/=16.4; 						   //读取计算X轴数据

//   BUF[2]=MPU9250_Read_Reg(GYRO_YOUT_L);
//   BUF[3]=MPU9250_Read_Reg(GYRO_YOUT_H);
//   mpu_value.Gyro[1]=	(BUF[3]<<8)|BUF[2];
//   mpu_value.Gyro[1]/=16.4; 						   //读取计算Y轴数据
//   BUF[4]=MPU9250_Read_Reg(GYRO_ZOUT_L);
//   BUF[5]=MPU9250_Read_Reg(GYRO_ZOUT_H);
//   mpu_value.Gyro[2]=	(BUF[5]<<8)|BUF[4];
//   mpu_value.Gyro[2]/=16.4; 					       //读取计算Z轴数据
//}


////**********************磁力计读取***************************/
////i2c_Mag_read(AK8963_ST2_REG) 此步读取不可省略
////数据读取结束寄存器，reading this register means data reading end
////AK8963_ST2_REG 同时具有数据非正常溢出检测功能
////详情参考 MPU9250 PDF
///**********************************************************/
//void READ_MPU9250_MAG(void)
//{ 	
//	u8 x_axis,y_axis,z_axis; 
//	
//	x_axis=i2c_Mag_read(AK8963_ASAX);// X轴灵敏度调整值
//	y_axis=i2c_Mag_read(AK8963_ASAY);
//	z_axis=i2c_Mag_read(AK8963_ASAZ);
//	
//	if((i2c_Mag_read(AK8963_ST1_REG)&AK8963_ST1_DOR)==0)//data ready
//	{
//			//读取计算X轴数据
//		 BUF[0]=i2c_Mag_read(MAG_XOUT_L); //Low data	
//		 if((i2c_Mag_read(AK8963_ST2_REG)&AK8963_ST2_HOFL)==1)// data reading end register & check Magnetic sensor overflow occurred 
//		 {
//			 BUF[0]=i2c_Mag_read(MAG_XOUT_L);//reload data
//		 } 
//		 BUF[1]=i2c_Mag_read(MAG_XOUT_H); //High data	
//		 if((i2c_Mag_read(AK8963_ST2_REG)&AK8963_ST2_HOFL)==1)// data reading end register
//		 {
//			 BUF[1]=i2c_Mag_read(MAG_XOUT_H);
//		 }
//		 mpu_value.Mag[0]=((BUF[1]<<8)|BUF[0])*(((x_axis-128)>>8)+1);		//灵敏度纠正 公式见/RM-MPU-9250A-00 PDF/ 5.13	
//		 
//		//读取计算Y轴数据
//			BUF[2]=i2c_Mag_read(MAG_YOUT_L); //Low data	
//		 if((i2c_Mag_read(AK8963_ST2_REG)&AK8963_ST2_HOFL)==1)// data reading end register
//		 {
//			 BUF[2]=i2c_Mag_read(MAG_YOUT_L);
//		 }		 
//		 BUF[3]=i2c_Mag_read(MAG_YOUT_H); //High data	
//		 if((i2c_Mag_read(AK8963_ST2_REG)&AK8963_ST2_HOFL)==1)// data reading end register
//		 {
//			 BUF[3]=i2c_Mag_read(MAG_YOUT_H);
//		 }
//		  mpu_value.Mag[1]=((BUF[3]<<8)|BUF[2])*(((y_axis-128)>>8)+1);	
//		 
//		//读取计算Z轴数据
//		 BUF[4]=i2c_Mag_read(MAG_ZOUT_L); //Low data	
//		 if((i2c_Mag_read(AK8963_ST2_REG)&AK8963_ST2_HOFL)==1)// data reading end register
//		 {
//			 BUF[4]=i2c_Mag_read(MAG_ZOUT_L);
//		 }	 
//		 BUF[5]=i2c_Mag_read(MAG_ZOUT_H); //High data	
//		 if((i2c_Mag_read(AK8963_ST2_REG)&AK8963_ST2_HOFL)==1)// data reading end register
//		 {
//			 BUF[5]=i2c_Mag_read(MAG_ZOUT_H);
//		 }
//		  mpu_value.Mag[2]=((BUF[5]<<8)|BUF[4])*(((z_axis-128)>>8)+1);	
//	}					       
//}

void MPU9250_ReadValue(void)
{
	uint8_t i;
	
	MPU9250_CS(0); 																	//使能SPI传输

	SPI1_Read_Write_Byte(ACCEL_XOUT_H|0x80); 				//从加速度计的寄存器开始进行读取陀螺仪和加速度计的值//发送读命令+寄存器号
	
	for(i	=	0;i	<	14;i++)														//一共读取14字节的数据
	{
		mpu9250_buf[i]	=	SPI1_Read_Write_Byte(0xff);	//输入0xff,因为slave不识别
	}	
	if(offset_flag == 0)
	{
		MPU9250_Acc.X = BYTE16(s16, mpu9250_buf[0],  mpu9250_buf[1]) - MPU9250_Acc_Offset.X;
		MPU9250_Acc.Y = BYTE16(s16, mpu9250_buf[2],  mpu9250_buf[3]) - MPU9250_Acc_Offset.Y;
		MPU9250_Acc.Z = BYTE16(s16, mpu9250_buf[4],  mpu9250_buf[5]);
		MPU9250_Gyro.X = BYTE16(s16, mpu9250_buf[8],  mpu9250_buf[9]) - MPU9250_Gyro_Offset.X;
		MPU9250_Gyro.Y = BYTE16(s16, mpu9250_buf[10],  mpu9250_buf[11]) - MPU9250_Gyro_Offset.Y;
		MPU9250_Gyro.Z = BYTE16(s16, mpu9250_buf[12],  mpu9250_buf[13]) - MPU9250_Gyro_Offset.Z;
		
		mpu9250_tempreature_temp	=	BYTE16(s16, mpu9250_buf[6],  mpu9250_buf[7]);
		mpu9250_tempreature	=	(float)(35000+((521+mpu9250_tempreature_temp)*100)/34); // 原来分母为340，现在分子*100，即：扩大1000倍；
		mpu9250_tempreature = mpu9250_tempreature/1000;                             
		if(( -4	<	MPU9250_Gyro.X ) && (MPU9250_Gyro.X < 4) ) MPU9250_Gyro.X = 0;
		if(( -4	<	MPU9250_Gyro.Y ) && (MPU9250_Gyro.Y < 4) ) MPU9250_Gyro.Y = 0;
		if(( -4	<	MPU9250_Gyro.Z ) && (MPU9250_Gyro.Z < 4) ) MPU9250_Gyro.Z = 0;
	}
	else if(offset_flag)  //MPU9250处于校准模式
	{
		MPU9250_Acc.X = BYTE16(s16, mpu9250_buf[0],  mpu9250_buf[1]);
		MPU9250_Acc.Y = BYTE16(s16, mpu9250_buf[2],  mpu9250_buf[3]);
		MPU9250_Acc.Z = BYTE16(s16, mpu9250_buf[4],  mpu9250_buf[5]);
		MPU9250_Gyro.X = BYTE16(s16, mpu9250_buf[8],  mpu9250_buf[9]);
		MPU9250_Gyro.Y = BYTE16(s16, mpu9250_buf[10],  mpu9250_buf[11]);
		MPU9250_Gyro.Z = BYTE16(s16, mpu9250_buf[12],  mpu9250_buf[13]);
	}
	
	MPU9250_CS(1);  	    //禁止SPI传输
}
