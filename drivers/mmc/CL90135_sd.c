/*
 * linux/drivers/mmc/CL90135_sd.c
 *
 * CL90135 sd via fpga controller file
 *
 * Copyright (C) 2006 Cielle srl.
 *
 * ----------------------------------------------------------------------------
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 * Modifications:
 * ver. 1.0: 26 Sep 2011 first release
 */

//#define defVerbosePrintK    

// usare questa define epr abilitare i messaggi di debug dev_dbg
//#define DEBUG 

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/mmc/host.h>
#include <linux/mmc/protocol.h>
#include <linux/mmc/card.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/scatterlist.h>

#include <asm/arch/mmc.h>
#include <asm/arch/edma.h>
#include <asm/arch/gpio.h>

#include "CL90135_fpga.h"
//
//  MICHELE: DEFINIZIONE GPIO 5_6 COME INGRESSO INTERRUPT SD CARD COMANDATO DA FPGA
// CL90135 SD CARD INTERRUPT PIN = GPIO5_6 = T5 = IO_L04N_2
//
#define SD_CARD_INTERRUPT_PIN	GPIO(5*16+6)	// GPIO5[6]


/* This macro could not be defined to 0 (ZERO) or -ve value.
 * This value is multiplied to "HZ"
 * while requesting for timer interrupt every time for probing card.
 */
#define MULTIPLIER_TO_HZ 1


#define RSP_TYPE(x)	((x) & ~(MMC_RSP_BUSY|MMC_RSP_OPCODE))
// deve essere assolutamente identico a quello dichiarato in device.c altrimenti non funziona niente, nel senso che il probe non viene mai chiamato
#define DRIVER_NAME "CL90135_sd"



// usare questa define per disabilitare l'uso del dma per la cl90135
#define defCL90135DisableDma


struct mmc_davinci_host {
	struct resource 	*reg_res;
	spinlock_t		mmc_lock;
	int			is_card_busy;
	int			is_card_detect_progress;
	int			is_card_initialized;
	int			is_card_removed;
	int			is_init_progress;
	int			is_req_queued_up;
	struct mmc_host		*que_mmc_host;
	struct mmc_request	*que_mmc_request;
	struct mmc_command	*cmd;
	struct mmc_data		*data;
	struct mmc_host		*mmc;
	struct device		*dev;
	struct clk		*clk;
	struct resource		*mem_res;
	void __iomem		*virt_base;
	unsigned int		phys_base;
	unsigned int		rw_threshold;
	unsigned int		option_read;
	unsigned int		option_write;
	int			flag_sd_mmc;
	int			irq;
	unsigned char		bus_mode;
	unsigned long ulNumBytesRead;
	unsigned long ulTotalNumBytesToRead;

#define DAVINCI_MMC_DATADIR_NONE	0
#define DAVINCI_MMC_DATADIR_READ	1
#define DAVINCI_MMC_DATADIR_WRITE	2
	unsigned char		data_dir;
	u32			*buffer;
	u32			bytes_left;

	int			use_dma;
	int			do_dma;
	unsigned int		dma_rx_event;
	unsigned int		dma_tx_event;

	struct timer_list	timer;
	unsigned int		is_core_command;
	unsigned int		cmd_code;
	unsigned int		old_card_state;
	unsigned int		new_card_state;

	unsigned char		sd_support;

#ifndef defCL90135DisableDma
	struct edma_ch_mmcsd	edma_ch_details;
#endif	

	unsigned int		sg_len;
	int			sg_idx;
	unsigned int		buffer_bytes_left;
	unsigned char		pio_set_dmatrig;
	int 			(*get_ro) (int);
};

// valori possibili del primo parametro in ingresso a uiCL90135_SDresult
typedef enum{
    enumSdResult_VerifyAllErrors=0,
    enumSdResult_IgnoreSdCrcError,
    enumSdResult_IgnoreTimeoutError,
    enumSdResult_NumOfparams
}enumSdResult_paramvalue;

// contains r1-like reply datas
TipoStructSdR1_union r1;
// contains r2 reply datas
TipoStructSdR2_union r2;

// funzione che crea un piccolo ritardo
void aDelay(struct mmc_davinci_host *host){
    static int i;
    static volatile short int iTheVersionLow;
	for (i=0;i<10;i++)
		iTheVersionLow=CL90135_MMC_READW(host, defCL90135_EMIFA_address_version_low);
}

// attende che sd card termini operazione in corso
// Ingresso: 1 se errore crc deve essere ignorato
//           0 se errore crc NON deve essere ignorato
// restituisce 1 se operazione terminata correttamente
// restituisce 0 se operazione terminata con errore
unsigned int uiCL90135_SDresult(struct mmc_davinci_host *host,unsigned int uiTreatErrors){
	int i;
	unsigned int ui;
	// sd result
    TipoStructSdCommandStatus sdCommandStatus;

	i=1e6;
	while(i){
		i--;
        ui=CL90135_MMC_READW(host, defCL90135_EMIFA_sdcard_result);		//		ui=*defCL90135_EMIFA_sdcard_result;
		// chiamo ritardo perché ad es le operazioni di read o write possono occupare parecchio tempo
		aDelay(host);
		memcpy(&sdCommandStatus,&ui,sizeof(ui));
		if (sdCommandStatus.uiBusy==0)
			break;
	}
	// timeout???
	if (!i)
		return 0;
    if (    (sdCommandStatus.uiCrcErr&&(uiTreatErrors!=enumSdResult_IgnoreSdCrcError))
		||	sdCommandStatus.uiStopbitErr
		||	(sdCommandStatus.uiTimeoutErr&&(uiTreatErrors!=enumSdResult_IgnoreTimeoutError))
		||	sdCommandStatus.uiCmdRunningErr
	   ){
	   return 0;
	}
	return 1;
}//unsigned int uiCL90135_SDresult(void)


// lettura del core u32 della risposta r1like; restituisce i 32 bit centrali di una risposta r1, r1b, r3, r4, r5, r6
u32 u32_read_R1_like_sdResponse(struct mmc_davinci_host *host){
 
    r1.u.b48_group16.uiBit15_0 =CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_0);
    r1.u.b48_group16.uiBit31_16=CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_1);
    r1.u.b48_group16.uiBit47_32=CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_2);
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd R1: 0-15:%X 16-31:%X 32-47:%X.\n"
	            ,(unsigned int)r1.u.b48_group16.uiBit15_0
	            ,(unsigned int)r1.u.b48_group16.uiBit31_16
	            ,(unsigned int)r1.u.b48_group16.uiBit47_32
	      );
#endif	

//    r1.u.b48_group16.uiBit15_0 =*defCL90135_EMIFA_sdcard_resp_body_0;
//    r1.u.b48_group16.uiBit31_16=*defCL90135_EMIFA_sdcard_resp_body_1;
//    r1.u.b48_group16.uiBit47_32=*defCL90135_EMIFA_sdcard_resp_body_2;
    return r1.u.r1_core.u32_core;
}//u32 u32_read_R1_like_sdResponse(void)

