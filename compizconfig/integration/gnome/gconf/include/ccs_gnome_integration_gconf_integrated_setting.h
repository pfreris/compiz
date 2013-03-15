#ifndef _CCS_MATE_MATECONF_INTEGRATED_SETTING_H
#define _CCS_MATE_MATECONF_INTEGRATED_SETTING_H

#include <ccs-defs.h>
#include <ccs-fwd.h>
#include <ccs_mate_fwd.h>
#include <mateconf/mateconf-client.h>

COMPIZCONFIG_BEGIN_DECLS

/**
 * @brief ccsMateConfIntegratedSettingNew
 * @param base a CCSMATEIntegratedSetting
 * @param client a MateConfClient
 * @param section the preceeding path to the keyname
 * @param ai a CCSObjectAllocationInterface
 * @return
 *
 * Creates the MateConf implementation of a CCSIntegratedSetting, which will
 * write to MateConf keys when necessary.
 */
CCSIntegratedSetting *
ccsMateConfIntegratedSettingNew (CCSMATEIntegratedSettingInfo *base,
			      MateConfClient		*client,
			      const char		*section,
			      CCSObjectAllocationInterface *ai);

COMPIZCONFIG_END_DECLS

#endif
