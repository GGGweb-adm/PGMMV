#include "DllPluginManager.h"

DllPluginManager* DllPluginManager::_dllPluginManager = nullptr;

DllPluginManager::DllPluginManager()
{
}

DllPluginManager::~DllPluginManager()
{
}

#if 0
DllPluginManager* DllPluginManager::getInstance()
{
	if (!_dllPluginManager) {
		_dllPluginManager = new DllPluginManager();
	}
	return _dllPluginManager;
}
#endif

void DllPluginManager::purge()
{
	if (!_dllPluginManager)
		return;

	DllPluginManager *fm = _dllPluginManager;
	_dllPluginManager = NULL;
	fm->release();
}