// lettura del core della risposta r2
void v_read_R2_like_sdResponse(struct mmc_davinci_host *host,u32 *pu32){
    r2.u.b144_group16.uiBit15_0   =CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_0);
    r2.u.b144_group16.uiBit31_16  =CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_1);
    r2.u.b144_group16.uiBit47_32  =CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_2);
    r2.u.b144_group16.uiBit63_48  =CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_3);
    r2.u.b144_group16.uiBit79_64  =CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_4);
    r2.u.b144_group16.uiBit95_80  =CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_5);
    r2.u.b144_group16.uiBit111_96 =CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_6);
    r2.u.b144_group16.uiBit127_112=CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_7);
    r2.u.b144_group16.uiBit143_128=CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_resp_body_8);
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd resp 15_0    %04X.\n",(int)r2.u.b144_group16.uiBit15_0);
	printk(KERN_WARNING "CL90135_sd resp 31_16   %04X.\n",(int)r2.u.b144_group16.uiBit31_16);
	printk(KERN_WARNING "CL90135_sd resp 47_32   %04X.\n",(int)r2.u.b144_group16.uiBit47_32);
	printk(KERN_WARNING "CL90135_sd resp 63_48   %04X.\n",(int)r2.u.b144_group16.uiBit63_48);
	printk(KERN_WARNING "CL90135_sd resp 79_64   %04X.\n",(int)r2.u.b144_group16.uiBit79_64);
	printk(KERN_WARNING "CL90135_sd resp 95_80   %04X.\n",(int)r2.u.b144_group16.uiBit95_80);
	printk(KERN_WARNING "CL90135_sd resp 111_96  %04X.\n",(int)r2.u.b144_group16.uiBit111_96);
	printk(KERN_WARNING "CL90135_sd resp 127_112 %04X.\n",(int)r2.u.b144_group16.uiBit127_112);
	printk(KERN_WARNING "CL90135_sd resp 143_128 %04X.\n",(int)r2.u.b144_group16.uiBit143_128);
#endif	


//    r2.u.b144_group16.uiBit15_0   =*defCL90135_EMIFA_sdcard_resp_body_0;
//    r2.u.b144_group16.uiBit31_16  =*defCL90135_EMIFA_sdcard_resp_body_1;
//    r2.u.b144_group16.uiBit47_32  =*defCL90135_EMIFA_sdcard_resp_body_2;
//    r2.u.b144_group16.uiBit63_48  =*defCL90135_EMIFA_sdcard_resp_body_3;
//    r2.u.b144_group16.uiBit79_64  =*defCL90135_EMIFA_sdcard_resp_body_4;
//    r2.u.b144_group16.uiBit95_80  =*defCL90135_EMIFA_sdcard_resp_body_5;
//    r2.u.b144_group16.uiBit111_96 =*defCL90135_EMIFA_sdcard_resp_body_6;
//    r2.u.b144_group16.uiBit127_112=*defCL90135_EMIFA_sdcard_resp_body_7;
//    r2.u.b144_group16.uiBit143_128=*defCL90135_EMIFA_sdcard_resp_body_8;

// a quanto sembra Linux vuole la risposta nell'ordine inverso rispetto a quella descritta nel datasheet della sd card
    pu32[3]=r2.u.core_u32.the_core[0];
    pu32[2]=r2.u.core_u32.the_core[1];
    pu32[1]=r2.u.core_u32.the_core[2];
    pu32[0]=r2.u.core_u32.the_core[3];
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd resp csd 0 %08X.\n",(int)pu32[0]);
	printk(KERN_WARNING "CL90135_sd resp csd 1 %08X.\n",(int)pu32[1]);
	printk(KERN_WARNING "CL90135_sd resp csd 2 %08X.\n",(int)pu32[2]);
	printk(KERN_WARNING "CL90135_sd resp csd 3 %08X.\n",(int)pu32[3]);
#endif	
}//void v_read_R2_like_sdResponse(u32 *pu32)

// inizializzazione a livello basso dell'sd card
static unsigned int init_CL90135_sd_host(struct mmc_davinci_host *host){
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd low level init.\n");
#endif	
    
    // reset read and write sd block, reset sd fpga module
    defCL90135_Issue_complete_reset;
    // seleziono SD clock speed bassa
	defCL90135_EMIFA_sdcard_speedSelect_Lo;
    // comando inizializzazione sd card...
	// vengono generati N impulsi di clock che permettono ad sdcard di inizializzarsi
	CL90135_MMC_WRITEW(host, defCL90135_EMIFA_sdcard_init,0);
//	*defCL90135_EMIFA_sdcard_init=0;
	// aspetto fine comando
    if (!uiCL90135_SDresult(host,enumSdResult_VerifyAllErrors))
        return 0;
    return 1;
}//static unsigned int init_CL90135_sd_host(struct mmc_davinci_host *host)




static void davinci_reinit_chan(struct mmc_davinci_host *host)
{
#if 0
	davinci_stop_dma(host->dma_tx_event);
	davinci_clean_channel(host->dma_tx_event);

	davinci_stop_dma(host->dma_rx_event);
	davinci_clean_channel(host->dma_rx_event);
#endif
}

#ifndef defCL90135DisableDma


    static int mmc_davinci_send_dma_request(struct mmc_davinci_host *host,
                        struct mmc_request *req)
    {
        int sync_dev;
        unsigned char i, j;
        unsigned short acnt, bcnt, ccnt;
        unsigned int src_port, dst_port, temp_ccnt;
        enum address_mode mode_src, mode_dst;
        enum fifo_width fifo_width_src, fifo_width_dst;
        unsigned short src_bidx, dst_bidx;
        unsigned short src_cidx, dst_cidx;
        unsigned short bcntrld;
        enum sync_dimension sync_mode;
        struct paramentry_descriptor temp;
        int edma_chan_num;
        struct mmc_data *data = host->data;
        struct scatterlist *sg = &data->sg[0];
        unsigned int count;
        int num_frames, frame;

    #define MAX_C_CNT       64000

        frame = data->blksz;
        count = sg_dma_len(sg);

        if ((data->blocks == 1) && (count > data->blksz))
            count = frame;

        if (count % host->rw_threshold == 0) {
            acnt = 4;
            bcnt = host->rw_threshold / 4;
            num_frames = count / host->rw_threshold;
        } else {
            acnt = count;
            bcnt = 1;
            num_frames = 1;
        }

        if (num_frames > MAX_C_CNT) {
            temp_ccnt = MAX_C_CNT;
            ccnt = temp_ccnt;
        } else {
            ccnt = num_frames;
            temp_ccnt = ccnt;
        }

        if (host->data_dir == DAVINCI_MMC_DATADIR_WRITE) {
            /*AB Sync Transfer */
            sync_dev = host->dma_tx_event;

            src_port = (unsigned int)sg_dma_address(sg);
            mode_src = INCR;
            fifo_width_src = W8BIT; /* It's not cared as modeDsr is INCR */
            src_bidx = acnt;
            src_cidx = acnt * bcnt;
            dst_port = host->phys_base + DAVINCI_MMC_REG_DXR;
            mode_dst = INCR;
            fifo_width_dst = W8BIT; /* It's not cared as modeDsr is INCR */
            dst_bidx = 0;
            dst_cidx = 0;
            bcntrld = 8;
            sync_mode = ABSYNC;
        } else {
            sync_dev = host->dma_rx_event;

            src_port = host->phys_base + DAVINCI_MMC_REG_DRR;
            mode_src = INCR;
            fifo_width_src = W8BIT;
            src_bidx = 0;
            src_cidx = 0;
            dst_port = (unsigned int)sg_dma_address(sg);
            mode_dst = INCR;
            fifo_width_dst = W8BIT; /* It's not cared as modeDsr is INCR */
            dst_bidx = acnt;
            dst_cidx = acnt * bcnt;
            bcntrld = 8;
            sync_mode = ABSYNC;
        }

        if (host->pio_set_dmatrig)
            davinci_dma_clear_event(sync_dev);
        davinci_set_dma_src_params(sync_dev, src_port, mode_src,
                    fifo_width_src);
        davinci_set_dma_dest_params(sync_dev, dst_port, mode_dst,
                        fifo_width_dst);
        davinci_set_dma_src_index(sync_dev, src_bidx, src_cidx);
        davinci_set_dma_dest_index(sync_dev, dst_bidx, dst_cidx);
        davinci_set_dma_transfer_params(sync_dev, acnt, bcnt, ccnt, bcntrld,
                        sync_mode);

        davinci_get_dma_params(sync_dev, &temp);
        if (sync_dev == host->dma_tx_event) {
            if (host->option_write == 0) {
                host->option_write = temp.opt;
            } else {
                temp.opt = host->option_write;
                davinci_set_dma_params(sync_dev, &temp);
            }
        }
        if (sync_dev == host->dma_rx_event) {
            if (host->option_read == 0) {
                host->option_read = temp.opt;
            } else {
                temp.opt = host->option_read;
                davinci_set_dma_params(sync_dev, &temp);
            }
        }

        if (host->sg_len > 1) {
            davinci_get_dma_params(sync_dev, &temp);
            temp.opt &= ~TCINTEN;
            davinci_set_dma_params(sync_dev, &temp);

            for (i = 0; i < host->sg_len - 1; i++) {

                sg = &data->sg[i + 1];

                if (i != 0) {
                    j = i - 1;
                    davinci_get_dma_params(host->edma_ch_details.
                                chanel_num[j], &temp);
                    temp.opt &= ~TCINTEN;
                    davinci_set_dma_params(host->edma_ch_details.
                                chanel_num[j], &temp);
                }

                edma_chan_num = host->edma_ch_details.chanel_num[0];

                frame = data->blksz;
                count = sg_dma_len(sg);

                if ((data->blocks == 1) && (count > data->blksz))
                    count = frame;

                ccnt = count / host->rw_threshold;

                if (sync_dev == host->dma_tx_event)
                    temp.src = (unsigned int)sg_dma_address(sg);
                else
                    temp.dst = (unsigned int)sg_dma_address(sg);

                temp.opt |= TCINTEN;

                temp.ccnt = (temp.ccnt & 0xFFFF0000) | (ccnt);

                davinci_set_dma_params(edma_chan_num, &temp);
                if (i != 0) {
                    j = i - 1;
                    davinci_dma_link_lch(host->edma_ch_details.
                                chanel_num[j],
                                edma_chan_num);
                }
            }
            davinci_dma_link_lch(sync_dev,
                        host->edma_ch_details.chanel_num[0]);
        }

        davinci_start_dma(sync_dev);
        return 0;
    }

    static int mmc_davinci_start_dma_transfer(struct mmc_davinci_host *host,
                        struct mmc_request *req)
    {
        int use_dma = 1, i;
        struct mmc_data *data = host->data;
        int block_size = (1 << data->blksz_bits);

        host->sg_len = dma_map_sg(host->mmc->dev, data->sg, host->sg_len,
                    ((data->
                        flags & MMC_DATA_WRITE) ? DMA_TO_DEVICE :
                    DMA_FROM_DEVICE));

        /* Decide if we can use DMA */
        for (i = 0; i < host->sg_len; i++) {
            if ((data->sg[i].length % block_size) != 0) {
                use_dma = 0;
                break;
            }
        }

        if (!use_dma) {
            dma_unmap_sg(host->mmc->dev, data->sg, host->sg_len,
                    (data->
                    flags & MMC_DATA_WRITE) ? DMA_TO_DEVICE :
                    DMA_FROM_DEVICE);
            return -1;
        }

        host->do_dma = 1;

        mmc_davinci_send_dma_request(host, req);

        return 0;

    }

    static int davinci_release_dma_channels(struct mmc_davinci_host *host)
    {
        davinci_free_dma(host->dma_tx_event);
        davinci_free_dma(host->dma_rx_event);

        if (host->edma_ch_details.cnt_chanel) {
            davinci_free_dma(host->edma_ch_details.chanel_num[0]);
            host->edma_ch_details.cnt_chanel = 0;
        }

        return 0;
    }
