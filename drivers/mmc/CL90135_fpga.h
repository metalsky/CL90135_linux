// mettere su file header globale!!!
extern volatile char * pCL90135_base;

#pragma pack(1)

#define CL90135_MMC_READW(host, offset) __raw_readw((host)->virt_base + offset)
#define CL90135_MMC_WRITEW(host, offset, val) __raw_writew((val), (host)->virt_base + offset)


// indirizzo base fpga
#define defCL90135_FpgaBaseAddress_Value 			(0x60000000)
#define defCL90135_FpgaBaseAddress 			((volatile char *)pCL90135_base)

#define defCL90135_EMIFA_address_test                 ( 0*2)
#define defCL90135_EMIFA_address_version_low          ( 1*2)
#define defCL90135_EMIFA_address_version_hi           ( 2*2)
#define defCL90135_EMIFA_address_version_mmss         ( 3*2)
#define defCL90135_EMIFA_address_version_hh           ( 4*2)
#define defCL90135_EMIFA_address_version_MMDD         ( 5*2)
#define defCL90135_EMIFA_address_version_YYYY         ( 6*2)
#define defCL90135_EMIFA_address_version_buildNumber  ( 7*2)
#define defCL90135_EMIFA_address_version_name_first   ( 8*2)
#define defCL90135_EMIFA_address_version_name_last    (12*2)
#define defCL90135_EMIFA_address_test_counter         (13*2)


// indirizzi SDCARD
#define defCL90135_EMIFA_sdcard_BASE        (32*2)
// a write on the cmd register triggers a command
#define defCL90135_EMIFA_sdcard_cmd        	((defCL90135_EMIFA_sdcard_BASE +  0*2))
#define defCL90135_EMIFA_sdcard_arglo      	((defCL90135_EMIFA_sdcard_BASE +  1*2))
#define defCL90135_EMIFA_sdcard_arghi      	((defCL90135_EMIFA_sdcard_BASE +  2*2))
#define defCL90135_EMIFA_sdcard_resp_len   	((defCL90135_EMIFA_sdcard_BASE +  3*2))
#define defCL90135_EMIFA_sdcard_init        ((defCL90135_EMIFA_sdcard_BASE +  4*2))
#define defCL90135_EMIFA_sdcard_write_block ((defCL90135_EMIFA_sdcard_BASE +  5*2))
#define defCL90135_EMIFA_sdcard_read_block  ((defCL90135_EMIFA_sdcard_BASE +  6*2))
// CL90135_EMIFA_sdcard_speedSelect:
// bit  0: select lo clock speed (about 390.6kHz=100MHz/256)
// bit  1: select hi clock speed (about 12.5MHz)
#define defCL90135_EMIFA_sdcard_speedSelect	((defCL90135_EMIFA_sdcard_BASE +  7*2))

#define defCL90135_EMIFA_sdcard_resp_body_0	((defCL90135_EMIFA_sdcard_BASE +  8*2))
#define defCL90135_EMIFA_sdcard_resp_body_1	((defCL90135_EMIFA_sdcard_BASE +  9*2))
#define defCL90135_EMIFA_sdcard_resp_body_2	((defCL90135_EMIFA_sdcard_BASE + 10*2))
#define defCL90135_EMIFA_sdcard_resp_body_3	((defCL90135_EMIFA_sdcard_BASE + 11*2))
#define defCL90135_EMIFA_sdcard_resp_body_4	((defCL90135_EMIFA_sdcard_BASE + 12*2))
#define defCL90135_EMIFA_sdcard_resp_body_5	((defCL90135_EMIFA_sdcard_BASE + 13*2))
#define defCL90135_EMIFA_sdcard_resp_body_6	((defCL90135_EMIFA_sdcard_BASE + 14*2))
#define defCL90135_EMIFA_sdcard_resp_body_7	((defCL90135_EMIFA_sdcard_BASE + 15*2))
#define defCL90135_EMIFA_sdcard_resp_body_8	((defCL90135_EMIFA_sdcard_BASE + 16*2))
#define defCL90135_EMIFA_sdcard_result		((defCL90135_EMIFA_sdcard_BASE + 17*2))

