#include <string.h>
#include "netx_io_areas.h"
#include "setup_dpm.h"

#define MESSAGE_DPM_PARALLEL "netX90 MPW parallel DPM"
#define MESSAGE_DPM_SERIAL "netX90 MPW serial DPM"

#define __IRQ_LOCK__   {__asm__ volatile ("cpsid   i");}
#define __IRQ_UNLOCK__ {__asm__ volatile ("cpsie   i");}

static void dpm_set_hif_io_cfg(unsigned long ulVal)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	/* Configure the HIF pins. */
	__IRQ_LOCK__;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
	ptHifIoCtrlArea->ulHif_io_cfg = ulVal;
	__IRQ_UNLOCK__;
}

static int dpm_deinit_registers(unsigned int uiUnit)
{
	HOSTDEF(ptDpm0ComArea);
	HOSTDEF(ptDpm1ComArea);
	HOSTADEF(DPM) *ptDpmArea;
	HOSTDEF(ptHandshakeComArea);
	int iResult;
	unsigned int sizCnt;


	/* Be pessimistic. */
	iResult = -1;
	if( uiUnit==0 )
	{
		ptDpmArea = ptDpm0ComArea;
		iResult = 0;
	}
	else if( uiUnit==1 )
	{
		ptDpmArea = ptDpm1ComArea;
		iResult = 0;
	}

	if( iResult==0 )
	{
		/*
		 * disable the interface
		 */

		/* Configure the HIF pins. */
		//dpm_set_hif_io_cfg(HOSTDFLT(hif_io_cfg));

		/* Disable all windows and write protect them. */
		ptDpmArea->ulDpm_win1_end = 0U;
		ptDpmArea->ulDpm_win1_map = HOSTMSK(dpm_win1_map_wp_cfg_win);
		ptDpmArea->ulDpm_win2_end = 0U;
		ptDpmArea->ulDpm_win2_map = 0U;
		ptDpmArea->ulDpm_win3_end = 0U;
		ptDpmArea->ulDpm_win3_map = 0U;
		ptDpmArea->ulDpm_win4_end = 0U;
		ptDpmArea->ulDpm_win4_map = 0U;

		/* Disable the tunnel and write protect it. */
		ptDpmArea->ulDpm_tunnel_cfg = HOSTMSK(dpm_tunnel_cfg_wp_cfg_win);

		/* Read all handshake registers and disable them. */
		sizCnt = sizeof(ptHandshakeComArea->aulHandshake_hsc_ctrl)/sizeof(unsigned long);
		do
		{
			ptHandshakeComArea->aulHandshake_hsc_ctrl[--sizCnt] = 0;
		} while( sizCnt!=0 );

		/* Disable all handshake IRQs. */
		ptHandshakeComArea->ulHandshake_dpm_irq_raw_clear = 0xffffffff;
		ptHandshakeComArea->ulHandshake_dpm_irq_msk_reset = 0xffffffff;
		ptHandshakeComArea->ulHandshake_arm_irq_raw_clear = 0xffffffff;
		ptHandshakeComArea->ulHandshake_arm_irq_msk_reset = 0xffffffff;
		ptHandshakeComArea->ulHandshake_xpic_irq_raw_clear = 0xffffffff;
		ptHandshakeComArea->ulHandshake_xpic_irq_msk_reset = 0xffffffff;

		sizCnt = sizeof(ptHandshakeComArea->asHandshake_buf_man)/sizeof(ptHandshakeComArea->asHandshake_buf_man[0]);
		do
		{
			--sizCnt;
			ptHandshakeComArea->asHandshake_buf_man[sizCnt].ulCtrl = HOSTDFLT(handshake_buf_man0_ctrl);
			ptHandshakeComArea->asHandshake_buf_man[sizCnt].ulStatus_ctrl_netx = HOSTDFLT(handshake_buf_man0_status_ctrl_netx);
			ptHandshakeComArea->asHandshake_buf_man[sizCnt].ulWin_map = 0;
		} while( sizCnt!=0 );

		/* Reset all IRQ bits. */
		ptDpmArea->aulDpm_irq_host_dirq_mask_reset[0] = 0xffffffff;
		ptDpmArea->aulDpm_irq_host_dirq_mask_reset[1] = 0xffffffff;
		ptDpmArea->aulDpm_irq_host_sirq_mask_reset[0] = 0xffffffff;
		ptDpmArea->aulDpm_irq_host_sirq_mask_reset[1] = 0xffffffff;
		ptDpmArea->ulDpm_firmware_irq_mask = 0;
	}

	return iResult;
}