#endif

static void mmc_davinci_sg_to_buf(struct mmc_davinci_host *host)
{
	struct scatterlist *sg;

	sg = host->data->sg + host->sg_idx;
	host->buffer_bytes_left = sg->length;
	host->buffer = page_address(sg->page) + sg->offset;
	if (host->buffer_bytes_left > host->bytes_left)
		host->buffer_bytes_left = host->bytes_left;
}


static void CL90135_prepare_data(struct mmc_davinci_host *host,struct mmc_request *req){
	int timeout, sg_len;
	host->data = req->data;

    // se non è richiesto trasferimento dati in read/write... esco subito
	if (req->data == NULL) {
		host->data_dir = DAVINCI_MMC_DATADIR_NONE;
        return;
	}
	/* Init idx */
	host->sg_idx = 0;

	dev_dbg(host->mmc->dev,
		"MMCSD : Data xfer (%s %s), "
		"DTO %d cycles + %d ns, %d blocks of %d bytes\n",
		(req->data->flags & MMC_DATA_STREAM) ? "stream" : "block",
		(req->data->flags & MMC_DATA_WRITE) ? "write" : "read",
		req->data->timeout_clks, req->data->timeout_ns,
		req->data->blocks, 1 << req->data->blksz_bits);

    // Convert ns to clock cycles by assuming 20MHz frequency
    // 1 cycle at 20MHz = 500 ns
    timeout = req->data->timeout_clks + req->data->timeout_ns / 500;
	if (timeout > 0xffff)
		timeout = 0xffff;


    // comando reset stato buffers sd fpga...
    defCL90135_Issue_block_reset;


    // QUESTE SCRITTURE SONO SPECIFICHE PER IL CONTROLLER DAVINCI
    host->data_dir = (req->data->flags & MMC_DATA_WRITE) ? DAVINCI_MMC_DATADIR_WRITE : DAVINCI_MMC_DATADIR_READ;

    #if 0
        switch (host->data_dir) {
            case DAVINCI_MMC_DATADIR_WRITE:
                break;
            case DAVINCI_MMC_DATADIR_READ:
                break;
            default:
                break;
        }
    #endif

	sg_len = (req->data->blocks == 1) ? 1 : req->data->sg_len;
	host->sg_len = sg_len;

	host->bytes_left = req->data->blocks * (1 << req->data->blksz_bits);

#ifndef defCL90135DisableDma
    if (
           (host->use_dma == 1)
        && (host->bytes_left % host->rw_threshold == 0)
        && (mmc_davinci_start_dma_transfer(host, req) == 0)
        ){
		host->buffer = NULL;
		host->bytes_left = 0;
    } else
#endif
    {
        // Revert to CPU Copy
        host->do_dma = 0;
		mmc_davinci_sg_to_buf(host);
	}
}

static void davinci_fifo_data_trans(struct mmc_davinci_host *host)
{
	int n, i;
	unsigned int u1,u2;

	if (host->buffer_bytes_left == 0) {
		host->sg_idx++;
		BUG_ON(host->sg_idx == host->sg_len);
		mmc_davinci_sg_to_buf(host);
	}


    if (host->data_dir == DAVINCI_MMC_DATADIR_WRITE){
    
        
        n = host->rw_threshold;
        if (n > host->buffer_bytes_left)
            n = host->buffer_bytes_left;
        if (n > defCL90135_EMIFA_max_bytes_WriteBlock)
            n = defCL90135_EMIFA_max_bytes_WriteBlock;

        host->buffer_bytes_left -= n;
        host->bytes_left -= n;

        for (i = 0; i < (n / 4); i++) {
	        CL90135_MMC_WRITEW(host, defCL90135_EMIFA_sdcard_block_word_write,((*host->buffer)&0xFFFF)      );
	        CL90135_MMC_WRITEW(host, defCL90135_EMIFA_sdcard_block_word_write,(((*host->buffer)>>16)&0xFFFF));       
//            *defCL90135_EMIFA_sdcard_block_word_write=((*host->buffer)&0xFFFF);
//            *defCL90135_EMIFA_sdcard_block_word_write=(((*host->buffer)>>16)&0xFFFF);
            host->buffer++;
		}
        // IMPOSTO IL NUMERO DI BLOCCHI DA SCRIVERE
        // che è dato dal numero di bytes/512
        // sperabilmente non ci sarà mai un resto nella divisione
#warning by now, set hi number of blocks to 0!        
	    CL90135_MMC_WRITEW(host, defCL90135_EMIFA_sdcard_cnt_write_multiple_blocks_hi,0);       
	    
	    CL90135_MMC_WRITEW(host, defCL90135_EMIFA_sdcard_cnt_write_multiple_blocks_low,n/512+((n%512)?1:0));       
//        *defCL90135_EMIFA_sdcard_cnt_write_multiple_blocks=n/512+((n%512)?1:0);
    }
    else{
    
        // trovo quanti bytes abbia a disposizione fpga per noi
        // occhio che fpga mi dà il numero di word da 16 bit, non il numero di bytes... quindi devo mettere un *2
        n=2*CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_block_read_numOf);
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd %i databytes ready from block read.\n",n);
#endif	
        //n=*defCL90135_EMIFA_sdcard_block_read_numOf;
        // se superiore al numero atteso, clippo
        if (n > host->buffer_bytes_left)
            n = host->buffer_bytes_left;

        host->buffer_bytes_left -= n;
        host->bytes_left -= n;
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd %i databytes read, %i left.\n",n,host->buffer_bytes_left);
#endif	
        
        // nel caso di una lettura, devo estrarre i bytes che sono a disposizione
        // nella fpga, e quanti sono???
        // posso saperlo da fpga, oppure mi devo fidare della richiesta
        for (i = 0; i < (n / 4); i++) {           
            u1=CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_block_word_read);
            u2=CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_block_word_read);
