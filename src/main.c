#include "netx_io_areas.h"
#include "rdy_run.h"
#include "setup_dpm.h"


/* Define the structures for the hwconfig data. */
typedef struct NETX90MPW_HWCONFIG_MMIO_STRUCT
{
	unsigned long aulMMIO[16];
} NETX90MPW_HWCONFIG_MMIO_T;

typedef struct NETX90MPW_HWCONFIG_PAD_CTRL_STRUCT
{
	unsigned long aulPadCtrl[101];
} NETX90MPW_HWCONFIG_PAD_CTRL_T;

typedef struct NETX90MPW_HWCONFIG_IO_CONFIG_STRUCT
{
	unsigned long aulIoConfig[9];
} NETX90MPW_HWCONFIG_IO_CONFIG_T;

typedef struct NETX90MPW_HWCONFIG_HIF_IO_CTRL_STRUCT
{
	unsigned long  ulHif_io_cfg;
	unsigned long  ulHif_pio_cfg;
	unsigned long  ulHif_pio_out0;
	unsigned long  ulHif_pio_out1;
	unsigned long  ulHif_pio_oe0;
	unsigned long  ulHif_pio_oe1;
} NETX90MPW_HWCONFIG_HIF_IO_CTRL_T;

typedef struct NETX90MPW_HWCONFIG_HIF_ASYNCMEM_CTRL_STRUCT
{
	unsigned long  ulExtsram0_ctrl;
	unsigned long  ulExtsram1_ctrl;
	unsigned long  ulExtsram2_ctrl;
	unsigned long  ulExtsram3_ctrl;
	unsigned long  ulExt_cs0_apm_ctrl;
	unsigned long  ulExt_rdy_cfg;
} NETX90MPW_HWCONFIG_HIF_ASYNCMEM_CTRL_T;

typedef struct NETX90MPW_HWCONFIG_HIF_SDRAM_CTRL_STRUCT
{
	unsigned long  ulSdram_general_ctrl;
	unsigned long  ulSdram_timing_ctrl;
	unsigned long  ulSdram_mr;
} NETX90MPW_HWCONFIG_HIF_SDRAM_CTRL_T;

typedef struct NETX90MPW_HWCONFIG_HIFMEM_PRIORITY_CTRL_STRUCT
{
	unsigned long  ulExtmem_prio_timslot_ctrl;
	unsigned long  ulExtmem_prio_accesstime_ctrl;
} NETX90MPW_HWCONFIG_HIFMEM_PRIORITY_CTRL_T;


typedef struct NETX90MPW_HWCONFIG_STRUCT
{
	NETX90MPW_HWCONFIG_MMIO_T tMmio;
	NETX90MPW_HWCONFIG_PAD_CTRL_T tPadCtrl;
	NETX90MPW_HWCONFIG_IO_CONFIG_T tIoConfig;
	NETX90MPW_HWCONFIG_HIF_IO_CTRL_T tHifIoCtrl;
	NETX90MPW_HWCONFIG_HIF_ASYNCMEM_CTRL_T tHifAsyncmemCtrl;
	NETX90MPW_HWCONFIG_HIF_SDRAM_CTRL_T tHifSdramCtrl;
	NETX90MPW_HWCONFIG_HIFMEM_PRIORITY_CTRL_T tHifmemPriorityCtrl;
	NETX90MPW_HWCONFIG_DPM_T tDpm0Cfg;
	NETX90MPW_HWCONFIG_DPM_T tDpm1Cfg;
} NETX90MPW_HWCONFIG_T;



static void apply_mmios(const NETX90MPW_HWCONFIG_MMIO_T * const ptCfg)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	unsigned int uiCnt;
	unsigned long ulValue;


	/* Loop over all MMIO pins and set the complete register. */
	for(uiCnt=0; uiCnt<16; uiCnt++)
	{
		ulValue = ptCfg->aulMMIO[uiCnt];
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
		ptMmioCtrlArea->aulMmio_cfg[uiCnt] = ulValue;
	}
}



