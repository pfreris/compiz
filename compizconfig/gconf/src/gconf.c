/**
 *
 * MateConf libccs backend
 *
 * mateconf.c
 *
 * Copyright (c) 2007 Danny Baumann <maniac@opencompositing.org>
 *
 * Parts of this code are taken from libberylsettings 
 * mateconf backend, written by:
 *
 * Copyright (c) 2006 Robert Carr <racarr@opencompositing.org>
 * Copyright (c) 2007 Dennis Kasprzyk <onestone@opencompositing.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 **/

#define CCS_LOG_DOMAIN "mateconf"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <string.h>
#include <dirent.h>

#include <ccs.h>
#include <ccs-backend.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <mateconf/mateconf.h>
#include <mateconf/mateconf-client.h>
#include <mateconf/mateconf-value.h>

#include "ccs_mate_integration.h"
#include "ccs_mate_integration_mateconf_integrated_setting_factory.h"

#define COMPIZ       "/apps/compiz-1"
#define COMPIZCONFIG "/apps/compizconfig-1"
#define PROFILEPATH  COMPIZCONFIG "/profiles"
#define DEFAULTPROF "Default"
#define CORE_NAME   "core"

#define BUFSIZE 512

#define KEYNAME(sn)     char keyName[BUFSIZE]; \
                    snprintf (keyName, BUFSIZE, "screen%i", sn);

#define PATHNAME    char pathName[BUFSIZE]; \
		    if (!ccsPluginGetName (ccsSettingGetParent (setting)) || \
			strcmp (ccsPluginGetName (ccsSettingGetParent (setting)), "core") == 0) \
                        snprintf (pathName, BUFSIZE, \
				 "%s/general/%s/options/%s", COMPIZ, \
				 keyName, ccsSettingGetName (setting)); \
                    else \
			snprintf(pathName, BUFSIZE, \
				 "%s/plugins/%s/%s/options/%s", COMPIZ, \
				 ccsPluginGetName (ccsSettingGetParent (setting)), keyName, ccsSettingGetName (setting));

static MateConfClient *client = NULL;
static MateConfEngine *conf = NULL;
static guint compizNotifyId;
static char *currentProfile = NULL;
static CCSMATEValueChangeData valueChangeData =
{
    NULL,
    NULL,
    NULL,
    NULL
};

/* some forward declarations */
static Bool readInit (CCSBackend *backend, CCSContext * context);
static void readSetting (CCSBackend *backend, CCSContext * context, CCSSetting * setting);
static Bool readOption (CCSSetting * setting);
static Bool writeInit (CCSBackend *backend, CCSContext * context);
static void writeIntegratedOption (CCSContext *context, CCSSetting *setting, CCSIntegratedSetting *integrated);

static Bool
isIntegratedOption (CCSSetting *setting,
		    CCSIntegratedSetting **integrated)
{
    CCSPlugin    *plugin = ccsSettingGetParent (setting);
    const char   *pluginName = ccsPluginGetName (plugin);
    const char   *settingName = ccsSettingGetName (setting);
    CCSIntegratedSetting *tmp = ccsIntegrationGetIntegratedSetting (valueChangeData.integration, pluginName, settingName);

    if (integrated)
	*integrated = tmp;

    return tmp != NULL;
}

static void
updateSetting (CCSBackend *backend, CCSContext *context, CCSPlugin *plugin, CCSSetting *setting)
{
    CCSIntegratedSetting *integrated;
    readInit (backend, context);
    if (!readOption (setting))
	ccsResetToDefault (setting, TRUE);

    if (ccsGetIntegrationEnabled (context) &&
	isIntegratedOption (setting, &integrated))
    {
	writeInit (backend, context);
	writeIntegratedOption (context, setting, integrated);
    }
}

