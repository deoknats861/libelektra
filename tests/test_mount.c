/*************************************************************************** 
 *           test_trie.c  - Test suite for trie data structure
 *                  -------------------
 *  begin                : Thu Oct 24 2007
 *  copyright            : (C) 2007 by Patrick Sabin
 *  email                : patricksabin@gmx.at
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <tests.h>

/*Needs private declarations*/
#include <kdbprivate.h>

KDB* kdb_new()
{
	KDB *kdb = elektraCalloc (sizeof (KDB));
	return kdb;
}

Backend *b_new(const char *name, const char *value)
{
	Backend *backend = elektraCalloc (sizeof (Backend));
	backend->refcounter = 1;

	backend->mountpoint = keyNew (name, KEY_VALUE, value, KEY_END);
	keyIncRef (backend->mountpoint);

	return backend;
}

void kdb_del(KDB *kdb)
{
	elektraBackendClose (kdb->defaultBackend, 0);
	elektraTrieClose(kdb->trie, 0);

	elektraFree (kdb);
}

void test_mount()
{
	printf ("test mount backend\n");

	KDB *kdb = kdb_new();
	elektraMountBackend (kdb, b_new("user", "user"), 0);
	succeed_if (kdb->trie, "there should be a trie");

	Key *mp = keyNew ("user", KEY_VALUE, "user", KEY_END);
	Key *sk = keyNew ("user", KEY_VALUE, "user", KEY_END);

	succeed_if (compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp) == 0, "could not find mp");
	succeed_if (compare_key (elektraMountGetMountpoint (kdb, sk), mp) == 0, "could not find mp");

	keySetName (sk, "user/below");
	succeed_if (compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp) == 0, "could not find mp");
	succeed_if (compare_key (elektraMountGetMountpoint (kdb, sk), mp) == 0, "could not find mp below");

	keySetName (sk, "system");
	kdb->defaultBackend = b_new("", "default");
	succeed_if (elektraMountGetBackend (kdb, sk) == kdb->defaultBackend, "did not return default backend");

	keySetName (mp, "");
	keySetString (mp, "default");
	succeed_if (compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp) == 0, "could not find mp");
	succeed_if (compare_key (elektraMountGetMountpoint (kdb, sk), mp) == 0, "could not find mp below");

	keyDel (sk);
	keyDel (mp);

	kdb_del (kdb);
}

KeySet *modules_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/modules", KEY_END),
		KS_END);
}

KeySet *minimal_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		KS_END);
}


void test_minimaltrie()
{
	printf ("Test minimal mount\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, minimal_config(), modules, errorKey) == 0, "could not open minimal config")

	output_warnings (errorKey);
	output_errors (errorKey);

	succeed_if (!kdb->trie, "minimal trie is null");

	keyDel (errorKey);
	ksDel (modules);
	kdb_del (kdb);
}

KeySet *simple_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/simple", KEY_END),
		KS_END);
}