static void apply_pad_ctrl(const NETX90MPW_HWCONFIG_PAD_CTRL_T * const ptCfg)
{
	HOSTDEF(ptAsicCtrlArea);
	volatile unsigned long *pulPadCtrl = (volatile unsigned long *)HOSTADDR(pad_ctrl);
	unsigned int uiCnt;
	unsigned long ulValue;


	/* Loop over all pad controls and set the complete register. */
	for(uiCnt=0; uiCnt<101; uiCnt++)
	{
		ulValue = ptCfg->aulPadCtrl[uiCnt];
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
		pulPadCtrl[uiCnt] = ulValue;
	}
}



static void apply_io_config(const NETX90MPW_HWCONFIG_IO_CONFIG_T * const ptCfg)
{
	HOSTDEF(ptAsicCtrlArea);
	unsigned int uiCnt;
	unsigned long ulValue;


	/* Loop over all IO configuration register. */
	for(uiCnt=0; uiCnt<9; uiCnt++)
	{
		ulValue = ptCfg->aulIoConfig[uiCnt];
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
		ptAsicCtrlArea->asIo_config[uiCnt].ulConfig = ulValue;
	}
}



static void apply_hif_io_ctrl(const NETX90MPW_HWCONFIG_HIF_IO_CTRL_T * const ptCfg)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	unsigned long ulValue;


	ulValue = ptCfg->ulHif_io_cfg;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
	ptHifIoCtrlArea->ulHif_io_cfg = ulValue;

	ptHifIoCtrlArea->ulHif_pio_cfg = ptCfg->ulHif_pio_cfg;
	ptHifIoCtrlArea->aulHif_pio_out[0] = ptCfg->ulHif_pio_out0;
	ptHifIoCtrlArea->aulHif_pio_out[1] = ptCfg->ulHif_pio_out1;
	ptHifIoCtrlArea->aulHif_pio_oe[0] = ptCfg->ulHif_pio_oe0;
	ptHifIoCtrlArea->aulHif_pio_oe[1] = ptCfg->ulHif_pio_oe1;
}


typedef struct NX90_HIF_ASYNCMEM_CTRL_AREA_STRUCT
{
	volatile unsigned long aulExtsram_ctrl[4];
	volatile unsigned long  ulExt_cs0_apm_ctrl;
	volatile unsigned long aulReserved14[3];
	volatile unsigned long  ulExt_rdy_cfg;
	volatile unsigned long  ulExt_rdy_status;
} NX90_HIF_ASYNCMEM_CTRL_AREA_T;

#define NX90_DEF_ptHifAsyncmemCtrlArea NX90_HIF_ASYNCMEM_CTRL_AREA_T * const ptHifAsyncmemCtrlArea = (NX90_HIF_ASYNCMEM_CTRL_AREA_T * const)Addr_NX90_hif_asyncmem_ctrl;

static void apply_hif_asyncmem_ctrl(const NETX90MPW_HWCONFIG_HIF_ASYNCMEM_CTRL_T * const ptCfg)
{
	HOSTDEF(ptHifAsyncmemCtrlArea);


	ptHifAsyncmemCtrlArea->aulExtsram_ctrl[0] = ptCfg->ulExtsram0_ctrl;
	ptHifAsyncmemCtrlArea->aulExtsram_ctrl[1] = ptCfg->ulExtsram1_ctrl;
	ptHifAsyncmemCtrlArea->aulExtsram_ctrl[2] = ptCfg->ulExtsram2_ctrl;
	ptHifAsyncmemCtrlArea->aulExtsram_ctrl[3] = ptCfg->ulExtsram3_ctrl;
	ptHifAsyncmemCtrlArea->ulExt_rdy_cfg = ptCfg->ulExt_cs0_apm_ctrl;
}



typedef struct NX90_HIF_SDRAM_CTRL_AREA_Ttag
{
  volatile unsigned long  ulSdram_general_ctrl;
  volatile unsigned long  ulSdram_timing_ctrl;
  volatile unsigned long  ulSdram_mr;
} NX90_HIF_SDRAM_CTRL_AREA_T;

