/*  $Date: 2009/10/23 $
 *  $Revision: 1.2 $
 */

/*
* Copyright (C) 2009 Bosch Sensortec GmbH
*
* qma6981 pressure sensor API
* 
* Usage:  Application Programming Interface for qma6981 configuration and data read out
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
* The resale and/or use of the Work are at the purchaser�s own risk and his own responsibility. The 
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


/*! \file qma6981.c
    \brief This file contains all function implementations for the qma6981 API
    
    Details.
*/
#include "MMI_features.h"
#include "Dcl.h"
#include "Dcl_i2c.h"
#include "TimerEvents.h"
#include "MMI_features.h"
#include "qma6981.h"


#define __CENON_COMPASS_qma6981__   //haiming temp removed



#ifdef __CENON_COMPASS_qma6981__
DCL_HANDLE qma6981_i2c_handle = 0;
int qma6981_i2c_configure_done = 0;
extern const char gpio_ms_i2c_data_pin;
extern DCL_HANDLE timer_handle_test_mode;

extern void ms_i2c_udelay(unsigned int delay);

void qma6981_i2c_mdelay(unsigned int delay)
{
	ms_i2c_udelay(delay*1000);
}

void qma6981_i2c_configure(unsigned int slave_addr, unsigned int speed)
{
	I2C_CONFIG_T cfg;
	
	if(gpio_ms_i2c_data_pin == 0xFF) // HW I2C
	{
		if(!qma6981_i2c_configure_done)
		{
			qma6981_i2c_handle = DclSI2C_Open(DCL_I2C, DCL_I2C_USER_DEV1);
		}
		cfg.eOwner = DCL_I2C_USER_DEV1;
		cfg.fgGetHandleWait = 1;
		cfg.u1SlaveAddress = slave_addr;
		cfg.u1DelayLen = 0;
		cfg.eTransactionMode = 0;//DCL_I2C_TRANSACTION_FAST_MODE;
		cfg.u4FastModeSpeed = speed;
		cfg.u4HSModeSpeed = 0;
		cfg.fgEnableDMA = 0;
  	
		DclSI2C_Configure(qma6981_i2c_handle, (DCL_CONFIGURE_T *)&cfg);
	}
	
	qma6981_i2c_configure_done = 1;
}

// I2C send data fuction
int qma6981_i2c_send(unsigned char ucBufferIndex, unsigned char* pucData, unsigned int unDataLength)
{
	unsigned int i;
	unsigned char write_buf[9];
	int bRet = 1;
	I2C_CTRL_CONT_WRITE_T write;
	DCL_STATUS status;

	if(gpio_ms_i2c_data_pin == 0xFF) // HW I2C
	{
		if(qma6981_i2c_configure_done)
		{
			write_buf[0] = ucBufferIndex;
			for(i=0;i<unDataLength;i++)
			{
				write_buf[i+1] = *(pucData+i);
			}
			write.pu1Data = write_buf;
			write.u4DataLen = unDataLength+1;
			write.u4TransferNum = 1;
			status = DclSI2C_Control(qma6981_i2c_handle, I2C_CMD_CONT_WRITE, (DCL_CTRL_DATA_T *)&write);
			if(status != STATUS_OK)
				return 0;
		}
	}
	
	return bRet;
}
// I2C receive data function
int qma6981_i2c_receive(unsigned char ucBufferIndex, unsigned char* pucData, unsigned int unDataLength)
{
	unsigned int i;
	int bRet = 1;
	I2C_CTRL_WRITE_AND_READE_T write_read;
	DCL_STATUS status;

	if(gpio_ms_i2c_data_pin == 0xFF) // HW I2C
	{
		if(qma6981_i2c_configure_done)
		{
			write_read.pu1InData = pucData;
			write_read.u4InDataLen = unDataLength;
			write_read.pu1OutData = &ucBufferIndex;			
			write_read.u4OutDataLen = 1;
			status = DclSI2C_Control(qma6981_i2c_handle, I2C_CMD_WRITE_AND_READ, (DCL_CTRL_DATA_T *)&write_read);
			if(status != STATUS_OK)
				return 0;
		}
	}

	return bRet;
}

 //�β� addr:��ַ