static void
valueChanged (MateConfClient *client,
	      guint       cnxn_id,
	      MateConfEntry  *entry,
	      gpointer    user_data)
{
    CCSContext   *context = (CCSContext *)user_data;
    char         *keyName = (char*) mateconf_entry_get_key (entry);
    char         *pluginName;
    char         *token;
    unsigned int screenNum;
    CCSPlugin    *plugin;
    CCSSetting   *setting;

    keyName += strlen (COMPIZ) + 1;

    token = strsep (&keyName, "/"); /* plugin */
    if (!token)
	return;

    if (strcmp (token, "general") == 0)
    {
	pluginName = "core";
    }
    else
    {
	token = strsep (&keyName, "/");
	if (!token)
	    return;
	pluginName = token;
    }

    plugin = ccsFindPlugin (context, pluginName);
    if (!plugin)
	return;

    token = strsep (&keyName, "/");
    if (!token)
	return;

    sscanf (token, "screen%d", &screenNum);

    token = strsep (&keyName, "/"); /* 'options' */
    if (!token)
	return;

    token = strsep (&keyName, "/");
    if (!token)
	return;

    setting = ccsFindSetting (plugin, token);
    if (!setting)
	return;

    /* Passing null here is not optimal, but we are not
     * maintaining mateconf actively here */
    updateSetting (NULL, context, plugin, setting);
}

static void
initClient (CCSBackend *backend, CCSContext *context)
{
    client = mateconf_client_get_for_engine (conf);

    valueChangeData.context = context;
    valueChangeData.storage = ccsIntegratedSettingsStorageDefaultImplNew (&ccsDefaultObjectAllocator);
    valueChangeData.factory = ccsMateConfIntegratedSettingFactoryNew (client, &valueChangeData, &ccsDefaultObjectAllocator);

    valueChangeData.integration = ccsMATEIntegrationBackendNew (backend,
								 context,
								 valueChangeData.factory,
								 valueChangeData.storage,
								 &ccsDefaultObjectAllocator);

    compizNotifyId = mateconf_client_notify_add (client, COMPIZ, valueChanged,
					      context, NULL, NULL);
    mateconf_client_add_dir (client, COMPIZ, MATECONF_CLIENT_PRELOAD_NONE, NULL);
}

static void
finiClient (void)
{
    ccsIntegrationUnref (valueChangeData.integration);

    if (compizNotifyId)
    {
	mateconf_client_notify_remove (client, compizNotifyId);
	compizNotifyId = 0;
    }
    mateconf_client_remove_dir (client, COMPIZ, NULL);
    mateconf_client_suggest_sync (client, NULL);

    g_object_unref (client);
    client = NULL;

    memset (&valueChangeData, 0, sizeof (CCSMATEValueChangeData));
}

static void
copyGconfValues (MateConfEngine *conf,
		 const gchar *from,
		 const gchar *to,
		 Bool        associate,
		 const gchar *schemaPath)
{
    GSList *values, *tmp;
    GError *err = NULL;

    values = mateconf_engine_all_entries (conf, from, &err);
    tmp = values;

    while (tmp)
    {
	MateConfEntry *entry = tmp->data;
	const char *key = mateconf_entry_get_key (entry);
	char       *name, *newKey, *newSchema = NULL;

	name = strrchr (key, '/');
	if (!name)
	    continue;

	if (to)
	{
	    MateConfValue *value;

	    if (asprintf (&newKey, "%s/%s", to, name + 1) == -1)
		newKey = NULL;

	    if (associate && schemaPath)
		if (asprintf (&newSchema, "%s/%s", schemaPath, name + 1) == -1)
		    newSchema = NULL;

	    if (newKey && newSchema)
		mateconf_engine_associate_schema (conf, newKey, newSchema, NULL);

	    if (newKey)
	    {
		value = mateconf_engine_get_without_default (conf, key, NULL);
		if (value)
		{
		    mateconf_engine_set (conf, newKey, value, NULL);
		    mateconf_value_free (value);
		}
	    }

	    if (newSchema)
		free (newSchema);
	    if (newKey)
		free (newKey);
	}
	else
	{
	    if (associate)
		mateconf_engine_associate_schema (conf, key, NULL, NULL);
	    mateconf_engine_unset (conf, key, NULL);
	}

	mateconf_entry_unref (entry);
	tmp = g_slist_next (tmp);
    }

    if (values)
	g_slist_free (values);
}

