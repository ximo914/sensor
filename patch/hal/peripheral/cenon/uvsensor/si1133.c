/*  $Date: 2009/10/23 $
 *  $Revision: 1.2 $
 */

/*
* Copyright (C) 2009 Bosch Sensortec GmbH
*
* si113 pressure sensor API
* 
* Usage:  Application Programming Interface for si113 configuration and data read out
*
* 
* Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in 
  compliance with the License and the following stipulations. The Apache License , Version 2.0 is applicable unless 
  otherwise stated by the stipulations of the disclaimer below. 
 
* You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0
  
 

Disclaimer 

* Common:
* This Work is developed for the consumer goods industry. It may only be used 
* within the parameters of the respective valid product data sheet.  The Work 
* provided with the express understanding that there is no warranty of fitness for a particular purpose. 
* It is not fit for use in life-sustaining, safety or security sensitive systems or any system or device 
* that may lead to bodily harm or property damage if the system or device malfunctions. In addition, 
* the Work is not fit for use in products which interact with motor vehicle systems.  
* The resale and/or use of the Work are at the purchaser抯 own risk and his own responsibility. The 
* examination of fitness for the intended use is the sole responsibility of the Purchaser. 
*
* The purchaser shall indemnify Bosch Sensortec from all third party claims, including any claims for 
* incidental, or consequential damages, arising from any Work or Derivative Work use not covered by the parameters of 
* the respective valid product data sheet or not approved by Bosch Sensortec and reimburse Bosch 
* Sensortec for all costs in connection with such claims.
*
* The purchaser must monitor the market for the purchased Work and Derivative Works, particularly with regard to 
* product safety and inform Bosch Sensortec without delay of all security relevant incidents.
*
* Engineering Samples are marked with an asterisk (*) or (e). Samples may vary from the valid 
* technical specifications of the product series. They are therefore not intended or fit for resale to third 
* parties or for use in end products. Their sole purpose is internal client testing. The testing of an 
* engineering sample may in no way replace the testing of a product series. Bosch Sensortec 
* assumes no liability for the use of engineering samples. By accepting the engineering samples, the 
* Purchaser agrees to indemnify Bosch Sensortec from all claims arising from the use of engineering 
* samples.
*
* Special:
* This Work and any related information (hereinafter called "Information") is provided free of charge 
* for the sole purpose to support your application work. The Woek and Information is subject to the 
* following terms and conditions: 
*
* The Work is specifically designed for the exclusive use for Bosch Sensortec products by 
* personnel who have special experience and training. Do not use this Work or Derivative Works if you do not have the 
* proper experience or training. Do not use this Work or Derivative Works fot other products than Bosch Sensortec products.  
*
* The Information provided is believed to be accurate and reliable. Bosch Sensortec assumes no 
* responsibility for the consequences of use of such Information nor for any infringement of patents or 
* other rights of third parties which may result from its use. No license is granted by implication or 
* otherwise under any patent or patent rights of Bosch. Specifications mentioned in the Information are 
* subject to change without notice.
*
*/



#include "MMI_features.h"
#include "Dcl.h"
#include "Dcl_i2c.h"
#include "TimerEvents.h"
#include "MMI_features.h"
#include "si1133.h"


//#define __CENON_COMPASS_si113__   //haiming temp removed



#if 1//def __CENON_COMPASS_si113__
DCL_HANDLE si113_i2c_handle = 0;
int si113_i2c_configure_done = 0;
extern const char gpio_ms_i2c_data_pin;
extern DCL_HANDLE timer_handle_test_mode;

extern void ms_i2c_udelay(unsigned int delay);



void si113_i2c_mdelay(unsigned int delay)
{
	ms_i2c_udelay(delay*1000);
}