void test_simple()
{
	printf ("Test simple mount\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, simple_config(), modules, errorKey) == 0, "could not open trie");

	output_warnings (errorKey);
	output_errors (errorKey);

	exit_if_fail (kdb->trie, "kdb->trie was not build up successfully");

	Key *searchKey = keyNew("user", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");


	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/below");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	keyDel (errorKey);
	ksDel (modules);
	keyDel (mp);
	keyDel (searchKey);
	kdb_del (kdb);
}

KeySet *set_simple()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints/simple", KEY_END),

		keyNew("system/elektra/mountpoints/simple/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/anything", KEY_VALUE, "backend", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/simple/errorplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/errorplugins/#1default", KEY_VALUE, "default", KEY_END),

		keyNew("system/elektra/mountpoints/simple/getplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default", KEY_VALUE, "default", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/anything", KEY_VALUE, "plugin", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/backend/simple", KEY_END),

		keyNew("system/elektra/mountpoints/simple/setplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/setplugins/#1default", KEY_VALUE, "default", KEY_END),

		keyNew("system/elektra/mountpoints/simple/errorplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/errorplugins/#1default", KEY_VALUE, "default", KEY_END),
		KS_END);

}


KeySet *set_pluginconf()
{
	return ksNew( 10 ,
		keyNew ("system/anything", KEY_VALUE, "backend", KEY_END),
		keyNew ("system/more", KEY_END),
		keyNew ("system/more/config", KEY_END),
		keyNew ("system/more/config/below", KEY_END),
		keyNew ("system/path", KEY_END),
		keyNew ("user/anything", KEY_VALUE, "plugin", KEY_END),
		keyNew ("user/more", KEY_END),
		keyNew ("user/more/config", KEY_END),
		keyNew ("user/more/config/below", KEY_END),
		keyNew ("user/path", KEY_END),
		KS_END);
}

void test_simpletrie()
{
	printf ("Test simple mount with plugins\n");

	KDB *kdb = kdb_new();
	KeySet *modules = ksNew(0);
	elektraModulesInit(modules, 0);

	KeySet *config = set_simple();
	ksAppendKey(config, keyNew("system/elektra/mountpoints", KEY_END));
	succeed_if (elektraMountOpen(kdb, config, modules, 0) == 0, "could not open mount");

	Key *key = keyNew("user/tests/backend/simple", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, key);

	keyAddBaseName(key, "somewhere"); keyAddBaseName(key, "deep"); keyAddBaseName(key, "below");
	Backend *backend2 = elektraTrieLookup(kdb->trie, key);
	succeed_if (backend == backend2, "should be same backend");

	succeed_if (backend->getplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->getplugins[1] != 0, "there should be a plugin");
	succeed_if (backend->getplugins[2] == 0, "there should be no plugin");

	succeed_if (backend->setplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->setplugins[1] != 0, "there should be a plugin");
	succeed_if (backend->setplugins[2] == 0, "there should be no plugin");

	Key *mp;
	succeed_if ((mp = backend->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user/tests/backend/simple"), "wrong mountpoint for backend");
	succeed_if (!strcmp(keyString(mp), "simple"), "wrong name for backend");

	Plugin *plugin = backend->getplugins[1];

	KeySet *test_config = set_pluginconf();
	KeySet *cconfig = elektraPluginGetConfig (plugin);
	succeed_if (cconfig != 0, "there should be a config");
	succeed_if (compare_keyset(cconfig, test_config) == 0, "error comparing keyset");
	ksDel (test_config);

	succeed_if (plugin->kdbGet != 0, "no get pointer");
	succeed_if (plugin->kdbSet != 0, "no set pointer");


	keyDel (key);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	kdb_del (kdb);
}


KeySet *set_two()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),

		keyNew("system/elektra/mountpoints/simple/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/anything", KEY_VALUE, "backend", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/simple/getplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default", KEY_VALUE, "default", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/anything", KEY_VALUE, "plugin", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/backend/simple", KEY_END),

		keyNew("system/elektra/mountpoints/simple/setplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/setplugins/#1default", KEY_VALUE, "default", KEY_END),


		keyNew("system/elektra/mountpoints/two", KEY_END),

		keyNew("system/elektra/mountpoints/two/config", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/anything", KEY_VALUE, "backend", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/two/getplugins", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default", KEY_VALUE, "default", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/anything", KEY_VALUE, "plugin", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/two/mountpoint", KEY_VALUE, "user/tests/backend/two", KEY_END),

		keyNew("system/elektra/mountpoints/two/setplugins", KEY_END),
		keyNew("system/elektra/mountpoints/two/setplugins/#1default", KEY_VALUE, "default", KEY_END),
		keyNew("system/elektra/mountpoints/two/setplugins/#2default", KEY_VALUE, "default", KEY_END),
		KS_END);
}

void test_two()
{
	printf ("Test two mounts\n");

	KDB *kdb = kdb_new();
	KeySet *modules = ksNew(0);
	elektraModulesInit(modules, 0);

	KeySet *config = set_two();
	ksAppendKey(config, keyNew("system/elektra/mountpoints", KEY_END));
	succeed_if (elektraMountOpen (kdb, config, modules, 0) == 0, "could not open mount");

	Key *key = keyNew("user/tests/backend/simple", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, key);

	keyAddBaseName(key, "somewhere"); keyAddBaseName(key, "deep"); keyAddBaseName(key, "below");
	Backend *backend2 = elektraTrieLookup(kdb->trie, key);
	succeed_if (backend == backend2, "should be same backend");

	succeed_if (backend->getplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->getplugins[1] != 0, "there should be a plugin");
	succeed_if (backend->getplugins[2] == 0, "there should be no plugin");

	succeed_if (backend->setplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->setplugins[1] != 0, "there should be a plugin");
	succeed_if (backend->setplugins[2] == 0, "there should be no plugin");

	Key *mp;
	succeed_if ((mp = backend->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user/tests/backend/simple"), "wrong mountpoint for backend");
	succeed_if (!strcmp(keyString(mp), "simple"), "wrong name for backend");

	Plugin *plugin = backend->getplugins[1];

	KeySet *test_config = set_pluginconf();
	KeySet *cconfig = elektraPluginGetConfig (plugin);
	succeed_if (cconfig != 0, "there should be a config");
	compare_keyset(cconfig, test_config);
	ksDel (test_config);

	succeed_if (plugin->kdbGet != 0, "no get pointer");
	succeed_if (plugin->kdbSet != 0, "no set pointer");

	keySetName(key, "user/tests/backend/two");
	Backend *two = elektraTrieLookup(kdb->trie, key);
	succeed_if (two != backend, "should be differnt backend");

	succeed_if ((mp = two->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user/tests/backend/two"), "wrong mountpoint for backend two");
	succeed_if (!strcmp(keyString(mp), "two"), "wrong name for backend");

	keyDel (key);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	kdb_del (kdb);
}


KeySet *set_us()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/user", KEY_END),
		keyNew("system/elektra/mountpoints/user/mountpoint", KEY_VALUE, "user", KEY_END),
		keyNew("system/elektra/mountpoints/system", KEY_END),
		keyNew("system/elektra/mountpoints/system/mountpoint", KEY_VALUE, "system", KEY_END),
		KS_END);

}

void test_us()
{
	printf ("Test mounting of user and system backends\n");

	KDB *kdb = kdb_new();
	KeySet *modules = ksNew(0);
	elektraModulesInit(modules, 0);

	KeySet *config = set_us();
	ksAppendKey(config, keyNew("system/elektra/mountpoints", KEY_END));
	succeed_if (elektraMountOpen(kdb, config, modules, 0) == 0, "could not open mount");

	Key *key = keyNew("user/anywhere/backend/simple", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, key);

	keyAddBaseName(key, "somewhere"); keyAddBaseName(key, "deep"); keyAddBaseName(key, "below");
	Backend *backend2 = elektraTrieLookup(kdb->trie, key);
	succeed_if (backend == backend2, "should be same backend");

	succeed_if (backend->getplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->getplugins[1] == 0, "there should be no plugin");
	succeed_if (backend->getplugins[2] == 0, "there should be no plugin");

	succeed_if (backend->setplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->setplugins[1] == 0, "there should be no plugin");
	succeed_if (backend->setplugins[2] == 0, "there should be no plugin");

	Key *mp;
	succeed_if ((mp = backend->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user"), "wrong mountpoint for backend");
	succeed_if (!strcmp(keyString(mp), "user"), "wrong name for backend");


	keySetName(key, "system/anywhere/tests/backend/two");
	Backend *two = elektraTrieLookup(kdb->trie, key);
	succeed_if (two != backend, "should be differnt backend");

	succeed_if ((mp = two->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "system"), "wrong mountpoint for backend two");
	succeed_if (!strcmp(keyString(mp), "system"), "wrong name for backend");

	keyDel (key);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	kdb_del (kdb);

}

KeySet *endings_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/slash", KEY_END),
		keyNew("system/elektra/mountpoints/slash/mountpoint", KEY_VALUE, "user/endings", KEY_END),
		keyNew("system/elektra/mountpoints/hash", KEY_END),
		keyNew("system/elektra/mountpoints/hash/mountpoint", KEY_VALUE, "user/endings#", KEY_END),
		keyNew("system/elektra/mountpoints/space", KEY_END),
		keyNew("system/elektra/mountpoints/space/mountpoint", KEY_VALUE, "user/endings ", KEY_END),
		keyNew("system/elektra/mountpoints/endings", KEY_END),
		keyNew("system/elektra/mountpoints/endings/mountpoint", KEY_VALUE, "user/endings\200", KEY_END),
		KS_END);
}

void test_endings()
{
	printf ("Test mounting with different endings\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, endings_config(), modules, errorKey) == 0, "could not open mount");

	output_warnings (errorKey);
	output_errors (errorKey);

	exit_if_fail (kdb->trie, "kdb->trie was not build up successfully");

	Key *searchKey = keyNew("user", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");


	Key *mp = keyNew("user/endings", KEY_VALUE, "slash", KEY_END);
	keySetName(searchKey, "user/endings");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/endings#");
	keySetName(mp, "user/endings#");
	keySetString(mp, "hash");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/endings/_");
	keySetName(mp, "user/endings");
	keySetString(mp, "slash");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be the same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/endings/X");
	keySetName(mp, "user/endings");
	keySetString(mp, "slash");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be the same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/endings_");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!b2, "there should be no backend");


	keySetName(searchKey, "user/endingsX");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!b2, "there should be no backend");


	keySetName(searchKey, "user/endings!");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!b2, "there should be no backend");


	keySetName(searchKey, "user/endings ");
	keySetName(mp, "user/endings ");
	keySetString(mp, "space");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	keySetName(searchKey, "user/endings\200");
	keySetName(mp, "user/endings\200");
	keySetString(mp, "endings");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	// output_trie(trie);

	keyDel (errorKey);
	ksDel (modules);
	keyDel (mp);
	keyDel (searchKey);
	kdb_del (kdb);
}

KeySet *oldroot_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/root", KEY_END),
		keyNew("system/elektra/mountpoints/root/mountpoint", KEY_VALUE, "", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/simple", KEY_END),
		KS_END);
}

void test_oldroot()
{
	printf ("Test mounting with old root\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, oldroot_config(), modules, errorKey) == -1, "no warning issued?");

	succeed_if(keyGetMeta(errorKey, "warnings") != 0, "there should be warnings");

	/*
	output_warnings (errorKey);
	output_errors (errorKey);
	*/

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	Key *searchKey = keyNew("user", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no root backend");


	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/below");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

KeySet *cascading_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "/tests/simple", KEY_END),
		KS_END);
}

void keySetCascading(Key *key, const char* name)
{
	size_t size = elektraStrLen(name) + 1;
	if (size < 2) return;

	elektraFree (key->key);
	key->key = elektraMalloc (size);
	key->keySize = size;
	key->key[0] = '/';
	strcpy (key->key+1, name);
}

void test_cascading()
{
	printf ("Test simple mount with cascading\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, cascading_config(), modules, errorKey) == 0, "could not open trie");

	output_warnings (errorKey);
	output_errors (errorKey);

	exit_if_fail (kdb->trie, "kdb->trie was not build up successfully");

	output_trie (kdb->trie);

	Key *searchKey = keyNew("user", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");

	keySetName(searchKey, "system");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");


	Key *mp = keyNew("", KEY_VALUE, "simple", KEY_END);
	keySetCascading (mp, "tests/simple");

	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");
	output_key (backend->mountpoint);


	keySetName(searchKey, "user/tests/simple/below");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "system/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");

	keySetName(searchKey, "system/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "system/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keyDel (errorKey);
	ksDel (modules);
	keyDel (mp);
	keyDel (searchKey);
	kdb_del (kdb);
}


KeySet *root_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/root", KEY_END),
		keyNew("system/elektra/mountpoints/root/mountpoint", KEY_VALUE, "/", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/simple", KEY_END),
		KS_END);
}

void test_root()
{
	printf ("Test mounting with root\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, root_config(), modules, errorKey) == 0, "could not buildup mount");

	/*
	output_warnings (errorKey);
	output_errors (errorKey);
	*/

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	Key *searchKey = keyNew("", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	keySetCascading (rmp, "");
	Backend *b2 = 0;

	keySetName (searchKey, "user");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (compare_key(b2->mountpoint, rmp) == 0, "mountpoint key not correct");


	Backend *backend = 0;
	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

int main(int argc, char** argv)
{
	printf("TRIE       TESTS\n");
	printf("==================\n\n");

	init (argc, argv);

	test_mount();
	test_minimaltrie();
	test_simple();
	test_simpletrie();
	test_two();
	test_us();
	test_endings();
	test_oldroot();
	test_cascading();
	test_root();

	printf("\ntest_trie RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}