static void
copyGconfRecursively (MateConfEngine *conf,
		      GSList      *subdirs,
		      const gchar *to,
		      Bool        associate,
		      const gchar *schemaPath)
{
    GSList* tmp;

    tmp = subdirs;

    while (tmp)
    {
 	gchar *path = tmp->data;
	char  *newKey, *newSchema = NULL, *name;

	name = strrchr (path, '/');
	if (name)
	{
	    if (!(to && asprintf (&newKey, "%s/%s", to, name + 1) != -1))
		newKey = NULL;

	    if (associate && schemaPath)
		if (asprintf (&newSchema, "%s/%s", schemaPath, name + 1) == -1)
		    newSchema = NULL;

	    copyGconfValues (conf, path, newKey, associate, newSchema);
	    copyGconfRecursively (conf,
				  mateconf_engine_all_dirs (conf, path, NULL),
				  newKey, associate, newSchema);

	    if (newSchema)
		free (newSchema);

	    if (newKey)
		free (newKey);

	    if (!to)
		mateconf_engine_remove_dir (conf, path, NULL);
	}

	g_free (path);
	tmp = g_slist_next (tmp);
    }

    if (subdirs)
	g_slist_free (subdirs);
}

static void
copyGconfTree (CCSBackend  *backend,
	       CCSContext  *context,
	       const gchar *from,
	       const gchar *to,
	       Bool        associate,
	       const gchar *schemaPath)
{
    GSList* subdirs;

    /* we aren't allowed to have an open MateConfClient object while
       using MateConfEngine, so shut it down and open it again afterwards */
    finiClient ();

    subdirs = mateconf_engine_all_dirs (conf, from, NULL);
    mateconf_engine_suggest_sync (conf, NULL);

    copyGconfRecursively (conf, subdirs, to, associate, schemaPath);

    mateconf_engine_suggest_sync (conf, NULL);

    initClient (backend, context);
}

static Bool
readListValue (CCSSetting *setting,
	       MateConfValue *mateconfValue)
{
    MateConfValueType      valueType;
    unsigned int        nItems, i = 0;
    CCSSettingValueList list = NULL;
    GSList              *valueList = NULL;

    switch (ccsSettingGetInfo (setting)->forList.listType)
    {
    case TypeString:
    case TypeMatch:
    case TypeColor:
	valueType = MATECONF_VALUE_STRING;
	break;
    case TypeBool:
	valueType = MATECONF_VALUE_BOOL;
	break;
    case TypeInt:
	valueType = MATECONF_VALUE_INT;
	break;
    case TypeFloat:
	valueType = MATECONF_VALUE_FLOAT;
	break;
    default:
	valueType = MATECONF_VALUE_INVALID;
	break;
    }

    if (valueType == MATECONF_VALUE_INVALID)
	return FALSE;

    if (valueType != mateconf_value_get_list_type (mateconfValue))
	return FALSE;

    valueList = mateconf_value_get_list (mateconfValue);
    if (!valueList)
    {
	ccsSetList (setting, NULL, TRUE);
	return TRUE;
    }

    nItems = g_slist_length (valueList);

    switch (ccsSettingGetInfo (setting)->forList.listType)
    {
    case TypeBool:
	{
	    Bool *array = malloc (nItems * sizeof (Bool));
	    if (!array)
		break;

	    for (; valueList; valueList = valueList->next, i++)
		array[i] =
		    mateconf_value_get_bool (valueList->data) ? TRUE : FALSE;
	    list = ccsGetValueListFromBoolArray (array, nItems, setting);
	    free (array);
	}
	break;
    case TypeInt:
	{
	    int *array = malloc (nItems * sizeof (int));
	    if (!array)
		break;

	    for (; valueList; valueList = valueList->next, i++)
		array[i] = mateconf_value_get_int (valueList->data);
	    list = ccsGetValueListFromIntArray (array, nItems, setting);
	    free (array);
	}
	break;
    case TypeFloat:
	{
	    float *array = malloc (nItems * sizeof (float));
	    if (!array)
		break;

	    for (; valueList; valueList = valueList->next, i++)
		array[i] = mateconf_value_get_float (valueList->data);
	    list = ccsGetValueListFromFloatArray (array, nItems, setting);
	    free (array);
	}
	break;
    case TypeString:
    case TypeMatch:
	{
	    gchar **array = malloc ((nItems + 1) * sizeof (char*));
	    if (!array)
		break;

	    for (; valueList; valueList = valueList->next, i++)
		array[i] = strdup (mateconf_value_get_string (valueList->data));

	    array[nItems] = NULL;

	    list = ccsGetValueListFromStringArray ((const char **) array, nItems, setting);
	    g_strfreev (array);
	}
	break;
    case TypeColor:
	{
	    CCSSettingColorValue *array;
	    array = malloc (nItems * sizeof (CCSSettingColorValue));
	    if (!array)
		break;

	    for (; valueList; valueList = valueList->next, i++)
    	    {
		memset (&array[i], 0, sizeof (CCSSettingColorValue));
		ccsStringToColor (mateconf_value_get_string (valueList->data),
				  &array[i]);
	    }
	    list = ccsGetValueListFromColorArray (array, nItems, setting);
	    free (array);
	}
	break;
    default:
	break;
    }

    if (list)
    {
	ccsSetList (setting, list, TRUE);
	ccsSettingValueListFree (list, TRUE);
	return TRUE;
    }

    return FALSE;
}