kal_uint8 qma6981_reg_read(kal_uint8 addr)
{
	kal_uint8 data[2];
	kal_uint8 data_ret = 0;
	kal_bool ret_bool = 0;
	//kal_uint8 i = 0;
	
		
	ret_bool = qma6981_i2c_receive(addr,data,1);//			
	if(ret_bool != KAL_TRUE)
	{
		kal_prompt_trace(MOD_WAP,"qma6981_reg_read error line=%d\n",__LINE__);
		return 0xEEEE;
	}
	
	data_ret = data[0];
    return data_ret;
    
}
//�β� addr:reg ��ַ val:ֵ
kal_bool qma6981_reg_write(kal_uint8 addr, kal_uint8 val)
{	
	kal_uint8 write_buf[9] = {0};
	kal_bool ret_bool = 0;
   
    	     write_buf[0] = val;
		ret_bool = qma6981_i2c_send(addr,write_buf,1);
		if(ret_bool != KAL_TRUE)
		{
			kal_prompt_trace(MOD_WAP,"qma6981_reg_write error line=%d\n",__LINE__);
		}
		return ret_bool;
		
   
}
kal_bool qma6981_reg_read_buf(kal_uint8 addr, kal_uint8 *val,kal_uint32 uLength)
{
	kal_bool ret_bool = 0;
    
	
		ret_bool = qma6981_i2c_receive(addr,val,uLength);//			
		if(ret_bool != KAL_TRUE)
		{
			kal_prompt_trace(MOD_WAP," qma6981_reg_read_buf error line=%d\n",__LINE__);
			return KAL_FALSE;
		}
	    return KAL_TRUE;
    
}


int qma6981_ReadChipID(void)
{
	kal_uint8 chip_id = 0;
	chip_id = qma6981_reg_read(qma6981_CHIPID_REG);//read cm36682 id
	
	if(qma6981_CHIP_ID==chip_id)
	{
		//success (is CM36682)
	}
	kal_prompt_trace(MOD_WAP,"haiming  qma6981_ReadChipID %s,%d==%x",__FILE__,__LINE__,chip_id);
	return chip_id;
}

 static void qma6981_start_measure(void)
{

	unsigned char data[2];
	int err;
	err = qma6981_reg_write(CTL_REG_ONE, 0x1d);//continuous mode
	if(err != KAL_TRUE)
	{
		kal_prompt_trace(MOD_WAP," qma6981_start_measure write error line=%d\n",__LINE__);
	}

}
 static void qma6981_stop_measure()
{

	unsigned char data[2];
	int err;

	err = qma6981_reg_write(CTL_REG_ONE, 0x1c);//standby mode
	if(err != KAL_TRUE)
	{
		kal_prompt_trace(MOD_WAP," qma6981_stop_measure write error line=%d\n",__LINE__);
	}

}

static int qma6981_set_ratio(char ratio)
{
	int err = 0;


	err = qma6981_reg_write(RATIO_REG, 0x0e);//
	if(err != KAL_TRUE)
	{
		kal_prompt_trace(MOD_WAP," qma6981_set_ratio write error line=%d\n",__LINE__);
	}
	return err;
}
 int qma6981_enable(void)
{
	kal_prompt_trace(MOD_WAP,"haiming  qma6981_enable %s,%d",__FILE__,__LINE__);
	qma6981_start_measure();

	//qma6981_set_range(qma6981_RNG_20G);
	qma6981_set_ratio(4);				//the ratio must not be 0, different with qmc5983


	return 0;
}

static int qma6981_disable(void)
{
	
	qma6981_stop_measure();

	return 0;
}