#ifdef defVerbosePrintK    
// la lettura sembra ok, i bytes sono corretti
//    if (i<10){
//    	printk(KERN_WARNING "CL90135_sd read u1=%X, u2=%X.\n",u1,u2);
//    }
#endif	
            *host->buffer=(u2<<16)|u1;
            host->buffer++;
		}
		// attenzione!!! distinguere tra sd card a capacità standard (indirizzo byte) e capacità estesa (indirizzo a 512bytes)
	    host->ulNumBytesRead+=n;
	    
        n = host->buffer_bytes_left;
        if (n){
            if (n>defCL90135_EMIFA_max_bytes_ReadBlock)
                n=defCL90135_EMIFA_max_bytes_ReadBlock;
            // comando reset stato buffers sd fpga...
            defCL90135_Issue_block_reset;
            // IMPOSTO IL NUMERO DI BLOCCHI DA LEGGERE, GIUSTO PRIMA DEL COMANDO DI START LETTURA BLOCCHI MULTIPLI
#warning by now, set hi number of blocks to read to zero!            
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cnt_read_multiple_blocks_hi,0);
            
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cnt_read_multiple_blocks_low,n/512+((n%512)?1:0));
	        
            // comando lettura blocchi		
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_resp_len,48);   // lunghezza della risposta attesa
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arglo,((host->cmd->arg+host->ulNumBytesRead)&0xFFFF));
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arghi,((host->cmd->arg+host->ulNumBytesRead)>>16));
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cmd,host->cmd->opcode); //faccio partire il comando
        }
		
    }
}



static void mmc_davinci_start_command(struct mmc_davinci_host *host,
				      struct mmc_command *cmd)
{
    u32 resptype,n;
    u32 u32_expected_length_resp_nbit;
    unsigned long flags;

	host->cmd = cmd;
    

    resptype = 0;

	switch (RSP_TYPE(mmc_resp_type(cmd))) {
        case RSP_TYPE(MMC_RSP_R1):
            /* resp 1, resp 1b */
            resptype = 1;
            u32_expected_length_resp_nbit=48;
            break;
        case RSP_TYPE(MMC_RSP_R2):
            resptype = 2;
            u32_expected_length_resp_nbit=136;
            break;
        case RSP_TYPE(MMC_RSP_R3):
            resptype = 3;
            u32_expected_length_resp_nbit=48;
            break;
        default:
            resptype = 3;
            u32_expected_length_resp_nbit=48;
            break;
	}
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd command %i, argLo=0x%X, argHi=0x%X, respLen_Nbit=%i.\n",(int)cmd->opcode,((cmd->arg)&0xFFFF),((cmd->arg)>>16),u32_expected_length_resp_nbit);
#endif	

    // se devo scrivere dati, li devo prima inviare al buffer fpga, poi
    // indicare il numero di blocchi da scrivere, infine spedire il comando di scrittura
    if (  (host->data_dir == DAVINCI_MMC_DATADIR_WRITE)
        &&(host->do_dma != 1)
        ){
		davinci_fifo_data_trans(host);
	}
// se richiesta lettura n blocchi, devo scrivere in fpga il numero di blocchi da leggere PRIMA  che venga spedito il comando
    if (  (cmd->opcode == 18)
        &&(host->do_dma != 1)
        ){
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "Requested block read of %u bytes\n",host->buffer_bytes_left);
#endif	
	    host->ulNumBytesRead=0;
    	host->ulTotalNumBytesToRead=host->buffer_bytes_left;
    	

        n = host->rw_threshold;
        if (n>defCL90135_EMIFA_max_bytes_ReadBlock)
            n=defCL90135_EMIFA_max_bytes_ReadBlock;
        if (n > host->buffer_bytes_left)
            n = host->buffer_bytes_left;
        // comando reset stato buffers sd fpga...
        defCL90135_Issue_block_reset;
        // IMPOSTO IL NUMERO DI BLOCCHI DA LEGGERE, GIUSTO PRIMA DEL COMANDO DI START LETTURA BLOCCHI MULTIPLI
#warning by now, set hi number of blocks to read to zero!            
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "Executing block read of %u bytes\n",n);
#endif	
        CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cnt_read_multiple_blocks_hi,0);
        
        CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cnt_read_multiple_blocks_low,n/512+((n%512)?1:0));
    }

    // cmd7-->passaggio in tran state
	if (cmd->opcode == 7) {
		spin_lock_irqsave(&host->mmc_lock, flags);
		host->is_card_removed = 0;
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd card in tran state\n");
#endif	            	
		host->new_card_state = 1;
		host->is_card_initialized = 1;
		host->old_card_state = host->new_card_state;
		host->is_init_progress = 0;
		spin_unlock_irqrestore(&host->mmc_lock, flags);
	}
//  ACMD41=CMD55+CMD41=INIT SD CARD
    if (cmd->opcode == 1 || cmd->opcode == 41) {
		spin_lock_irqsave(&host->mmc_lock, flags);
		host->is_card_initialized = 0;
		host->is_init_progress = 1;
		spin_unlock_irqrestore(&host->mmc_lock, flags);
	}

	host->is_core_command = 1;

    CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_resp_len,u32_expected_length_resp_nbit);   // lunghezza della risposta attesa
    CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arglo,((cmd->arg)&0xFFFF));
    CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arghi,((cmd->arg)>>16));
    CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cmd,cmd->opcode); //faccio partire il comando
}


static inline void CL90135_wait_on_data(struct mmc_davinci_host *host)
{
	int cnt = 900000;
    unsigned short int ui;
    TipoStructSdCommandStatus sdCommandStatus;
    
    while(cnt){
        cnt--;
		ui=CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_result);
        udelay(1);
        memcpy(&sdCommandStatus,&ui,sizeof(ui));
		if (sdCommandStatus.uiBusy==0)
			break;
	}

    if (!cnt)
		dev_warn(host->mmc->dev, "ERROR: TOUT waiting for BUSY\n");
}//static inline void CL90135_wait_on_data(struct mmc_davinci_host *host)