void si113_i2c_configure(unsigned int slave_addr, unsigned int speed)
{
	I2C_CONFIG_T cfg;
	
	if(gpio_ms_i2c_data_pin == 0xFF) // HW I2C
	{
		if(!si113_i2c_configure_done)
		{
			si113_i2c_handle = DclSI2C_Open(DCL_I2C, DCL_I2C_USER_DEV2);
		}
		cfg.eOwner = DCL_I2C_USER_DEV2;
		cfg.fgGetHandleWait = 1;
		cfg.u1SlaveAddress = slave_addr;
		cfg.u1DelayLen = 0;
		cfg.eTransactionMode = 0;//DCL_I2C_TRANSACTION_FAST_MODE;
		cfg.u4FastModeSpeed = speed;
		cfg.u4HSModeSpeed = 0;
		cfg.fgEnableDMA = 0;
  	
		DclSI2C_Configure(si113_i2c_handle, (DCL_CONFIGURE_T *)&cfg);
	}
	
	si113_i2c_configure_done = 1;
}

// I2C send data fuction
int si113_i2c_send(unsigned char ucBufferIndex, unsigned char* pucData, unsigned int unDataLength)
{
	unsigned int i;
	unsigned char write_buf[9];
	int bRet = 1;
	I2C_CTRL_CONT_WRITE_T write;
	DCL_STATUS status;

	if(gpio_ms_i2c_data_pin == 0xFF) // HW I2C
	{
		if(si113_i2c_configure_done)
		{
			write_buf[0] = ucBufferIndex;
			for(i=0;i<unDataLength;i++)
			{
				write_buf[i+1] = *(pucData+i);
			}
			write.pu1Data = write_buf;
			write.u4DataLen = unDataLength+1;
			write.u4TransferNum = 1;
			status = DclSI2C_Control(si113_i2c_handle, I2C_CMD_CONT_WRITE, (DCL_CTRL_DATA_T *)&write);
			if(status != STATUS_OK)
				return 0;
		}
	}
	
	return bRet;
}
// I2C receive data function
int si113_i2c_receive(unsigned char ucBufferIndex, unsigned char* pucData, unsigned int unDataLength)
{
	unsigned int i;
	int bRet = 1;
	I2C_CTRL_WRITE_AND_READE_T write_read;
	DCL_STATUS status;

	if(gpio_ms_i2c_data_pin == 0xFF) // HW I2C
	{
		if(si113_i2c_configure_done)
		{
			write_read.pu1InData = pucData;
			write_read.u4InDataLen = unDataLength;
			write_read.pu1OutData = &ucBufferIndex;			
			write_read.u4OutDataLen = 1;
			status = DclSI2C_Control(si113_i2c_handle, I2C_CMD_WRITE_AND_READ, (DCL_CTRL_DATA_T *)&write_read);
			if(status != STATUS_OK)
				return 0;
		}
	}

	return bRet;
}

 //形参 addr:地址
kal_uint8 si113_reg_read(kal_uint8 addr)
{
	kal_uint8 data[2];
	kal_uint8 data_ret = 0;
	kal_bool ret_bool = 0;
	//kal_uint8 i = 0;
	
		
	ret_bool = si113_i2c_receive(addr,data,1);//			
	if(ret_bool != KAL_TRUE)
	{
		kal_prompt_trace(MOD_WAP,"si113_reg_read error line=%d\n",__LINE__);
		return 0xEEEE;
	}
	
	data_ret = data[0];
    return data_ret;
    
}
//形参 addr:reg 地址 val:值
kal_bool si113_reg_write(kal_uint8 addr, kal_uint8 val)
{	
	kal_uint8 write_buf[9] = {0};
	kal_bool ret_bool = 0;
   
    	     write_buf[0] = val;
		ret_bool = si113_i2c_send(addr,write_buf,1);
		if(ret_bool != KAL_TRUE)
		{
			kal_prompt_trace(MOD_WAP,"si113_reg_write error line=%d\n",__LINE__);
		}
		return ret_bool;
		
   
}
kal_bool si113_reg_read_buf(kal_uint8 addr, kal_uint8 *val,kal_uint32 uLength)
{
	kal_bool ret_bool = 0;
    
	
		ret_bool = si113_i2c_receive(addr,val,uLength);//			
		if(ret_bool != KAL_TRUE)
		{
			kal_prompt_trace(MOD_WAP," si113_reg_read_buf error line=%d\n",__LINE__);
			return KAL_FALSE;
		}
	    return KAL_TRUE;
    
}

