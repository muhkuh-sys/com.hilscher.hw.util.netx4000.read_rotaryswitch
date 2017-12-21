
typedef struct STRUCT_DPM_CONFIGURATION
{
	unsigned long   ulEnable;
	unsigned long   ulDpmCfg0x0;
	unsigned long   ulDpmIfCfg; 
	unsigned long   ulDpmPioCfg0;
	unsigned long   ulDpmPioCfg1;
	unsigned long   ulDpmAddrCfg;
	unsigned long   ulDpmTimingCfg;
	unsigned long   ulDpmRdyCfg;
	unsigned long   ulDpmMiscCfg;
	unsigned long   ulDpmIoCfgMisc;
} NETX90MPW_HWCONFIG_DPM_T;

int init_sdpm(const NETX90MPW_HWCONFIG_DPM_T* pt_hif_options, unsigned int uiUnit);
int init_pdpm(const NETX90MPW_HWCONFIG_DPM_T* pt_hif_options);
