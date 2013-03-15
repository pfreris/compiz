#include <stdlib.h>
#include <string.h>

#include <mateconf/mateconf.h>
#include <mateconf/mateconf-client.h>
#include <mateconf/mateconf-value.h>

#include <ccs.h>
#include <ccs-backend.h>
#include <ccs-object.h>

#include "ccs_mate_integration.h"
#include "ccs_mate_integrated_setting.h"
#include "ccs_mate_integration_constants.h"
#include "ccs_mate_integration_mateconf_integrated_setting.h"

typedef struct _CCSMateConfIntegratedSettingFactoryPrivate CCSMateConfIntegratedSettingFactoryPrivate;

struct _CCSMateConfIntegratedSettingFactoryPrivate
{
    MateConfClient *client;
    guint       mateMateConfNotifyIds[NUM_WATCHED_DIRS];
    GHashTable  *pluginsToSettingsSectionsHashTable;
    GHashTable  *pluginsToSettingsSpecialTypesHashTable;
    GHashTable  *pluginsToSettingNameMATENameHashTable;
    CCSMATEValueChangeData *valueChangedData;
};

static void
mateMateConfValueChanged (MateConfClient *client,
			guint       cnxn_id,
			MateConfEntry  *entry,
			gpointer    user_data)
{
    CCSMATEValueChangeData *data = (CCSMATEValueChangeData *) user_data;
    const gchar *keyName = mateconf_entry_get_key (entry);
    gchar       *baseName = g_path_get_basename (keyName);

    /* We don't care if integration is not enabled */
    if (!ccsGetIntegrationEnabled (data->context))
	return;

    CCSIntegratedSettingList settingList = ccsIntegratedSettingsStorageFindMatchingSettingsByPredicate (data->storage,
													ccsMATEIntegrationFindSettingsMatchingPredicate,
													baseName);

    ccsIntegrationUpdateIntegratedSettings (data->integration,
					    data->context,
					    settingList);

    g_free (baseName);
}

static CCSIntegratedSetting *
createNewMateConfIntegratedSetting (MateConfClient *client,
				 const char  *sectionName,
				 const char  *mateName,
				 const char  *pluginName,
				 const char  *settingName,
				 CCSSettingType type,
				 SpecialOptionType specialOptionType,
				 CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSettingInfo *sharedIntegratedSettingInfo = ccsSharedIntegratedSettingInfoNew (pluginName, settingName, type, ai);

    if (!sharedIntegratedSettingInfo)
	return NULL;

    CCSMATEIntegratedSettingInfo *mateIntegratedSettingInfo = ccsMATEIntegratedSettingInfoNew (sharedIntegratedSettingInfo, specialOptionType, mateName, ai);

    if (!mateIntegratedSettingInfo)
    {
	ccsIntegratedSettingInfoUnref (sharedIntegratedSettingInfo);
	return NULL;
    }

    CCSIntegratedSetting *mateconfIntegratedSetting = ccsMateConfIntegratedSettingNew (mateIntegratedSettingInfo, client, sectionName, ai);

    if (!mateconfIntegratedSetting)
    {
	ccsIntegratedSettingInfoUnref ((CCSIntegratedSettingInfo *) mateIntegratedSettingInfo);
	return NULL;
    }

    return mateconfIntegratedSetting;
}

static void
finiMateConfClient (MateConfClient    *client,
		 guint		*mateMateConfNotifyIds)
{
    int i;

    mateconf_client_clear_cache (client);

    for (i = 0; i < NUM_WATCHED_DIRS; i++)
    {
	if (mateMateConfNotifyIds[i])
	{
	    mateconf_client_notify_remove (client, mateMateConfNotifyIds[0]);
	    mateMateConfNotifyIds[i] = 0;
	}
	mateconf_client_remove_dir (client, watchedMateConfMateDirectories[i], NULL);
    }
    mateconf_client_suggest_sync (client, NULL);

    g_object_unref (client);
}

static void
registerMateConfClient (MateConfClient    *client,
		     guint	    *mateMateConfNotifyIds,
		     CCSMATEValueChangeData *data,
		     MateConfClientNotifyFunc func)
{
    int i;

    for (i = 0; i < NUM_WATCHED_DIRS; i++)
	mateMateConfNotifyIds[i] = mateconf_client_notify_add (client,
							  watchedMateConfMateDirectories[i],
							  func, (gpointer) data,
							  NULL, NULL);
}

static void
initMateConfClient (CCSIntegratedSettingFactory *factory)
{
    int i;
    CCSMateConfIntegratedSettingFactoryPrivate *priv = (CCSMateConfIntegratedSettingFactoryPrivate *) ccsObjectGetPrivate (factory);
    priv->client = mateconf_client_get_default ();

    for (i = 0; i < NUM_WATCHED_DIRS; i++)
	mateconf_client_add_dir (priv->client, watchedMateConfMateDirectories[i],
			      MATECONF_CLIENT_PRELOAD_NONE, NULL);
}