#define defCL90135_EMIFA_sdcard_block_word_write 	((defCL90135_EMIFA_sdcard_BASE + 18*2))
#define defCL90135_EMIFA_sdcard_block_word_read  	((defCL90135_EMIFA_sdcard_BASE + 19*2))
#define defCL90135_EMIFA_sdcard_block_write_status 	((defCL90135_EMIFA_sdcard_BASE + 20*2))
#define defCL90135_EMIFA_sdcard_block_read_status	((defCL90135_EMIFA_sdcard_BASE + 21*2))
#define defCL90135_EMIFA_sdcard_block_write_numOf	((defCL90135_EMIFA_sdcard_BASE + 22*2))
#define defCL90135_EMIFA_sdcard_block_read_numOf	((defCL90135_EMIFA_sdcard_BASE + 23*2))
#define defCL90135_EMIFA_sdcard_debug           	((defCL90135_EMIFA_sdcard_BASE + 24*2))
// CL90135_EMIFA_sdcard_block_reset:
// bit  0: reset sd block read
// bit  1: reset sd block write
// bit  2: reset sd module
#define defCL90135_EMIFA_sdcard_block_reset        	((defCL90135_EMIFA_sdcard_BASE + 25*2))
#define defCL90135_EMIFA_sdcard_debug_1           	((defCL90135_EMIFA_sdcard_BASE + 26*2))
#define defCL90135_EMIFA_sdcard_debug_2           	((defCL90135_EMIFA_sdcard_BASE + 27*2))
#define defCL90135_EMIFA_sdcard_cnt_write_multiple_blocks_low ((defCL90135_EMIFA_sdcard_BASE + 28*2))
#define defCL90135_EMIFA_sdcard_cnt_write_multiple_blocks_hi  ((defCL90135_EMIFA_sdcard_BASE + 29*2))
#define defCL90135_EMIFA_sdcard_cnt_read_multiple_blocks_low  ((defCL90135_EMIFA_sdcard_BASE + 30*2))
#define defCL90135_EMIFA_sdcard_cnt_read_multiple_blocks_hi   ((defCL90135_EMIFA_sdcard_BASE + 31*2))

// word che indica lo stato degli i/o della scheda sd
// bit  0: --> ingresso WP=write protect: 0=card NOT locked, 1=card locked
// bit  1: --> ingresso INS=card inserita: 0=card not inserted, 1=card inserted
// bit  2: --> valore attualmente presente sulla linea CMD
// bit  3: --> valore attualmente presente sulla linea DATA(0)
// bit  4: --> valore attualmente presente sulla linea DATA(1)
// bit  5: --> valore attualmente presente sulla linea DATA(2)
// bit  6: --> valore attualmente presente sulla linea DATA(3)
#define defCL90135_EMIFA_sdcard_IO_STATUS  ((defCL90135_EMIFA_sdcard_BASE + 32*2))

// stato dei comandi e delle operazioni di read/write dati
// bit 14= data write stopbit error
// bit 13= data read stopbit error
// bit 12= cmd error
// bit 11= data write crc error
// bit 10= data read crc error
// bit 9= cmd crc error
// bit 8= data write timeout
// bit 7= data read timeout
// bit 6= cmd timeout
// bit 5= data write busy
// bit 4= data read busy
// bit 3= command busy
// bit 2= end of data write
// bit 1= end of data read
// bit 0= end of command
#define defCL90135_EMIFA_sdcard_cmd_rw_STATUS ((defCL90135_EMIFA_sdcard_BASE + 33*2))

// lettura di una word dalla coda delle irq della sd card (read only)
// se non ci sono irq pendenti, allora la word contiene 0x0000
// altrimenti viene restituita una word in cui i bit hanno il seguente significato
// bit 0 ==>endOfCommand    va ad 1 quando l'ultimo comando inviato termina correttamente
// bit 1 ==>endOfReadBlock  va ad 1 quando l'ultima readblock inviata termina correttamente
// bit 2 ==>endOfWriteBlock va ad 1 quando l'ultima writeblock inviata termina correttamente
// bit 3 ==>cmd_timeout     va ad 1 quando l'ultimo comando inviato genera timeout

