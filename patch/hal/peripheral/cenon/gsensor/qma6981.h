/*  $Date: 2009/10/23 $
 *  $Revision: 1.2 $
 */

/** \mainpage qma6981 Pressure Sensor API
 * Copyright (C) 2009 Bosch Sensortec GmbH
 *  \section intro_sec Introduction
 * qma6981 digital Altimeter Programming Interface
 * The qma6981 API enables quick access to Bosch Sensortec's digital altimeter.
 * The only mandatory steps are: 
 *
 * 1. linking the target application's communication functions to the API (\ref qma6981_WR_FUNC_PTR, \ref qma6981_RD_FUNC_PTR)
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

 /** \file qma6981.h
    \brief Header file for all #define constants and function prototypes
*/
#ifndef __qma6981_H__
#define __qma6981_H__


/*
	CHIP_TYPE CONSTANTS
*/

#define qma6981_CHIP_ID			0xB0


/*
	qmc6981 I2C Address
*/
#define qmc6981_I2C_ADDR		(0x24) //(0x2c<<1)

/* 
 *	
 *	register definitions 	
 *
 */

#define qma6981_I2C_SPEED		100
#define qma6981_CHIP_ID    (0XFF)
#define qma6981_CHIPID_REG  0x00
/* Magnetometer full scale  */
#define qma6981_RNG_2G		0x00
#define qma6981_RNG_8G		0x01
#define qma6981_RNG_12G		0x02
#define qma6981_RNG_20G		0x03
/* Contrl register one */
#define CTL_REG_ONE	0x09  
/*different from qma6981,the ratio register*/
#define RATIO_REG		0x0b

/*Status registers */
#define STA_REG_ONE    0x06
#define STA_REG_TWO    0x0c

/*data output register*/
#define QMA6981_XOUTL			0x01	// 4-bit output value X
#define QMA6981_XOUTH			0x02	// 6-bit output value X
#define QMA6981_YOUTL			0x03	
#define QMA6981_YOUTH			0x04	
#define QMA6981_ZOUTL			0x05	
#define QMA6981_ZOUTH			0x06	

#define RNG_2G		2
#define RNG_8G		8
#define RNG_12G		12
#define RNG_20G		20


/* General Setup Functions */
//int qma6981_init(void);




#endif   // __qma6981_H__