static void CL90135_sd_request(struct mmc_host *mmc, struct mmc_request *req){
	struct mmc_davinci_host *host = mmc_priv(mmc);
	unsigned long flags;

	if (host->is_card_removed) {
		if (req->cmd) {
			req->cmd->error |= MMC_ERR_TIMEOUT;
			mmc_request_done(mmc, req);
		}
        dev_dbg(host->mmc->dev, "From code segment excuted when card removed\n");
		return;
	}

    CL90135_wait_on_data(host);
    

	if (!host->is_card_detect_progress) {
		spin_lock_irqsave(&host->mmc_lock, flags);
		host->is_card_busy = 1;
		spin_unlock_irqrestore(&host->mmc_lock, flags);
		host->do_dma = 0;
        CL90135_prepare_data(host, req);
		mmc_davinci_start_command(host, req->cmd);
	} else {
		/* Queue up the request as card dectection is being excuted */
		host->que_mmc_host = mmc;
		host->que_mmc_request = req;
		spin_lock_irqsave(&host->mmc_lock, flags);
		host->is_req_queued_up = 1;
		spin_unlock_irqrestore(&host->mmc_lock, flags);
	}
}//static void mmc_davinci_request(struct mmc_host *mmc, struct mmc_request *req)


static void CL90135_sd_set_ios(struct mmc_host *mmc, struct mmc_ios *ios){
    struct mmc_davinci_host *host = mmc_priv(mmc);

    dev_dbg(host->mmc->dev, "clock %dHz busmode %d powermode %d Vdd %d.%02d\n",ios->clock, ios->bus_mode, ios->power_mode,ios->vdd / 100, ios->vdd % 100);

    switch (ios->bus_width) {
        case MMC_BUS_WIDTH_8:
            dev_dbg(host->mmc->dev, "\nError! Unhandled 8 bit mode!!!\n");
            break;
        case MMC_BUS_WIDTH_4:
            dev_dbg(host->mmc->dev, "\nAlways enabled 4 bit mode\n");
            break;
        default:
            //dev_dbg(host->mmc->dev, "\nError! Unhandled 1 bit mode\n");
            break;
    }

	if (ios->bus_mode == MMC_BUSMODE_OPENDRAIN) {
        dev_dbg(host->mmc->dev, "Selecting low speed (opendrain) mode!!!\n");
        defCL90135_EMIFA_sdcard_speedSelect_Lo;
    }
    else {
        if (ios->clock>10000000){
            dev_dbg(host->mmc->dev, "Selecting HI speed mode!!!\n");
            defCL90135_EMIFA_sdcard_speedSelect_Hi;
        }
        else{
            dev_dbg(host->mmc->dev, "Selecting low speed mode!!!\n");
            defCL90135_EMIFA_sdcard_speedSelect_Lo;
        }
    }
	host->bus_mode = ios->bus_mode;
	if (ios->power_mode == MMC_POWER_UP) {
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135 set ios->powerup.\n");
#endif	
        init_CL90135_sd_host(host);
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135 end of powerup.\n");
#endif	
    }
}


static void mmc_davinci_xfer_done(struct mmc_davinci_host *host,struct mmc_data *data){
	unsigned long flags;
	host->data = NULL;
	host->data_dir = DAVINCI_MMC_DATADIR_NONE;
	if (data->error == MMC_ERR_NONE)
		data->bytes_xfered += data->blocks * (1 << data->blksz_bits);

#ifndef defCL90135DisableDma
    if (host->do_dma) {
		davinci_abort_dma(host);
        dma_unmap_sg(host->mmc->dev, data->sg, host->sg_len,(data->flags & MMC_DATA_WRITE) ? DMA_TO_DEVICE :DMA_FROM_DEVICE);
	}
#endif

	if (data->error == MMC_ERR_TIMEOUT) {
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd xfer timeout\n");
#endif	            	
		spin_lock_irqsave(&host->mmc_lock, flags);
		host->is_card_busy = 0;
		spin_unlock_irqrestore(&host->mmc_lock, flags);
		mmc_request_done(host->mmc, data->mrq);
		return;
	}
// la scheda cl90135 non necessita dello stop...
    if (1){
//	if (!data->stop) {
		spin_lock_irqsave(&host->mmc_lock, flags);
		host->is_card_busy = 0;
		spin_unlock_irqrestore(&host->mmc_lock, flags);
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd end of data transfer .\n");
#endif	
		mmc_request_done(host->mmc, data->mrq);
		return;
	}
    // il comando di stop viene inviato automaticamente da fpga...
    //mmc_davinci_start_command(host, data->stop);
}//static void mmc_davinci_xfer_done(struct mmc_davinci_host *host,struct mmc_data *data)




static void mmc_CL90135_cmd_done(struct mmc_davinci_host *host,struct mmc_command *cmd){
	unsigned long flags;
	host->cmd = NULL;

	if (!cmd) {
		dev_warn(host->mmc->dev, "%s(): No cmd ptr\n", __func__);
		return;
	}

	if (cmd->flags & MMC_RSP_PRESENT) {
		if (cmd->flags & MMC_RSP_136) {
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd resp 136bit.\n");
#endif	
            // response type 2
            v_read_R2_like_sdResponse(host,cmd->resp);
        } else {
            // response types 1, 1b, 3, 4, 5, 6
            cmd->resp[0] = u32_read_R1_like_sdResponse(host);
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd resp R1-like is %i.\n", (int)cmd->resp[0]);
	if (cmd->error != MMC_ERR_NONE)
	    printk(KERN_WARNING "CL90135_sd error is %i.\n", (int)cmd->error);
    else
	    printk(KERN_WARNING "CL90135_sd no error.\n");
#endif	
		}
	}

	if (host->data == NULL || cmd->error != MMC_ERR_NONE) {
		if (cmd->error == MMC_ERR_TIMEOUT)
			cmd->mrq->cmd->retries = 0;
		spin_lock_irqsave(&host->mmc_lock, flags);
		host->is_card_busy = 0;
		spin_unlock_irqrestore(&host->mmc_lock, flags);
		mmc_request_done(host->mmc, cmd->mrq);
	}

}//static void mmc_CL90135_cmd_done(struct mmc_davinci_host *host,struct mmc_command *cmd)



static irqreturn_t mmc_CL90135_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	struct mmc_davinci_host *host = (struct mmc_davinci_host *)dev_id;
	volatile u16 status;
	int end_command;
	int end_transfer;
	unsigned long flags;

	if (host->is_core_command) {
		if (host->cmd == NULL && host->data == NULL) {
            status = CL90135_MMC_READ_IRQ_STATUS;
            dev_dbg(host->mmc->dev, "Spurious interrupt 0x%04x\n",status);
            // Disable the interrupt from mmcsd
            // warning! Actually there is no way to disable the interrupts direct from fpga: use dsp instead!
//            #error disable interrupts from sd //DAVINCI_MMC_WRITEW(host, IM, 0);
			return IRQ_HANDLED;
		}
	}
	end_command = 0;
	end_transfer = 0;

    status = CL90135_MMC_READ_IRQ_STATUS;
#ifdef defVerbosePrintK    
    // debug only ints from block read
    if (host->cmd&&(host->cmd->opcode==18))
	    printk(KERN_WARNING "CL90135_sd interrupt, status is 0x%X.\n", (int)status);
