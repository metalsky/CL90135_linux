/* *
 * Copyright (C) 2006 Texas Instruments Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* dm355_af.h file */
#ifndef AF_DM355_DRIVER_H
#define AF_DM355_DRIVER_H

/* Kernel Header files */
#include <linux/ioctl.h>

#ifdef __KERNEL__
#include <linux/kernel.h>	/* printk  */
#include <linux/wait.h>		/* Wait queue */
#include <asm/semaphore.h>	/* Semphores */
#include <asm/io.h>
#endif				/*End of __KERNEL_ */

#ifdef __KERNEL__
/* Device Constants */
#define AF_MAJOR_NUMBER                 0
#define DEVICE_NAME                     "dm355_af"
#define AF_NR_DEVS                      1
#define AF_TIMEOUT                      (300*HZ)/1000
#endif				/*enf of #ifdef __KERNEL__ */

/* Range Constants */

#define AF_PAXEL_HORIZONTAL_COUNT_MIN       1
#define AF_PAXEL_HORIZONTAL_COUNT_MAX       36

#define AF_PAXEL_VERTICAL_COUNT_MIN         1
#define AF_PAXEL_VERTICAL_COUNT_MAX         128

#define AF_PAXEL_SIZE                       48

#define AF_WIDTH_MIN                        8
#define AF_WIDTH_MAX                        256

#define AF_LINE_INCR_MIN                    2
#define AF_LINE_INCR_MAX                    32

#define AF_HEIGHT_MIN                       2
#define AF_HEIGHT_MAX                       256

#define AF_HZSTART_MIN                      2
#define AF_HZSTART_MAX                      4094

#define AF_VTSTART_MIN                      0
#define AF_VTSTART_MAX                      4095

#define AF_MEDTH_MAX                        255

#define AF_IIRSH_MAX                        4094
#define AF_COEF_MIN                         0xFFFFF800
#define AF_COEF_MAX                         0x000007FF


//#define IRQ_H3AINT  3
#define AF_NUMBER_OF_COEF                   11


/* list of ioctls */
#pragma pack(1)
#define  AF_IOC_MAXNR       5
#define  AF_MAGIC_NO        'a'
#define  AF_S_PARAM         _IOWR(AF_MAGIC_NO,1,struct af_configuration *)
#define  AF_G_PARAM         _IOWR(AF_MAGIC_NO,2,struct af_configuration *)
#define  AF_ENABLE          _IO(AF_MAGIC_NO,3)
#define  AF_DISABLE         _IO(AF_MAGIC_NO,4)
#pragma  pack()

/* enum used for status of specific feature */
typedef enum {
	H3A_AF_DISABLE = 0,
	H3A_AF_ENABLE = 1
} af_alaw_enable, af_hmf_enable;
typedef enum {
	H3A_AF_CONFIG_NOT_DONE,
	H3A_AF_CONFIG
} af_config_flag;

struct af_reg_dump {
	unsigned int addr;
	unsigned int val;
};

/* enum used for keep track of whether hardware is used */
typedef enum {
	AF_NOT_IN_USE = 0,
	AF_IN_USE = 1
} af_In_use;

typedef enum {
	ACCUMULATOR_SUMMED = 0,
	ACCUMULATOR_PEAK = 1
} af_mode;

/* Red, Green, and blue pixel location in the AF windows */
typedef enum {
	GR_GB_BAYER = 0,	/* GR and GB as Bayer pattern */
	RG_GB_BAYER = 1,	/* RG and GB as Bayer pattern */
	GR_BG_BAYER = 2,	/* GR and BG as Bayer pattern */
	RG_BG_BAYER = 3,	/* RG and BG as Bayer pattern */
	GG_RB_CUSTOM = 4,	/* GG and RB as custom pattern */
	RB_GG_CUSTOM = 5	/* RB and GG as custom pattern */
} rgbpos;

/* Contains the information regarding the Horizontal Median Filter */
struct af_hmf {
	af_hmf_enable enable;	/* Status of Horizontal Median Filter */
	unsigned int threshold;	/* Threshhold Value for */
	/*Horizontal Median Filter */
};

/* Contains the information regarding the IIR Filters */
struct af_iir {
	unsigned int hz_start_pos;	/* IIR Start Register Value */
	int coeff_set0[AF_NUMBER_OF_COEF];	/* IIR Filter Coefficient for Set 0 */
	int coeff_set1[AF_NUMBER_OF_COEF];	/* IIR Filter Coefficient for Set 1 */
};

/* Contains the information regarding the Paxels Structure in AF Engine */
struct af_paxel {
	unsigned int width;	/* Width of the Paxel */
	unsigned int height;	/* Height of the Paxel */
	unsigned int hz_start;	/* Horizontal Start Position */
	unsigned int vt_start;	/* Vertical Start Position */
	unsigned int hz_cnt;	/* Horizontal Count */
	unsigned int vt_cnt;	/* vertical Count */
	unsigned int line_incr;	/* Line Increment */
};


/* Contains the parameters required for hardware set up of AF Engine */
struct af_configuration {
	af_alaw_enable alaw_enable;	/*ALAW status */
	struct af_hmf hmf_config;	/*HMF configurations */
	rgbpos rgb_pos;		/*RGB Positions */
	struct af_iir iir_config;	/*IIR filter configurations */
	struct af_paxel paxel_config;	/*Paxel parameters */
	af_mode mode;		/*Accumulator mode */

};

#ifdef __KERNEL__
/* Structure for device of AF Engine */
struct af_device {
	af_In_use in_use;	/*Driver usage counter */
	struct af_configuration *config;	/*Device configuration structure */
	void *buff_old;		/*Contains the latest statistics */
	void *buff_curr;	/*Buffer in which HW will */
	/*fill the statistics */
	/*or HW is already filling statistics */
	void *buff_app;		/*Buffer which will be passed to */
	/* user space on read call */
	unsigned int buff_size;	/* Size of image buffer */
	int buffer_filled;	/*Flag indicates */
	/*statistics are available */
	int size_paxel;		/*Paxel size in bytes */
	wait_queue_head_t af_wait_queue;	/*Wait queue for driver */
	struct semaphore read_blocked;	/* Semaphore for driver */
	af_config_flag af_config;	/*Flag indicates Engine is configured */

};

#endif				/* __KERNEL__ */

#endif				/* AF_DM355_DRIVER_H */