static int init_dpm(const NETX90MPW_HWCONFIG_DPM_T* pt_hif_options, unsigned int uiUnit)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptDpm0ComArea);
	HOSTDEF(ptDpm1ComArea);
	HOSTADEF(DPM) * ptDpmArea;
	int iResult;
	unsigned long ulValue;
	unsigned long ulNetxAdr;


	/* Be pessimistic. */
	iResult = -1;
	if( uiUnit==0 )
	{
		ptDpmArea = ptDpm0ComArea;
		iResult = 0;
	}
	else if( uiUnit==1 )
	{
		ptDpmArea = ptDpm1ComArea;
		iResult = 0;
	}

	if( iResult==0 )
	{
		/* Can the DPM clocks be enabled? */
		if( (ptAsicCtrlArea->asClock_enable[0].ulMask & HOSTMSK(clock_enable0_dpm))==0 )
		{
			/* No, the clocks can not be enabled. */
			iResult = -1;
		}
		else
		{
			/* Enable the DPM clock. */
			ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
			ulValue |= HOSTMSK(clock_enable0_dpm);
			__IRQ_LOCK__;
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
			ptAsicCtrlArea->asClock_enable[0].ulEnable = ulValue;
			__IRQ_UNLOCK__;

			/* Disable the DPM for new configuration. */
			iResult = dpm_deinit_registers(uiUnit);
			if( iResult==0 )
			{
				/* DPM mapping:
				 * 0x0000 - 0xffff : intramhs_dpm_mirror
				 */
				ptDpmArea->ulDpm_win1_end = 0; /* 0xffffU + 1; */
				ulNetxAdr = HOSTADDR(intramhs_dpm_mirror);
				ulValue  = (ulNetxAdr-0x0100U) & HOSTMSK(dpm_win1_map_win_map);
				ulValue |= ulNetxAdr & HOSTMSK(dpm_win1_map_win_page);
				ulValue |= HOSTMSK(dpm_win1_map_wp_cfg_win);
				ulValue |= HOSTMSK(dpm_win1_map_read_ahead);
				ulValue |= HOSTMSK(dpm_win1_map_byte_area);
				ptDpmArea->ulDpm_win1_map = ulValue;

				ptDpmArea->ulDpm_win2_end = 0U;
				ptDpmArea->ulDpm_win2_map = 0U;

				ptDpmArea->ulDpm_win3_end = 0U;
				ptDpmArea->ulDpm_win3_map = 0U;

				ptDpmArea->ulDpm_win4_end = 0U;
				ptDpmArea->ulDpm_win4_map = 0U;

				/* Disable the tunnel and write protect it. */
				ptDpmArea->ulDpm_tunnel_cfg = HOSTMSK(dpm_tunnel_cfg_wp_cfg_win);

				/* configure DPM */
				ptDpmArea->ulDpm_if_cfg       = pt_hif_options->ulDpmIfCfg;
				ptDpmArea->ulDpm_cfg0x0       = pt_hif_options->ulDpmCfg0x0;
				/* Disable the configuration window. */
				ptDpmArea->ulDpm_addr_cfg     = pt_hif_options->ulDpmAddrCfg | HOSTMSK(dpm_addr_cfg_cfg_win_addr_cfg);
				ptDpmArea->ulDpm_timing_cfg   = pt_hif_options->ulDpmTimingCfg;
				ptDpmArea->ulDpm_rdy_cfg      = pt_hif_options->ulDpmRdyCfg;
				ptDpmArea->ulDpm_misc_cfg     = pt_hif_options->ulDpmMiscCfg;
				ptDpmArea->ulDpm_io_cfg_misc  = pt_hif_options->ulDpmIoCfgMisc;
				ptDpmArea->aulDpm_pio_cfg[0]  = pt_hif_options->ulDpmPioCfg0;
				ptDpmArea->aulDpm_pio_cfg[1]  = pt_hif_options->ulDpmPioCfg1;

				/* Configure the HIF pins */
				//dpm_set_hif_io_cfg(pt_hif_options->ulHifIoCfg;);

				iResult = 0;
			}
		}
	}

	return iResult;
}



int init_pdpm(const NETX90MPW_HWCONFIG_DPM_T* pt_hif_options)
{
	void *pvDPM;
	unsigned int uiDPMUnit;
	int iResult;

	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 65536);
	memcpy(pvDPM, MESSAGE_DPM_PARALLEL, sizeof(MESSAGE_DPM_PARALLEL));

	/* Always use DPM unit 0 for the parallel DPM. */
	uiDPMUnit = 0;
	iResult = init_dpm(pt_hif_options, uiDPMUnit);

	return iResult;
}



int init_sdpm(const NETX90MPW_HWCONFIG_DPM_T* pt_hif_options, unsigned int uiUnit)
{
	void *pvDPM;
	int iResult;

	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 65536);
	memcpy(pvDPM, MESSAGE_DPM_SERIAL, sizeof(MESSAGE_DPM_SERIAL));

	iResult = init_dpm(pt_hif_options, uiUnit);

	return iResult;
}