#endif



void Delay_ms(kal_uint16 delay)
{
  si113_i2c_mdelay(delay);
}


kal_uint8 I2C_ReadByte(kal_uint8 slave_addr8bit,kal_uint8 reg_addr,kal_uint8 *value)
{
  kal_uint8 read_value;
  read_value =si113_reg_read( reg_addr);
  value=&read_value;
}

kal_uint8 I2C_WriteByte(kal_uint8 slave_addr8bit,kal_uint8 reg_addr,kal_uint8 value)
{
   si113_reg_write(reg_addr,value);
}


kal_bool I2C_WriteBlock(kal_uint8 slave_addr8bit, kal_uint8 reg_addr, kal_uint8* p_data, kal_uint16 len)
{
    kal_uint8 write_buf[9] = {0};
    kal_bool ret_bool = 0;

   
    ret_bool = si113_i2c_send(reg_addr,p_data,len);
    if(ret_bool != KAL_TRUE)
    {
    kal_prompt_trace(MOD_WAP,"si113_reg_write error line=%d\n",__LINE__);
    }
    return ret_bool;
}

kal_bool I2C_ReadBlock(kal_uint8 slave_addr8bit, kal_uint8 reg_addr, kal_uint8* p_data, kal_uint16 len)
{
   kal_bool ret_bool = 0;
    
	
		ret_bool = si113_i2c_receive(reg_addr,p_data,len);//			
		if(ret_bool != KAL_TRUE)
		{
			kal_prompt_trace(MOD_WAP," si113_reg_read_buf error line=%d\n",__LINE__);
			return KAL_FALSE;
		}
	    return KAL_TRUE;
    
}
 
kal_uint16 SI1133_ReadRegister(kal_uint8 addr)
{
	kal_uint8 x;
	//I2C_ReadByte(SI1133_PART_ID, addr, x);
	x=si113_reg_read(addr);
  //  kal_prompt_trace(MOD_WAP," SI1133_ReadRegister x=%x\n",x);
    
	return x;
}
 
kal_uint16 SI1133_WriteRegister(kal_uint8 addr, kal_uint8 data)
{
	//I2C_WriteByte(SI1133_PART_ID, addr, data);
	si113_reg_write(addr,data);
	return 0;
}
 
static kal_uint16 SI1133_WriteBlock(kal_uint8 addr, kal_uint8* p_data, kal_uint16 len)
{
	I2C_WriteBlock(SI1133_PART_ID, addr, p_data, len);
	return 0;
}
 
static kal_uint16 SI1133_ReadBlock(kal_uint8 addr, kal_uint8* p_data, kal_uint16 len)
{
    kal_bool ret_bool = 0;
   ret_bool =si113_reg_read_buf(addr,p_data,len);
    return ret_bool;
}
 
static kal_uint16 WaitUntilSleep(void)
{
	kal_uint16 retval = -1;
	kal_uint8 count = 0;
	while (count < 5)
	{
		retval = SI1133_ReadRegister(REG_RESPONSE0);
		if ((retval & RSP0_CHIPSTAT_MASK) == RSP0_SLEEP)   break;
		if (retval <  0)   return retval;
		count++;
	}
	return 0;
}
 
static kal_uint16 SI1133_SendCMD(kal_uint8 command)
{
	kal_uint16  response, retval;
	kal_uint8  count = 0;
	response = SI1133_ReadRegister(REG_RESPONSE0);
	if (response < 0) 
        return response;
    
	response = response & RSP0_COUNTER_MASK;
	for (count = 0 ; count < 5; count++)
	{
		if ((retval = WaitUntilSleep()) != 0)
            return retval;
		if (command == 0) 
            break;
		retval = SI1133_ReadRegister(REG_RESPONSE0);
		if ((retval & RSP0_COUNTER_MASK) == response)
            break;
		else if (retval < 0)
            return retval;
		else
			response = retval & RSP0_COUNTER_MASK;
	}
	if ((retval = (SI1133_WriteRegister(REG_COMMAND, command)) != 0))
	{
		return retval;
	}
	for (count = 0 ; count < 5; count++)
	{
		if (command == 0)  
            break;
		retval = SI1133_ReadRegister(REG_RESPONSE0);
		if ((retval & RSP0_COUNTER_MASK) != response) 
            break;
		else if (retval < 0) 
            return retval;
	}
	return 0;
}
 
