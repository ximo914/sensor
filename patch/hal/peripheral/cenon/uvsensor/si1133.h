/*  $Date: 2009/10/23 $
 *  $Revision: 1.2 $
 */

/** \mainpage si113 Pressure Sensor API
 * Copyright (C) 2009 Bosch Sensortec GmbH
 *  \section intro_sec Introduction
 * si113 digital Altimeter Programming Interface
 * The si113 API enables quick access to Bosch Sensortec's digital altimeter.
 * The only mandatory steps are: 
 *
 * 1. linking the target application's communication functions to the API (\ref si113_WR_FUNC_PTR, \ref si113_RD_FUNC_PTR)
 * *
 *
 * 
 * \section disclaimer_sec Disclaimer
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
* The resale and/or use of the Work are at the purchaser’s own risk and his own responsibility. The 
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

 /** \file si113.h
    \brief Header file for all #define constants and function prototypes
*/


#ifndef __SI1133_H__
#define __SI1133_H__
 
#define SI1133_I2C_SPEED		100
 
#define SI1133_PART_ID				  0xAA
 
//*******************************************************//
//*                   ADCCONFIGx                        *//
//*******************************************************//
 
//=========================================================
//  measurement rate,Offsets for Parameter Table ADCCONFIGx
//=========================================================
#define CFG_RATE_SHORT  		  0X60 //24.4us
#define CFG_RATE_NORMAL 		  0X00 //48.8us
#define CFG_RATE_LONG   		  0X20 //97.6us
#define CFG_RATE_VLONG  		  0X40 //195us
//==========================================================
// Photodiodos select,Offsets for Parameter Table ADCCONFIGx
//==========================================================
#define CFG_SMALL_IR 			  0X00 
#define CFG_MEDIUM_IR			  0X01
#define CFG_LARGE_IR			  0X02
#define CFG_WHITE   			  0X0B
#define CFG_LARGE_WHITE 		  0X0C
#define CFG_UV  				  0X18
#define CFG_UV_DEEP 			  0X19
 
//------------------------END------------------------------
 
//*******************************************************//
//*                   MEASCONFIGx                       *//
//*******************************************************//
 
//===========================
// MEASCOUNTER select,Value for Parameter Table MEASCONFIGx
//===========================
#define MEAS_COUNT_NONE 		  0X00
#define MEAS_COUNT_0 			  0X40
#define MEAS_COUNT_1 			  0X80
#define MEAS_COUNT_2 			  0XC0
 
//------------------------END------------------------------
 
//*******************************************************//
//*                      ADCPOSTx                       *//
//*******************************************************//
//===========================
// OUTPUT setting,Offsets for Parameter Table ADCPOSTx
//===========================
#define POST_BITS_16			  0X00
#define POST_BITS_24			  0X40
//===========================
// THRESHOLD setting,Offsets for Parameter Table ADCPOSTx
//===========================
#define POST_THRESHOLD_NONE		  0X00
#define POST_THRESHOLD_0		  0X01
#define POST_THRESHOLD_1		  0X02
#define POST_THRESHOLD_2		  0X03
//------------------------END------------------------------
 
//===========================
// I2C Registers Address
//===========================
#define REG_PART_ID               0x00
#define REG_REV_ID                0x01
#define REG_MFR_ID                0x02
#define REG_INFO0             	  0x03
#define REG_INFO1                 0x04
 
#define REG_HOSTIN3               0x07
#define REG_HOSTIN2               0x08
#define REG_HOSTIN1               0x09
#define REG_HOSTIN0               0x0A
#define REG_COMMAND               0x0B
 
#define REG_IRQ_ENABLE            0x0F
#define REG_RESET                 0x0F
#define REG_RESPONSE1             0x10
#define REG_RESPONSE0             0x11
#define REG_IRQ_STATUS            0x12
 