CCSIntegratedSetting *
ccsMateConfIntegratedSettingFactoryCreateIntegratedSettingForCCSSettingNameAndType (CCSIntegratedSettingFactory *factory,
										 CCSIntegration		     *integration,
										 const char		     *pluginName,
										 const char		     *settingName,
										 CCSSettingType		     type)
{
    CCSMateConfIntegratedSettingFactoryPrivate *priv = (CCSMateConfIntegratedSettingFactoryPrivate *) ccsObjectGetPrivate (factory);
    GHashTable                              *settingsSectionsHashTable = g_hash_table_lookup (priv->pluginsToSettingsSectionsHashTable, pluginName);
    GHashTable                              *settingsSpecialTypesHashTable = g_hash_table_lookup (priv->pluginsToSettingsSpecialTypesHashTable, pluginName);
    GHashTable				    *settingsSettingNameMATENameHashTable = g_hash_table_lookup (priv->pluginsToSettingNameMATENameHashTable, pluginName);

    if (!priv->client)
	initMateConfClient (factory);

    if (!priv->mateMateConfNotifyIds[0])
	registerMateConfClient (priv->client, priv->mateMateConfNotifyIds, priv->valueChangedData, mateMateConfValueChanged);

    if (settingsSectionsHashTable &&
	settingsSpecialTypesHashTable &&
	settingsSettingNameMATENameHashTable)
    {
	const gchar *sectionName = g_hash_table_lookup (settingsSectionsHashTable, settingName);
	SpecialOptionType specialType = (SpecialOptionType) GPOINTER_TO_INT (g_hash_table_lookup (settingsSpecialTypesHashTable, settingName));
	const gchar *integratedName = g_hash_table_lookup (settingsSettingNameMATENameHashTable, settingName);

	return createNewMateConfIntegratedSetting (priv->client,
						sectionName,
						integratedName,
						pluginName,
						settingName,
						type,
						specialType,
						factory->object.object_allocation);
    }


    return NULL;
}

void
ccsMateConfIntegratedSettingFactoryFree (CCSIntegratedSettingFactory *factory)
{
    CCSMateConfIntegratedSettingFactoryPrivate *priv = (CCSMateConfIntegratedSettingFactoryPrivate *) ccsObjectGetPrivate (factory);

    if (priv->client)
	finiMateConfClient (priv->client, priv->mateMateConfNotifyIds);

    if (priv->pluginsToSettingsSectionsHashTable)
	g_hash_table_unref (priv->pluginsToSettingsSectionsHashTable);

    if (priv->pluginsToSettingsSpecialTypesHashTable)
	g_hash_table_unref (priv->pluginsToSettingsSpecialTypesHashTable);

    if (priv->pluginsToSettingNameMATENameHashTable)
	g_hash_table_unref (priv->pluginsToSettingNameMATENameHashTable);

    ccsObjectFinalize (factory);
    (*factory->object.object_allocation->free_) (factory->object.object_allocation->allocator, factory);
}


const CCSIntegratedSettingFactoryInterface ccsMateConfIntegratedSettingFactoryInterface =
{
    ccsMateConfIntegratedSettingFactoryCreateIntegratedSettingForCCSSettingNameAndType,
    ccsMateConfIntegratedSettingFactoryFree
};

CCSIntegratedSettingFactory *
ccsMateConfIntegratedSettingFactoryNew (MateConfClient		  *client,
				     CCSMATEValueChangeData	  *valueChangedData,
				     CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSettingFactory *factory = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSettingFactory));

    if (!factory)
	return NULL;

    CCSMateConfIntegratedSettingFactoryPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSMateConfIntegratedSettingFactoryPrivate));

    if (!priv)
    {
	(*ai->free_) (ai->allocator, factory);
	return NULL;
    }

    if (client)
    {
	int i;
	priv->client = (MateConfClient *) g_object_ref (client);
	for (i = 0; i < NUM_WATCHED_DIRS; i++)
	    mateconf_client_add_dir (priv->client, watchedMateConfMateDirectories[i],
				  MATECONF_CLIENT_PRELOAD_NONE, NULL);
    }
    else
	priv->client = NULL;

    priv->pluginsToSettingsSectionsHashTable = ccsMATEIntegrationPopulateCategoriesHashTables ();
    priv->pluginsToSettingsSpecialTypesHashTable = ccsMATEIntegrationPopulateSpecialTypesHashTables ();
    priv->pluginsToSettingNameMATENameHashTable = ccsMATEIntegrationPopulateSettingNameToMATENameHashTables ();
    priv->valueChangedData = valueChangedData;

    ccsObjectInit (factory, ai);
    ccsObjectSetPrivate (factory, (CCSPrivate *) priv);
    ccsObjectAddInterface (factory, (const CCSInterface *) &ccsMateConfIntegratedSettingFactoryInterface, GET_INTERFACE_TYPE (CCSIntegratedSettingFactoryInterface));

    ccsIntegratedSettingFactoryRef (factory);

    return factory;
}

