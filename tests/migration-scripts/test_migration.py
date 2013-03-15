#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (C) 2012 Canonical
#
# Authors:
#  Francis 'fginther' Ginther <francis.ginther@canonical.com>
#  Łukasz 'sil2100' Zemczak <lukasz.zemczak@canonical.com>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; version 3.
#
# This program is distributed in the hope that it will be useful, but WITHOUTa
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


import sys
import unittest
from mock import MagicMock, patch
from StringIO import StringIO

import mateconf
import glib
import os.path
import subprocess

# Module Under Test (mut)
try:
    mut = __import__("02_migrate_to_gsettings")
except (ImportError):
    sys.exit("Error! 02_migrate_to_gsettings module-under-test not found - perhaps you missed including it in the PYTHONPATH environment variable? Add the migration module directory and re-run the script again.")

class MigrateGconfToGsettingsTests(unittest.TestCase):
    """Test suite for method migrate_mateconf_to_gsettings"""

    def setUp(self):
        """Redirects stdout so that tests can assert print statements"""
        self.stdout = sys.stdout
        self.out = StringIO()
        sys.stdout = self.out

    def tearDown(self):
        """Restores stdout"""
        sys.stdout = self.stdout
        # Dump the content that was sent to stdout
        # print(self.out.getvalue())

    def testClientGetDefaultFalse(self):
        """Test missing mateconf client"""
        mateconf.client_get_default = MagicMock(return_value=False)
        mut.migrate_mateconf_to_gsettings()
        self.assertEqual(self.out.getvalue().strip(), "WARNING: no mateconf client found. No transitionning will be done")

    def testGetSchemaException(self):
        """Test exception handling for get_schema"""
        mateconf_mock = MagicMock(name="mateconf-Mock")
        mateconf.client_get_default = MagicMock(return_value=mateconf_mock)
        mateconf_schema_mock = MagicMock(name="mateconf.Schema-Mock")
        mateconf_mock.get_schema = MagicMock(return_value=mateconf_schema_mock,
                                 side_effect=glib.GError)
        self.assertRaises(glib.GError, mut.migrate_mateconf_to_gsettings())

    def testGetSchemaNone(self):
        """Test missing schema"""
        mateconf_mock = MagicMock(name="mateconf-Mock")
        mateconf.client_get_default = MagicMock(return_value=mateconf_mock)
        #mateconf_schema_mock = MagicMock(name="mateconf.Schema-Mock")
        mateconf_mock.get_schema = MagicMock(return_value=False)
        mut.migrate_mateconf_to_gsettings()
        self.assertEqual(self.out.getvalue().strip(), "No current profile set, no migration needed")

    def setupGetSchema(self, profile):
        """Set up mock objects for testing a valid schema"""
        # mateconf.client_get_default
        mateconf_mock = MagicMock(name="mateconf-Mock")
        mateconf.client_get_default = MagicMock(return_value=mateconf_mock)
        # client.get_schema
        mateconf_schema_mock = MagicMock(name="mateconf.Schema-Mock")
        mateconf_mock.get_schema = MagicMock(return_value=mateconf_schema_mock)
        # current_profile_schema.get_default_value
        mateconf_mateconfvalue_mock = MagicMock(name="mateconf.Schema.mateconfvalue-Mock")
        mateconf_schema_mock.get_default_value = MagicMock(return_value=mateconf_mateconfvalue_mock)
        # current_profile_mateconfvalue.get_string
        mateconf_mateconfvalue_mock.get_string = MagicMock(return_value=profile)

        # Popen
        subprocess_mock = MagicMock(name="subprocess-Mock")
        popen_mock = MagicMock(name="popen-Mock", return_value=subprocess_mock)
        subprocess.Popen = popen_mock
        return popen_mock

    def testGetSchemaUnity(self):
        """Test the 'unity' schema"""
        popen_mock = self.setupGetSchema("unity")
        mut.migrate_mateconf_to_gsettings()
        # 2 files should be converted
        self.assertEqual(len(popen_mock.call_args_list), 2)
        self.assertEqual(popen_mock.call_args_list[0][0][0][1],
                         '--file=/usr/lib/compiz/migration/compiz-profile-active-unity.convert')
        self.assertEqual(popen_mock.call_args_list[1][0][0][1],
                         '--file=/usr/lib/compiz/migration/compiz-profile-Default.convert')

    def testGetSchemaDefaultConvertPathInvalid(self):
        """Test the default schema"""
        popen_mock = self.setupGetSchema("Default")
        os.path.exists = MagicMock(return_value=False)
        mut.migrate_mateconf_to_gsettings()
        # 1 file should be converted
        self.assertEqual(len(popen_mock.call_args_list), 1)
        self.assertEqual(popen_mock.call_args_list[0][0][0][1],
                         '--file=/usr/lib/compiz/migration/compiz-profile-active-Default.convert')

    def testGetSchemaDefaultConvertPathValid(self):
        """Test the default schema with a valid unity convert file"""
        popen_mock = self.setupGetSchema("Default")
        exists_mock = MagicMock(return_value=True)
        os.path.exists = exists_mock
        mut.migrate_mateconf_to_gsettings()
        self.assertEqual(exists_mock.call_args_list[0][0][0], 
                         mut.CONVERT_PATH + 'compiz-profile-unity.convert')
        # 2 files should be converted
        self.assertEqual(len(popen_mock.call_args_list), 2)
        self.assertEqual(popen_mock.call_args_list[0][0][0][1],
                         '--file=/usr/lib/compiz/migration/compiz-profile-active-Default.convert')
        self.assertEqual(popen_mock.call_args_list[1][0][0][1],
                         '--file=/usr/lib/compiz/migration/compiz-profile-unity.convert')


if __name__ == '__main__':
    unittest.main()