// bit 4 ==>data_read_timeout   va ad 1 se una dataread va in timeout
// bit 5 ==>data_write_timeout  va ad 1 se una datawrite va in timeout
// bit 6 ==>cmd_crc_err         va ad 1 se un comando dà crc error
// bit 7 ==>data_read_crc_err   va ad 1 se una dataread dà crc error

// bit 8 ==>data_write_crc_err  va ad 1 se una datawrite dà crc error
// bit 9 ==>cmd_err             va ad 1 se un comando dà errore generico (stopbit error)
// bit 10 ==>data_read_err       va ad 1 se una dataread dà errore generico (stopbit error)
// bit 11 ==>data_write_err      va ad 1 se una datawrite dà errore generico (non dovrebbe mai capitare)
//
// fifo-specific irqs
//
// bit 12 ==>read_full          interrupt generata quando il buffer ram che memorizza dati in lettura da sd è pieno
// bit 13 ==>read_almost_full   interrupt generata quando il buffer ram che memorizza dati in lettura da sd è mezzo pieno
// bit 14 ==>write_empty        interrupt generata quando il buffer ram che memorizza dati da scrivere su sd è vuoto
// bit 15 ==>write_almost_empty interrupt generata quando il buffer ram che memorizza dati da scrivere su sd è mezzo vuoto

#define defCL90135_EMIFA_sdcard_IRQ 		  ((defCL90135_EMIFA_sdcard_BASE + 34*2))

#define CL90135_MMC_COMMAND_ERROR    ((1<<3)|(1<<6)|(1<<9))
#define CL90135_MMC_DATA_READ_ERROR  ((1<<4)|(1<<4)|(1<<7)|(1<<10))
#define CL90135_MMC_DATA_WRITE_ERROR ((1<<4)|(1<<5)|(1<<8)|(1<<11))

// impostazione flags vari per esecuzione comandi
// bit0 => 1--> forza il check del busy alla fine del comando, come deve avvenire ad esempio
//         con tutti i comandi che richiedono una risposta di tipo r1b
// bit1 => 1--> disabilita il check del crc alla fine del comando, come deve avvenire ad esempio
//         con i comandi cmd2, cmd10, cmd9, cmd41
#define defCL90135_EMIFA_sdcard_FLAGS         ((defCL90135_EMIFA_sdcard_BASE + 35*2))

// dimensione in word a 16 bit del buffer delle word ricevute da sd
// se ad es vale 1024, il buffer delel word in lettura da sd può ospitare max 1024 word, cioè 2048 bytes
// (read_only)
#define defCL90135_EMIFA_sdcard_size_block_read  ((defCL90135_EMIFA_sdcard_BASE + 36*2))
// dimensione in word a 16 bit del buffer delle word da scrivere su sd
// se ad es vale 1024, il buffer delel word da scrivere su sd può ospitare max 1024 word, cioè 2048 bytes
// (read_only)
#define defCL90135_EMIFA_sdcard_size_block_write ((defCL90135_EMIFA_sdcard_BASE + 37*2))


// macro che serve a leggere irq status
#define CL90135_MMC_READ_IRQ_STATUS CL90135_MMC_READW(host,defCL90135_EMIFA_sdcard_IRQ)

#define defCL90135_EMIFA_sdcard_cmd1   1
#define defCL90135_EMIFA_sdcard_cmd55 55

#define defCL90135_Issue_CMD55         {CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_resp_len,48);                       \
                                        CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arglo,0x0000);                      \
                                        CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arghi,0x0000);                      \
                                        CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cmd,defCL90135_EMIFA_sdcard_cmd55); \
                                       }

#define defCL90135_Issue_CMD1          {CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_resp_len,48);                       \
                                        CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arglo,0x0000);                      \
                                        CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_arghi,0x0000);                      \
                                        CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_cmd,defCL90135_EMIFA_sdcard_cmd1);  \
                                       }