#define REG_HOSTOUT0              0x13
#define REG_HOSTOUT1              0x14
#define REG_HOSTOUT2              0x15
#define REG_HOSTOUT3              0x16
#define REG_HOSTOUT4              0x17
#define REG_HOSTOUT5              0x18
#define REG_HOSTOUT6              0x19
#define REG_HOSTOUT7              0x1A
#define REG_HOSTOUT8              0x1B
#define REG_HOSTOUT9              0x1C
#define REG_HOSTOUT10             0x1D
#define REG_HOSTOUT11             0x1E
#define REG_HOSTOUT12             0x1F
#define REG_HOSTOUT13             0x20
#define REG_HOSTOUT14             0x21
#define REG_HOSTOUT15             0x22
#define REG_HOSTOUT16             0x23
#define REG_HOSTOUT17             0x24
#define REG_HOSTOUT18             0x25
#define REG_HOSTOUT19             0x26
#define REG_HOSTOUT20             0x27
#define REG_HOSTOUT21             0x28
#define REG_HOSTOUT22             0x29
#define REG_HOSTOUT23             0x2A
#define REG_HOSTOUT24             0x2B
#define REG_HOSTOUT25             0x2C
//============================================================
// Parameter Table Offsets
//============================================================
#define PARAM_I2C_ADDR            0x00
#define PARAM_CHAN_LIST           0x01
 
#define PARAM_ADCCONFIG0          0x02
#define PARAM_ADCSENS0            0x03
#define PARAM_ADCPOST0            0x04
#define PARAM_MEASCONFIG0         0x05
 
#define PARAM_ADCCONFIG1          0x06
#define PARAM_ADCSENS1            0x07
#define PARAM_ADCPOST1            0x08
#define PARAM_MEASCONFIG1         0x09
 
#define PARAM_ADCCONFIG2          0x0A
#define PARAM_ADCSENS2            0x0B
#define PARAM_ADCPOST2            0x0C
#define PARAM_MEASCONFIG2         0x0D
 
#define PARAM_ADCCONFIG3          0x0E
#define PARAM_ADCSENS3            0x0F
#define PARAM_ADCPOST3            0x10
#define PARAM_MEASCONFIG3         0x11
 
#define PARAM_ADCCONFIG4          0x12
#define PARAM_ADCSENS4            0x13
#define PARAM_ADCPOST4            0x14
#define PARAM_MEASCONFIG4         0x15
 
#define PARAM_ADCCONFIG5          0x16
#define PARAM_ADCSENS5            0x17
#define PARAM_ADCPOST5            0x18
#define PARAM_MEASCONFIG5         0x19
 
#define PARAM_MEASRATE_H          0x1A
#define PARAM_MEASRATE_L          0x1B
#define PARAM_MEASCOUNT0          0x1C
#define PARAM_MEASCOUNT1          0x1D
#define PARAM_MEASCOUNT2          0x1E
 
#define PARAM_LED1_A        	  0x1F
#define PARAM_LED1_B        	  0x20
#define PARAM_LED2_A        	  0x21
#define PARAM_LED2_B        	  0x22
#define PARAM_LED3_A        	  0x23
#define PARAM_LED3_B        	  0x24
 
#define PARAM_THRESHOLD0_H  	  0x25
#define PARAM_THRESHOLD0_L  	  0x26
#define PARAM_THRESHOLD1_H  	  0x27
#define PARAM_THRESHOLD1_L  	  0x28
#define PARAM_THRESHOLD2_H  	  0x29
#define PARAM_THRESHOLD2_L  	  0x2A
#define PARAM_BURST         	  0x2B
 
//==================================================================
//COMMAND Register Value
//==================================================================
#define CMD_RESET_CTR			  0x00
#define CMD_RESET_SW			  0x01
#define CMD_FORCE				  0x11
#define CMD_PAUSE				  0x12
#define CMD_START				  0x13
#define CMD_WRITE				  0x80
#define CMD_READ				  0x40
 
/*******************************************************************************
 *******    Si115x Register and Parameter Bit Definitions  *********************
 ******************************************************************************/
#define RSP0_CHIPSTAT_MASK      0xe0
#define RSP0_COUNTER_MASK       0x1f
#define RSP0_SLEEP              0x20
 
extern kal_uint16 SI1133_ReadRegister(kal_uint8 addr);
extern kal_uint16 SI1133_WriteRegister(kal_uint8 addr, kal_uint8 data);
extern kal_uint16 SI1133_ReadParameter(kal_uint8 addr);
extern kal_uint16 SI1133_WriteParameter(kal_uint8 addr, kal_uint8 data);
extern kal_uint16 SI1133_Init(void);
extern kal_uint16 SI1133_NOP(void);
extern kal_uint16 SI1133_Force(void);
extern kal_uint16 SI1133_Start (void);
extern kal_uint16 SI1133_Pause(void);
 
#endif








