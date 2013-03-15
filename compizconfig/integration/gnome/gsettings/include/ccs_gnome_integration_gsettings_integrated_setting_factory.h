#ifndef _CCS_MATE_GSETTINGS_INTEGRATED_SETTING_FACTORY_H
#define _CCS_MATE_GSETTINGS_INTEGRATED_SETTING_FACTORY_H

#include <ccs-defs.h>
#include <ccs-fwd.h>
#include <ccs_mate_fwd.h>
#include <ccs_gsettings_backend_fwd.h>
#include <gio/gio.h>

COMPIZCONFIG_BEGIN_DECLS

/**
 * @brief ccsGSettingsIntegratedSettingsTranslateNewMATEKeyForCCS
 * @param key the old style mate key to translate
 * @return new-style key. Caller should free
 *
 * This translates new style keys (eg foo-bar) to old style keys
 * foo_bar and special cases a few keys
 */
char *
ccsGSettingsIntegratedSettingsTranslateNewMATEKeyForCCS (const char *key);

/**
 * @brief ccsGSettingsIntegratedSettingsChangeCallback
 * @return callback for settings change data
 *
 * This returns the default callback used for settings changes
 *
 * TODO: This API doesn't make a whole lot of sense, but we need
 * it if we want to inject CCSGSettingsWrapperFactory into
 * ccsGSettingsIntegratedSettingFactoryNew.
 *
 * The return type is GCallback to hide the details of this function
 * from callers
 */
GCallback
ccsGSettingsIntegratedSettingsChangeCallback ();

CCSIntegratedSettingFactory *
ccsGSettingsIntegratedSettingFactoryNew (CCSGSettingsWrapperFactory   *wrapperFactory,
					 CCSMATEValueChangeData      *data,
					 CCSObjectAllocationInterface *ai);

COMPIZCONFIG_END_DECLS

#endif