// realizza una reset della parte che realizza block read/write
#define CL90135_MMC_WRITEW(host, offset, val) __raw_writew((val), (host)->virt_base + offset)

#define defCL90135_Issue_block_reset {CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_block_reset,3);}
//#define defCL90135_Issue_block_reset {*defCL90135_EMIFA_sdcard_block_reset=3;}

// realizza una reset completo del modulo sd
#define defCL90135_Issue_complete_reset {CL90135_MMC_WRITEW(host,defCL90135_EMIFA_sdcard_block_reset,7);}
//#define defCL90135_Issue_complete_reset {*defCL90135_EMIFA_sdcard_block_reset=7;}

typedef enum{
    enum_sd_current_state_idle  =0,
    enum_sd_current_state_ready =1,
    enum_sd_current_state_ident =2,
    enum_sd_current_state_stby  =3,
    enum_sd_current_state_tran  =4,
    enum_sd_current_state_data  =5,
    enum_sd_current_state_rcv   =6,
    enum_sd_current_state_prg   =7,
    enum_sd_current_state_dis   =8,
    enum_sd_current_state_res   =9,
    enum_sd_current_numOf
}enum_sd_current_state;


//stato della card: occupa 32 bit
typedef struct _TipoCardStatus{
    unsigned int uiReserved     :3; //bit 2..0-->
    unsigned int uiAkeSeqError  :1; //bit 3--> 1-->errore nella sequenza di autenticazione
    unsigned int uiReserved_2   :1; //bit 4-->
    unsigned int uiApp_cmd      :1; //bit 5--> 1-->waiti for app cmd
    unsigned int uiReserved_3   :2; //bit 7..6-->
    unsigned int uiReadyForData :1; //bit 8-->  1--> ready
    enum_sd_current_state sd_current_state:4; //bit 12..9-->  sd current state
    unsigned int uiEraseReset   :1; //bit 13-->  1--> erase was cleared
    unsigned int uiCardEccDisabled:1;  //bit 14-->  1--> card ecc was disabled
    unsigned int uiWpEraseSkip  :1;    //bit 15-->  1--> only partial erase was done cause some blocks was protected
    unsigned int uiCsdOverwrite :1;    //bit 16-->  1--> csd overwrite
    unsigned int uiReserved_4   :2;    //bit 18..17-->
    unsigned int uiError        :1;    //bit 19-->
    unsigned int uiCC_Error     :1;    //bit 20-->  1-> internal card controller error
    unsigned int uiCardEccFailed:1;    //bit 21-->   1-> card ecc failed
    unsigned int uiIllegalCmd   :1;    //bit 22-->   1-> illegal command
    unsigned int uiCrcError     :1;    //bit 23-->   1-> crc error
    unsigned int uiLockUnlockFailed :1;//bit 24-->   1-> lock unlock failed
    unsigned int uiCardIsLocked :1;    //bit 25-->   1-> card is locked
    unsigned int uiWpViolation  :1;    //bit 26-->   1-> wp violation
    unsigned int uiEraseParamErr:1;    //bit 27-->   1-> erase param error
    unsigned int uiEraseSeqErr  :1;    //bit 28-->   1-> erase seq error
    unsigned int uiBlockLenErr  :1;    //bit 29-->   1-> number of bytes transferred not equal to set value, or block length not allowed
    unsigned int uiAddressErr   :1;    //bit 30-->   1-> address error
    unsigned int uiOutOfRange   :1;    //bit 31-->   1-> argument out of range
}TipoCardStatus;


typedef struct _TipoStruct48bit_group16{
    unsigned int uiBit15_0   :16;
    unsigned int uiBit31_16  :16;
    unsigned int uiBit47_32  :16;
}TipoStruct48bit_group16;

