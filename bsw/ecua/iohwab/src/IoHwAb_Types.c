#include "IoHwAb_Types.h"
#include "IoHwAb_Adc.h"
#include "IoHwAb_Digital.h"
void IoHwAb_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
    if (versioninfo != NULL)
    {
        versioninfo->vendorID = IoHwAb_VENDOR_ID;
        versioninfo->moduleID = IoHwAb_MODULE_ID;
        versioninfo->sw_major_version = IoHwAb_SW_MAJOR_VERSION;
        versioninfo->sw_minor_version = IoHwAb_SW_MINOR_VERSION;
        versioninfo->sw_patch_version = IoHwAb_SW_PATCH_VERSION;
    }
}

void IoHwAb_MainFunction(void)
{
    // Implement periodic tasks here if needed
}