kal_uint16 SI1133_ReadParameter(kal_uint8 addr)
{
	kal_uint16 retval;
	retval = SI1133_SendCMD(CMD_READ | (addr & 0x3F));
	if ( retval != 0 )    return retval;
	retval = SI1133_ReadRegister(REG_RESPONSE1);
	return retval;
}
 
kal_uint16 SI1133_WriteParameter(kal_uint8 addr, kal_uint8 data)
{
	kal_uint8 buffer[2];
	kal_uint16 retval, response_stored, response;
	buffer[0] = data; buffer[1] = CMD_WRITE | (addr & 0x3F);
	if ((retval = WaitUntilSleep()) != 0)    return retval;
	response_stored = RSP0_COUNTER_MASK & SI1133_ReadRegister(REG_RESPONSE0);
	retval = SI1133_WriteBlock(REG_HOSTIN0, (kal_uint8 *)buffer, 2);
	if (retval != 0)  return retval;
	response = SI1133_ReadRegister(REG_RESPONSE0);
	while ((response & RSP0_COUNTER_MASK) == response_stored)
	{
		response = SI1133_ReadRegister(REG_RESPONSE0);
	}
	if (retval < 0)   return retval;
	else   return 0;
}
 
static kal_uint16 SI1133_Reset(void)
{
	kal_uint16 retval = 0;
	Delay_ms(10);
	Delay_ms(10);
	Delay_ms(10);
	retval += SI1133_WriteRegister(REG_COMMAND, CMD_RESET_SW);
	Delay_ms(10);
	return retval;
}
 
kal_uint16 SI1133_NOP(void)
{
	return SI1133_SendCMD(CMD_RESET_CTR);
}
 
kal_uint16 SI1133_Force(void)
{
	return SI1133_SendCMD(CMD_FORCE);
}
 
kal_uint16 SI1133_Start (void)
{
	return SI1133_SendCMD(CMD_START);
}
 
kal_uint16 SI1133_Pause(void)
{
	kal_uint8 countA, countB;
	kal_uint8  retval = 0;
	while ((RSP0_CHIPSTAT_MASK & retval) != RSP0_SLEEP)
	{
		retval = SI1133_ReadRegister(REG_RESPONSE0);
	}
	countA = 0;
	while (countA < 5)
	{
		countB = 0;
		while (countB < 5)
		{
			retval = SI1133_ReadRegister(REG_RESPONSE0);
			if ((retval & RSP0_COUNTER_MASK) == 0)     break;
			else
			{
				SI1133_WriteRegister(REG_COMMAND, 0x00);
			}
			countB++;
		}
		SI1133_SendCMD(CMD_PAUSE);
		countB = 0;
		while (countB < 5)
		{
			retval = SI1133_ReadRegister(REG_RESPONSE0);
			if ((retval & RSP0_COUNTER_MASK) != 0)        break;
			countB++;
		}
		retval = SI1133_ReadRegister(REG_RESPONSE0);
		if ((retval & RSP0_COUNTER_MASK) == 1 )      break;
		countA++;
	}
	return 0;
}

kal_uint8 Cenon_read_PART_ID(void)
{
   kal_uint8 si1133_part_id;
   kal_uint8 ret;
   si1133_part_id =SI1133_ReadRegister(REG_PART_ID);
    kal_prompt_trace(MOD_WAP," Cenon_read_PART_ID si1133_part_id=%x\n",si1133_part_id);
   if(si1133_part_id==0x33)
   {
     kal_prompt_trace(MOD_WAP," Cenon_read_PART_ID success\n");
     ret=1;
   }
   else
   {
     kal_prompt_trace(MOD_WAP," Cenon_read_PART_ID fail\n");
     ret=0;
   }
   return ret;
}















