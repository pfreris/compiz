#include <stdlib.h>
#include <string.h>

#include <mateconf/mateconf.h>
#include <mateconf/mateconf-client.h>
#include <mateconf/mateconf-value.h>

#include <ccs.h>
#include <ccs-backend.h>
#include <ccs-object.h>

#include "ccs_mate_integration_mateconf_integrated_setting.h"
#include "ccs_mate_integrated_setting.h"
#include "ccs_mate_integration_constants.h"


/* CCSMateConfIntegratedSetting implementation */
typedef struct _CCSMateConfIntegratedSettingPrivate CCSMateConfIntegratedSettingPrivate;

struct _CCSMateConfIntegratedSettingPrivate
{
    CCSMATEIntegratedSettingInfo *mateIntegratedSettingInfo;
    MateConfClient		      *client;
    const char		      *sectionName;
};

SpecialOptionType
ccsMateConfIntegratedSettingGetSpecialOptionType (CCSMATEIntegratedSettingInfo *setting)
{
    CCSMateConfIntegratedSettingPrivate *priv = (CCSMateConfIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    return ccsMATEIntegratedSettingInfoGetSpecialOptionType (priv->mateIntegratedSettingInfo);
}

const char *
ccsMateConfIntegratedSettingGetMATEName (CCSMATEIntegratedSettingInfo *setting)
{
    CCSMateConfIntegratedSettingPrivate *priv = (CCSMateConfIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    return ccsMATEIntegratedSettingInfoGetMATEName (priv->mateIntegratedSettingInfo);
}

CCSSettingValue *
ccsMateConfIntegratedSettingReadValue (CCSIntegratedSetting *setting, CCSSettingType type)
{
    CCSMateConfIntegratedSettingPrivate *priv = (CCSMateConfIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);
    CCSSettingValue		     *v = calloc (1, sizeof (CCSSettingValue));
    const char			     *mateKeyName = ccsMATEIntegratedSettingInfoGetMATEName ((CCSMATEIntegratedSettingInfo *) setting);
    char			     *mateKeyPath = g_strconcat (priv->sectionName, mateKeyName, NULL);

    v->isListChild = FALSE;
    v->parent = NULL;
    v->refCount = 1;

    MateConfValue *mateconfValue;
    GError     *err = NULL;

    mateconfValue = mateconf_client_get (priv->client,
				   mateKeyPath,
				   &err);

    if (!mateconfValue)
    {
	ccsError ("NULL encountered while reading MateConf setting");
	free (mateKeyPath);
	free (v);
	return NULL;
    }

    if (err)
    {
	ccsError ("%s", err->message);
	g_error_free (err);
	free (mateKeyPath);
	free (v);
	return NULL;
    }

    switch (type)
    {
	case TypeInt:
	    if (mateconfValue->type != MATECONF_VALUE_INT)
	    {
		ccsError ("Expected integer value");
		free (v);
		v = NULL;
		break;
	    }

	    v->value.asInt = mateconf_value_get_int (mateconfValue);
	    break;
	case TypeBool:
	    if (mateconfValue->type != MATECONF_VALUE_BOOL)
	    {
		ccsError ("Expected boolean value");
		free (v);
		v = NULL;
		break;
	    }

	    v->value.asBool = mateconf_value_get_bool (mateconfValue) ? TRUE : FALSE;
	    break;
	case TypeString:
	case TypeKey:
	    if (mateconfValue->type != MATECONF_VALUE_STRING)
	    {
		ccsError ("Expected string value");
		free (v);
		v = NULL;
		break;
	    }

	    const char *str = mateconf_value_get_string (mateconfValue);

	    v->value.asString = strdup (str ? str : "");
	    break;
	default:
	    g_assert_not_reached ();
    }

    mateconf_value_free (mateconfValue);
    free (mateKeyPath);

    return v;
}

void
ccsMateConfIntegratedSettingWriteValue (CCSIntegratedSetting *setting, CCSSettingValue *v, CCSSettingType type)
{
    CCSMateConfIntegratedSettingPrivate *priv = (CCSMateConfIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);
    const char			     *mateKeyName = ccsMATEIntegratedSettingInfoGetMATEName ((CCSMATEIntegratedSettingInfo *) setting);
    char			     *mateKeyPath = g_strconcat (priv->sectionName, mateKeyName, NULL);
    GError			     *err = NULL;

    switch (type)
    {
	case TypeInt:
	    {
		int currentValue = mateconf_client_get_int (priv->client, mateKeyPath, &err);

		if (!err && (currentValue != v->value.asInt))
		    mateconf_client_set_int(priv->client, mateKeyPath,
					 v->value.asInt, NULL);
	    }
	    break;
	case TypeBool:
	    {
		Bool     newValue = v->value.asBool;
		gboolean currentValue;

		currentValue = mateconf_client_get_bool (priv->client, mateKeyPath, &err);

		if (!err && ((currentValue && !newValue) ||
			     (!currentValue && newValue)))
		    mateconf_client_set_bool (priv->client, mateKeyPath,
					   newValue, NULL);
	    }
	    break;
	case TypeString:
	case TypeKey:
	    {
		char  *newValue = v->value.asString;
		gchar *currentValue;

		currentValue = mateconf_client_get_string (priv->client, mateKeyPath, &err);

		if (!err && currentValue)
		{
		    if (strcmp (currentValue, newValue) != 0)
			mateconf_client_set_string (priv->client, mateKeyPath,
						 newValue, NULL);
		    g_free (currentValue);
		}
	    }
	    break;
	default:
	    g_assert_not_reached ();
	    break;
    }

    if (err)
    {
	ccsError ("%s", err->message);
	g_error_free (err);
    }

    free (mateKeyPath);
}

const char *
ccsMateConfIntegratedSettingInfoPluginName (CCSIntegratedSettingInfo *info)
{
    CCSMateConfIntegratedSettingPrivate *priv = (CCSMateConfIntegratedSettingPrivate *) ccsObjectGetPrivate (info);

    return ccsIntegratedSettingInfoPluginName ((CCSIntegratedSettingInfo *) priv->mateIntegratedSettingInfo);
}

const char *
ccsMateConfIntegratedSettingInfoSettingName (CCSIntegratedSettingInfo *info)
{
    CCSMateConfIntegratedSettingPrivate *priv = (CCSMateConfIntegratedSettingPrivate *) ccsObjectGetPrivate (info);

    return ccsIntegratedSettingInfoSettingName ((CCSIntegratedSettingInfo *) priv->mateIntegratedSettingInfo);
}

CCSSettingType
ccsMateConfIntegratedSettingInfoGetType (CCSIntegratedSettingInfo *info)
{
    CCSMateConfIntegratedSettingPrivate *priv = (CCSMateConfIntegratedSettingPrivate *) ccsObjectGetPrivate (info);

    return ccsIntegratedSettingInfoGetType ((CCSIntegratedSettingInfo *) priv->mateIntegratedSettingInfo);
}

void
ccsMateConfIntegratedSettingFree (CCSIntegratedSetting *setting)
{
    CCSMateConfIntegratedSettingPrivate *priv = (CCSMateConfIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    ccsIntegratedSettingInfoUnref ((CCSIntegratedSettingInfo *) priv->mateIntegratedSettingInfo);
    ccsObjectFinalize (setting);

    (*setting->object.object_allocation->free_) (setting->object.object_allocation->allocator, setting);
}

void
ccsMateConfIntegratedSettingInfoFree (CCSIntegratedSettingInfo *info)
{
    ccsMateConfIntegratedSettingFree ((CCSIntegratedSetting *) info);
}

void
ccsMateConfMATEIntegratedSettingInfoFree (CCSMATEIntegratedSettingInfo *info)
{
    ccsMateConfIntegratedSettingFree ((CCSIntegratedSetting *) info);
}

const CCSMATEIntegratedSettingInfoInterface ccsMateConfMATEIntegratedSettingInfoInterface =
{
    ccsMateConfIntegratedSettingGetSpecialOptionType,
    ccsMateConfIntegratedSettingGetMATEName,
    ccsMateConfMATEIntegratedSettingInfoFree
};

const CCSIntegratedSettingInterface ccsMateConfIntegratedSettingInterface =
{
    ccsMateConfIntegratedSettingReadValue,
    ccsMateConfIntegratedSettingWriteValue,
    ccsMateConfIntegratedSettingFree
};

const CCSIntegratedSettingInfoInterface ccsMateConfIntegratedSettingInfoInterface =
{
    ccsMateConfIntegratedSettingInfoPluginName,
    ccsMateConfIntegratedSettingInfoSettingName,
    ccsMateConfIntegratedSettingInfoGetType,
    ccsMateConfIntegratedSettingInfoFree
};

CCSIntegratedSetting *
ccsMateConfIntegratedSettingNew (CCSMATEIntegratedSettingInfo *base,
			      MateConfClient		*client,
			      const char		*section,
			      CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSetting *setting = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSetting));

    if (!setting)
	return NULL;

    CCSMateConfIntegratedSettingPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSMateConfIntegratedSettingPrivate));

    if (!priv)
    {
	(*ai->free_) (ai->allocator, priv);
	return NULL;
    }

    priv->mateIntegratedSettingInfo = base;
    priv->client = client;
    priv->sectionName = section;

    ccsObjectInit (setting, ai);
    ccsObjectSetPrivate (setting, (CCSPrivate *) priv);
    ccsObjectAddInterface (setting, (const CCSInterface *) &ccsMateConfIntegratedSettingInfoInterface, GET_INTERFACE_TYPE (CCSIntegratedSettingInfoInterface));
    ccsObjectAddInterface (setting, (const CCSInterface *) &ccsMateConfIntegratedSettingInterface, GET_INTERFACE_TYPE (CCSIntegratedSettingInterface));
    ccsObjectAddInterface (setting, (const CCSInterface *) &ccsMateConfMATEIntegratedSettingInfoInterface, GET_INTERFACE_TYPE (CCSMATEIntegratedSettingInfoInterface));
    ccsIntegratedSettingRef (setting);

    return setting;
}