static Bool
readIntegratedOption (CCSContext *context,
		      CCSSetting *setting,
		      CCSIntegratedSetting *integrated)
{
    return ccsIntegrationReadOptionIntoSetting (valueChangeData.integration, context, setting, integrated);
}

static Bool
readOption (CCSSetting * setting)
{
    MateConfValue *mateconfValue = NULL;
    GError     *err = NULL;
    Bool       ret = FALSE;
    Bool       valid = TRUE;

    KEYNAME (ccsContextGetScreenNum (ccsPluginGetContext (ccsSettingGetParent (setting))));
    PATHNAME;

    /* first check if the key is set */
    mateconfValue = mateconf_client_get_without_default (client, pathName, &err);
    if (err)
    {
	g_error_free (err);
	return FALSE;
    }
    if (!mateconfValue)
	/* value is not set */
	return FALSE;

    /* setting type sanity check */
    switch (ccsSettingGetType (setting))
    {
    case TypeString:
    case TypeMatch:
    case TypeColor:
    case TypeKey:
    case TypeButton:
    case TypeEdge:
	valid = (mateconfValue->type == MATECONF_VALUE_STRING);
	break;
    case TypeInt:
	valid = (mateconfValue->type == MATECONF_VALUE_INT);
	break;
    case TypeBool:
    case TypeBell:
	valid = (mateconfValue->type == MATECONF_VALUE_BOOL);
	break;
    case TypeFloat:
	valid = (mateconfValue->type == MATECONF_VALUE_FLOAT);
	break;
    case TypeList:
	valid = (mateconfValue->type == MATECONF_VALUE_LIST);
	break;
    default:
	break;
    }
    if (!valid)
    {
	ccsWarning ("There is an unsupported value at path %s. "
		"Settings from this path won't be read. Try to remove "
		"that value so that operation can continue properly.",
		pathName);
	return FALSE;
    }

    switch (ccsSettingGetType (setting))
    {
    case TypeString:
	{
	    const char *value;
	    value = mateconf_value_get_string (mateconfValue);
	    if (value)
	    {
		ccsSetString (setting, value, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeMatch:
	{
	    const char * value;
	    value = mateconf_value_get_string (mateconfValue);
	    if (value)
	    {
		ccsSetMatch (setting, value, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeInt:
	{
	    int value;
	    value = mateconf_value_get_int (mateconfValue);

	    ccsSetInt (setting, value, TRUE);
	    ret = TRUE;
	}
	break;
    case TypeBool:
	{
	    gboolean value;
	    value = mateconf_value_get_bool (mateconfValue);

	    ccsSetBool (setting, value ? TRUE : FALSE, TRUE);
	    ret = TRUE;
	}
	break;
    case TypeFloat:
	{
	    double value;
	    value = mateconf_value_get_float (mateconfValue);

	    ccsSetFloat (setting, (float)value, TRUE);
    	    ret = TRUE;
	}
	break;
    case TypeColor:
	{
	    const char           *value;
	    CCSSettingColorValue color;
	    value = mateconf_value_get_string (mateconfValue);

	    if (value && ccsStringToColor (value, &color))
	    {
		ccsSetColor (setting, color, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeKey:
	{
	    const char         *value;
	    CCSSettingKeyValue key;
	    value = mateconf_value_get_string (mateconfValue);

	    if (value && ccsStringToKeyBinding (value, &key))
	    {
		ccsSetKey (setting, key, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeButton:
	{
	    const char            *value;
	    CCSSettingButtonValue button;
	    value = mateconf_value_get_string (mateconfValue);

	    if (value && ccsStringToButtonBinding (value, &button))
	    {
		ccsSetButton (setting, button, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeEdge:
	{
	    const char   *value;
	    value = mateconf_value_get_string (mateconfValue);

	    if (value)
	    {
		unsigned int edges;
		edges = ccsStringToEdges (value);
		ccsSetEdge (setting, edges, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeBell:
	{
	    gboolean value;
	    value = mateconf_value_get_bool (mateconfValue);

	    ccsSetBell (setting, value ? TRUE : FALSE, TRUE);
	    ret = TRUE;
	}
	break;
    case TypeList:
	ret = readListValue (setting, mateconfValue);
	break;
    default:
	ccsWarning ("Attempt to read unsupported setting type %d from path %s!",
	       ccsSettingGetType (setting), pathName);
	break;
    }

    if (mateconfValue)
	mateconf_value_free (mateconfValue);

    return ret;
}

static void
writeListValue (CCSSetting *setting,
		char       *pathName)
{
    GSList              *valueList = NULL;
    MateConfValueType      valueType;
    Bool                freeItems = FALSE;
    CCSSettingValueList list;
    gpointer            data;

    if (!ccsGetList (setting, &list))
	return;

    switch (ccsSettingGetInfo (setting)->forList.listType)
    {
    case TypeBool:
	{
	    while (list)
	    {
		data = GINT_TO_POINTER (list->data->value.asBool);
		valueList = g_slist_append (valueList, data);
		list = list->next;
	    }
	    valueType = MATECONF_VALUE_BOOL;
	}
	break;
    case TypeInt:
	{
	    while (list)
	    {
		data = GINT_TO_POINTER (list->data->value.asInt);
		valueList = g_slist_append(valueList, data);
		list = list->next;
    	    }
	    valueType = MATECONF_VALUE_INT;
	}
	break;
    case TypeFloat:
	{
	    gdouble *item;
	    while (list)
	    {
		item = malloc (sizeof (gdouble));
		if (item)
		{
		    *item = list->data->value.asFloat;
		    valueList = g_slist_append (valueList, item);
		}
		list = list->next;
	    }
	    freeItems = TRUE;
	    valueType = MATECONF_VALUE_FLOAT;
	}
	break;
    case TypeString:
	{
	    while (list)
	    {
		valueList = g_slist_append(valueList,
		   			   list->data->value.asString);
		list = list->next;
	    }
	    valueType = MATECONF_VALUE_STRING;
	}
	break;
    case TypeMatch:
	{
	    while (list)
	    {
		valueList = g_slist_append(valueList,
		   			   list->data->value.asMatch);
		list = list->next;
	    }
	    valueType = MATECONF_VALUE_STRING;
	}
	break;
    case TypeColor:
	{
	    char *item;
	    while (list)
	    {
		item = ccsColorToString (&list->data->value.asColor);
		valueList = g_slist_append (valueList, item);
		list = list->next;
	    }
	    freeItems = TRUE;
	    valueType = MATECONF_VALUE_STRING;
	}
	break;
    default:
	ccsWarning ("Attempt to write unsupported list type %d at path %s!",
	       ccsSettingGetInfo (setting)->forList.listType, pathName);
	valueType = MATECONF_VALUE_INVALID;
	break;
    }

    if (valueType != MATECONF_VALUE_INVALID)
    {
	mateconf_client_set_list (client, pathName, valueType, valueList, NULL);

	if (freeItems)
	{
	    GSList *tmpList = valueList;
	    for (; tmpList; tmpList = tmpList->next)
		if (tmpList->data)
		    free (tmpList->data);
	}
    }
    if (valueList)
	g_slist_free (valueList);
}

static void
writeIntegratedOption (CCSContext *context,
		       CCSSetting *setting,
		       CCSIntegratedSetting *integrated)
{
    ccsIntegrationWriteSettingIntoOption (valueChangeData.integration, context, setting, integrated);
}

static void
resetOptionToDefault (CCSSetting * setting)
{
    KEYNAME (ccsContextGetScreenNum (ccsPluginGetContext (ccsSettingGetParent (setting))));
    PATHNAME;

    mateconf_client_recursive_unset (client, pathName, 0, NULL);
    mateconf_client_suggest_sync (client, NULL);
}

static void
writeOption (CCSSetting * setting)
{
    KEYNAME (ccsContextGetScreenNum (ccsPluginGetContext (ccsSettingGetParent (setting))));
    PATHNAME;

    switch (ccsSettingGetType (setting))
    {
    case TypeString:
	{
	    const char *value;
	    if (ccsGetString (setting, &value))
		mateconf_client_set_string (client, pathName, value, NULL);
	}
	break;
    case TypeMatch:
	{
	    const char *value;
	    if (ccsGetMatch (setting, &value))
		mateconf_client_set_string (client, pathName, value, NULL);
	}
    case TypeFloat:
	{
	    float value;
	    if (ccsGetFloat (setting, &value))
		mateconf_client_set_float (client, pathName, value, NULL);
	}
	break;
    case TypeInt:
	{
	    int value;
	    if (ccsGetInt (setting, &value))
		mateconf_client_set_int (client, pathName, value, NULL);
	}
	break;
    case TypeBool:
	{
	    Bool value;
	    if (ccsGetBool (setting, &value))
		mateconf_client_set_bool (client, pathName, value, NULL);
	}
	break;
    case TypeColor:
	{
	    CCSSettingColorValue value;
	    char                 *colString;

	    if (!ccsGetColor (setting, &value))
		break;

	    colString = ccsColorToString (&value);
	    if (!colString)
		break;

	    mateconf_client_set_string (client, pathName, colString, NULL);
	    free (colString);
	}
	break;
    case TypeKey:
	{
	    CCSSettingKeyValue key;
	    char               *keyString;

	    if (!ccsGetKey (setting, &key))
		break;

	    keyString = ccsKeyBindingToString (&key);
	    if (!keyString)
		break;

	    mateconf_client_set_string (client, pathName, keyString, NULL);
	    free (keyString);
	}
	break;
    case TypeButton:
	{
	    CCSSettingButtonValue button;
	    char                  *buttonString;

	    if (!ccsGetButton (setting, &button))
		break;

	    buttonString = ccsButtonBindingToString (&button);
	    if (!buttonString)
		break;

	    mateconf_client_set_string (client, pathName, buttonString, NULL);
	    free (buttonString);
	}
	break;
    case TypeEdge:
	{
	    unsigned int edges;
	    char         *edgeString;

	    if (!ccsGetEdge (setting, &edges))
		break;

	    edgeString = ccsEdgesToString (edges);
	    if (!edgeString)
		break;

	    mateconf_client_set_string (client, pathName, edgeString, NULL);
	    free (edgeString);
	}
	break;
    case TypeBell:
	{
	    Bool value;
	    if (ccsGetBell (setting, &value))
		mateconf_client_set_bool (client, pathName, value, NULL);
	}
	break;
    case TypeList:
	writeListValue (setting, pathName);
	break;
    default:
	ccsWarning ("Attempt to write unsupported setting type %d",
	       ccsSettingGetType (setting));
	break;
    }
}

static void
updateCurrentProfileName (char *profile)
{
    MateConfSchema *schema;
    MateConfValue  *value;
    
    schema = mateconf_schema_new ();
    if (!schema)
	return;

    value = mateconf_value_new (MATECONF_VALUE_STRING);
    if (!value)
    {
	mateconf_schema_free (schema);
	return;
    }

    mateconf_schema_set_type (schema, MATECONF_VALUE_STRING);
    mateconf_schema_set_locale (schema, "C");
    mateconf_schema_set_short_desc (schema, "Current profile");
    mateconf_schema_set_long_desc (schema, "Current profile of mateconf backend");
    mateconf_schema_set_owner (schema, "compizconfig-1");
    mateconf_value_set_string (value, profile);
    mateconf_schema_set_default_value (schema, value);

    mateconf_client_set_schema (client, COMPIZCONFIG "/current_profile",
			     schema, NULL);

    mateconf_schema_free (schema);
    mateconf_value_free (value);
}

static char*
getCurrentProfileName (void)
{
    MateConfSchema *schema = NULL;

    schema = mateconf_client_get_schema (client,
    				      COMPIZCONFIG "/current_profile", NULL);

    if (schema)
    {
	MateConfValue *value;
	char       *ret = NULL;

	value = mateconf_schema_get_default_value (schema);
	if (value)
	    ret = strdup (mateconf_value_get_string (value));
	mateconf_schema_free (schema);

	return ret;
    }

    return NULL;
}

static Bool
checkProfile (CCSBackend *backend,
	      CCSContext *context)
{
    const char *profileCCS;
    char *lastProfile;

    lastProfile = currentProfile;

    profileCCS = ccsGetProfile (context);
    if (!profileCCS || !strlen (profileCCS))
	currentProfile = strdup (DEFAULTPROF);
    else
	currentProfile = strdup (profileCCS);

    if (!lastProfile || strcmp (lastProfile, currentProfile) != 0)
    {
	char *pathName;

	if (lastProfile)
	{
	    /* copy /apps/compiz-1 tree to profile path */
	    if (asprintf (&pathName, "%s/%s", PROFILEPATH, lastProfile) == -1)
		pathName = NULL;

	    if (pathName)
	    {
		copyGconfTree (backend,
			       context, COMPIZ, pathName,
			       TRUE, "/schemas" COMPIZ);
		free (pathName);
	    }
	}

	/* reset /apps/compiz-1 tree */
	mateconf_client_recursive_unset (client, COMPIZ, 0, NULL);

	/* copy new profile tree to /apps/compiz-1 */
	if (asprintf (&pathName, "%s/%s", PROFILEPATH, currentProfile) == -1)
	    pathName = NULL;

	if (pathName)
	{
	    copyGconfTree (backend, context, pathName, COMPIZ, FALSE, NULL);

    	    /* delete the new profile tree in /apps/compizconfig-1
    	       to avoid user modification in the wrong tree */
	    copyGconfTree (backend, context, pathName, NULL, TRUE, NULL);
    	    free (pathName);
	}

	/* update current profile name */
	updateCurrentProfileName (currentProfile);
    }

    if (lastProfile)
	free (lastProfile);

    return TRUE;
}

static void
processEvents (CCSBackend *backend, unsigned int flags)
{
    if (!(flags & ProcessEventsNoGlibMainLoopMask))
    {
	while (g_main_context_pending(NULL))
	    g_main_context_iteration(NULL, FALSE);
    }
}

static Bool
initBackend (CCSBackend *backend, CCSContext * context)
{
    g_type_init ();

    conf = mateconf_engine_get_default ();
    initClient (backend, context);

    currentProfile = getCurrentProfileName ();

    return TRUE;
}

static Bool
finiBackend (CCSBackend *backend)
{
    mateconf_client_clear_cache (client);
    finiClient ();

    if (currentProfile)
    {
	free (currentProfile);
	currentProfile = NULL;
    }

    mateconf_engine_unref (conf);
    conf = NULL;

    return TRUE;
}

static Bool
readInit (CCSBackend *backend, CCSContext * context)
{
    return checkProfile (backend, context);
}

static void
readSetting (CCSBackend *backend,
	     CCSContext *context,
	     CCSSetting *setting)
{
    Bool status;
    CCSIntegratedSetting *integrated;

    if (ccsGetIntegrationEnabled (context) &&
	isIntegratedOption (setting, &integrated))
    {
	status = readIntegratedOption (context, setting, integrated);
    }
    else
	status = readOption (setting);

    if (!status)
	ccsResetToDefault (setting, TRUE);
}

static Bool
writeInit (CCSBackend *backend, CCSContext * context)
{
    return checkProfile (backend, context);
}

static void
writeSetting (CCSBackend *backend,
	      CCSContext *context,
	      CCSSetting *setting)
{
    CCSIntegratedSetting *integrated;

    if (ccsGetIntegrationEnabled (context) &&
	isIntegratedOption (setting, &integrated))
    {
	writeIntegratedOption (context, setting, integrated);
    }
    else if (ccsSettingGetIsDefault (setting))
    {
	resetOptionToDefault (setting);
    }
    else
	writeOption (setting);

}

static Bool
getSettingIsIntegrated (CCSBackend *backend, CCSSetting * setting)
{
    if (!ccsGetIntegrationEnabled (ccsPluginGetContext (ccsSettingGetParent (setting))))
	return FALSE;

    if (!isIntegratedOption (setting, NULL))
	return FALSE;

    return TRUE;
}

static Bool
getSettingIsReadOnly (CCSBackend *backend, CCSSetting * setting)
{
    /* FIXME */
    return FALSE;
}

static CCSStringList
getExistingProfiles (CCSBackend *backend, CCSContext *context)
{
    GSList        *data, *tmp;
    CCSStringList ret = NULL;
    char          *name;

    mateconf_client_suggest_sync (client, NULL);
    data = mateconf_client_all_dirs (client, PROFILEPATH, NULL);

    for (tmp = data; tmp; tmp = g_slist_next (tmp))
    {
	name = strrchr (tmp->data, '/');
	if (name && (strcmp (name + 1, DEFAULTPROF) != 0))
	{
	    CCSString *str = calloc (1, sizeof (CCSString));
	    str->value = strdup (name + 1);
	    ret = ccsStringListAppend (ret, str);
	}

	g_free (tmp->data);
    }

    g_slist_free (data);

    name = getCurrentProfileName ();
    if (name && strcmp (name, DEFAULTPROF) != 0)
    {
	CCSString *str = calloc (1, sizeof (CCSString));
	str->value = name;
	ret = ccsStringListAppend (ret, str);
    }
    else if (name)
	free (name);

    return ret;
}

static Bool
deleteProfile (CCSBackend *backend,
	       CCSContext *context,
	       char       *profile)
{
    char     path[BUFSIZE];
    gboolean status = FALSE;

    checkProfile (backend, context);

    snprintf (path, BUFSIZE, "%s/%s", PROFILEPATH, profile);

    if (mateconf_client_dir_exists (client, path, NULL))
    {
	status =
	    mateconf_client_recursive_unset (client, path,
	   				  MATECONF_UNSET_INCLUDING_SCHEMA_NAMES,
					  NULL);
	mateconf_client_suggest_sync (client, NULL);
    }

    return status;
}

const CCSBackendInfo mateconfBackendInfo =
{
    "mateconf",
    "MateConf Configuration Backend",
    "MateConf Configuration Backend for libccs",
    TRUE,
    TRUE,
    1
};

static const CCSBackendInfo *
getInfo (CCSBackend *backend)
{
    return &mateconfBackendInfo;
}

static CCSBackendInterface mateconfVTable = {
    getInfo,
    processEvents,
    initBackend,
    finiBackend,
    readInit,
    readSetting,
    0,
    writeInit,
    writeSetting,
    0,
    updateSetting,
    getSettingIsIntegrated,
    getSettingIsReadOnly,
    getExistingProfiles,
    deleteProfile
};

CCSBackendInterface *
getBackendInfo (void)
{
    return &mateconfVTable;
}