/*! \file si113.c
   SI1133传感器是基于I2C通信的，关于I2C部分请参考：
   关于传感器初始化参数的说明：PARAM_CHAN_LIST:配置通道，从bit0到bit5,分别代表通道0到5，共六个通道，最高两位无效；
   PARAM_ADCCONFIGx:半导体功能配置，0x78表示配置成UV紫外线检测,积分时间基数短24.4us，即CFG_RATE_SHORT|CFG_UV，其余见datasheet;
   PARAM_ADCSENSx:检测信号范围，最高位置1表示高,可防止结果溢出，低4位表示积分时间；
   PARAM_ADCPOSTx:测量结果设置，0x00表示结果为16位模式，0x40表示24位模式；
   PARAM_MEASCONFIGx:计数器选择，0x00表示不设置，0x40设置计数器0，0x80设置计数器1，0xc0设置计数器
   
   PARAM_MEASRATE_H:采样频率设置高位
   PARAM_MEASRATE_L:采样频率设置低位

   REG_IRQ_ENABLE:开启通道中断

   按照SI1133_init()中的设置，积分时间基数为24.4us,积分时间t = 24.4us * 2^0x09 = 12.5ms，采样频率f = 1250Hz/0x09c4 = 0.5Hz；

   关于结果：根据以上配置，传感器检测结果保存在REG_HOSTOUT0-REG_HOSTOUT11中，每个通道占用两个字节，高位在前；
   CH0H,CH0L,CH1H,CH1L..............
   如设置了24位模式，则结果如下：CH0H,CH0L,CH1H,CH1M,CH1L........

*/
kal_uint16 SI1133_Init(void)
{
	kal_uint16    retval;
    kal_uint16    returnval;
	retval  = SI1133_Reset();
	Delay_ms(10);
   
    returnval=Cenon_read_PART_ID();
    if(returnval==0)
    return;
    
	retval += SI1133_WriteParameter( PARAM_CHAN_LIST, 	0x3f);//6 channel  0~5 all open
	//UV
	retval += SI1133_WriteParameter( PARAM_ADCCONFIG0, 	0x78);
	retval += SI1133_WriteParameter( PARAM_ADCSENS0, 	0x89);
	retval += SI1133_WriteParameter( PARAM_ADCPOST0, 	0x00);
	retval += SI1133_WriteParameter( PARAM_MEASCONFIG0, 0x40);
	//
	retval += SI1133_WriteParameter( PARAM_ADCCONFIG1, 	0x6d);
	retval += SI1133_WriteParameter( PARAM_ADCSENS1, 	0x89);
	retval += SI1133_WriteParameter( PARAM_ADCPOST1, 	0x00);
	retval += SI1133_WriteParameter( PARAM_MEASCONFIG1, 0x40);
 
	retval += SI1133_WriteParameter( PARAM_ADCCONFIG2, 	0x6b);
	retval += SI1133_WriteParameter( PARAM_ADCSENS2, 	0x89);
	retval += SI1133_WriteParameter( PARAM_ADCPOST2, 	0x00);
	retval += SI1133_WriteParameter( PARAM_MEASCONFIG2, 0x40);
 
	retval += SI1133_WriteParameter( PARAM_ADCCONFIG3, 	0x61);
	retval += SI1133_WriteParameter( PARAM_ADCSENS3, 	0x89);
	retval += SI1133_WriteParameter( PARAM_ADCPOST3, 	0x00);
	retval += SI1133_WriteParameter( PARAM_MEASCONFIG3, 0x40);
 
	retval += SI1133_WriteParameter( PARAM_ADCCONFIG4, 	0x60);
	retval += SI1133_WriteParameter( PARAM_ADCSENS4, 	0x89);
	retval += SI1133_WriteParameter( PARAM_ADCPOST4, 	0x00);
	retval += SI1133_WriteParameter( PARAM_MEASCONFIG4, 0x40);
 
	retval += SI1133_WriteParameter( PARAM_ADCCONFIG5, 	0x79);
	retval += SI1133_WriteParameter( PARAM_ADCSENS5, 	0x89);
	retval += SI1133_WriteParameter( PARAM_ADCPOST5, 	0x00);
	retval += SI1133_WriteParameter( PARAM_MEASCONFIG5, 0x40);
 
	retval += SI1133_WriteParameter( PARAM_MEASRATE_H, 	0x09);
	retval += SI1133_WriteParameter( PARAM_MEASRATE_L, 	0xc4);
	retval += SI1133_WriteRegister(  REG_IRQ_ENABLE, 	0x3f);
	retval += SI1133_Start();
	//SI1133_Force();
	return retval;
}