#endif	
    if (status == 0)
		return IRQ_HANDLED;

	if (host->is_core_command) {
		if (host->is_card_initialized) {
			if (host->new_card_state == 0) {
				if (host->cmd) {
					host->cmd->error |= MMC_ERR_TIMEOUT;
#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd interrupt, timeout.\n");
#endif	
                    mmc_CL90135_cmd_done(host, host->cmd);
				}
                dev_dbg(host->mmc->dev,"From code segment excuted when card removed\n");
				return IRQ_HANDLED;
			}
		}
// WHILE NO-FIFO EVENTS IN QUEUE...
//		while ((status&CL90135_MMC_EVENT_MASK_NO_FIFO) != 0) {
		while (status) {

            if (    (status & CL90135_MMC_EVENT_END_OF_WRITEBLOCK)
                  ||(status & CL90135_MMC_EVENT_END_OF_READBLOCK )
                ) {
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd interrupt, end of r/w block.\n");
#endif	
				/* Block sent/received */
				if (host->data != NULL) {
					if (host->do_dma == 1)
						end_transfer = 1;
					else {
						if ((status & CL90135_MMC_EVENT_END_OF_WRITEBLOCK )&&(host->bytes_left == 0)){
    						end_transfer = 1;
    					}
                        /* if datasize <host_rw_threshold no RX ints are generated */
						if (host->bytes_left > 0)
                            davinci_fifo_data_trans(host);
						if ((status & CL90135_MMC_EVENT_END_OF_READBLOCK )&&(host->bytes_left == 0)){
    						end_transfer = 1;
    					}
					}
                }
                else
                    dev_warn(host->mmc->dev,"TC:host->data is NULL\n");
			}

            if (  (status & CL90135_MMC_EVENT_DATA_READ_TIMEOUT)
                ||(status & CL90135_MMC_EVENT_DATA_WRITE_TIMEOUT)
               ){
printk(KERN_WARNING "CL90135_sd interrupt, timeout r/w block.\n");
#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd interrupt, timeout r/w block.\n");
#endif	
				/* Data timeout */
				if ((host->data) &&	(host->new_card_state != 0)) {
					host->data->error |= MMC_ERR_TIMEOUT;
                    spin_lock_irqsave(&host->mmc_lock,flags);
					host->is_card_removed = 1;
					host->new_card_state = 0;
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd card removed 3\n");
#endif	            	
					host->is_card_initialized = 0;
                    spin_unlock_irqrestore(&host->mmc_lock,flags);
                    dev_dbg(host->mmc->dev,"MMCSD: Data timeout, CMD%d and status is %x\n",host->cmd->opcode, status);
                    if (host->cmd)
                        host->cmd->error |=MMC_ERR_TIMEOUT;
					end_transfer = 1;
				}
			}

            if (   (status & CL90135_MMC_EVENT_DATA_READ_CRCERR)
                || (status & CL90135_MMC_EVENT_DATA_WRITE_CRCERR)
               ){
#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd interrupt, crcerr r/w block.\n");
#endif	
                #if 0
                    /* DAT line portion is diabled and in reset state */
                    reg = DAVINCI_MMC_READW(host, CTL);
                    reg |= (1 << 1);
                    DAVINCI_MMC_WRITEW(host, CTL, reg);
                    udelay(10);
                    reg = DAVINCI_MMC_READW(host, CTL);
                    reg &= ~(1 << 1);
                    DAVINCI_MMC_WRITEW(host, CTL, reg);
                #endif

				/* Data CRC error */
				if (host->data) {
					host->data->error |= MMC_ERR_BADCRC;
                    dev_dbg(host->mmc->dev,"MMCSD: Data CRC error, bytes left %d\n",host->bytes_left);
					end_transfer = 1;
				} else
                    dev_dbg(host->mmc->dev,"MMCSD: Data CRC error\n");
			}

            if (status & CL90135_MMC_EVENT_COMMAND_TIMEOUT) {
#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd interrupt, command timeout.\n");
#endif	
#ifndef defCL90135DisableDma
                if (host->do_dma)
					davinci_abort_dma(host);
#endif
				/* Command timeout */
				if (host->cmd) {
                    /* Timeouts are normal in case of MMC_SEND_STATUS */
                    if (host->cmd->opcode !=MMC_ALL_SEND_CID) {
                        dev_dbg(host->mmc->dev,"MMCSD: Command timeout, CMD%d and status is %x\n",host->cmd->opcode,status);
                        spin_lock_irqsave(&host->mmc_lock,flags);
						host->new_card_state = 0;
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd card removed 4\n");
#endif	            	
						host->is_card_initialized = 0;
                        spin_unlock_irqrestore(&host->mmc_lock, flags);
					}
					host->cmd->error |= MMC_ERR_TIMEOUT;
					end_command = 1;

#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd interrupt, registering command timeout.\n");
#endif	
				}
			}

            if (status & CL90135_MMC_EVENT_CMD_CRCERR) {
#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd interrupt, crcerr.\n");
#endif	
				/* Command CRC error */
				dev_dbg(host->mmc->dev, "Command CRC error\n");
				if (host->cmd) {
                    // ON CL90135 platform, the crc error control is always enabled!!!
                    host->cmd->error |=MMC_ERR_BADCRC;
                    #if 0
                        /* Ignore CMD CRC errors during high speed operation */
                        if (host->mmc->ios.clock <= 25000000) {
                            host->cmd->error |=MMC_ERR_BADCRC;
                        }
                    #endif
					end_command = 1;
				}
			}

            if (status & CL90135_MMC_EVENT_END_OF_COMMAND){
#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd interrupt, end of command.\n");
#endif	            
				end_command = 1;
			}

			if (host->data == NULL) {
                status = CL90135_MMC_READ_IRQ_STATUS;
				if (status != 0) {
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd Status (no data) is %x at end of ISR when host->data is NULL\n",status);
#endif	            
                    dev_dbg(host->mmc->dev,"Status is %x at end of ISR when host->data is NULL",status);
					status = 0;
                }
			} else{
                status = CL90135_MMC_READ_IRQ_STATUS;
				if (status != 0) {
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd Status is %x at end of ISR when host->data is not NULL\n",status);
#endif	            
					//status = 0;
                }
            }
		}
#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd end of loop interrupt\n");
#endif	            

		if (end_command){
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd end command maybe\n");
#endif	            	
#ifdef defVerbosePrintK
//    if (host && host->cmd)
//    	printk(KERN_WARNING "CL90135_sd irq handled, err is %X\n",(int)(host->cmd->error));
//    else
//    	printk(KERN_WARNING "CL90135_sd irq handled, no cmd\n");
#endif
            if (host){
                if (host->cmd){
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd end command stated\n");
#endif	            	
                    mmc_CL90135_cmd_done(host, host->cmd);
                }
            }
        }
		if (end_transfer){
#ifdef defVerbosePrintK    
//	printk(KERN_WARNING "CL90135_sd xfer done\n");
#endif	            	
			mmc_davinci_xfer_done(host, host->data);
		}
	} else {
        // cmd13=get status
		if (host&&(host->cmd_code == 13)) {
            if (status & CL90135_MMC_EVENT_END_OF_COMMAND) {
#ifdef defVerbosePrintK    
// this message is generated once every second
//	printk(KERN_WARNING "CL90135_sd card cmd13 end of command, status=%X\n",(int)status);
#endif	            	
				spin_lock_irqsave(&host->mmc_lock, flags);
				host->new_card_state = 1;
				spin_unlock_irqrestore(&host->mmc_lock, flags);

			}
			
			else if (status & CL90135_MMC_COMMAND_ERROR)
			{
#warning use another system to check if card removed!			
				spin_lock_irqsave(&host->mmc_lock, flags);
				host->is_card_removed = 1;
				host->new_card_state = 0;
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd card removed 5, status=%X\n",(int)status);
#endif	            	
				host->is_card_initialized = 0;
				spin_unlock_irqrestore(&host->mmc_lock, flags);
			}

			spin_lock_irqsave(&host->mmc_lock, flags);
			host->is_card_detect_progress = 0;
			spin_unlock_irqrestore(&host->mmc_lock, flags);

			if (host->is_req_queued_up) {
                CL90135_sd_request(host->que_mmc_host,host->que_mmc_request);
				spin_lock_irqsave(&host->mmc_lock, flags);
				host->is_req_queued_up = 0;
				spin_unlock_irqrestore(&host->mmc_lock, flags);
			}

		}

		if (host&&(host->cmd_code == 1 || host->cmd_code == 55)) {
            if (status & CL90135_MMC_EVENT_END_OF_COMMAND) {
				spin_lock_irqsave(&host->mmc_lock, flags);
				host->is_card_removed = 0;
				host->new_card_state = 1;
				host->is_card_initialized = 0;
				spin_unlock_irqrestore(&host->mmc_lock, flags);
			} else if (status & CL90135_MMC_COMMAND_ERROR){

				spin_lock_irqsave(&host->mmc_lock, flags);
				host->is_card_removed = 1;
				host->new_card_state = 0;
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd card removed 6\n");
#endif	            	
				host->is_card_initialized = 0;
				spin_unlock_irqrestore(&host->mmc_lock, flags);
			}

			spin_lock_irqsave(&host->mmc_lock, flags);
			host->is_card_detect_progress = 0;
			spin_unlock_irqrestore(&host->mmc_lock, flags);

			if (host->is_req_queued_up) {
				CL90135_sd_request(host->que_mmc_host,host->que_mmc_request);
				spin_lock_irqsave(&host->mmc_lock, flags);
				host->is_req_queued_up = 0;
				spin_unlock_irqrestore(&host->mmc_lock, flags);
			}
		}

		if (host&&(host->cmd_code == 0)) {
            if (status & CL90135_MMC_EVENT_END_OF_COMMAND) {
				host->is_core_command = 0;
                // se è già stata provata la inizializzazione sd_mmc, adesso proviamo con sd_spi
				if (host->flag_sd_mmc) {
					host->flag_sd_mmc = 0;
					host->cmd_code = 1;
					/* Issue cmd1 */
                    defCL90135_Issue_CMD1;
                } else {
					host->flag_sd_mmc = 1;
					host->cmd_code = 55;
					/* Issue cmd55 */
                    defCL90135_Issue_CMD55;
                    //DAVINCI_MMC_WRITEL(host, ARGHL, 0x0);
                    //DAVINCI_MMC_WRITEL(host, CMD,((0x0 | (1 << 9) | 55)));
				}

                dev_dbg(host->mmc->dev, "MMC-Probing mmc with cmd%d\n",host->cmd_code);
			} else {
				spin_lock_irqsave(&host->mmc_lock, flags);
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd card not initialized\n");
#endif	            	
				host->new_card_state = 0;
				host->is_card_initialized = 0;
				host->is_card_detect_progress = 0;
				spin_unlock_irqrestore(&host->mmc_lock, flags);
			}
		}

	}

	return IRQ_HANDLED;
}//static irqreturn_t mmc_CL90135_irq(int irq, void *dev_id, struct pt_regs *regs)