static int qma6981_read_acc_xyz(kal_int16 *data)
{
	int res;
    unsigned char i;
	kal_uint8 acc_data[6];
	kal_uint8 databuf[6];
    kal_uint16 data_raw[3]= {0};
	

    

	

	

	

	res = qma6981_reg_read_buf(QMA6981_XOUTL, databuf, 6);
	if(!res)
      {
		kal_prompt_trace(MOD_WAP,"[I]----[%s]: i2c error\r\n", __func__);
		//return -EFAULT;
	}
	for(i=0;i<6;i++)
		acc_data[i]=databuf[i];
	
kal_prompt_trace(MOD_WAP,"haiming  qma6981 acc_data[%02x, %02x, %02x, %02x, %02x, %02x]",
		acc_data[0], acc_data[1], acc_data[2],
		acc_data[3], acc_data[4], acc_data[5]);
/*  
    databuf[0]=XL  reg_01 ��2λ�����ݵ� DX<1:0>
    databuf[1]=XH  reg_02      �����ݵ� DX<9:2>
*/
	data_raw[0] = (kal_int16)((databuf[1]<<2) |( databuf[0]>>6));
	data_raw[1] = (kal_int16)((databuf[3]<<2) |( databuf[2]>>6));
	data_raw[2] = (kal_int16)((databuf[5]<<2) |( databuf[4]>>6));
    kal_prompt_trace(MOD_WAP,"[I]----[%s]: raw:(X=%d,Y=%d,Z=%d).\r\n", __func__, data_raw[0], data_raw[1], data_raw[2]);

    data[0]=data_raw[0];
    data[1]=data_raw[1];
    data[2]=data_raw[2];


	
	return res;
}

int get_gsensor(kal_int16 *x,kal_int16 *y,kal_int16 *z)
{
	kal_int16 data[3];

	qma6981_read_acc_xyz((kal_int16 *)&data[0]);
	*x = data[0];
	*y = data[1];
	*z = data[2];
}

void qma6981_test_read_data(void)
{
    kal_uint16 data_mag[3];
     StopTimer(TIMER_ID_COMPASS_GET_DATA);
     qma6981_read_acc_xyz(data_mag);
     StartTimer(TIMER_ID_COMPASS_GET_DATA, 1000*20, qma6981_test_read_data);  

}

int qma6981_init(void )
{
	int ret=0;
	kal_uint16 data_acc[3];
	
	qma6981_i2c_configure(qmc6981_I2C_ADDR,qma6981_I2C_SPEED);
	qma6981_ReadChipID();


    //init
    qma6981_reg_write(0x36, 0xb6);

    qma6981_reg_write(0x0f, 0x04); // 0x01 :+-2g(1g=256)    0x04:+-8g(1g=64)
    qma6981_reg_write(0x12, 0x0f);
    qma6981_reg_write(0x27, 0x00);
    qma6981_reg_write(0x28, 0x00);
    qma6981_reg_write(0x29, 0x00);

   qma6981_reg_write(0x19, 0x00);
   qma6981_reg_write(0x16, 0x00);
  
   qma6981_reg_write(0x20, 0x00); // �͵�ƽ�����½��ش���
   


    // odr
   qma6981_reg_write(0x10, 0x06); // 6:ODR��500HZ����  5:250HZ  ������6��5��̫����
	// TAP_QUIET<7>: tap quiet time, 1: 30ms, 0: 20ms 
	// TAP_SHOCK<6>: tap shock time, 1: 50ms, 0: 75ms
	// TAP_DUR<2:0>: the time window of the second tap event for double tap
	
	//TAP_DUR			Duration of TAP_DUR
	//000					50ms
	//001					100ms
	//010					150ms
	//011					200ms
	//100					250ms
	//101					375ms
	//110					500ms
	//111					700ms
	
    qma6981_reg_write(0x2A, 0x84); // 

	// TAP_TH<4:0>
	// 62.5*9=562.5 mg, TAP_TH is 62.5mg in 2g-range, 125mg in 4g-range, 250mg in 8g-range.	
     qma6981_reg_write(0x2B, 0x18); // 
	
	// register 0x16 bit5 S_TAP_EN, bit4 D_TAP_EN
     qma6981_reg_write(0x16, 0x20); // 
    
	// register 0x16 bit5 INT1_S_TAP,  bit4 INT1_D_TAP
    qma6981_reg_write(0x19, 0x20); // 
     qma6981_reg_write(0x11, 0x80); 

    

	//qma6981_read_acc_xyz(data_acc);  //haiming �ճ�ʼ���������������0����Ҫ��һ�����ȥ��

    //kal_prompt_trace(MOD_WAP,"[I]----[%s]: raw:(X=%d,Y=%d,Z=%d).\r\n", __func__, data_acc[0], data_acc[1], data_acc[2]);

       StartTimer(TIMER_ID_COMPASS_GET_DATA, 1000*20, qma6981_test_read_data); 
	
	return ret;
}

#endif

