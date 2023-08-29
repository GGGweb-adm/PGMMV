#include "OutputManager.h"
#include "platform/CCVibration.h"

USING_NS_CC;

OutputManager* OutputManager::output_manager_ = NULL;

OutputManager::OutputManager()
{
}

OutputManager::~OutputManager()
{
}

OutputManager* OutputManager::getInstance()
{
	if (output_manager_ == NULL) {
		output_manager_ = new OutputManager();
	}
	return output_manager_;
}

void OutputManager::purge()
{
	if (output_manager_ == NULL) return;
	
	OutputManager* pointer = output_manager_;
	output_manager_ = NULL;
	pointer->release();
}

void OutputManager::playVibrationFile(int file_index, int controller_id)
{
	CC_ASSERT(output_manager_ != NULL);
	Vibration::playVibrationFile(file_index, controller_id);
}