// verifica se sd card è nello stato read only
static int CL90135_sd_get_ro(struct mmc_host *mmc)
{
	struct mmc_davinci_host *host = mmc_priv(mmc);
    TipoCL90135_StructSdCardIoStatus cardIoStatus;
    short unsigned int uiShortInt;
    uiShortInt=CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_IO_STATUS);
    memcpy(&cardIoStatus,&uiShortInt,sizeof(cardIoStatus));

    return cardIoStatus.uiWriteProtected;
}



static struct mmc_host_ops CL90135_sd_ops = {
    .request = CL90135_sd_request,
    .set_ios = CL90135_sd_set_ios,
    .get_ro  = CL90135_sd_get_ro
};

// verify if card status has changed
void mmc_check_card(unsigned long data)
{
	struct mmc_davinci_host *host = (struct mmc_davinci_host *)data;
	unsigned long flags;
	struct mmc_card *card = NULL;

	if (host->mmc && host->mmc->card_selected)
		card = host->mmc->card_selected;

	if ((!host->is_card_detect_progress) || (!host->is_init_progress)) {
		if (host->is_card_initialized) {
			host->is_core_command = 0;
			host->cmd_code = 13;
			spin_lock_irqsave(&host->mmc_lock, flags);
			host->is_card_detect_progress = 1;
			spin_unlock_irqrestore(&host->mmc_lock, flags);
            // Issue cmd13 --> get card status
            // comando CMD13
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_resp_len,48);   // risposta attesa ha lunghezza 48
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arglo,0x0000);
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arghi,(card && (card->state& MMC_STATE_SDCARD))? (card->rca) : 0x0000);
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cmd,host->cmd_code); //faccio partire il comando 13

		} else {
			host->is_core_command = 0;
			host->cmd_code = 0;
			spin_lock_irqsave(&host->mmc_lock, flags);
			host->is_card_detect_progress = 1;
			spin_unlock_irqrestore(&host->mmc_lock, flags);
            // Issue cmd0 --> initialize card
            // comando CMD0
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_resp_len,0);    // risposta attesa ha lunghezza 0
            CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cmd,0); // faccio partire il comando 0=cmd 0
        }//if (host->is_card_initialized)
    }//if ((!host->is_card_detect_progress) || (!host->is_init_progress))
}//void mmc_check_card(unsigned long data)

// check status
static void CL90135_sd_check_status(unsigned long data)
{
	unsigned long flags;
	struct mmc_davinci_host *host = (struct mmc_davinci_host *)data;
	if (!host->is_card_busy) {
		if (host->old_card_state ^ host->new_card_state) {
			davinci_reinit_chan(host);
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135 card state changed: old=%X, new=%X.\n",(int)host->old_card_state,(int)host->new_card_state);
#endif	
			init_CL90135_sd_host(host);
			mmc_detect_change(host->mmc, 0);
			spin_lock_irqsave(&host->mmc_lock, flags);
			host->old_card_state = host->new_card_state;
			spin_unlock_irqrestore(&host->mmc_lock, flags);
		} else {
			mmc_check_card(data);
		}

	}
	mod_timer(&host->timer, jiffies + MULTIPLIER_TO_HZ * HZ);
}



// verifica esistenza
static int cl90135_sd_probe(struct platform_device *pdev)
{
	struct davinci_mmc_platform_data *minfo = pdev->dev.platform_data;
	struct mmc_host *mmc;
	struct mmc_davinci_host *host = NULL;
	struct resource *res;
	int ret = 0;
	int irq; 
	
	
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd probe.\n");
#endif	
	if (minfo == NULL) {
		dev_err(&pdev->dev, "platform data missing\n");
		return -ENODEV;
	}

#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd probe pass 1.\n");
#endif	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (res == NULL || irq < 0)
		return -ENODEV;

#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd probe pass 2.\n");
#endif	
    res = request_mem_region(res->start, res->end - res->start + 1,pdev->name);
	if (res == NULL)
		return -EBUSY;

	mmc = mmc_alloc_host(sizeof(struct mmc_davinci_host), &pdev->dev);
	if (mmc == NULL) {
		ret = -ENOMEM;
		goto err_free_mem_region;
	}
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd probe pass 3.\n");
#endif	

	host = mmc_priv(mmc);
	host->mmc = mmc;

	spin_lock_init(&host->mmc_lock);

	host->mem_res = res;
//
// CL90135 SD CARD INTERRUPT PIN = GPIO5_6 = T5 = IO_L04N_2
//
	if (gpio_request(SD_CARD_INTERRUPT_PIN, "SD_CARD_INTERRUPT_PIN")) {
		printk(KERN_ERR "%s: could not request GPIOs for SD_CARD_INTERRUPT_PIN \n", __func__);
		return -ENODEV;
	}
	printk(KERN_WARNING "CL90135: SD_CARD_INTERRUPT_PIN set as input.\n");
	
    gpio_direction_input(SD_CARD_INTERRUPT_PIN);   
    gpio_free(SD_CARD_INTERRUPT_PIN);
    
	host->irq = gpio_to_irq(SD_CARD_INTERRUPT_PIN);

	host->phys_base = host->mem_res->start;
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd phys_base=%X\n",(unsigned int)host->phys_base);
#endif	
	host->virt_base = ioremap_nocache(host->mem_res->start, host->mem_res->end + 1 - host->mem_res->start);

#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd virt_base=%X YYYY=%i, phys_base=%X, end=%X\n",(int)host->virt_base, (int)CL90135_MMC_READW(host, defCL90135_EMIFA_address_version_YYYY),host->mem_res->start,host->mem_res->end);
#endif	

	host->use_dma = 0;

	res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (res > 0) {
		host->dma_rx_event = res->start;
		res = platform_get_resource(pdev, IORESOURCE_DMA, 1);
		if (res > 0) {
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd probe dma enabled.\n");
#endif	
			host->dma_tx_event = res->start;
#ifndef defCL90135DisableDma
            host->use_dma = 1;
#endif
		} else
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd probe no dma.\n");
#endif	
			host->dma_rx_event = 0;
	}
// the clk is generated by fpga, it is not necessary to enable it
    // host->clk = clk_get(&pdev->dev, minfo->mmc_clk);
    // if (IS_ERR(host->clk)) {
    //     ret = -ENODEV;
    //     goto err_free_mmc_host;
    // }
    // ret = clk_enable(host->clk);
    // if (ret)
    //    goto err_put_clk;
    // inizializzazione i/o a livello fisico
    //init_mmcsd_host(host);

// inizializzazione a livello fisico degli i/o...
// dato che noi abbiamo fpga dedicata, ne approfittiamo per inizializzare sd card con la sequenza di init
// da N colpi di clock
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "host init.\n");
#endif	
    if (!init_CL90135_sd_host(host)){
        dev_err(&pdev->dev, "sd card init: unable to get response from fpga...!\n");
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "err on host init.\n");
#endif	
    }

