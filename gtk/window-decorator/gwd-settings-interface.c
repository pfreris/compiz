/*
 * Copyright © 2012 Canonical Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authored By: Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#include "gwd-settings-interface.h"

const gboolean USE_TOOLTIPS_DEFAULT = FALSE;

const gdouble ACTIVE_SHADOW_RADIUS_DEFAULT = 8.0;
const gdouble ACTIVE_SHADOW_OPACITY_DEFAULT = 0.5;
const gint    ACTIVE_SHADOW_OFFSET_X_DEFAULT = 1;
const gint    ACTIVE_SHADOW_OFFSET_Y_DEFAULT = 1;
const gchar   *ACTIVE_SHADOW_COLOR_DEFAULT = "#00000000";

const gdouble INACTIVE_SHADOW_RADIUS_DEFAULT = 8.0;
const gdouble INACTIVE_SHADOW_OPACITY_DEFAULT = 0/5;
const gint    INACTIVE_SHADOW_OFFSET_X_DEFAULT = 1;
const gint    INACTIVE_SHADOW_OFFSET_Y_DEFAULT = 1;
const gchar   *INACTIVE_SHADOW_COLOR_DEFAULT = "#00000000";

const guint   DRAGGABLE_BORDER_WIDTH_DEFAULT = 7;
const gboolean ATTACH_MODAL_DIALOGS_DEFAULT = FALSE;
const gint    BLUR_TYPE_DEFAULT = BLUR_TYPE_NONE;

const gchar   *MARCO_THEME_DEFAULT = "Adwaita";
const gdouble MARCO_ACTIVE_OPACITY_DEFAULT = 1.0;
const gdouble MARCO_INACTIVE_OPACITY_DEFAULT = 0.75;
const gboolean MARCO_ACTIVE_SHADE_OPACITY_DEFAULT = TRUE;
const gboolean MARCO_INACTIVE_SHADE_OPACITY_DEFAULT = TRUE;

const gchar *  MARCO_BUTTON_LAYOUT_DEFAULT = ":minimize,maximize,close";

const guint DOUBLE_CLICK_ACTION_DEFAULT = CLICK_ACTION_MAXIMIZE;
const guint MIDDLE_CLICK_ACTION_DEFAULT = CLICK_ACTION_LOWER;
const guint RIGHT_CLICK_ACTION_DEFAULT = CLICK_ACTION_MENU;
const guint WHEEL_ACTION_DEFAULT = WHEEL_ACTION_NONE;

const gchar * TITLEBAR_FONT_DEFAULT = "Sans 12";

static void gwd_settings_interface_default_init (GWDSettingsInterface *settings_interface);

G_DEFINE_INTERFACE (GWDSettings, gwd_settings_interface, G_TYPE_OBJECT);

static void gwd_settings_interface_default_init (GWDSettingsInterface *settings_interface)
{
    g_object_interface_install_property (settings_interface,
					 g_param_spec_pointer ("active-shadow",
							       "Active Shadow",
							       "Active Shadow Settings",
							       G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_pointer ("inactive-shadow",
							       "Inactive Shadow",
							       "Inactive Shadow",
							       G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_boolean ("use-tooltips",
							       "Use Tooltips",
							       "Use Tooltips Setting",
							       USE_TOOLTIPS_DEFAULT,
							       G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_int ("draggable-border-width",
							   "Draggable Border Width",
							   "Draggable Border Width Setting",
							   0,
							   64,
							   DRAGGABLE_BORDER_WIDTH_DEFAULT,
							   G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_boolean ("attach-modal-dialogs",
							       "Attach modal dialogs",
							       "Attach modal dialogs setting",
							       ATTACH_MODAL_DIALOGS_DEFAULT,
							       G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_int ("blur",
							   "Blur Type",
							   "Blur type property",
							   BLUR_TYPE_NONE,
							   BLUR_TYPE_ALL,
							   BLUR_TYPE_NONE,
							   G_PARAM_READABLE |
							   G_PARAM_WRITABLE |
							   G_PARAM_CONSTRUCT_ONLY));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_string ("marco-theme",
							      "Marco Theme",
							      "Marco Theme Setting",
							      MARCO_THEME_DEFAULT,
							      G_PARAM_READABLE |
							      G_PARAM_WRITABLE |
							      G_PARAM_CONSTRUCT_ONLY));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_double ("marco-active-opacity",
							      "Marco Active Opacity",
							      "Marco Active Opacity",
							      0.0,
							      1.0,
							      MARCO_ACTIVE_OPACITY_DEFAULT,
							      G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_double ("marco-inactive-opacity",
							      "Marco Inactive Opacity",
							      "Marco Inactive Opacity",
							      0.0,
							      1.0,
							      MARCO_INACTIVE_OPACITY_DEFAULT,
							      G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_boolean ("marco-active-shade-opacity",
							      "Marco Active Shade Opacity",
							      "Marco Active Shade Opacity",
							      MARCO_ACTIVE_SHADE_OPACITY_DEFAULT,
							      G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_boolean ("marco-inactive-shade-opacity",
							      "Marco Inactive Shade Opacity",
							      "Marco Inactive Shade Opacity",
							      MARCO_INACTIVE_SHADE_OPACITY_DEFAULT,
							      G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_string ("marco-button-layout",
							      "Marco Button Layout",
							      "Marco Button Layout",
							      MARCO_BUTTON_LAYOUT_DEFAULT,
							      G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_int ("titlebar-double-click-action",
							   "Titlebar Action Double Click",
							   "Titlebar Action Double Click",
							   CLICK_ACTION_NONE,
							   CLICK_ACTION_MENU,
							   CLICK_ACTION_MAXIMIZE,
							   G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_int ("titlebar-middle-click-action",
							   "Titlebar Action Middle Click",
							   "Titlebar Action Middle Click",
							   CLICK_ACTION_NONE,
							   CLICK_ACTION_MENU,
							   CLICK_ACTION_LOWER,
							   G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_int ("titlebar-right-click-action",
							   "Titlebar Action Right Click",
							   "Titlebar Action Right Click",
							   CLICK_ACTION_NONE,
							   CLICK_ACTION_MENU,
							   CLICK_ACTION_MENU,
							   G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_int ("mouse-wheel-action",
							   "Mouse Wheel Action",
							   "Mouse Wheel Action",
							   WHEEL_ACTION_NONE,
							   WHEEL_ACTION_SHADE,
							   WHEEL_ACTION_SHADE,
							   G_PARAM_READABLE));
    g_object_interface_install_property (settings_interface,
					 g_param_spec_string ("titlebar-font",
							      "Titlebar Font",
							      "Titlebar Font",
							      TITLEBAR_FONT_DEFAULT,
							      G_PARAM_READABLE));
}