#define NX90_DEF_ptHifSdramCtrlArea NX90_HIF_SDRAM_CTRL_AREA_T * const ptHifSdramCtrlArea = (NX90_HIF_SDRAM_CTRL_AREA_T * const)Addr_NX90_hif_sdram_ctrl;

static void apply_hif_sdram_ctrl(const NETX90MPW_HWCONFIG_HIF_SDRAM_CTRL_T * const ptCfg)
{
	HOSTDEF(ptHifSdramCtrlArea);


	/* Deactivate the SDRAM controller. */
	ptHifSdramCtrlArea->ulSdram_general_ctrl = 0;

	ptHifSdramCtrlArea->ulSdram_timing_ctrl = ptCfg->ulSdram_timing_ctrl;
	ptHifSdramCtrlArea->ulSdram_mr = ptCfg->ulSdram_mr;
	ptHifSdramCtrlArea->ulSdram_general_ctrl = ptCfg->ulSdram_general_ctrl;
}

/* If Ctrl_en is 1, wait until sdram_ready is 1 */
static void hif_sdram_wait_for_ready(void)
{
	HOSTDEF(ptHifSdramCtrlArea);
	if ((ptHifSdramCtrlArea->ulSdram_general_ctrl & MSK_NX90_sdram_general_ctrl_ctrl_en) != 0)
	{
		while ((ptHifSdramCtrlArea->ulSdram_general_ctrl &  MSK_NX90_sdram_general_ctrl_sdram_ready) == 0 ) {}
	}
}


typedef struct NX90_HIFMEM_PRIORITY_CTRL_AREA_STRUCT
{
	volatile unsigned long  ulExtmem_prio_timslot_ctrl;
	volatile unsigned long  ulExtmem_prio_accesstime_ctrl;
} NX90_HIFMEM_PRIORITY_CTRL_AREA_T;

#define NX90_DEF_ptHifmemPriorityCtrlArea NX90_HIFMEM_PRIORITY_CTRL_AREA_T * const ptHifmemPriorityCtrlArea = (NX90_HIFMEM_PRIORITY_CTRL_AREA_T * const)Addr_NX90_hifmem_priority_ctrl;

static void apply_hifmem_priority_ctrl(const NETX90MPW_HWCONFIG_HIFMEM_PRIORITY_CTRL_T * const ptCfg)
{
	HOSTDEF(ptHifmemPriorityCtrlArea);


	ptHifmemPriorityCtrlArea->ulExtmem_prio_timslot_ctrl = ptCfg->ulExtmem_prio_timslot_ctrl;
	ptHifmemPriorityCtrlArea->ulExtmem_prio_accesstime_ctrl = ptCfg->ulExtmem_prio_accesstime_ctrl;
}



void start(const NETX90MPW_HWCONFIG_T * const ptHwconfig);
void __attribute__ ((section (".init_code"))) start(const NETX90MPW_HWCONFIG_T * const ptHwconfig)
{
	apply_mmios( &(ptHwconfig->tMmio) );
	apply_pad_ctrl( &(ptHwconfig->tPadCtrl) );
	apply_io_config( &(ptHwconfig->tIoConfig) );
	apply_hif_io_ctrl( &(ptHwconfig->tHifIoCtrl) );
	apply_hif_asyncmem_ctrl( &(ptHwconfig->tHifAsyncmemCtrl) );
	apply_hif_sdram_ctrl( &(ptHwconfig->tHifSdramCtrl) );
	apply_hifmem_priority_ctrl( &(ptHwconfig->tHifmemPriorityCtrl) );
	if (ptHwconfig->tDpm0Cfg.ulEnable != 0) 
	{
		if ((ptHwconfig->tHifIoCtrl.ulHif_io_cfg & MSK_NX90_hif_io_cfg_sel_dpm_serial) == 0)
		{
			init_pdpm( &(ptHwconfig->tDpm0Cfg) );
		}
		else
		{
			init_sdpm( &(ptHwconfig->tDpm0Cfg), 0 );
			if (ptHwconfig->tDpm1Cfg.ulEnable != 0) 
			{
				init_sdpm( &(ptHwconfig->tDpm1Cfg), 1 );
			}
		}
		
	}
	
	hif_sdram_wait_for_ready();
}