// questo driver non supporta 8bit mode, ma solo 4 bit mode!!!
	if (minfo->use_8bit_mode) {
        dev_info(mmc->dev, "8-bit mode not supported!\n");
        mmc->caps &= ~MMC_CAP_8_BIT_DATA;
	}

// 4 bit mode always supported
    if (1||minfo->use_4bit_mode) {
		dev_info(mmc->dev, "Supporting 4-bit mode\n");
		mmc->caps |= MMC_CAP_4_BIT_DATA;
	}

	if (!minfo->use_8bit_mode && !minfo->use_4bit_mode)
		dev_info(mmc->dev, "Supporting 1-bit mode\n");

// read-only flag
	host->get_ro = minfo->get_ro;

	host->pio_set_dmatrig = minfo->pio_set_dmatrig;

	host->rw_threshold = minfo->rw_threshold;

    mmc->ops = &CL90135_sd_ops;
	mmc->f_min = 312500;
	if (minfo->max_frq)
		mmc->f_max = minfo->max_frq;
	else
		mmc->f_max = 25000000;
	mmc->ocr_avail = MMC_VDD_32_33;

//Functions that set parameters describing the requests that can be satisfied
//by this device.

//blk_queue_max_phys_segments and blk_queue_max_hw_segments
//    both control how many physical segments (nonadjacent areas in system memory)
//    may be contained within a single request. Use
//blk_queue_max_phys_segments to say how many segments your driver is
//    prepared to cope with; this may be the size of a staticly
//    allocated scatterlist, for example.
    mmc->max_phys_segs = 2;
//blk_queue_max_hw_segments, in contrast, is the maximum number of segments
//    that the device itself can handle. Both of these parameters
//    default to 128.
    mmc->max_hw_segs = 1;
//blk_queue_max_sectors can be used to set the maximum size of
//    any request in (512-byte) sectors; the default is 255.
    mmc->max_sectors = 4;

	/* Restrict the max size of seg we can handle */
//Finally, blk_queue_max_segment_size tells the kernel how large any
//    individual segment of a request can be in bytes;
//    the default is 65,536 bytes.
    mmc->max_seg_size = mmc->max_sectors * 512;

// per abilitare dev_dbg, inserire "debug" nei bootargs
// oppure definire DEBUG in testa al file o ancora usare sistema sviluppo per abilitare il debug
// dei device
	dev_dbg(mmc->dev, "max_phys_segs=%d\n", mmc->max_phys_segs);
	dev_dbg(mmc->dev, "max_hw_segs=%d\n", mmc->max_hw_segs);
	dev_dbg(mmc->dev, "max_sect=%d\n", mmc->max_sectors);
	dev_dbg(mmc->dev, "max_seg_size=%d\n", mmc->max_seg_size);

#ifndef defCL90135DisableDma
	if (host->use_dma) {
		dev_info(mmc->dev, "Using DMA mode\n");
		ret = davinci_acquire_dma_channels(host);
		if (ret)
			goto err_release_clk;
	} else 
#endif	
	{
		dev_info(mmc->dev, "Not Using DMA mode\n");
	}

	host->sd_support = 1;
	
    ret = request_irq(host->irq, mmc_CL90135_irq, IRQF_TRIGGER_RISING, DRIVER_NAME, host);
	if (ret){
	    dev_info(mmc->dev, "CL90135 request irq error %i, irq=%i\n", ret, host->irq);
		goto err_release_dma;
	}	
	dev_info(mmc->dev, "CL90135 irq requested OK\n");

	host->dev = &pdev->dev;
	platform_set_drvdata(pdev, host);
	mmc_add_host(mmc);

	init_timer(&host->timer);
	host->timer.data = (unsigned long)host;
    host->timer.function = CL90135_sd_check_status;
	host->timer.expires = jiffies + MULTIPLIER_TO_HZ * HZ;
	add_timer(&host->timer);
	dev_info(mmc->dev, "CL90135 end of probe function\n");

	return 0;

err_release_dma:
#ifndef defCL90135DisableDma
    davinci_release_dma_channels(host);
#endif
//err_release_clk:
	clk_disable(host->clk);
//err_put_clk:
	clk_put(host->clk);
//err_free_mmc_host:
	mmc_free_host(mmc);
err_free_mem_region:
	release_mem_region(res->start, res->end - res->start + 1);

	return ret;
}

// effettua il remove dell'sd card
static int cl90135_sd_remove(struct platform_device *dev)
{
	struct mmc_davinci_host *host = platform_get_drvdata(dev);
	unsigned long flags;

	if (host) {
		spin_lock_irqsave(&host->mmc_lock, flags);
		del_timer(&host->timer);
		spin_unlock_irqrestore(&host->mmc_lock, flags);

		mmc_remove_host(host->mmc);
		platform_set_drvdata(dev, NULL);
		free_irq(host->irq, host);
#ifndef defCL90135DisableDma
        davinci_release_dma_channels(host);
#endif
		clk_disable(host->clk);
		clk_put(host->clk);
		release_resource(host->mem_res);
		kfree(host->mem_res);
		mmc_free_host(host->mmc);
	}

	return 0;
}


// no power management at this moment
#define cl90135_sd_suspend   NULL
#define cl90135_sd_resume	 NULL

// struttura di puntatori alle funzioni del driver
static struct platform_driver cl90135_sd_driver = {
    .probe      = cl90135_sd_probe,
    .remove     = cl90135_sd_remove,
    .suspend    = cl90135_sd_suspend,
    .resume     = cl90135_sd_resume,
	.driver		= {
		.name	= DRIVER_NAME,
	},
};


// registrazione del driver cl90135 sd card
static int cl90135_sd_init(void)
{
    // chiamo la registrazione del driver...
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd init.\n");
#endif

    return platform_driver_register(&cl90135_sd_driver);
}

// de-registrazione del driver cl90135 sd card
static void __exit cl90135_sd_exit(void)
{
#ifdef defVerbosePrintK    
	printk(KERN_WARNING "CL90135_sd exit.\n");
#endif
    // de-registro il driver
    platform_driver_unregister(&cl90135_sd_driver);
}

module_init(cl90135_sd_init);
module_exit(cl90135_sd_exit);

MODULE_DESCRIPTION("CL90135 Sd Card driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS(DRIVER_NAME);
MODULE_AUTHOR("Cielle srl");

//#error do not complete compiling please

