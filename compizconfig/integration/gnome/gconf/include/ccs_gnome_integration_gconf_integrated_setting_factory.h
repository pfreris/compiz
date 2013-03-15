#ifndef _CCS_MATE_MATECONF_INTEGRATED_SETTING_FACTORY_H
#define _CCS_MATE_MATECONF_INTEGRATED_SETTING_FACTORY_H

#include <ccs-defs.h>
#include <ccs-object.h>
#include <ccs-fwd.h>
#include <ccs_mate_fwd.h>
#include <mateconf/mateconf-client.h>

COMPIZCONFIG_BEGIN_DECLS

/**
 * @brief ccsMateConfIntegratedSettingFactoryNew
 * @param client an existing MateConfClient or NULL
 * @param data some data to pass to the change callback
 * @param ai a CCSObjectAllocationInterface
 * @return a new CCSIntegratedSettingFactory
 *
 * CCSMateConfIntegratedSettingFactory implements CCSIntegratedSettingFactory *
 * and will create CCSMateConfIntegratedSetting objects (which implement
 * CCSIntegratedSetting).
 */
CCSIntegratedSettingFactory *
ccsMateConfIntegratedSettingFactoryNew (MateConfClient		  *client,
				     CCSMATEValueChangeData	  *data,
				     CCSObjectAllocationInterface *ai);

COMPIZCONFIG_END_DECLS

#endif