typedef struct _TipoStruct48bit_group8{
    unsigned int uiBit7_0      :16;
    unsigned int uiBit15_8     :16;
    unsigned int uiBit23_16    :16;
    unsigned int uiBit31_24    :16;
    unsigned int uiBit39_32    :16;
    unsigned int uiBit47_40    :16;
}TipoStruct48bit_group8;

// risposta di tipo r1: 48 bit
typedef struct _TipoStructSdR1{
    unsigned int uiEndBit   :1  ;
    unsigned int uiCrc      :7  ;
    TipoCardStatus cardStatus   ;
    unsigned int uiCmdIdx   :6  ;
    unsigned int uiTxBit    :1  ;
    unsigned int uiStartBit :1  ;
    unsigned int uiFill         ;
}TipoStructSdR1;

// risposta di tipo r1_core: 48 bit
typedef struct _TipoStructSdR1_core{
    unsigned int uiEndBit   :1  ;
    unsigned int uiCrc      :7  ;
    u32          u32_core   ;
    unsigned int uiCmdIdx   :6  ;
    unsigned int uiTxBit    :1  ;
    unsigned int uiStartBit :1  ;
    unsigned int uiFill         ;
}TipoStructSdR1_core;

typedef struct _TipoStructSdR1_union{
    union{
        TipoStructSdR1 r1;
        TipoStruct48bit_group16 b48_group16;
        TipoStruct48bit_group8 b48_group8;
        TipoStructSdR1_core r1_core;
    }u;
}TipoStructSdR1_union;

typedef struct _TipoStruct144bit_group16{
    unsigned int uiBit15_0   :16;
    unsigned int uiBit31_16  :16;
    unsigned int uiBit47_32  :16;
    unsigned int uiBit63_48  :16;
    unsigned int uiBit79_64  :16;
    unsigned int uiBit95_80  :16;
    unsigned int uiBit111_96  :16;
    unsigned int uiBit127_112 :16;
    unsigned int uiBit143_128 :16;
}TipoStruct144bit_group16;

// R2: 136 bit
typedef struct _TipoStructSdR2{
    unsigned int uiEndBit   :1;
	unsigned int uiCrc:7;
	unsigned int uiManufacturingDate:12;
	unsigned int uiReserved_2:4;
	unsigned int uiProductSerialNumber_0:8;
	unsigned int uiProductSerialNumber_1:8;
	unsigned int uiProductSerialNumber_2:8;
	unsigned int uiProductSerialNumber_3:8;
	unsigned int uiProductRevision:8;
	unsigned int uiProductName_0:16;
	unsigned int uiProductName_1:16;
	unsigned int uiProductName_2:8;
	unsigned int uiOemId:16;
	unsigned int uiManufacturerId:8;
    unsigned int uiCmdIdx   :6; // dovrebbe essere 111111b
    unsigned int uiTxBit    :1; // dovrebbe essere 0
    unsigned int uiStartBit :1; // dovrebbe essere 0
}TipoStructSdR2;

// R2_core: 136 bit in 4 gruppi u32 + spare parts
typedef struct _TipoStructSdR2_core_u32{
    u32 the_core[4]             ;
    unsigned int uiCmdIdx   :6  ; // dovrebbe essere 111111b
    unsigned int uiTxBit    :1  ; // dovrebbe essere 0
    unsigned int uiStartBit :1  ; // dovrebbe essere 0
    unsigned int uiFill         ;
}TipoStructSdR2_core_u32;


typedef struct _TipoStructSdR2_union{
    union{
        TipoStructSdR2 r2;
        TipoStruct144bit_group16 b144_group16;
        TipoStructSdR2_core_u32 core_u32;
    }u;
}TipoStructSdR2_union;


// definizione dei codici evento che possono essere presenti nella maschera letta da registro defCL90135_EMIFA_sdcard_IRQ

// command ends ok
#define CL90135_MMC_EVENT_END_OF_COMMAND        0x0001
// readblock ends ok
#define CL90135_MMC_EVENT_END_OF_READBLOCK      0x0002
// writeblock ends ok
#define CL90135_MMC_EVENT_END_OF_WRITEBLOCK     0x0004
// last command haa given a timeout
#define CL90135_MMC_EVENT_COMMAND_TIMEOUT       0x0008