//参数，data指向一个6个kal_uint16的数组
void si1133_get_uv_data(kal_uint16 *data)
{
   int ret;
    kal_uint8 uv_data_0_5[6];
    kal_uint8 uv_data_6_11[6];
    kal_uint16 uv_data_raw[6]= {0};
    int i;

    ret=SI1133_ReadBlock(REG_HOSTOUT0, uv_data_0_5, 6);
    if(!ret)
      {
		kal_prompt_trace(MOD_WAP,"[I]----[%s]: i2c error\r\n", __func__);
		return;
	}
    ret=SI1133_ReadBlock(REG_HOSTOUT6, uv_data_6_11, 6);
    if(!ret)
      {
		kal_prompt_trace(MOD_WAP,"[I]----[%s]: i2c error xx\r\n", __func__);
		return;
	}
    uv_data_raw[0]=(kal_int16)((uv_data_0_5[0]<<8) |( uv_data_0_5[1]));
    uv_data_raw[1]=(kal_int16)((uv_data_0_5[2]<<8) |( uv_data_0_5[3]));
    uv_data_raw[2]=(kal_int16)((uv_data_0_5[4]<<8) |( uv_data_0_5[5]));

    kal_prompt_trace(MOD_WAP,"[%s]: raw:(chanenel 0=%d,chanenel 1=%d,chanenel 2=%d).\r\n", __func__, uv_data_raw[0], uv_data_raw[1], uv_data_raw[2]);


    uv_data_raw[3]=(kal_int16)((uv_data_6_11[0]<<8) |( uv_data_6_11[1]));
    uv_data_raw[4]=(kal_int16)((uv_data_6_11[2]<<8) |( uv_data_6_11[3]));
    uv_data_raw[5]=(kal_int16)((uv_data_6_11[4]<<8) |( uv_data_6_11[5]));


    

    kal_prompt_trace(MOD_WAP,"[%s]: raw:(chanenel 3=%d,chanenel 4=%d,chanenel 5=%d).\r\n", __func__, uv_data_raw[3], uv_data_raw[4], uv_data_raw[5]);
    for(i=0;i<6;i++)
    {
        data[i]=uv_data_raw[i];
    }

}



void si1133_test_read_data(void)
{
    kal_uint16 uv_test_data[6]={0};
    StopTimer(TIMER_ID_UVSENSOR_GET_DATA);


    si1133_get_uv_data(uv_test_data);
    //SI1133_Force();
    kal_prompt_trace(MOD_WAP,"[%s]: raw:(chanenel 0=%d,chanenel 1=%d,chanenel 2=%d).\r\n", __func__, uv_test_data[0], uv_test_data[1], uv_test_data[2]); 
    kal_prompt_trace(MOD_WAP,"[%s]: raw:(chanenel 3=%d,chanenel 4=%d,chanenel 5=%d).\r\n", __func__, uv_test_data[3], uv_test_data[4], uv_test_data[5]);
    StartTimer(TIMER_ID_UVSENSOR_GET_DATA, 1000*20, si1133_test_read_data); 
    
}



void Cenon_si1133_init(void)
{

   	si113_i2c_configure(SI1133_PART_ID,SI1133_I2C_SPEED);

    SI1133_Init();

     StartTimer(TIMER_ID_UVSENSOR_GET_DATA, 1000*20, si1133_test_read_data); 
}