// last readblock has given a timeout
#define CL90135_MMC_EVENT_DATA_READ_TIMEOUT     0x0010
// last writeblock has given a timeout
#define CL90135_MMC_EVENT_DATA_WRITE_TIMEOUT    0x0020
// last comamnd has given a crc error
#define CL90135_MMC_EVENT_CMD_CRCERR            0x0040
// last readblock has given a crc error
#define CL90135_MMC_EVENT_DATA_READ_CRCERR      0x0080

// last writelock has given a crc error
#define CL90135_MMC_EVENT_DATA_WRITE_CRCERR     0x0100
// last command has given a stopbit error
#define CL90135_MMC_EVENT_CMD_ERR               0x0200
// last dataread has given a stopbit error
#define CL90135_MMC_EVENT_DATA_READ_ERR         0x0400
// last datawrite has given a stopbit error (should NEVER happens)
#define CL90135_MMC_EVENT_DATA_WRITE_ERR        0x0800

#define CL90135_MMC_EVENT_MASK_NO_FIFO          0x0fff



// macro che permette di selezionare hi SD clock speed
#define defCL90135_EMIFA_sdcard_speedSelect_Hi { CL90135_MMC_WRITEW(host, defCL90135_EMIFA_sdcard_speedSelect,2);}
    //#define defCL90135_EMIFA_sdcard_speedSelect_Hi {*defCL90135_EMIFA_sdcard_speedSelect=2;}
// macro che permette di selezionare lo SD clock speed
#define defCL90135_EMIFA_sdcard_speedSelect_Lo { CL90135_MMC_WRITEW(host, defCL90135_EMIFA_sdcard_speedSelect,1);}
    //#define defCL90135_EMIFA_sdcard_speedSelect_Lo {*defCL90135_EMIFA_sdcard_speedSelect=1;}


// struttura con le info sullo stato degli i/o della sd card
// letta da defCL90135_EMIFA_sdcard_IO_STATUS
typedef struct _TipoCL90135_StructSdCardIoStatus{
    unsigned int uiWriteProtected:1; // bit  0: --> ingresso WP=write protect: 0=card NOT locked, 1=card locked
    unsigned int uiPresent:1;        // bit  1: --> ingresso INS=card inserita: 0=card not inserted, 1=card inserted
    unsigned int uiCmd:1;            // bit  2: --> valore attualmente presente sulla linea CMD
    unsigned int uiData_0:1;         // bit  3: --> valore attualmente presente sulla linea DATA(0)
    unsigned int uiData_1:1;         // bit  4: --> valore attualmente presente sulla linea DATA(1)
    unsigned int uiData_2:1;         // bit  5: --> valore attualmente presente sulla linea DATA(2)
    unsigned int uiData_3:1;         // bit  6: --> valore attualmente presente sulla linea DATA(3)
	unsigned int uiFill;
}TipoCL90135_StructSdCardIoStatus;


// numero max di blocchi da 512bytes in lettura/scrittura
#define defCL90135_EMIFA_max_Block_ReadBlock 4
#define defCL90135_EMIFA_max_Block_WriteBlock 4
// numero max di bytes in lettura/scrittura blocchi
#define defCL90135_EMIFA_max_bytes_ReadBlock (defCL90135_EMIFA_max_Block_ReadBlock*512)
#define defCL90135_EMIFA_max_bytes_WriteBlock (defCL90135_EMIFA_max_Block_WriteBlock*512)// sd result
typedef struct _TipoStructSdCommandStatus{
    unsigned int uiBusy:1;
    unsigned int uiValidResp:1;
    unsigned int uiCrcErr:1;
    unsigned int uiStopbitErr:1;
    unsigned int uiTimeoutErr:1;
    unsigned int uiCmdRunningErr:1;
    unsigned int uiBlockWriteErr:1;
    unsigned int uiToggleOnCommand:1;
    unsigned int uiFill;
}TipoStructSdCommandStatus;

