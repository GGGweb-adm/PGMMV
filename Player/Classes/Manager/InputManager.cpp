#include "InputManager.h"
#include "GameManager.h"
#include "AppMacros.h"
#include "Manager/DebugManager.h"

//-------------------------------------------------------------------------------------------------------------------
InputMouseData::InputMouseData()
{
	_inputList = nullptr;
}

InputMouseData::~InputMouseData()
{
	CC_SAFE_RELEASE_NULL(_inputList);
}

bool InputMouseData::init()
{
	auto inputList = cocos2d::__Dictionary::create();
	if (inputList == nullptr) {
		return false;
	}
	this->setInputList(inputList);
	_move = false;
	_updateMove = false;
	_updateWheel = false;
	_wheelTrigger = false;
	_wheelRelease = 0;
	return true;
}

void InputMouseData::reset()
{
	_point = cocos2d::Vec2::ZERO;
	_oldPoint = cocos2d::Vec2::ZERO;
	_movePoint = cocos2d::Vec2::ZERO;
	_startPoint = cocos2d::Vec2::ZERO;
	_scrollPoint = cocos2d::Vec2::ZERO;
	_oldScrollPoint = cocos2d::Vec2::ZERO;
	_move = false;
	_updateMove = false;
	_updateWheel = false;
	_wheelTrigger = false;
	_wheelRelease = 0;
	this->getInputList()->removeAllObjects();
}

void InputMouseData::update()
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(_inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<InputData *>(el->getObject());
#else
		auto data = dynamic_cast<InputData *>(el->getObject());
#endif
		data->update();
	}
	//move
	if (_updateMove && _move) {
		_move = false;
	}
	_updateMove = true;
	//wheel
	if (_updateWheel) {
		if (_wheelRelease) {
			_wheelRelease = 0;
			_oldScrollPoint = cocos2d::Vec2::ZERO;
			_scrollPoint = cocos2d::Vec2::ZERO;
		}
		else {
			if (_scrollPoint.y != 0) {
				_wheelRelease = (_scrollPoint.y < 0) ? -1 : 1;
			}
			_oldScrollPoint = _scrollPoint;
			_scrollPoint = cocos2d::Vec2::ZERO;
		}
		_wheelTrigger = false;
	}
	_updateWheel = true;
}

InputMouseData::InputData *InputMouseData::getInputData(int keyCode)
{
	auto inputData = dynamic_cast<InputData *>(this->getInputList()->objectForKey(keyCode));
	if (inputData == nullptr) {
		inputData = InputData::create(keyCode);
		CC_ASSERT(inputData);
		this->getInputList()->setObject(inputData, keyCode);
	}
	return inputData;
}

bool InputMouseData::isPressOr()
{
	auto inputList = this->getInputList();
	cocos2d::DictElement *el = nullptr;
	bool ret = false;
	CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto inputData = static_cast<InputData *>(el->getObject());
#else
		auto inputData = dynamic_cast<InputData *>(el->getObject());
#endif
		ret |= inputData->getPress();
	}
	return ret;
}

cocos2d::Vec2 InputMouseData::setPoint(cocos2d::Vec2 v)
{
	_oldPoint = _point;
	_point = v;
	return _oldPoint;
}

cocos2d::Vec2 InputMouseData::setScrollPoint(cocos2d::Vec2 v)
{
	if (v.y < 0 && _scrollPoint.y >= 0 || v.y > 0 && _scrollPoint.y <= 0) {
		_wheelTrigger = true;
	}
	else if (_updateWheel) {
		_wheelTrigger = false;
	}
	_updateWheel = false;
	_wheelRelease = 0;
	_oldScrollPoint = _scrollPoint;
	_scrollPoint = v;
	return _oldScrollPoint;
}

cocos2d::Vec2 InputMouseData::CalcTransLeftUp(cocos2d::Vec2 v, cocos2d::Size size, cocos2d::Size displaySize)
{
	return cocos2d::Vec2(
		v.x * displaySize.width / size.width,
		displaySize.height - (v.y * displaySize.height / size.height)
	);
}

void InputMouseData::setMove(bool move)
{
	_move = move;
	_updateMove = false;
}

void InputMouseData::InputData::update()
{
	//trigger
	if (_updateTrigger && _trigger) {
		_trigger = false;
	}
	_updateTrigger = true;
	//release
	if (_updateRelease && _release) {
		_release = false;
	}
	_updateRelease = true;
}

void InputMouseData::InputData::setTrigger(bool trigger)
{
	_updateTrigger = false;
	_trigger = trigger;
}

void InputMouseData::InputData::setRelease(bool release)
{
	_updateRelease = false;
	_release = release;
}

void InputMouseData::InputData::reset()
{
	_press = false;
	_release = false;
	_trigger = false;
	_updateTrigger = true;
	_updateRelease = true;
}

bool InputMouseData::InputData::init(int keyCode)
{
	_keyCode = keyCode;
	_press = false;
	_release = false;
	_trigger = false;
	_updateTrigger = true;
	_updateRelease = true;
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
int InputKeyboardData::upperKeyCode(int keyCode)
{
	if (124 <= keyCode && keyCode <= 149) {
		keyCode -= 31;
	}
	return keyCode;
}
InputKeyboardData::InputKeyboardData()
{
	_inputList = nullptr;
	_pressDataList = nullptr;
	_emptyData = nullptr;
}

InputKeyboardData::~InputKeyboardData()
{
	CC_SAFE_RELEASE_NULL(_inputList);
	CC_SAFE_RELEASE_NULL(_pressDataList);
	CC_SAFE_RELEASE_NULL(_emptyData);
}

bool InputKeyboardData::init()
{
	auto inputList = cocos2d::__Dictionary::create();
	if (inputList == nullptr) {
		return false;
	}
	this->setInputList(inputList);
	this->setEmptyData(InputData::create(-1));
	this->setPressDataList(cocos2d::__Array::create());
	return true;
}

void InputKeyboardData::reset(bool isReset/*=false*/)
{
	//入力中のデータは残す。
	bool bRetry = false;
	auto inputList = this->getInputList();
	do {
		bRetry = false;
		cocos2d::DictElement *el;
		CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<InputData *>(el->getObject());
#else
			auto p = dynamic_cast<InputData *>(el->getObject());
#endif
			if (isReset || (p->getPress() == false && p->getTrigger() == false && p->getRelease() == false)) {
				inputList->removeObjectForElememt(el);
				bRetry = true;
				break;
			}
		}
	} while (bRetry);
	this->setIgnored(false);
	this->getPressDataList()->removeAllObjects();
}

void InputKeyboardData::update()
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(_inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<InputData *>(el->getObject());
#else
		auto data = dynamic_cast<InputData *>(el->getObject());
#endif
		data->update();
	}
}

InputKeyboardData::InputData *InputKeyboardData::setPressData(int keyCode, int scancode)
{
	auto inputList = this->getInputList();
	auto inputData = dynamic_cast<InputData *>(inputList->objectForKey(keyCode));
	if (inputData == nullptr) {
		inputData = InputData::create(keyCode);
		inputList->setObject(inputData, keyCode);
	}
	inputData->setCharData(-1);
	inputData->setScancode(scancode);
	inputData->setTrigger(true);
	inputData->setPress(true);
	this->getPressDataList()->addObject(inputData);
	return inputData;
}

InputKeyboardData::InputData *InputKeyboardData::setReleaseData(int keyCode, int scancode)
{
	auto inputList = this->getInputList();
	auto inputData = dynamic_cast<InputData *>(inputList->objectForKey(keyCode));
	if (inputData == nullptr) {
		cocos2d::DictElement *el;
		CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<InputData *>(el->getObject());
#else
			auto p = dynamic_cast<InputData *>(el->getObject());
#endif
			if (p->_keyCode == keyCode && p->getScancode() == scancode) {
				inputData = p;
				break;
			}
		}
	}
	if (inputData != nullptr) {
		inputData->setPress(false);
		inputData->setRelease(true);
		this->getPressDataList()->removeObject(inputData);
	}
	return inputData;
}

InputKeyboardData::InputData *InputKeyboardData::setRepeatData(int keyCode, int scancode)
{
	auto inputList = this->getInputList();
	auto inputData = dynamic_cast<InputData *>(inputList->objectForKey(keyCode));
	if (inputData == nullptr) {
		inputData = InputData::create(keyCode);
		inputList->setObject(inputData, keyCode);
	}
	inputData->setCharData(-1);
	inputData->setScancode(scancode);
	inputData->setPress(true);
	this->getPressDataList()->addObject(inputData);
	return inputData;
}

InputKeyboardData::InputData *InputKeyboardData::setCharInputData(int keyCode)
{
	auto pressDataList = this->getPressDataList();
	if (pressDataList->count() > 0) {
		auto inputData = dynamic_cast<InputData *>(pressDataList->getObjectAtIndex(0));
		if (inputData != nullptr) {
			auto p = dynamic_cast<InputData *>(this->getInputList()->objectForKey(keyCode));
			if (p && p->getKeyCode() != keyCode) {
				this->getInputList()->removeObjectForKey(keyCode);
			}
			pressDataList->removeObject(inputData);
		}
		return inputData;
	}
	return nullptr;
}

InputKeyboardData::InputData *InputKeyboardData::getInputData(int keyCode)
{
	auto inputList = this->getInputList();
	auto inputData = dynamic_cast<InputData *>(inputList->objectForKey(keyCode));
	if (inputData == nullptr) {
		cocos2d::DictElement *el;
		CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<InputData *>(el->getObject());
#else
			auto p = dynamic_cast<InputData *>(el->getObject());
#endif
			if (p->getKeyCode() == keyCode) {
				return p;
			}
		}
		//無い場合は、空データを渡す。
		inputData = this->getEmptyData();
		inputData->_keyCode = keyCode;
	}
	return inputData;
}

void InputKeyboardData::InputData::update()
{
	//trigger
	if (_updateTrigger && _trigger) {
		_trigger = false;
	}
	_updateTrigger = true;
	//release
	if (_updateRelease && _release) {
		_release = false;
	}
	_updateRelease = true;
}

void InputKeyboardData::InputData::setTrigger(bool trigger)
{
	_updateTrigger = false;
	_trigger = trigger;
}

void InputKeyboardData::InputData::setRelease(bool release)
{
	_updateRelease = false;
	_release = release;
}

void InputKeyboardData::InputData::setCharData(int charCode)
{
	_charCode = charCode;
}

int InputKeyboardData::InputData::getKeyCode()
{
	return _charCode < 0 ? _keyCode : _charCode;
}

bool InputKeyboardData::InputData::init(int keyCode)
{
	_keyCode = keyCode;
	_charCode = -1;
	_press = false;
	_release = false;
	_trigger = false;
	_updateTrigger = true;
	_updateRelease = true;
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
InputGamepadData::InputGamepadData()
{
	_name = nullptr;
	_inputList = nullptr;
	_connected = false;
	_ignored = false;
}

InputGamepadData::~InputGamepadData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_inputList);
}

bool InputGamepadData::init()
{
	auto inputList = cocos2d::__Dictionary::create();
	if (inputList == nullptr) {
		return false;
	}
	//button
	for (int keyCode = kInputButton1 ; keyCode <= kInputButton18; keyCode++) {
		auto inputData = InputData::create(InputData::kTypeButton);
		if (inputData == nullptr) {
			return false;
		}
		inputList->setObject(inputData, keyCode);
	}
	//axis
	for (int keyCode = kInputAxis1; keyCode <= kInputAxis6; keyCode++) {
		auto inputData = InputData::create(InputData::kTypeAxis);
		if (inputData == nullptr) {
			return false;
		}
		inputList->setObject(inputData, keyCode);
	}
	//logic
	for (int keyCode = kInputLeftStickUp; keyCode < kInputMax; keyCode++) {
		auto inputData = InputData::create(InputData::kTypeLogic);
		if (inputData == nullptr) {
			return false;
		}
		inputList->setObject(inputData, keyCode);
	}
	this->setInputList(inputList);
	this->setConnected(false);
	this->setIgnored(false);
	return true;
}

void InputGamepadData::setup(EnumGamepadType gamepadType)
{
	auto inputList = this->getInputList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto inputData = static_cast<InputData *>(el->getObject());
#else
		auto inputData = dynamic_cast<InputData *>(el->getObject());
#endif
		auto keyCode = el->getIntKey();
		switch (gamepadType) {
		case kGamepadXbox360:
		case kGamepadPlayStation4:
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			{
			if (kInputButton1 <= keyCode && keyCode <= kInputButton18) {
				inputData->setType(InputData::kTypeButton);
			}
			else if (kInputAxis1 <= keyCode && keyCode <= kInputAxis6) {
				inputData->setType(InputData::kTypeAxis);
			}
			else if (kInputLeftStickUp <= keyCode && keyCode <= kInputRightStickLeft) {
				inputData->setType(InputData::kTypeLogic);
			}
			break; }
		case kGamepadDirectInput: {
			if (kInputButton1 <= keyCode && keyCode <= kInputButton18) {
				inputData->setType(InputData::kTypeButton);
			}
#if 0
			else if (kInputAxis1Minus <= keyCode && keyCode < kInputMax) {
				inputData->setType(InputData::kTypeLogic);
			}
#endif
			// ACT2-5845
			else if (kInputAxis1Minus <= keyCode && keyCode < kInputMax) {
				inputData->setType(InputData::kTypeAxis);
			}

			break; }
		default: CC_ASSERT(0);
		}
	}
}

void InputGamepadData::reset(bool isReset/*=false*/)
{
	cocos2d::DictElement *el = nullptr;
	auto inputList = this->getInputList();
	CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<InputData *>(el->getObject());
#else
		auto p = dynamic_cast<InputData *>(el->getObject());
#endif
		if (isReset || !p->isValue()) {
			p->reset();
		}
	}
	this->setName(nullptr);
	this->setConnected(false);
	this->setIgnored(false);
}

void InputGamepadData::setNameWithChar(const char *name)
{
	auto n = cocos2d::__String::create(name);
	this->setName(n);
}

const char *InputGamepadData::getName()
{
	if (_name == nullptr) {
		return nullptr;
	}
	return _name->getCString();
}

InputGamepadData::EnumGamepadType InputGamepadData::getGamepadType()
{
	auto name = this->getName();
	//xbox360
	if (name && strstr(name, "Xbox 360 Controller") != nullptr) {
		return kGamepadXbox360;
	}
	//ps4
	else if (name && strstr(name, "Wireless Controller") != nullptr) {
		return kGamepadPlayStation4;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
	//direct input
	return kGamepadDirectInput;
}

InputGamepadData::InputData *InputGamepadData::getInput(int keyCode)
{
	return dynamic_cast<InputData *>(this->getInputList()->objectForKey(keyCode));
}

#define AXIS_THRESHOLD	0.5f
void InputGamepadData::update()
{
	//物理入力
	cocos2d::DictElement *el = nullptr;
	auto inputList = this->getInputList();
	for (int keyCode = kInputButton1; keyCode <= kInputAxis6; keyCode++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<InputData *>(inputList->objectForKey(keyCode));
#else
		auto p = dynamic_cast<InputData *>(inputList->objectForKey(keyCode));
#endif
		p->update();
	}

	//論理入力
	InputData *inputData = nullptr;
	float value = 0.0f;
	switch (this->getGamepadType()) {
#if 1
	// ACT2-6361
	case kGamepadXbox360: {
		//Left stick(X)
		inputData = this->getInput(kInputAxis1);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//right
			auto p = this->getInput(kInputLeftStickRight);
			if (p != nullptr) p->setValue((value > AXIS_THRESHOLD) ? 1 : 0);
			//left
			p = this->getInput(kInputLeftStickLeft);
			if (p != nullptr) p->setValue((value < -AXIS_THRESHOLD) ? -1 : 0);
		}
		//Left stick(Y)
		inputData = this->getInput(kInputAxis2);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//up
			auto p = this->getInput(kInputLeftStickUp);
			if (p != nullptr) p->setValue((value > AXIS_THRESHOLD) ? 1 : 0);
			//down
			p = this->getInput(kInputLeftStickDown);
			if (p != nullptr) p->setValue((value < -AXIS_THRESHOLD) ? -1 : 0);
		}
		//Right stick(X)
		inputData = this->getInput(kInputAxis3);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//right
			auto p = this->getInput(kInputRightStickRight);
			if (p != nullptr) p->setValue((value > AXIS_THRESHOLD) ? 1 : 0);
			//left
			p = this->getInput(kInputRightStickLeft);
			if (p != nullptr) p->setValue((value < -AXIS_THRESHOLD) ? -1 : 0);
		}
		//Right stick(Y)
		inputData = this->getInput(kInputAxis4);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//up
			auto p = this->getInput(kInputRightStickUp);
			if (p != nullptr) p->setValue((value > AXIS_THRESHOLD) ? 1 : 0);
			//down
			p = this->getInput(kInputRightStickDown);
			if (p != nullptr) p->setValue((value < -AXIS_THRESHOLD) ? -1 : 0);
		}
		break; }
	case kGamepadPlayStation4: {
		//Left stick(X)
		inputData = this->getInput(kInputAxis1);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//right
			auto p = this->getInput(kInputLeftStickRight);
			if (p != nullptr) p->setValue((value > AXIS_THRESHOLD) ? 1 : 0);
			//left
			p = this->getInput(kInputLeftStickLeft);
			if (p != nullptr) p->setValue((value < -AXIS_THRESHOLD) ? -1 : 0);
		}
		//Left stick(Y)
		inputData = this->getInput(kInputAxis2);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//up
			auto p = this->getInput(kInputLeftStickUp);
			if (p != nullptr) p->setValue((value < -AXIS_THRESHOLD) ? -1 : 0);
			//down
			p = this->getInput(kInputLeftStickDown);
			if (p != nullptr) p->setValue((value > AXIS_THRESHOLD) ? 1 : 0);
		}
		//Right stick(X)
		inputData = this->getInput(kInputAxis3);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//right
			auto p = this->getInput(kInputRightStickRight);
			if (p != nullptr) p->setValue((value > AXIS_THRESHOLD) ? 1 : 0);
			//left
			p = this->getInput(kInputRightStickLeft);
			if (p != nullptr) p->setValue((value < -AXIS_THRESHOLD) ? -1 : 0);
		}
		//Right stick(Y)
		inputData = this->getInput(kInputAxis6);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//up
			auto p = this->getInput(kInputRightStickUp);
			if (p != nullptr) p->setValue((value < -AXIS_THRESHOLD) ? -1 : 0);
			//down
			p = this->getInput(kInputRightStickDown);
			if (p != nullptr) p->setValue((value > AXIS_THRESHOLD) ? 1 : 0);
		}
		break;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif 
#if 0
	case kGamepadXbox360: {
		//Left stick(X)
		inputData = this->getInput(kInputAxis1);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//right
			auto p = this->getInput(kInputLeftStickRight);
			if (p != nullptr) p->setValue(value);
			//left
			p = this->getInput(kInputLeftStickLeft);
			if (p != nullptr) p->setValue(value);
		}
		//Left stick(Y)
		inputData = this->getInput(kInputAxis2);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//up
			auto p = this->getInput(kInputLeftStickUp);
			if (p != nullptr) p->setValue(value);
			//down
			p = this->getInput(kInputLeftStickDown);
			if (p != nullptr) p->setValue(value);
		}
		//Right stick(X)
		inputData = this->getInput(kInputAxis3);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//right
			auto p = this->getInput(kInputRightStickRight);
			if (p != nullptr) p->setValue(value);
			//left
			p = this->getInput(kInputRightStickLeft);
			if (p != nullptr) p->setValue(value);
		}
		//Right stick(Y)
		inputData = this->getInput(kInputAxis4);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//up
			auto p = this->getInput(kInputRightStickUp);
			if (p != nullptr) p->setValue(value);
			//down
			p = this->getInput(kInputRightStickDown);
			if (p != nullptr) p->setValue(value);
		}
		break; }
	case kGamepadPlayStation4: {
		//Left stick(X)
		inputData = this->getInput(kInputAxis1);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//right
			auto p = this->getInput(kInputLeftStickRight);
			if (p != nullptr) p->setValue(value);
			//left
			p = this->getInput(kInputLeftStickLeft);
			if (p != nullptr) p->setValue(value);
		}
		//Left stick(Y)
		inputData = this->getInput(kInputAxis2);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//up
			auto p = this->getInput(kInputLeftStickUp);
			if (p != nullptr) p->setValue(value);
			//down
			p = this->getInput(kInputLeftStickDown);
			if (p != nullptr) p->setValue(value);
		}
		//Right stick(X)
		inputData = this->getInput(kInputAxis3);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//right
			auto p = this->getInput(kInputRightStickRight);
			if (p != nullptr) p->setValue(value);
			//left
			p = this->getInput(kInputRightStickLeft);
			if (p != nullptr) p->setValue(value);
		}
		//Right stick(Y)
		inputData = this->getInput(kInputAxis6);
		if (inputData != nullptr) {
			value = inputData->getValue();
			//up
			auto p = this->getInput(kInputRightStickUp);
			if (p != nullptr) p->setValue(value);
			//down
			p = this->getInput(kInputRightStickDown);
			if (p != nullptr) p->setValue(value);
		}
		break;
	}
#endif
	case kGamepadDirectInput:
		break;
	default:CC_ASSERT(0);
	}
	for (int keyCode = kInputLeftStickUp; keyCode <= kInputRightStickLeft; keyCode++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<InputData *>(inputList->objectForKey(keyCode));
#else
		auto p = dynamic_cast<InputData *>(inputList->objectForKey(keyCode));
#endif
		p->update();
	}
}

float InputGamepadData::InputData::StickThreshold = 0.5f;
bool InputGamepadData::InputData::init(EnumType type)
{
	_type = type;
	_press = false;
	_release = false;
	_trigger = false;
	_value = 0.0f;
	_oldValue = 0.0f;
	_minValue = 0.0f;
	_maxValue = 0.0f;
	_updateTrigger = true;
	_updateRelease = true;
	_firstSetValueFlag = false;
	return true;
}

void InputGamepadData::InputData::reset()
{
	_press = false;
	_release = false;
	_trigger = false;
	_value = 0.0f;
	_oldValue = 0.0f;
	_minValue = 0.0f;
	_maxValue = 0.0f;
	_updateTrigger = true;
	_updateRelease = true;
	_firstSetValueFlag = false;
}

void InputGamepadData::InputData::update()
{
	//trigger
	if (_updateTrigger && _trigger) {
		_trigger = false;
	}
	_updateTrigger = true;
	//release
	if (_updateRelease && _release) {
		_release = false;
	}
	_updateRelease = true;
}

void InputGamepadData::InputData::setTrigger(bool trigger)
{
	_updateTrigger = false;
	_trigger = trigger;
}

void InputGamepadData::InputData::setRelease(bool release)
{
	_updateRelease = false;
	_release = release;
}

void InputGamepadData::InputData::setValue(float value)
{
	if (_firstSetValueFlag == false) {
		_oldValue = value;
		_value = value;
		_firstSetValueFlag = true;
	}
	else {
		_oldValue = _value;
		_value = value;
	}

	if (this->isChanged()) {
		if (std::abs(value) > StickThreshold) {
			this->setPress(true);
			this->setTrigger(true);
		}
		else {
			this->setRelease(true);
			this->setPress(false);
		}
	}
}

void InputGamepadData::InputData::setValue(float value, float minValue, float maxValue)
{
	if (_firstSetValueFlag == false) {
		_oldValue = value;
		_value = value;
		_firstSetValueFlag = true;
	}
	else {
		_oldValue = _value;
		_value = value;
	}

	//valueの最大・最小値。
	_minValue = minValue;
	_maxValue = maxValue;

	if (this->isChanged()) {
		if ((minValue + StickThreshold < value) && this->getPress() == false) {
			this->setPress(true);
			this->setTrigger(true);
		}
		else if (minValue + StickThreshold >= value && this->getPress() == true) {
			this->setRelease(true);
			this->setPress(false);
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------
InputDataRaw::InputDataRaw()
{
	_mouse = nullptr;
	_gamepadList = nullptr;
	_keyboard = nullptr;
#ifdef USE_PREVIEW
	_recording = false;
	_playing = false;
    _recordFp = nullptr;
    _playBuf.clear();
    _playHead = 0;
    _playFrameOffset = 0;
    _recordedFrameDeviceDataList.clear();
    _gamePadAxisDic = new cocos2d::__Dictionary *[MAX_GAMEPAD];
	for (int i = 0; i < MAX_GAMEPAD; i++) {
        _gamePadAxisDic[i] = cocos2d::__Dictionary::create();
        _gamePadAxisDic[i]->retain();
    }
#endif
    _recordedDeviceDataList.clear();
}

InputDataRaw::~InputDataRaw()
{
	CC_SAFE_RELEASE_NULL(_mouse);
	CC_SAFE_RELEASE_NULL(_gamepadList);
	CC_SAFE_RELEASE_NULL(_keyboard);
#ifdef USE_PREVIEW
    if(_recordFp){
        fclose(_recordFp);
        _recordFp = nullptr;
    }
    _playBuf.clear();
	for (int i = 0; i < MAX_GAMEPAD; i++) {
        CC_SAFE_RELEASE_NULL(_gamePadAxisDic[i]);
    }
    delete[] _gamePadAxisDic;
    _gamePadAxisDic = nullptr;
#endif
}

bool InputDataRaw::init()
{
	//mouse
	auto mouse = InputMouseData::create();
	if (mouse == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setMouse(mouse);
	//gamepad
	auto gamepadList = cocos2d::__Dictionary::create();
	if (gamepadList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	for (int i = 0; i < MAX_GAMEPAD; i++) {
		auto gamepad = InputGamepadData::create();
		if (gamepad == nullptr) {
			return false;
		}
		gamepadList->setObject(gamepad, i);
	}
	this->setGamepadList(gamepadList);
	//keyboard
	auto keyboard = InputKeyboardData::create();
	if (keyboard == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setKeyboard(keyboard);
	return true;
}

void InputDataRaw::reset(bool isReset/*=false*/)
{
	if (_mouse) {
		this->getMouse()->reset();
	}
	if (_keyboard) {
		this->getKeyboard()->reset(isReset);
	}
	for (int i = 0; i < MAX_GAMEPAD; i++) {
		auto gamepad = this->getGamepad(i);
		if (gamepad) {
			gamepad->reset(isReset);
		}
#ifdef USE_PREVIEW
        if(_gamePadAxisDic[i]){
			_gamePadAxisDic[i]->removeAllObjects();
        }
#endif
	}
}

void InputDataRaw::connected(const DeviceData &deviceData)
{
	auto gamepad = this->getGamepad(deviceData.deviceId);
	if (gamepad->getConnected()) {
		return;
	}
	gamepad->setNameWithChar(deviceData.deviceName.c_str());
	gamepad->setConnected(true);
	gamepad->setup(gamepad->getGamepadType());
}

void InputDataRaw::disconnected(const DeviceData &deviceData)
{
	auto gamepad = this->getGamepad(deviceData.deviceId);
	if (gamepad->getConnected()) {
		CCLOG("device(%d):%s", deviceData.deviceId, gamepad->getName(), deviceData.deviceName.c_str());
		// ACT2-6327 コントローラー再接続で切断前の入力状態がリセットされないためここでリセット
		gamepad->reset(true);
	}
}

void InputDataRaw::update()
{
	for (int i = 0; i < InputDataRaw::MAX_GAMEPAD; i++) {
		auto gamepad = this->getGamepad(i);
		gamepad->update();
	}
	this->getKeyboard()->update();
	this->getMouse()->update();
}

InputGamepadData *InputDataRaw::getGamepad(int padId)
{
	return dynamic_cast<InputGamepadData *>(this->getGamepadList()->objectForKey(padId));
}

static short sKeyCodeMapping[] = {
	(short)EventKeyboard::KeyCode::KEY_NONE, //NONE
	(short)EventKeyboard::KeyCode::KEY_PAUSE,    //PAUSE
	(short)EventKeyboard::KeyCode::KEY_SCROLL_LOCK,  //SCROLL_LOCK
	(short)EventKeyboard::KeyCode::KEY_PRINT,    //PRINT
	(short)EventKeyboard::KeyCode::KEY_SYSREQ,   //SYSREQ
	(short)EventKeyboard::KeyCode::KEY_BREAK,    //BREAK
	(short)EventKeyboard::KeyCode::KEY_ESCAPE,   //ESCAPE
	(short)EventKeyboard::KeyCode::KEY_BACKSPACE,    //BACKSPACE
	(short)EventKeyboard::KeyCode::KEY_TAB,  //TAB
	(short)EventKeyboard::KeyCode::KEY_BACK_TAB, //BACK_TAB
	(short)EventKeyboard::KeyCode::KEY_RETURN,   //RETURN
	-1,    //CAPS_LOCK
	(short)EventKeyboard::KeyCode::KEY_LEFT_SHIFT,   //LEFT_SHIFT
	(short)EventKeyboard::KeyCode::KEY_LEFT_SHIFT,  //RIGHT_SHIFT
	(short)EventKeyboard::KeyCode::KEY_LEFT_CTRL,    //LEFT_CTRL
	(short)EventKeyboard::KeyCode::KEY_LEFT_CTRL,   //RIGHT_CTRL
	(short)EventKeyboard::KeyCode::KEY_LEFT_ALT, //LEFT_ALT
	(short)EventKeyboard::KeyCode::KEY_LEFT_ALT,    //RIGHT_ALT
	(short)EventKeyboard::KeyCode::KEY_MENU, //MENU
	-1,    //HYPER
	(short)EventKeyboard::KeyCode::KEY_INSERT,   //INSERT
	(short)EventKeyboard::KeyCode::KEY_HOME, //HOME
	(short)EventKeyboard::KeyCode::KEY_PG_UP,    //PG_UP
	(short)EventKeyboard::KeyCode::KEY_DELETE,   //DELETE
	(short)EventKeyboard::KeyCode::KEY_END,  //END
	(short)EventKeyboard::KeyCode::KEY_PG_DOWN,  //PG_DOWN
	(short)EventKeyboard::KeyCode::KEY_LEFT_ARROW,   //LEFT_ARROW"
	(short)EventKeyboard::KeyCode::KEY_RIGHT_ARROW,  //RIGHT_ARROW
	(short)EventKeyboard::KeyCode::KEY_UP_ARROW, //UP_ARROW
	(short)EventKeyboard::KeyCode::KEY_DOWN_ARROW,   //DOWN_ARROW
	(short)EventKeyboard::KeyCode::KEY_NUM_LOCK, //NUM_LOCK
	(short)EventKeyboard::KeyCode::KEY_PLUS,  //KP_PLUS
	(short)EventKeyboard::KeyCode::KEY_MINUS, //KP_MINUS
	(short)EventKeyboard::KeyCode::KEY_ASTERISK,  //KP_MULTIPLY
	(short)EventKeyboard::KeyCode::KEY_SLASH,    //KP_DIVIDE
	(short)EventKeyboard::KeyCode::KEY_KP_ENTER, //KP_ENTER
	(short)EventKeyboard::KeyCode::KEY_HOME,  //KP_HOME
	(short)EventKeyboard::KeyCode::KEY_KP_UP,    //KP_UP
	(short)EventKeyboard::KeyCode::KEY_PG_UP, //KP_PG_UP
	(short)EventKeyboard::KeyCode::KEY_LEFT_ARROW,  //KP_LEFT
	(short)EventKeyboard::KeyCode::KEY_5,  //KP_FIVE
	(short)EventKeyboard::KeyCode::KEY_RIGHT_ARROW, //KP_RIGHT
	(short)EventKeyboard::KeyCode::KEY_END,   //KP_END
	(short)EventKeyboard::KeyCode::KEY_DOWN_ARROW,  //KP_DOWN
	(short)EventKeyboard::KeyCode::KEY_PG_DOWN,   //KP_PG_DOWN
	(short)EventKeyboard::KeyCode::KEY_INSERT,    //KP_INSERT
	(short)EventKeyboard::KeyCode::KEY_DELETE,    //KP_DELETE
	(short)EventKeyboard::KeyCode::KEY_F1,   //F1
	(short)EventKeyboard::KeyCode::KEY_F2,   //F2
	(short)EventKeyboard::KeyCode::KEY_F3,   //F3
	(short)EventKeyboard::KeyCode::KEY_F4,   //F4
	(short)EventKeyboard::KeyCode::KEY_F5,   //F5
	(short)EventKeyboard::KeyCode::KEY_F6,   //F6
	(short)EventKeyboard::KeyCode::KEY_F7,   //F7
	(short)EventKeyboard::KeyCode::KEY_F8,   //F8
	(short)EventKeyboard::KeyCode::KEY_F9,   //F9
	(short)EventKeyboard::KeyCode::KEY_F10,  //F10
	(short)EventKeyboard::KeyCode::KEY_F11,  //F11
	(short)EventKeyboard::KeyCode::KEY_F12,  //F12
	(short)EventKeyboard::KeyCode::KEY_SPACE,    //SPACE
	(short)EventKeyboard::KeyCode::KEY_EXCLAM,   //EXCLAM
	(short)EventKeyboard::KeyCode::KEY_QUOTE,    //QUOTE
	(short)EventKeyboard::KeyCode::KEY_NUMBER,   //NUMBER
	(short)EventKeyboard::KeyCode::KEY_DOLLAR,   //DOLLAR
	(short)EventKeyboard::KeyCode::KEY_PERCENT,  //PERCENT
	(short)EventKeyboard::KeyCode::KEY_CIRCUMFLEX,   //CIRCUMFLEX
	(short)EventKeyboard::KeyCode::KEY_AMPERSAND,    //AMPERSAND
	(short)EventKeyboard::KeyCode::KEY_APOSTROPHE,   //APOSTROPHE
	(short)EventKeyboard::KeyCode::KEY_LEFT_PARENTHESIS, //LEFT_PARENTHESIS
	(short)EventKeyboard::KeyCode::KEY_RIGHT_PARENTHESIS,    //RIGHT_PARENTHESIS
	(short)EventKeyboard::KeyCode::KEY_ASTERISK, //ASTERISK
	(short)EventKeyboard::KeyCode::KEY_PLUS, //PLUS
	(short)EventKeyboard::KeyCode::KEY_COMMA,    //COMMA
	(short)EventKeyboard::KeyCode::KEY_MINUS,    //MINUS
	(short)EventKeyboard::KeyCode::KEY_PERIOD,   //PERIOD
	(short)EventKeyboard::KeyCode::KEY_SLASH,    //SLASH
	(short)EventKeyboard::KeyCode::KEY_0,    //0
	(short)EventKeyboard::KeyCode::KEY_1,    //1
	(short)EventKeyboard::KeyCode::KEY_2,    //2
	(short)EventKeyboard::KeyCode::KEY_3,    //3
	(short)EventKeyboard::KeyCode::KEY_4,    //4
	(short)EventKeyboard::KeyCode::KEY_5,    //5
	(short)EventKeyboard::KeyCode::KEY_6,    //6
	(short)EventKeyboard::KeyCode::KEY_7,    //7
	(short)EventKeyboard::KeyCode::KEY_8,    //8
	(short)EventKeyboard::KeyCode::KEY_9,    //9
	(short)EventKeyboard::KeyCode::KEY_COLON,    //COLON
	(short)EventKeyboard::KeyCode::KEY_SEMICOLON,    //SEMICOLON
	(short)EventKeyboard::KeyCode::KEY_LESS_THAN,    //LESS_THAN
	(short)EventKeyboard::KeyCode::KEY_EQUAL,    //EQUAL
	(short)EventKeyboard::KeyCode::KEY_GREATER_THAN, //GREATER_THAN
	(short)EventKeyboard::KeyCode::KEY_QUESTION, //QUESTION
	(short)EventKeyboard::KeyCode::KEY_AT,   //AT
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_A,    //CAPITAL_A
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_B,    //CAPITAL_B
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_C,    //CAPITAL_C
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_D,    //CAPITAL_D
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_E,    //CAPITAL_E
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_F,    //CAPITAL_F
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_G,    //CAPITAL_G
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_H,    //CAPITAL_H
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_I,    //CAPITAL_I
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_J,    //CAPITAL_J
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_K,    //CAPITAL_K
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_L,    //CAPITAL_L
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_M,    //CAPITAL_M
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_N,    //CAPITAL_N
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_O,    //CAPITAL_O
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_P,    //CAPITAL_P
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_Q,    //CAPITAL_Q
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_R,    //CAPITAL_R
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_S,    //CAPITAL_S
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_T,    //CAPITAL_T
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_U,    //CAPITAL_U
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_V,    //CAPITAL_V
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_W,    //CAPITAL_W
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_X,    //CAPITAL_X
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_Y,    //CAPITAL_Y
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_Z,    //CAPITAL_Z
	(short)EventKeyboard::KeyCode::KEY_LEFT_BRACKET, //LEFT_BRACKET
	(short)EventKeyboard::KeyCode::KEY_BACK_SLASH,   //BACK_SLASH
	(short)EventKeyboard::KeyCode::KEY_RIGHT_BRACKET,    //RIGHT_BRACKET
	(short)EventKeyboard::KeyCode::KEY_UNDERSCORE,   //UNDERSCORE
	-1,    //GRAVE
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_A,    //A
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_B,    //B
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_C,    //C
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_D,    //D
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_E,    //E
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_F,    //F
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_G,    //G
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_H,    //H
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_I,    //I
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_J,    //J
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_K,    //K
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_L,    //L
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_M,    //M
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_N,    //N
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_O,    //O
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_P,    //P
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_Q,    //Q
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_R,    //R
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_S,    //S
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_T,    //T
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_U,    //U
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_V,    //V
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_W,    //W
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_X,    //X
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_Y,    //Y
	(short)EventKeyboard::KeyCode::KEY_CAPITAL_Z,    //Z
	(short)EventKeyboard::KeyCode::KEY_LEFT_BRACE,   //LEFT_BRACE
	(short)EventKeyboard::KeyCode::KEY_BAR,  //BAR
	(short)EventKeyboard::KeyCode::KEY_RIGHT_BRACE,  //RIGHT_BRACE
	(short)EventKeyboard::KeyCode::KEY_TILDE,    //TILDE
	-1, //EURO
	-1,    //POUND
	(short)EventKeyboard::KeyCode::KEY_YEN,  //YEN
	(short)EventKeyboard::KeyCode::KEY_MIDDLE_DOT,   //MIDDLE_DOT
	(short)EventKeyboard::KeyCode::KEY_SEARCH,   //SEARCH
	-1,    //DPAD_LEFT
	-1,   //DPAD_RIGHT
	-1,  //DPAD_UP
	-1,    //DPAD_DOWN
	-1,  //DPAD_CENTER
	(short)EventKeyboard::KeyCode::KEY_RETURN,    //ENTER
	(short)EventKeyboard::KeyCode::KEY_PLAY, //PLAY
};

void InputDataRaw::applyRegisteredData()
{
    for(auto deviceData: _recordedDeviceDataList){
        switch(deviceData.inputType){
        case DeviceData::kConnected:
            connected(deviceData);
            break;
        case DeviceData::kDisconnected:
            disconnected(deviceData);
            break;
        case DeviceData::kButtonDown:{
            auto gamepad = getGamepad(deviceData.deviceId);
            if (!gamepad->getConnected()) {
                connected(deviceData);
            }
            auto input = gamepad->getInput(deviceData.keyCode);
            if (input) {
                input->setTrigger(true);
                input->setPress(true);
				input->setValue(1);
                CCLOG("keyCode:%d", deviceData.keyCode);
            }
            break;}
        case DeviceData::kButtonUp:{
            auto gamepad = getGamepad(deviceData.deviceId);
            if (!gamepad->getConnected()) {
                connected(deviceData);
            }
            auto input = gamepad->getInput(deviceData.keyCode);
            if (input) {
                input->setPress(false);
                input->setRelease(true);
				input->setValue(0);
            }
            break;}
        case DeviceData::kButtonRepeat:{
            auto gamepad = getGamepad(deviceData.deviceId);
            if (!gamepad->getConnected()) {
                connected(deviceData);
            }
            auto input = gamepad->getInput(deviceData.keyCode);
            if (input) {
                input->setPress(true);
            }
            break;}
        case DeviceData::kAxis:{
            auto gamepad = getGamepad(deviceData.deviceId);
            if (!gamepad->getConnected()) {
                break;
            }
            bool bTriggerType = false;
            auto value = deviceData.axisValue;
            auto keyCode = deviceData.keyCode;
            //value = abs(value) > 0.05f ? value : 0.0f;
            switch (gamepad->getGamepadType()) {
            case InputGamepadData::kGamepadXbox360: {
                auto input = gamepad->getInput(InputGamepadData::kInputAxis1 + keyCode);
                if (input != nullptr) {
                    //Left:4, Right:5
                    if (keyCode == 4 || keyCode == 5) {
                        bTriggerType = true;
                    }
                    if (bTriggerType) {
                        input->setValue(value, -1.0f, 1.0f);
                    }
                    else {
						//CCLOG("value:%f, keyCode:%d", value, keyCode);
                        input->setValue(value);
                    }
                }
                break; }
            case InputGamepadData::kGamepadPlayStation4: {
                auto input = gamepad->getInput(InputGamepadData::kInputAxis1 + keyCode);
                if (input != nullptr) {
                    //Left:3, Right:4
                    if (keyCode == 3 || keyCode == 4) {
                        bTriggerType = true;
                    }
                    if (bTriggerType) {
                        input->setValue(value, -1.0f, 1.0f);
                    }
                    else {
                        input->setValue(value);
                    }
                }
                break; }
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
            case InputGamepadData::kGamepadDirectInput: {
				// ACT2-5845
				// ACT2-6361
				auto input = gamepad->getInput(InputGamepadData::kInputAxis1Minus + keyCode * 2 + 0);
				if (input != nullptr) input->setValue((value < 0) ? abs(value) : 0);
				input = gamepad->getInput(InputGamepadData::kInputAxis1Minus + keyCode * 2 + 1);
				if (input != nullptr) input->setValue((value > 0) ? abs(value) : 0);
                break; }
            default:
                CC_ASSERT(0);
                break;
            }
            break;}
        case DeviceData::kMouseDown:{
            auto inputData = _mouse->getInputData(deviceData.mouseButton);
            if (inputData) {
                inputData->setTrigger(true);
                inputData->setPress(true);
            }
            _mouse->setMovePoint(cocos2d::Vec2(deviceData.cursorX, deviceData.cursorY));
            _mouse->setStartPoint(cocos2d::Vec2(deviceData.cursorX, deviceData.cursorY));
            break;}
        case DeviceData::kMouseMove:{
            _mouse->setPoint(cocos2d::Vec2(deviceData.cursorX, deviceData.cursorY));
            _mouse->setMove(true);
            if (_mouse->isPressOr() == false) {
                break;
            }
            _mouse->setMovePoint(cocos2d::Vec2(deviceData.cursorX, deviceData.cursorY));
            break;}
        case DeviceData::kMouseUp:{
            auto inputData = _mouse->getInputData(deviceData.mouseButton);
            if (inputData) {
                inputData->setRelease(true);
                inputData->setPress(false);
            }
            _mouse->setMovePoint(cocos2d::Vec2(deviceData.cursorX, deviceData.cursorY));
            //mouse->setMove(false);
            break;}
        case DeviceData::kMouseScroll:
            _mouse->setScrollPoint(cocos2d::Vec2(deviceData.scrollX, deviceData.scrollY));
            break;
        case DeviceData::kKeyPressed:{
            auto keyCode = deviceData.keyCode;
            if (keyCode < CC_ARRAYSIZE(sKeyCodeMapping)) {
                keyCode = sKeyCodeMapping[keyCode];
            } else {
                break;
            }
            if (keyCode < 0) break;
            _keyboard->setPressData(keyCode, deviceData.scancode);
            break;}
        case DeviceData::kKeyReleased:{
            auto keyCode = deviceData.keyCode;
            if (keyCode < CC_ARRAYSIZE(sKeyCodeMapping)) {
                keyCode = sKeyCodeMapping[keyCode];
            } else {
                break;
            }
            if (keyCode < 0) break;
            _keyboard->setReleaseData(keyCode, deviceData.scancode);
            break;}
		case DeviceData::kKeyRepeat: {
			auto keyCode = deviceData.keyCode;
			if (keyCode < CC_ARRAYSIZE(sKeyCodeMapping)) {
				keyCode = sKeyCodeMapping[keyCode];
			}
			else {
				break;
			}
			if (keyCode < 0) break;
			_keyboard->setRepeatData(keyCode, deviceData.scancode);
			break; }
        case DeviceData::kCharInput:
            _keyboard->setCharInputData(deviceData.keyCode);
            break;
        case DeviceData::kRandomSeed:
            srand((unsigned int)deviceData.keyCode);
            break;
		case DeviceData::kRestartProjectData:
			if (deviceData.keyCode > 0) {
				auto gm = GameManager::getInstance();
				//F5リセット
				gm->restartProjectData();
#ifdef USE_PREVIEW
				gm->setAutoTestRestartFlg(false);
#endif
			}
			break;
        default: CC_ASSERT(0);
        }
    }
    _recordedDeviceDataList.clear();
}

void InputDataRaw::registerConnected(int deviceId, const char *deviceName)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kConnected;
    deviceData.deviceId = deviceId;
    deviceData.deviceName = std::string(deviceName);
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerDisconnected(int deviceId, const char *deviceName)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kDisconnected;
    deviceData.deviceId = deviceId;
    deviceData.deviceName = std::string(deviceName);
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerButtonDown(int deviceId, const char *deviceName, int keyCode)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
	if (GameManager::getInstance()->isSceneChanging()) {
		return;
	}
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kButtonDown;
    deviceData.deviceId = deviceId;
    deviceData.deviceName = std::string(deviceName);
    deviceData.keyCode = keyCode;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerButtonUp(int deviceId, const char *deviceName, int keyCode)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kButtonUp;
    deviceData.deviceId = deviceId;
    deviceData.deviceName = std::string(deviceName);
    deviceData.keyCode = keyCode;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerButtonRepeat(int deviceId, const char *deviceName, int keyCode)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kButtonRepeat;
    deviceData.deviceId = deviceId;
    deviceData.deviceName = std::string(deviceName);
    deviceData.keyCode = keyCode;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerAxis(int deviceId, int keyCode, float value)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
    auto dic = _gamePadAxisDic[deviceId];
	auto obj = dic->objectForKey(keyCode);
	if (obj == nullptr) {
		dic->setObject(cocos2d::Float::create(value), keyCode);
	}
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kAxis;
    deviceData.deviceId = deviceId;
    deviceData.keyCode = keyCode;
    deviceData.axisValue = value;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerMouseDown(int button, int x, int y)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kMouseDown;
    deviceData.mouseButton = button;
    deviceData.cursorX = x;
    deviceData.cursorY = y;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerMouseMove(int x, int y)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kMouseMove;
    deviceData.cursorX = x;
    deviceData.cursorY = y;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerMouseUp(int button, int x, int y)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kMouseUp;
    deviceData.mouseButton = button;
    deviceData.cursorX = x;
    deviceData.cursorY = y;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerMouseScroll(int x, int y)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kMouseScroll;
    deviceData.scrollX = x;
    deviceData.scrollY = y;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerKeyPressed(int keyCode, int scancode)
{
#ifdef USE_PREVIEW
    if(_playing){
        if(keyCode == (int)EventKeyboard::KeyCode::KEY_ESCAPE){
            CCLOG("Playing cancelled");
            _playBuf.clear();
            _playHead = 0;
            _playing = false;
        }
        return;
    }
	if (GameManager::getInstance()->isSceneChanging()) {
		return;
	}
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kKeyPressed;
    deviceData.keyCode = keyCode;
    deviceData.scancode = scancode;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerKeyReleased(int keyCode, int scancode)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kKeyReleased;
    deviceData.keyCode = keyCode;
    deviceData.scancode = scancode;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerKeyRepeat(int keyCode, int scancode)
{
#ifdef USE_PREVIEW
	if (_playing) {
		return;
	}
	if (!GameManager::getInstance()->isSceneChanging()) {
		return;
	}
#endif
	DeviceData deviceData;
	deviceData.inputType = DeviceData::kKeyRepeat;
	deviceData.keyCode = keyCode;
	deviceData.scancode = scancode;
	_recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerCharInput(int keyCode)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kCharInput;
    deviceData.keyCode = keyCode;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerRandomSeed(int seed)
{
#ifdef USE_PREVIEW
    if(_playing){
        return;
    }
#endif
    DeviceData deviceData;
    deviceData.inputType = DeviceData::kRandomSeed;
    deviceData.keyCode = seed;
    _recordedDeviceDataList.push_back(deviceData);
}

void InputDataRaw::registerRestart(bool restart)
{
#ifdef USE_PREVIEW
	if (_playing) {
		return;
	}
#endif
	DeviceData deviceData;
	deviceData.inputType = DeviceData::kRestartProjectData;
	if (restart) {
		deviceData.keyCode = 1;
	}
	else {
		deviceData.keyCode = 0;
	}
	_recordedDeviceDataList.push_back(deviceData);
}

#ifdef USE_PREVIEW
bool InputDataRaw::startRecording()
{
    if(_playing){
        return false;
    }
    if(_recording){
        return true;
    }
	for (int i = 0; i < MAX_GAMEPAD; i++) {
        if(_gamePadAxisDic[i]){
			_gamePadAxisDic[i]->removeAllObjects();
        }
	}
	auto fileUtils = FileUtils::getInstance();
	char buf[256];
	time_t now = time(nullptr);
	struct tm *p = localtime(&now);
	snprintf(buf, sizeof(buf), "play-%04d%02d%02d-%02d%02d%02d.pgminput", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    _recordFp = fopen(fileUtils->getSuitableFOpen(fileUtils->getApplicationPath() + buf).c_str(), "wb");
    if(!_recordFp){
        return false;
    }
    CCLOG("InputDataRaw::startRecording()");
    _recording = true;
    auto seed = rand();
    //srand((unsigned int)seed);	// applyRegisteredData()でsrand行うのでここではしない
    registerRandomSeed(seed);
	bool restart = GameManager::getInstance()->getAutoTestRestartFlg();
	if (restart) {
		registerRestart(restart);
	}
    return true;
}

void InputDataRaw::stopRecording()
{
    if(!_recording){
        return;
    }
    CCLOG("InputDataRaw::stopRecording()");
    fclose(_recordFp);
    _recordFp = nullptr;
    _recording = false;
}

bool InputDataRaw::isRecording()
{
    return _recording;
}

bool InputDataRaw::startPlaying(const char *filename)
{
    if(_recording){
        return false;
    }
    if(_playing){
        _playBuf.clear();
        _playHead = 0;
        _playing = false;
    }
    _playBuf = FileUtils::getInstance()->getStringFromFile(filename);
    if(_playBuf.size() == 0){
        CCLOG("InputDataRaw::startPlaying: Failed to read: %s", filename);
        return false;
    }
    CCLOG("InputDataRaw::startPlaying(%s)", filename);
    _playHead = 0;
    _playFrameOffset = GameManager::getInstance()->getFrame() - atoi(&_playBuf[0]);
    _playing = true;
	return true;
}

bool InputDataRaw::isReplaying()
{
    return _playing;
}

static const char *sInputTypeNameList[] = {
    "connected",
    "disconnected",
    "buttonDown",
    "buttonUp",
    "buttonRepeat",
    "axis",
    "mouseDown",
    "mouseMove",
    "mouseUp",
    "mouseScroll",
    "keyPressed",
    "keyReleased",
	"keyRepeat",
	"charInput",
    "randomSeed",
	"restartProjectData",
};
void InputDataRaw::recordInput()
{
    CC_ASSERT(CC_ARRAYSIZE(sInputTypeNameList) == DeviceData::kInputTypeMax);
    if(_recordedDeviceDataList.size() == 0){
        return;
    }
    fprintf(_recordFp, "%d:[", GameManager::getInstance()->getFrame());
    bool first = true;
    for(auto &deviceData: _recordedDeviceDataList){
        if(first){
            first = false;
        } else {
            fprintf(_recordFp, ",");
        }
        switch(deviceData.inputType){
        case DeviceData::kConnected:
        case DeviceData::kDisconnected:
            fprintf(_recordFp, "{\"type\":\"%s\",\"deviceId\":%d,\"deviceName\":\"%s\"}", sInputTypeNameList[deviceData.inputType], deviceData.deviceId, deviceData.deviceName.c_str());
            break;
        case DeviceData::kButtonDown:
        case DeviceData::kButtonUp:
        case DeviceData::kButtonRepeat:
            fprintf(_recordFp, "{\"type\":\"%s\",\"deviceId\":%d,\"deviceName\":\"%s\",\"keyCode\":%d}", sInputTypeNameList[deviceData.inputType], deviceData.deviceId, deviceData.deviceName.c_str(), deviceData.keyCode);
            break;
        case DeviceData::kAxis:
            fprintf(_recordFp, "{\"type\":\"%s\",\"deviceId\":%d,\"keyCode\":%d,\"axisValue\":%f}", sInputTypeNameList[deviceData.inputType], deviceData.deviceId, deviceData.keyCode, deviceData.axisValue);
            break;
        case DeviceData::kMouseDown:
        case DeviceData::kMouseUp:
            fprintf(_recordFp, "{\"type\":\"%s\",\"mouseButton\":%d,\"cursorX\":%d,\"cursorY\":%d}", sInputTypeNameList[deviceData.inputType], deviceData.mouseButton, deviceData.cursorX, deviceData.cursorY);
            break;
        case DeviceData::kMouseMove:
            fprintf(_recordFp, "{\"type\":\"%s\",\"cursorX\":%d,\"cursorY\":%d}", sInputTypeNameList[deviceData.inputType], deviceData.cursorX, deviceData.cursorY);
            break;
        case DeviceData::kMouseScroll:
            fprintf(_recordFp, "{\"type\":\"%s\",\"scrollX\":%d,\"scrollY\":%d}", sInputTypeNameList[deviceData.inputType], deviceData.scrollX, deviceData.scrollY);
            break;
        case DeviceData::kKeyPressed:
        case DeviceData::kKeyReleased:
		case DeviceData::kKeyRepeat:
            fprintf(_recordFp, "{\"type\":\"%s\",\"keyCode\":%d,\"scancode\":%d}", sInputTypeNameList[deviceData.inputType], deviceData.keyCode, deviceData.scancode);
            break;
        case DeviceData::kCharInput:
            fprintf(_recordFp, "{\"type\":\"%s\",\"keyCode\":%d}", sInputTypeNameList[deviceData.inputType], deviceData.keyCode);
            break;
        case DeviceData::kRandomSeed:
            fprintf(_recordFp, "{\"type\":\"%s\",\"seed\":%d}", sInputTypeNameList[deviceData.inputType], deviceData.keyCode);
            break;
		case DeviceData::kRestartProjectData:
			fprintf(_recordFp, "{\"type\":\"%s\",\"restart\":%d}", sInputTypeNameList[deviceData.inputType], deviceData.keyCode);
			break;
        default:CC_ASSERT(0);
        }
    }
    fprintf(_recordFp, "]\n");
    fflush(_recordFp);
}

void InputDataRaw::applyPlayInput()
{
    if(_playBuf[_playHead] == '\0'){
        CCLOG("InputDataRaw: End of playing");
        _playBuf.clear();
        _playing = false;
        return;
    }
	while (true) {
		char *p = &_playBuf[_playHead];
		if (*p == '\0'){
			break;
		}
        int frame = GameManager::getInstance()->getFrame() - atoi(p) - _playFrameOffset;
        CCLOG("applyPlayInput: %d(%d + %d - %d)", frame, GameManager::getInstance()->getFrame(), atoi(p), _playFrameOffset);
        if(frame < 0){
            //この入力はまだ適用しない。
            return;
        }
        while(*p != '\n' && *p != '\0'){
            p++;
        }
        *p = '\0';
		auto newHead = p + 1 - &_playBuf[0];
        rapidjson::Document doc;
        p = &_playBuf[_playHead];
        _playHead = newHead;
        CCLOG("applyPlayInput: %s", p);
		p = strchr(p, ':');
		if (!p) {
			CCLOG("pplyPlayInput: Colon not found");
			_playing = false;
			return;
		}
		p++;
        doc.Parse(p);
        bool error = doc.HasParseError();
        if (error) {
            CCLOG("InputDataRaw::applyPlayInput: Json Parse error: %s", p);
            return;
        }
        _recordedDeviceDataList.clear();
        for(rapidjson::SizeType i = 0; i < doc.Size(); i++){
            DeviceData deviceData;
            auto typeStr = doc[i]["type"].GetString();
            deviceData.inputType = DeviceData::kInputTypeMax;
            for(int j = 0; j < CC_ARRAYSIZE(sInputTypeNameList); j++){
                if(strcmp(typeStr, sInputTypeNameList[j]) == 0){
                    deviceData.inputType = (DeviceData::EnumInputType)j;
                    break;
                }
            }
            if(deviceData.inputType == DeviceData::kInputTypeMax){
                CCLOG("InputDataRaw::applyPlayInput: Unknown type: %s", typeStr);
                continue;
            }
            switch(deviceData.inputType){
            case DeviceData::kConnected:
            case DeviceData::kDisconnected:
                deviceData.deviceId = doc[i]["deviceId"].GetInt();
                deviceData.deviceName = std::string(doc[i]["deviceName"].GetString());
                break;
            case DeviceData::kButtonDown:
            case DeviceData::kButtonUp:
            case DeviceData::kButtonRepeat:
                deviceData.deviceId = doc[i]["deviceId"].GetInt();
                deviceData.deviceName = std::string(doc[i]["deviceName"].GetString());
                deviceData.keyCode = doc[i]["keyCode"].GetInt();
                break;
            case DeviceData::kAxis:
                deviceData.deviceId = doc[i]["deviceId"].GetInt();
                //deviceData.deviceName = std::string(doc[i]["deviceName"].GetString());
                deviceData.keyCode = doc[i]["keyCode"].GetInt();
                deviceData.axisValue = doc[i]["axisValue"].GetFloat();
                break;
            case DeviceData::kMouseDown:
            case DeviceData::kMouseUp:
                deviceData.mouseButton = doc[i]["mouseButton"].GetInt();
                deviceData.cursorX = doc[i]["cursorX"].GetInt();
                deviceData.cursorY = doc[i]["cursorY"].GetInt();
                break;
            case DeviceData::kMouseMove:
                deviceData.cursorX = doc[i]["cursorX"].GetInt();
                deviceData.cursorY = doc[i]["cursorY"].GetInt();
                break;
            case DeviceData::kMouseScroll:
                deviceData.scrollX = doc[i]["scrollX"].GetInt();
                deviceData.scrollY = doc[i]["scrollY"].GetInt();
                break;
            case DeviceData::kKeyPressed:
            case DeviceData::kKeyReleased:
			case DeviceData::kKeyRepeat:
                deviceData.keyCode = doc[i]["keyCode"].GetInt();
                deviceData.scancode = doc[i]["scancode"].GetInt();
                break;
            case DeviceData::kCharInput:
                deviceData.keyCode = doc[i]["keyCode"].GetInt();
                break;
            case DeviceData::kRandomSeed:
                deviceData.keyCode = doc[i]["seed"].GetInt();
                break;
			case DeviceData::kRestartProjectData:
				deviceData.keyCode = doc[i]["restart"].GetInt();
				break;
            default:CC_ASSERT(0);
            }
            _recordedDeviceDataList.push_back(deviceData);
        }
        applyRegisteredData();
    }
}

// リプレイデータからリセット開始フラグを取得
bool InputDataRaw::getRestartFlg(const char *filename)
{
	bool ret = false;

	std::string _playBuf;
	int _playHead = 0;

	_playBuf = FileUtils::getInstance()->getStringFromFile(filename);
	if (_playBuf.size() == 0) {
		CCLOG("InputDataRaw::getRestartFlg: Failed to read: %s", filename);
		return ret;
	}

	//-----------------------
	if (_playBuf[_playHead] == '\0') {
		CCLOG("InputDataRaw: End of getRestartFlg");
		return ret;
	}
	while (true) {
		char *p = &_playBuf[_playHead];
		if (*p == '\0') {
			break;
		}

		while (*p != '\n' && *p != '\0') {
			p++;
		}
		*p = '\0';
		auto newHead = p + 1 - &_playBuf[0];
		rapidjson::Document doc;
		p = &_playBuf[_playHead];
		_playHead = newHead;
		CCLOG("InputDataRaw::getRestartFlg: %s", p);
		p = strchr(p, ':');
		if (!p) {
			CCLOG("InputDataRaw::getRestartFlg: Colon not found");
			return ret;
		}
		p++;
		doc.Parse(p);
		bool error = doc.HasParseError();
		if (error) {
			CCLOG("InputDataRaw::getRestartFlg: Json Parse error: %s", p);
			return ret;
		}

		for (rapidjson::SizeType i = 0; i < doc.Size(); i++) {
			auto typeStr = doc[i]["type"].GetString();
			if (strcmp(typeStr, sInputTypeNameList[14]) == 0) {	// 14:"restartProjectData"
				if (doc[i].HasMember("restart") && doc[i]["restart"].GetInt() > 0) {
					ret = true;
				}
				return ret;
			}
		}
	}

	return ret;
}

#endif

//-------------------------------------------------------------------------------------------------------------------
void InputEventListener::registerGamepadListener(cocos2d::EventDispatcher *eventDispatcher, cocos2d::Node *node)
{
	auto gamepadListener = EventListenerController::create();
	gamepadListener->onConnected = [](cocos2d::Controller* controller, Event* event){
		CCLOG("onConnectController");
		InputManager *im = InputManager::getInstance();
        int id = controller->getDeviceId();
        if (id < 0 || id >= InputDataRaw::MAX_GAMEPAD) {
            return;
        }
		im->getInputDataRaw()->registerConnected(id, controller->getDeviceName().c_str());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	};

	gamepadListener->onDisconnected = [](cocos2d::Controller* controller, Event* event){
		CCLOG("onDisconnectedController");
		InputManager *im = InputManager::getInstance();
        int id = controller->getDeviceId();
        if (id < 0 || id >= InputDataRaw::MAX_GAMEPAD) {
            return;
        }
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		im->getInputDataRaw()->registerDisconnected(id, controller->getDeviceName().c_str());
	};
	//------------------------------------------------------------------------------------------------------------------
	//Button
	gamepadListener->onButtonRawDown = [](cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
		int id = controller->getDeviceId();
		if (id < 0 || id >= InputDataRaw::MAX_GAMEPAD) {
			return;
		}
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerButtonDown(id, controller->getDeviceName().c_str(), keyCode);
	};
	gamepadListener->onButtonRawUp = [](cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
		int id = controller->getDeviceId();
		if (id >= InputDataRaw::MAX_GAMEPAD) {
			return;
		}
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerButtonUp(id, controller->getDeviceName().c_str(), keyCode);
	};
	gamepadListener->onButtonRawRepeat = [](cocos2d::Controller* controller, int keyCode, cocos2d::Event *event) {
		int id = controller->getDeviceId();
		if (id >= InputDataRaw::MAX_GAMEPAD) {
			return;
		}
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerButtonRepeat(id, controller->getDeviceName().c_str(), keyCode);
	};
	//------------------------------------------------------------------------------------------------------------------
	//Axis
	gamepadListener->onAxisRawEvent = [](cocos2d::Controller* controller, int keyCode, cocos2d::Event* event) {
		int id = controller->getDeviceId();
		if (id >= InputDataRaw::MAX_GAMEPAD) {
			return;
		}
		float value = controller->getAxisRawStatus(keyCode).value;
//※ACT2-4521 Axis情報でvalue=0も必要なため。
//#define THRESHOLD   0.001f
//        if(value < THRESHOLD && value >= -THRESHOLD){
//            return;
//        }
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerAxis(id, keyCode, value);
	};

	eventDispatcher->addEventListenerWithSceneGraphPriority(gamepadListener, node);
}

void InputEventListener::registerMouseListener(cocos2d::EventDispatcher *eventDispatcher, cocos2d::Node *node)
{
	auto mouseListener = EventListenerMouse::create();

	mouseListener->onMouseDown = [](cocos2d::Event* event){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		cocos2d::EventMouse* e = static_cast<cocos2d::EventMouse*>(event);
#else
		cocos2d::EventMouse* e = dynamic_cast<cocos2d::EventMouse*>(event);
#endif
		CCASSERT(e, "is not existed.");

		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerMouseDown((int)e->getMouseButton(), e->getCursorX(), e->getCursorY());
	};
	mouseListener->onMouseMove = [](cocos2d::Event* event){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		cocos2d::EventMouse* e = static_cast<cocos2d::EventMouse*>(event);
#else
		cocos2d::EventMouse* e = dynamic_cast<cocos2d::EventMouse*>(event);
#endif
		CCASSERT(e, "is not existed");

		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerMouseMove(e->getCursorX(), e->getCursorY());
	};

	mouseListener->onMouseUp = [](cocos2d::Event* event){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		cocos2d::EventMouse* e = static_cast<cocos2d::EventMouse*>(event);
#else
		cocos2d::EventMouse* e = dynamic_cast<cocos2d::EventMouse*>(event);
#endif
		CCASSERT(e, "is not existed");
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerMouseUp((int)e->getMouseButton(), e->getCursorX(), e->getCursorY());
	};

	mouseListener->onMouseScroll = [](cocos2d::Event* event){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		cocos2d::EventMouse* e = static_cast<cocos2d::EventMouse*>(event);
#else
		cocos2d::EventMouse* e = dynamic_cast<cocos2d::EventMouse*>(event);
#endif
		CCASSERT(e, "is not existed");
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerMouseScroll(e->getScrollX(), e->getScrollY());
	};

    eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, node);
}

void InputEventListener::registerKeyboardListener(cocos2d::EventDispatcher *eventDispatcher, cocos2d::Node *node)
{
	auto keyboardListener = EventListenerKeyboard::create();

	keyboardListener->onKeyPressed2 = [](cocos2d::EventKeyboard::KeyCode code, int scancode, cocos2d::Event* event){
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerKeyPressed((int)code, scancode);
	};
	keyboardListener->onKeyReleased2 = [](cocos2d::EventKeyboard::KeyCode code, int scancode, cocos2d::Event* event){
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerKeyReleased((int)code, scancode);
	};
	keyboardListener->onKeyRepeat = [](cocos2d::EventKeyboard::KeyCode code, int scancode, cocos2d::Event* event) {
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerKeyRepeat((int)code, scancode);
	};
	keyboardListener->onCharInputed = [](cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event) {
		InputManager *im = InputManager::getInstance();
		im->getInputDataRaw()->registerCharInput((int)code);
	};

	eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, node);
}

void InputEventListener::registerInputListener(cocos2d::EventDispatcher *eventDispatcher, cocos2d::Node *node)
{
	CCASSERT(eventDispatcher && node, "is not existed");
	registerGamepadListener(eventDispatcher, node);
	registerMouseListener(eventDispatcher, node);
	registerKeyboardListener(eventDispatcher, node);
    Controller::startDiscoveryController();
}

#if defined(USE_CONTROLLER_DATA)
//-------------------------------------------------------------------------------------------------------------------
enum MouseTemplateKind {
	MouseLeftClick = -1,
	MouseRightClick = -2,
	MouseMiddleClick = -3,
	MousePointer = -4,
	MouseWheelUp = -5,
	MouseWheelDown = -6,
};

static int gReservedKeyCodePc[InputController::kReservedKeyCodePc_Max] = {
	(int)EventKeyboard::KeyCode::KEY_CAPITAL_W,
	(int)EventKeyboard::KeyCode::KEY_CAPITAL_A,
	(int)EventKeyboard::KeyCode::KEY_CAPITAL_S,
	(int)EventKeyboard::KeyCode::KEY_CAPITAL_D,
	MouseLeftClick,//left click
	MouseRightClick,//right click
	0,
	0,
	0,
	0,
	(int)EventKeyboard::KeyCode::KEY_UP_ARROW,
	(int)EventKeyboard::KeyCode::KEY_RIGHT_ARROW,
	(int)EventKeyboard::KeyCode::KEY_DOWN_ARROW,
	(int)EventKeyboard::KeyCode::KEY_LEFT_ARROW,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	MouseMiddleClick,//Middle click
	0,
	MouseWheelUp,//Wheel(Up)
	0,
	MouseWheelDown,//Wheel(Down)
	0,
	MousePointer,//MousePointer
	0,
	0,
	0
};

class ReservedKeycodeList {
public:
	static void clear() {
		infoList.clear();
	}
	static void refresh() {
		clear();
		list();
	}
	static std::vector<ReservedKeycodeList> &list() {
		if (infoList.size() == 0) {
			infoList.push_back(ReservedKeycodeList(GameManager::tr("W"), InputController::kReservedKeyCodePc_W, (int)EventKeyboard::KeyCode::KEY_CAPITAL_W));
			infoList.push_back(ReservedKeycodeList(GameManager::tr("A"), InputController::kReservedKeyCodePc_A, (int)EventKeyboard::KeyCode::KEY_CAPITAL_A));
			infoList.push_back(ReservedKeycodeList(GameManager::tr("S"), InputController::kReservedKeyCodePc_S, (int)EventKeyboard::KeyCode::KEY_CAPITAL_S));
			infoList.push_back(ReservedKeycodeList(GameManager::tr("D"), InputController::kReservedKeyCodePc_D, (int)EventKeyboard::KeyCode::KEY_CAPITAL_D));
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Left click"), InputController::kReservedKeyCodePc_LeftClick, MouseLeftClick));//left click
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Right click"), InputController::kReservedKeyCodePc_RightClick, MouseRightClick));//right click
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Up"), InputController::kReservedKeyCodePc_Up, (int)EventKeyboard::KeyCode::KEY_UP_ARROW));
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Right"), InputController::kReservedKeyCodePc_Right, (int)EventKeyboard::KeyCode::KEY_RIGHT_ARROW));
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Down"), InputController::kReservedKeyCodePc_Down, (int)EventKeyboard::KeyCode::KEY_DOWN_ARROW));
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Left"), InputController::kReservedKeyCodePc_Left, (int)EventKeyboard::KeyCode::KEY_LEFT_ARROW));
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Middle click"), InputController::kReservedKeyCodePc_MiddleClick, MouseMiddleClick));//Middle click
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Wheel(Up)"), InputController::kReservedKeyCodePc_WheelUp, MouseWheelUp));//Wheel(Up)
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Wheel(Down)"), InputController::kReservedKeyCodePc_WhellDown, MouseWheelDown));//Wheel(Down)
			infoList.push_back(ReservedKeycodeList(GameManager::tr("Mouse"), InputController::kReservedKeyCodePc_MousePointer, MousePointer));//MousePointer
		}
		return infoList;
	}
protected:
	ReservedKeycodeList(const std::string &_name, char _reservedKeyCode, int _keyCode) {
		name = _name;
		reservedKeyCode = _reservedKeyCode;
		keyCode = _keyCode;
	}
public:
	static std::vector<ReservedKeycodeList> infoList;
	std::string name;
	char reservedKeyCode;
	int keyCode;
};
std::vector<ReservedKeycodeList> ReservedKeycodeList::infoList;

bool InputController::getReservedKeyCode(int reservedKeyCode, const char* &name, int &keyCode)
{
	auto &infoList = ReservedKeycodeList::list();
	for (unsigned int i = 0; i < infoList.size(); i++) {
		if (infoList[i].reservedKeyCode == reservedKeyCode) {
			name = infoList[i].name.c_str();
			keyCode = infoList[i].keyCode;
			return true;
		}
	}
	name = nullptr;
	keyCode = -1;
	return false;
}

InputController::InputController()
{
	_inputMappingData = nullptr;
	_inputDataRaw = nullptr;
	_inputGamepadData = nullptr;
	_inputKeyboardData = nullptr;
	_inputMouseData = nullptr;
	_reservedKeyCodePc = nullptr;
	_reservedKeyCodePcBase = nullptr;
	_gamepadNo = -1;
	_controller = kControllerPc;
	_containSystemKey = true;
}

InputController::~InputController()
{
	CC_SAFE_RELEASE_NULL(_inputMappingData);
	CC_SAFE_RELEASE_NULL(_inputDataRaw);
	CC_SAFE_RELEASE_NULL(_inputGamepadData);
	CC_SAFE_RELEASE_NULL(_inputKeyboardData);
	CC_SAFE_RELEASE_NULL(_inputMouseData);
	CC_SAFE_RELEASE_NULL(_reservedKeyCodePc);
	CC_SAFE_RELEASE_NULL(_reservedKeyCodePcBase);
}

InputController *InputController::create(int id, agtk::data::InputMappingData *inputMappingData, InputDataRaw *inputDataRaw)
{
	auto p = new (std::nothrow) InputController();
	if (p && p->init(id, inputMappingData, inputDataRaw)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool InputController::init(int id, agtk::data::InputMappingData *inputMappingData, InputDataRaw *inputDataRaw)
{
	CC_ASSERT(inputMappingData);
	CC_ASSERT(inputDataRaw);
	this->setId(id);
	this->setInputMappingData(inputMappingData);
	this->setInputDataRaw(inputDataRaw);
	this->setController(kControllerNone);
	return true;
}

void InputController::update(float delta)
{
	if (this->getController() == kControllerPc) {
		return;
	}
	if (this->getController() == kControllerNone) {
		if (this->getGamepadNo() >= 0) {
			auto gamepad = this->getInputDataRaw()->getGamepad(this->getGamepadNo());
			if (gamepad->getConnected()) {
				this->setGamepad(this->getGamepadNo());
			}
		}
	}
	else {
		int gamepadNo = this->getGamepadNo();
		auto gamepad = this->getInputDataRaw()->getGamepad(gamepadNo);
		if (gamepad->getConnected() == false) {
			this->setController(kControllerNone);
		}
	}
}

bool InputController::isPressed(int button)
{
	if (this->getController() == kControllerNone) {
		return false;
	}
	bool ret = false;
	auto keyCodeList = this->getTemplateButtonIds(button);
	if (this->getController() == kControllerPc) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
			auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
			ret |= this->isPressedPc(keyCode->getValue());
		}
		return ret;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
		auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
		auto p = this->getInputGamepadData()->getInput(keyCode->getValue());
		ret |= (p != nullptr) ? p->getPress() : false;
	}
	return ret;
}

bool InputController::isPressed(EnumMove move, cocos2d::Vec2 point, int button)
{
	if (this->getController() == kControllerNone) {
		return false;
	}
	bool ret = false;
	auto keyCodeList = this->getTemplateButtonIds(button, false);//※システムキーを含まない。
	if (this->getController() == kControllerPc) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
			auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
			ret |= this->isPressedPc(move, point, keyCode->getValue());
		}
		return ret;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
		auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
		auto p = this->getInputGamepadData()->getInput(keyCode->getValue());
		ret |= (p != nullptr) ? p->getPress() : false;
	}
	return ret;
}


bool InputController::isKeyPressed(int keyCode)
{
	if (this->getController() == kControllerNone) {
		return false;
	}
	bool ret = false;
	if (this->getController() == kControllerPc) {
		ret |= this->isPressedPc(keyCode);
		return ret;
	}
	auto p = this->getInputGamepadData()->getInput(keyCode);
	ret |= (p != nullptr) ? p->getPress() : false;
	return ret;
}

float InputController::getValue(int keyCode)
{
	if (this->getController() == kControllerNone) {
		return false;
	}
	bool ret = false;
	if (this->getController() == kControllerPc) {
		ret |= this->isPressedPc(keyCode);
		return ret;
	}
	auto p = this->getInputGamepadData()->getInput(keyCode);
	return (p != nullptr) ? p->getValue() : 0;
}

float InputController::getOperationKeyValue(int button)
{
	if (this->getController() == kControllerNone) {
		return 0;
	}
	float value = 0;
	auto keyCodeList = this->getTemplateButtonIds(button);
	if (this->getController() == kControllerPc) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
			auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
			value = std::max(value, this->isPressedPc(keyCode->getValue()) ? 1.0f : 0.0f);
		}
		return value;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
		auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
		auto inputGamepadData = this->getInputGamepadData();
		auto keyCodeValue = keyCode->getValue();
		auto p = inputGamepadData->getInput(keyCode->getValue());
		auto v = (p != nullptr) ? p->getValue() : 0;
		// Xbox360コントローラ/PS4コントローラの左右スティック入力は0/1に変換されているため、生の値をとるようにする。
		struct LogicPhysicMap {
			int logicId;
			int physicId;
			bool plus;
		};
		LogicPhysicMap *lpmList = nullptr;
		switch (inputGamepadData->getGamepadType()) {
		case InputGamepadData::kGamepadXbox360: {
			static LogicPhysicMap xbox360List[] = {
				//Left stick(X)
				{ InputGamepadData::kInputLeftStickRight,	InputGamepadData::kInputAxis1, true },
				{ InputGamepadData::kInputLeftStickLeft,	InputGamepadData::kInputAxis1, false },
				//Left stick(Y)
				{ InputGamepadData::kInputLeftStickUp,		InputGamepadData::kInputAxis2, true },
				{ InputGamepadData::kInputLeftStickDown,	InputGamepadData::kInputAxis2, false },
				//Right stick(X)
				{ InputGamepadData::kInputRightStickRight,	InputGamepadData::kInputAxis3, true },
				{ InputGamepadData::kInputRightStickLeft,	InputGamepadData::kInputAxis3, false },
				//Right stick(Y)
				{ InputGamepadData::kInputRightStickUp,		InputGamepadData::kInputAxis4, true },
				{ InputGamepadData::kInputRightStickDown,	InputGamepadData::kInputAxis4, false },
				{-1, -1, false}
			};
			lpmList = xbox360List;
			break; }
		case InputGamepadData::kGamepadPlayStation4: {
			static LogicPhysicMap ps4List[] = {
				//Left stick(X)
				{ InputGamepadData::kInputLeftStickRight,	InputGamepadData::kInputAxis1, true },
				{ InputGamepadData::kInputLeftStickLeft,	InputGamepadData::kInputAxis1, false },
				//Left stick(Y)
				{ InputGamepadData::kInputLeftStickUp,		InputGamepadData::kInputAxis2, false },
				{ InputGamepadData::kInputLeftStickDown,	InputGamepadData::kInputAxis2, true },
				//Right stick(X)
				{ InputGamepadData::kInputRightStickRight,	InputGamepadData::kInputAxis3, true },
				{ InputGamepadData::kInputRightStickLeft,	InputGamepadData::kInputAxis3, false },
				//Right stick(Y)
				{ InputGamepadData::kInputRightStickUp,		InputGamepadData::kInputAxis6, false },
				{ InputGamepadData::kInputRightStickDown,	InputGamepadData::kInputAxis6, true },
				{ -1, -1, false }
			};
			lpmList = ps4List;
			break; }
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
		case InputGamepadData::kGamepadDirectInput: {
			break; }
		default:
			break;
		}
		
		if (lpmList != nullptr) {
			LogicPhysicMap *head = lpmList;
			while (head->logicId >= 0) {
				if (keyCodeValue == head->logicId) {
					auto inputData = inputGamepadData->getInput(head->physicId);
					if (inputData != nullptr) {
						v = inputData->getValue();
						//if (v != 0) CCLOG("logicId:%d, physicId:%d, v:%f", head->logicId, head->physicId, v);
						if (head->plus) {
							if (v < 0) {
								v = 0;
							}
						} else {
							if (v > 0) {
								v = 0;
							}
						}
					}
				}
				head++;
			}
		}
		if (std::abs(v) > std::abs(value)) {
			value = std::abs(v);
		}
	}
	return value;
}

bool InputController::isTriggered(int button)
{
	if (this->getController() == kControllerNone) {
		return false;
	}
	bool ret = false;
	auto keyCodeList = this->getTemplateButtonIds(button);
	if (this->getController() == kControllerPc) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
			auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
			ret |= this->isTriggeredPc(keyCode->getValue());
		}
		return ret;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
		auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
		auto p = this->getInputGamepadData()->getInput(keyCode->getValue());
		ret |= (p != nullptr) ? p->getTrigger() : false;
	}
	return ret;
}

bool InputController::isReleased(int button)
{
	if (this->getController() == kControllerNone) {
		return false;
	}
	bool ret = false;
	auto keyCodeList = this->getTemplateButtonIds(button);
	if (this->getController() == kControllerPc) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
			auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
			ret |= this->isReleasedPc(keyCode->getValue());
		}
		return ret;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
		auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
		auto p = this->getInputGamepadData()->getInput(keyCode->getValue());
		ret |= (p != nullptr) ? p->getRelease() : false;

	}
	return ret;
}

bool InputController::isReleasing(int button)
{
	if (this->getController() == kControllerNone) {
		return true;
	}
	auto keyCodeList = this->getTemplateButtonIds(button, this->getContainSystemKey());
	if (keyCodeList->count() == 0) {
		return true;
	}
	if (this->getController() == kControllerPc) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
			auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
			if (this->isReleasingPc(keyCode->getValue()) == false) {
				return false;
			}
		}
		return true;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keyCodeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto keyCode = static_cast<cocos2d::Integer *>(ref);
#else
		auto keyCode = dynamic_cast<cocos2d::Integer *>(ref);
#endif
		auto p = this->getInputGamepadData()->getInput(keyCode->getValue());
#if 0
		bool ret = (p != nullptr) ? (p->getTrigger() == false && p->getPress() == false && p->getRelease() == false) : true;
		if (ret == false) {
			return false;
		}
#endif
		if (p != nullptr && (p->getTrigger() || p->getPress() || p->getRelease())) {
			if (button == LeftStickUp || button == LeftStickDown || button == LeftStickLeft || button == LeftStickRight ||
				button == RightStickUp || button == RightStickDown || button == RightStickLeft || button == RightStickRight) {
				//CCLOG("keyId:%d v:%f", button, p->getValue());
				if (std::abs(p->getValue()) >= AXIS_THRESHOLD) {
					return false;
				}
			}
			else {
				return false;
			}
		}
	}
	return true;
}

bool InputController::isNonePressedAll()
{
	auto inputMappingData = this->getInputMappingData();
	auto mappingList = inputMappingData->getInputMappingList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(mappingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto mapping = static_cast<agtk::data::InputMapping *>(el->getObject());
#else
		auto mapping = dynamic_cast<agtk::data::InputMapping *>(el->getObject());
#endif
		if (this->isReleasing(mapping->getKeyId()) == false) {
			return false;
		}
	}
	return true;
}

/**
* 何かのキー入力があったかチェック
* @param	ignoreCancel	キャンセルキーは除外するか？
* @return					True:入力あり / False:入力無し
*/
bool InputController::isPressedOr(bool ignoreCancel)
{
	auto inputMappingData = this->getInputMappingData();
	auto mappingList = inputMappingData->getInputMappingList();
	cocos2d::DictElement *el = nullptr;
	bool result = false;
	CCDICT_FOREACH(mappingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto mapping = static_cast<agtk::data::InputMapping *>(el->getObject());
#else
		auto mapping = dynamic_cast<agtk::data::InputMapping *>(el->getObject());
#endif
		auto keyId = mapping->getKeyId();
		auto containSystemKey = this->getContainSystemKey();
		this->setContainSystemKey(false);//システムキーを含まない。
		if (this->isReleasing(keyId) == false) {

			// キャンセルキー除外が真 かつ キーがキャンセルキーの場合
			if (ignoreCancel && keyId == CANCEL) {
				result = false;
			}
			else {
				result = true;
			}
		}
		this->setContainSystemKey(containSystemKey);
	}
	return result;
}

// #AGTK-NX
#if 1
/**
* 何かのキー入力があったかチェック
* @param	ignoreCancel	キャンセルキーは除外するか？
* @return					True:入力あり / False:入力無し
*/
bool InputController::isTriggeredOr(bool ignoreCancel)
{
	auto inputMappingData = this->getInputMappingData();
	auto mappingList = inputMappingData->getInputMappingList();
	cocos2d::DictElement *el = nullptr;
	bool result = false;
	CCDICT_FOREACH(mappingList, el) {
		// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto mapping = static_cast<agtk::data::InputMapping *>(el->getObject());
#else
		auto mapping = dynamic_cast<agtk::data::InputMapping *>(el->getObject());
#endif
		auto keyId = mapping->getKeyId();
		auto containSystemKey = this->getContainSystemKey();
		this->setContainSystemKey(false);//システムキーを含まない。
		if (this->isTriggered(keyId)) {

			// キャンセルキー除外が真 かつ キーがキャンセルキーの場合
			if (ignoreCancel && keyId == CANCEL) {
				result = false;
			}
			else {
				result = true;
			}
		}
		this->setContainSystemKey(containSystemKey);
	}
	return result;
}
#endif

cocos2d::__Array *InputController::getTemplateButtonIds(int button, bool bContainSystem)
{
	auto inputList = cocos2d::__Array::create();
	auto inputMappingList = this->getInputMappingData()->getInputMappingList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(inputMappingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto mapping = static_cast<agtk::data::InputMapping *>(el->getObject());
#else
		auto mapping = dynamic_cast<agtk::data::InputMapping *>(el->getObject());
#endif
		if (bContainSystem == false && agtk::data::InputMappingData::checkSystemKey(mapping->getKeyId())) {
			//システムキーを含まないようにする。
			continue;
		}
		if (mapping->getKeyId() == button) {
			switch (this->getController()) {
			case kControllerPc: {
				inputList->addObject(cocos2d::Integer::create(mapping->getPcInput()));
				break; }
			case kControllerXbox360: {
				inputList->addObject(cocos2d::Integer::create(mapping->getCustom1Input()));
				break; }
			case kControllerPs4: {
				inputList->addObject(cocos2d::Integer::create(mapping->getCustom2Input()));
				break; }
			case kControllerDirectInput: {
				inputList->addObject(cocos2d::Integer::create(mapping->getDiInput()));
				break; }
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			}
		}
	}
	return inputList;
}

bool InputController::isPressedPc(int keyCode)
{
	if (keyCode < 0) {
		return false;
	}
	int tmpKeyCode = 0;
	auto data = dynamic_cast<ReservedKeyCodeData *>(this->getReservedKeyCodePc()->objectForKey(keyCode));
	if (data) {
		tmpKeyCode = data->getKeyCode();
	}
	else {
		tmpKeyCode = keyCode - kReservedKeyCodePc_Max;
		if (tmpKeyCode < 0) {
			CC_ASSERT(0);
			tmpKeyCode = 0;
		}
	}
	if (tmpKeyCode > 0) {//keyboard
		auto keyboard = this->getInputKeyboardData()->getInputData(tmpKeyCode);
		return keyboard->getPress();
	}
	else if (tmpKeyCode < 0) {//mouse
		auto mouse = this->getInputMouseData();
		switch (tmpKeyCode) {
		case MouseLeftClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_LEFT);
			if (inputData) {
				return inputData->getPress();
			}
			break; }
		case MouseRightClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_RIGHT);
			if (inputData) {
				return inputData->getPress();
			}
			break; }
		case MouseMiddleClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_MIDDLE);
			if (inputData) {
				return inputData->getPress();
			}
			break; }
		case MousePointer: {
			return mouse->getMove(); }
		case MouseWheelUp: {
			if (mouse->getScrollPoint().y < 0.0f) {
				return true;
			}
			break; }
		case MouseWheelDown: {
			if (mouse->getScrollPoint().y > 0.0f) {
				return true;
			}
			break; }
		}
	}
	return false;
}

bool InputController::isPressedPc(EnumMove move, cocos2d::Vec2 point, int keyCode)
{
	if (keyCode < 0) {
		return false;
	}
	int tmpKeyCode = 0;
	auto data = dynamic_cast<ReservedKeyCodeData *>(this->getReservedKeyCodePc()->objectForKey(keyCode));
	if (data) {
		tmpKeyCode = data->getKeyCode();
	}
	else {
		tmpKeyCode = keyCode - kReservedKeyCodePc_Max;
		if (tmpKeyCode < 0) {
			CC_ASSERT(0);
			tmpKeyCode = 0;
		}
	}
	if (tmpKeyCode > 0) {//keyboard
		auto keyboard = this->getInputKeyboardData()->getInputData(tmpKeyCode);
		return keyboard->getPress();
	}
	else if (tmpKeyCode < 0) {//mouse
		auto mouse = this->getInputMouseData();
		switch (tmpKeyCode) {
		case MouseLeftClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_LEFT);
			if (inputData) {
				cocos2d::Vec2 p;
				if (this->calcSceneMousePoint(p)) {
					auto degree = agtk::GetDegreeFromVector(agtk::Scene::getPositionCocos2dFromScene(p) - agtk::Scene::getPositionCocos2dFromScene(point));
					if (inputData->getPress()) {
						return agtk::GetMoveDirectionId(degree) == move;
					}
				}
			}
			break; }
		case MouseRightClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_RIGHT);
			if (inputData) {
				cocos2d::Vec2 p;
				if (this->calcSceneMousePoint(p)) {
					auto degree = agtk::GetDegreeFromVector(agtk::Scene::getPositionCocos2dFromScene(p) - agtk::Scene::getPositionCocos2dFromScene(point));
					if (inputData->getPress()) {
						return agtk::GetMoveDirectionId(degree) == move;
					}
				}
			}
			break; }
		case MouseMiddleClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_MIDDLE);
			if (inputData) {
				cocos2d::Vec2 p;
				if (this->calcSceneMousePoint(p)) {
					auto degree = agtk::GetDegreeFromVector(agtk::Scene::getPositionCocos2dFromScene(p) - agtk::Scene::getPositionCocos2dFromScene(point));
					if (inputData->getPress()) {
						return agtk::GetMoveDirectionId(degree) == move;
					}
				}
			}
			break; }
		case MousePointer: {
			cocos2d::Vec2 p;
			if (this->calcSceneMousePoint(p)) {
				auto degree = agtk::GetDegreeFromVector(agtk::Scene::getPositionCocos2dFromScene(p) - agtk::Scene::getPositionCocos2dFromScene(point));
				if (mouse->getMove()) {
					return agtk::GetMoveDirectionId(degree) == move;
				}
			}
			break; }
		case MouseWheelUp: {
			if (mouse->getScrollPoint().y < 0.0f) {
				return true;
			}
			break; }
		case MouseWheelDown: {
			if (mouse->getScrollPoint().y > 0.0f) {
				return true;
			}
			break; }
		}
	}
	return false;
}

float InputController::getValuePc(int keyCode)
{
	if (keyCode < 0) {
		return 0;
	}
	int tmpKeyCode = 0;
	auto data = dynamic_cast<ReservedKeyCodeData *>(this->getReservedKeyCodePc()->objectForKey(keyCode));
	if (data) {
		tmpKeyCode = data->getKeyCode();
	}
	else {
		tmpKeyCode = keyCode - kReservedKeyCodePc_Max;
		if (tmpKeyCode < 0) {
			CC_ASSERT(0);
			tmpKeyCode = 0;
		}
	}
	if (tmpKeyCode > 0) {//keyboard
		auto keyboard = this->getInputKeyboardData()->getInputData(tmpKeyCode);
		return keyboard->getPress() ? 1 : 0;
	}
	else if (tmpKeyCode < 0) {//mouse
		auto mouse = this->getInputMouseData();
		switch (tmpKeyCode) {
		case MouseLeftClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_LEFT);
			if (inputData) {
				return inputData->getPress() ? 1 : 0;
			}
			break; }
		case MouseRightClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_RIGHT);
			if (inputData) {
				return inputData->getPress() ? 1 : 0;
			}
			break; }
		case MouseMiddleClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_MIDDLE);
			if (inputData) {
				return inputData->getPress() ? 1 : 0;
			}
			break; }
		case MousePointer: {
			return mouse->getMovePoint().getLength(); }
		case MouseWheelUp:
			if (mouse->getScrollPoint().y < 0.0f) {
				return -mouse->getScrollPoint().y;
			}
			break;
		case MouseWheelDown:
			if (mouse->getScrollPoint().y > 0.0f) {
				return mouse->getScrollPoint().y;
			}
			break;
		}
	}
	return 0;
}

bool InputController::isTriggeredPc(int keyCode)
{
	if (keyCode < 0) {
		return false;
	}
	int tmpKeyCode = 0;
	auto data = dynamic_cast<ReservedKeyCodeData *>(this->getReservedKeyCodePc()->objectForKey(keyCode));
	if (data) {
		tmpKeyCode = data->getKeyCode();
	}
	else {
		tmpKeyCode = keyCode - kReservedKeyCodePc_Max;
		if (tmpKeyCode < 0) {
			CC_ASSERT(0);
			tmpKeyCode = 0;
		}
	}
	if (tmpKeyCode > 0) {//keyboard
		auto keyboard = this->getInputKeyboardData()->getInputData(tmpKeyCode);
		return keyboard->getTrigger();
	}
	else if (tmpKeyCode < 0) {//mouse
		auto mouse = this->getInputMouseData();
		switch (tmpKeyCode) {
		case MouseLeftClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_LEFT);
			if (inputData) {
				return inputData->getTrigger();
			}
			break; }
		case MouseRightClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_RIGHT);
			if (inputData) {
				return inputData->getTrigger();
			}
			break; }
		case MouseMiddleClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_MIDDLE);
			if (inputData) {
				return inputData->getTrigger();
			}
			break; }
		case MousePointer: {
			return mouse->getMove(); }
		case MouseWheelUp:
			if (mouse->getWheelTrigger() && mouse->getScrollPoint().y < 0.0f) {
				return true;
			}
			break;
		case MouseWheelDown:
			if (mouse->getWheelTrigger() && mouse->getScrollPoint().y > 0.0f) {
				return true;
			}
			break;
		}
	}
	return false;
}

bool InputController::isReleasedPc(int keyCode)
{
	if (keyCode < 0) {
		return false;
	}
	int tmpKeyCode = 0;
	auto data = dynamic_cast<ReservedKeyCodeData *>(this->getReservedKeyCodePc()->objectForKey(keyCode));
	if (data) {
		tmpKeyCode = data->getKeyCode();
	}
	else {
		tmpKeyCode = keyCode - kReservedKeyCodePc_Max;
		if (tmpKeyCode < 0) {
			CC_ASSERT(0);
			tmpKeyCode = 0;
		}
	}
	if (tmpKeyCode > 0) {//keyboard
		auto keyboard = this->getInputKeyboardData()->getInputData(tmpKeyCode);
		return keyboard->getRelease();
	}
	else if (tmpKeyCode < 0) {//mouse
		auto mouse = this->getInputMouseData();
		switch (tmpKeyCode) {
		case MouseLeftClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_LEFT);
			if (inputData) {
				return inputData->getRelease();
			}
			break; }
		case MouseRightClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_RIGHT);
			if (inputData) {
				return inputData->getRelease();
			}
			break; }
		case MouseMiddleClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_MIDDLE);
			if (inputData) {
				return inputData->getRelease();
			}
			break; }
		case MousePointer: {
			return mouse->getMove() == false; }
		case MouseWheelUp:
			if (mouse->getWheelRelease() < 0 && !(mouse->getScrollPoint().y < 0.0f)) {
				return true;
			}
			break;
		case MouseWheelDown:
			if (mouse->getWheelRelease() > 0 && !(mouse->getScrollPoint().y > 0.0f)) {
				return true;
			}
			break;
		}
	}
	return false;
}

bool InputController::isReleasingPc(int keyCode)
{
	if (keyCode < 0) {
		return true;
	}
	int tmpKeyCode = 0;
	auto data = dynamic_cast<ReservedKeyCodeData *>(this->getReservedKeyCodePc()->objectForKey(keyCode));
	if (data) {
		tmpKeyCode = data->getKeyCode();
	}
	else {
		tmpKeyCode = keyCode - kReservedKeyCodePc_Max;
		if (tmpKeyCode < 0) {
			CC_ASSERT(0);
			tmpKeyCode = 0;
		}
	}
	if (tmpKeyCode > 0) {//keyboard
		auto keyboard = this->getInputKeyboardData()->getInputData(tmpKeyCode);
		return (keyboard->getTrigger() == false && keyboard->getPress() == false && keyboard->getRelease() == false);
	}
	else if (tmpKeyCode < 0) {//mouse
		auto mouse = this->getInputMouseData();
		switch (tmpKeyCode) {
		case MouseLeftClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_LEFT);
			if (inputData) {
				return (inputData->getTrigger() == false && inputData->getPress() == false && inputData->getRelease() == false);
			}
			break; }
		case MouseRightClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_RIGHT);
			if (inputData) {
				return (inputData->getTrigger() == false && inputData->getPress() == false && inputData->getRelease() == false);
			}
			break; }
		case MouseMiddleClick: {
			auto inputData = mouse->getInputData((int)cocos2d::EventMouse::MouseButton::BUTTON_MIDDLE);
			if (inputData) {
				return (inputData->getTrigger() == false && inputData->getPress() == false && inputData->getRelease() == false);
			}
			break; }
		case MousePointer:
			break;
		case MouseWheelUp:
			return !(mouse->getScrollPoint().y < 0.0f);
		case MouseWheelDown:
			return !(mouse->getScrollPoint().y > 0.0f);
		default:
			CC_ASSERT(0);
		}
	}
	return true;
}

void InputController::setGamepad(int gamepadNo)
{
	auto inputDataRaw = this->getInputDataRaw();
	if (gamepadNo < 0) {//pc
		this->setInputGamepadData(nullptr);
		this->setInputKeyboardData(inputDataRaw->getKeyboard());
		this->setInputMouseData(inputDataRaw->getMouse());
		this->setController(kControllerPc);

		auto reservedKeyCodePc = cocos2d::__Dictionary::create();
		auto reservedKeyCodePcBase = cocos2d::__Dictionary::create();
		for (int i = 0; i < kReservedKeyCodePc_Max; i++) {
			const char *name = nullptr;
			int keyCode = 0;
			if (getReservedKeyCode(i, name, keyCode)) {
				auto p = ReservedKeyCodeData::create(i, keyCode);
				reservedKeyCodePc->setObject(p, i);
				auto pp = ReservedKeyCodeData::create(i, keyCode);
				reservedKeyCodePcBase->setObject(pp, i);
			}
		}
		this->setReservedKeyCodePc(reservedKeyCodePc);
		this->setReservedKeyCodePcBase(reservedKeyCodePcBase);
	}
	else {//gamepad
		auto gamepad = inputDataRaw->getGamepad(gamepadNo);
		this->setInputGamepadData(gamepad);
		this->setInputKeyboardData(nullptr);
		this->setInputMouseData(nullptr);
		this->setGamepadNo(gamepadNo);
		if (gamepad->getConnected()) {
			switch (gamepad->getGamepadType()) {
			case InputGamepadData::kGamepadXbox360: this->setController(kControllerXbox360); break;
			case InputGamepadData::kGamepadPlayStation4: this->setController(kControllerPs4); break;
			case InputGamepadData::kGamepadDirectInput: this->setController(kControllerDirectInput); break;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			default: CC_ASSERT(0);
			}
		}
		else {
			this->setController(kControllerNone);
		}
	}
}

bool InputController::calcSceneMousePoint(cocos2d::Vec2& point)
{
	//マウスポインタ
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene) {
		auto director = Director::getInstance();
		auto camera = scene->getCamera();
		auto displaySize = camera->getDisplaySize();
		auto cameraScale = camera->getZoom();
		displaySize.width *= cameraScale.x;
		displaySize.height *= cameraScale.y;
		auto screenSize = director->getVisibleSize();
		auto sceneSize = scene->getSceneSize();
		auto mouse = this->getInputMouseData();
		point = InputMouseData::CalcTransLeftUp(mouse->getPoint(), screenSize, displaySize);
		auto layer = GameManager::getInstance()->getCurrentLayer();
		auto layerPos = layer->getPosition();
		point.x -= layerPos.x;
		point.y -= layerPos.y;
		point.x += camera->getNodePosition().x;
		point.y += (sceneSize.y - displaySize.height) - camera->getNodePosition().y;
		if (point.x < 0 || point.x >= sceneSize.x || point.y < 0 || point.y >= sceneSize.y) {
			//領域外は(x,y):(-1,-1)にする。
			point.x = -1;
			point.y = -1;
			return false;
		}
	}
	return true;
}

class ReservedKeycodeNameList {
public:
	static void clear() {
		infoList.clear();
	}
	static void refresh() {
		clear();
		list();
	}
	static std::vector<ReservedKeycodeNameList> &list() {
		if (infoList.size() == 0) {
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("W")));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("A")));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("S")));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("D")));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Left click")));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Right click")));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Up")));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Right")));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Down")));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Left")));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Middle click")));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Wheel(Up)")));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Wheel(Down)")));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(GameManager::tr("Mouse")));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
			infoList.push_back(ReservedKeycodeNameList(""));
		}
		return infoList;
	}
protected:
	ReservedKeycodeNameList(const std::string &_name) {
		name = _name;
	}
public:
	static std::vector<ReservedKeycodeNameList> infoList;
	std::string name;
};
std::vector<ReservedKeycodeNameList> ReservedKeycodeNameList::infoList;

const char *InputController::getKeyCodeNamePc(int keyCode)
{
	//"*"は別のKeyCodeに変換されるため実際には使われない文字となります。
	static char *keyCodeName[] = {
		"*", //NONE
		"Pause",    //PAUSE
		"Scrol Lock",  //SCROLL_LOCK
		"Print",    //PRINT
		"SysRq",   //SYSREQ
		"Break",    //BREAK
		"Esc",   //ESCAPE
		"Backspace",    //BACKSPACE
		"Tab",  //TAB
		"Back Tab", //BACK_TAB
		"Return",   //RETURN
		"Caps Lock",    //CAPS_LOCK
		"Shift",   //LEFT_SHIFT
		"*",  //RIGHT_SHIFT
		"Ctrl",    //LEFT_CTRL
		"*",   //RIGHT_CTRL
		"Alt", //LEFT_ALT
		"*",    //RIGHT_ALT
		"Menu", //MENU
		"Hyper",    //HYPER
		"Insert",   //INSERT
		"Home", //HOME
		"Page Up",    //PG_UP
		"Delete",   //DELETE
		"End",  //END
		"Page Down",  //PG_DOWN
		"←",   //LEFT_ARROW
		"→",  //RIGHT_ARROW
		"↑", //UP_ARROW
		"↓",   //DOWN_ARROW
		"Num Lock", //NUM_LOCK
		"*",  //KP_PLUS
		"*", //KP_MINUS
		"*",  //KP_MULTIPLY
		"*",    //KP_DIVIDE
		"Enter", //KP_ENTER
		"*",  //KP_HOME
		"*",    //KP_UP
		"*", //KP_PG_UP
		"*",  //KP_LEFT
		"*",  //KP_FIVE
		"*", //KP_RIGHT
		"*",   //KP_END
		"*",  //KP_DOWN
		"*",   //KP_PG_DOWN
		"*",    //KP_INSERT
		"*",    //KP_DELETE
		"F1",   //F1
		"F2",   //F2
		"F3",   //F3
		"F4",   //F4
		"F5",   //F5
		"F6",   //F6
		"F7",   //F7
		"F8",   //F8
		"F9",   //F9
		"F10",  //F10
		"F11",  //F11
		"F12",  //F12
		"Space",    //SPACE
		"!",   //EXCLAM
		"\"",    //QUOTE
		"#",   //NUMBER
		"$",   //DOLLAR
		"%",  //PERCENT
		"CIRCUMFLEX",   //CIRCUMFLEX
		"%",    //AMPERSAND
		"'",   //APOSTROPHE
		"(", //LEFT_PARENTHESIS
		")",    //RIGHT_PARENTHESIS
		"*", //ASTERISK
		"+", //PLUS
		",",    //COMMA
		"-",    //MINUS
		".",   //PERIOD
		"/",    //SLASH
		"0",    //0
		"1",    //1
		"2",    //2
		"3",    //3
		"4",    //4
		"5",    //5
		"6",    //6
		"7",    //7
		"8",    //8
		"9",    //9
		":",    //COLON
		";",    //SEMICOLON
		"<",    //LESS_THAN
		"=",    //EQUAL
		">", //GREATER_THAN
		"?", //QUESTION
		"@",   //AT
		"A",    //CAPITAL_A
		"B",    //CAPITAL_B
		"C",    //CAPITAL_C
		"D",    //CAPITAL_D
		"E",    //CAPITAL_E
		"F",    //CAPITAL_F
		"G",    //CAPITAL_G
		"H",    //CAPITAL_H
		"I",    //CAPITAL_I
		"J",    //CAPITAL_J
		"K",    //CAPITAL_K
		"L",    //CAPITAL_L
		"M",    //CAPITAL_M
		"N",    //CAPITAL_N
		"O",    //CAPITAL_O
		"P",    //CAPITAL_P
		"Q",    //CAPITAL_Q
		"R",    //CAPITAL_R
		"S",    //CAPITAL_S
		"T",    //CAPITAL_T
		"U",    //CAPITAL_U
		"V",    //CAPITAL_V
		"W",    //CAPITAL_W
		"X",    //CAPITAL_X
		"Y",    //CAPITAL_Y
		"Z",    //CAPITAL_Z
		"[", //LEFT_BRACKET
		"\\",   //BACK_SLASH
		"]",    //RIGHT_BRACKET
		"_",   //UNDERSCORE
		"`",    //GRAVE
		"a",    //A
		"b",    //B
		"c",    //C
		"d",    //D
		"e",    //E
		"f",    //F
		"g",    //G
		"h",    //H
		"i",    //I
		"j",    //J
		"k",    //K
		"l",    //L
		"m",    //M
		"n",    //N
		"o",    //O
		"p",    //P
		"q",    //Q
		"r",    //R
		"s",    //S
		"t",    //T
		"u",    //U
		"v",    //V
		"w",    //W
		"x",    //X
		"y",    //Y
		"z",    //Z
		"{",   //LEFT_BRACE
		"|",  //BAR
		"}",  //RIGHT_BRACE
		"~",    //TILDE
		"*", //EURO
		"*",    //POUND
		"Yen",  //YEN
		"Middle Dot",   //MIDDLE_DOT
		"Search",   //SEARCH
		"*",    //DPAD_LEFT
		"*",   //DPAD_RIGHT
		"*",  //DPAD_UP
		"*",    //DPAD_DOWN
		"*",  //DPAD_CENTER
		"*",    //ENTER
		"Play", //PLAY
	};
	if (keyCode < 0) {
		return "";
	}
	auto &infoList = ReservedKeycodeNameList::list();
	if (keyCode < static_cast<int>(infoList.size())) {
		return infoList[keyCode].name.c_str();
	}
	CC_ASSERT(keyCode - infoList.size() >= 0 || keyCode < CC_ARRAYSIZE(keyCodeName));
	return keyCodeName[keyCode - infoList.size()];
}
#endif

//-------------------------------------------------------------------------------------------------------------------
InputManager* InputManager::_inputManager = NULL;
InputManager::InputManager()
{
	_inputMappingData = nullptr;
	_inputDataRaw = nullptr;
#if defined(USE_CONTROLLER_DATA)
	_inputControllerList = nullptr;
	_selectInputControllerList = nullptr;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#endif
	_lastControllerCount = 0;
	_precedeInputAcceptDataList.clear();
	_precedeInputDataList.clear();
	_ignoreInput = false;

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

InputManager::~InputManager()
{
	CC_SAFE_RELEASE_NULL(_inputMappingData);
	CC_SAFE_RELEASE_NULL(_inputDataRaw);
#if defined(USE_CONTROLLER_DATA)
	CC_SAFE_RELEASE_NULL(_inputControllerList);
	CC_SAFE_RELEASE_NULL(_selectInputControllerList);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#endif
}

InputManager* InputManager::getInstance()
{
	if (!_inputManager) {
		_inputManager = new InputManager();
		Scheduler *s = Director::getInstance()->getScheduler();
		s->scheduleUpdate(_inputManager, kSchedulePriorityInputManager, false);
		s->schedule([&](float delta) {
			_inputManager->afterUpdate(delta);
		}, _inputManager, 0.0f, false, "afterUpdate");
	}
	return _inputManager;
}

void InputManager::purge()
{
	ReservedKeycodeNameList::clear();
	ReservedKeycodeList::clear();
	if (!_inputManager)
		return;

	// スケジューラーを停止する。
	Scheduler *s = Director::getInstance()->getScheduler();
	s->unscheduleUpdate(_inputManager);
	s->unschedule("afterUpdate", _inputManager);

	InputManager *im = _inputManager;
	_inputManager = NULL;
	im->release();
}

bool InputManager::init(agtk::data::InputMappingData *inputMappingData)
{
	this->setInputMappingData(inputMappingData);
	auto inputDataRaw = InputDataRaw::create();
	if (inputDataRaw == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setInputDataRaw(inputDataRaw);

	//PC:0, Gamepad:1～16
	auto inputControllerList = cocos2d::__Dictionary::create();
	if (inputControllerList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	for (int i = 0; i <= InputDataRaw::MAX_GAMEPAD; i++) {
		auto controller = InputController::create(i, inputMappingData, inputDataRaw);
		if (controller == nullptr) {
			CC_ASSERT(0);
			return false;
		}
		controller->setGamepad(i - 1);
		inputControllerList->setObject(controller, i);
	}
	this->setInputControllerList(inputControllerList);

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	auto selectInputControllerList = cocos2d::__Dictionary::create();
	if (selectInputControllerList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setSelectInputControllerList(selectInputControllerList);

	return true;
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

InputGamepadData *InputManager::getGamepad(int id)
{
	return this->getInputDataRaw()->getGamepad(id);
}

int InputManager::getGamepadCount()
{
	return this->getInputDataRaw()->getGamepadList()->count();
}

const char *InputManager::getInputName(int inputId)
{
	CC_ASSERT(_inputMappingData);
	auto data = _inputMappingData->getInputMapping(inputId);
	auto operationKey = _inputMappingData->getOperationKey(data->getKeyId());
	return operationKey->getName();
}

int InputManager::getInputIdByName(const char *name, int button, int id)
{
	CC_ASSERT(_inputMappingData);
	auto data = _inputMappingData->getInputMapping(button);
	if (data == nullptr) {
		return -1;
	}
	auto gamepad = this->getInputDataRaw()->getGamepad(id);
	int inputId;
	if (name && strstr(name, "Xbox 360 Controller") != nullptr) {
		inputId = data->getCustom1Input();
	}
	//ps4
	else if (name && strstr(name, "Wireless Controller") != nullptr) {
		inputId = data->getCustom2Input();
	}
	//dinput
	else {
		inputId = data->getDiInput();
	}
	return inputId;
}

void InputManager::setInputMappingData(agtk::data::InputMappingData *inputMappingData)
{
	if (_inputMappingData != inputMappingData)
	{
		CC_SAFE_RETAIN(inputMappingData);
		CC_SAFE_RELEASE(_inputMappingData);
		_inputMappingData = inputMappingData;

		auto inputControllerList = this->getInputControllerList();
		if (inputControllerList) {
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(inputControllerList, el) {
				auto controller = dynamic_cast<InputController *>(el->getObject());
				controller->setInputMappingData(inputMappingData);
			}
		}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	}
}

bool InputManager::isPressed(int button, int id)
{
	if (GameManager::getInstance()->isSceneChanging() || _ignoreInput) {
		return false;
	}

	auto inputControllerList = this->getInputControllerList();
	if (id <= 0) {
		bool ret = false;
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
			ret |= controller->isPressed(button);
		}
		return ret;
	}
	auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
	if (controller) {
		return controller->isPressed(button);
	}
	return false;
}

bool InputManager::isPressed(InputController::EnumMove move, cocos2d::Vec2 point, int button, int id)
{
	if (GameManager::getInstance()->isSceneChanging() || _ignoreInput) {
		return false;
	}

	auto inputControllerList = this->getInputControllerList();
	if (id <= 0) {
		bool ret = false;
		cocos2d::DictElement *el;
		CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
			ret |= controller->isPressed(move, point, button);
		}
		return ret;
	}
	auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
	if (controller) {
		return controller->isPressed(move, point, button);
	}
	return false;
}

bool InputManager::isTriggered(int button, int id, agtk::data::ObjectInputConditionData::EnumTriggerType type, int instanceId, int acceptFrameCount)
{
	if (GameManager::getInstance()->isSceneChanging() || _ignoreInput) {
		return false;
	}

	bool ret = false;

	auto inputControllerList = this->getInputControllerList();
	if (id <= 0) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
			ret |= controller->isTriggered(button);
		}
	}
	else
	{
		auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
		if (controller) {
			ret =  controller->isTriggered(button);
		}
	}

	if (acceptFrameCount > 1) {
		// 先行入力させるキーのデータがなければ登録しておく
		auto result = find_if(_precedeInputAcceptDataList.begin(), _precedeInputAcceptDataList.end(),
			[type, button, id, instanceId](auto const& data) {
			return data.getType() == type && data.getButton() == button && data.getId() == id && data.getInstanceId() == instanceId;
		});
		if (result == _precedeInputAcceptDataList.end()) {
			PrecedeInputData precedeInputData(type, button, id, instanceId);
			_precedeInputAcceptDataList.emplace_back(precedeInputData);
		}

		// キー入力有効
		if (ret) {
			// 先行入力キーを削除しておく
			auto result = find_if(_precedeInputDataList.begin(), _precedeInputDataList.end(),
				[type, button, id, instanceId](auto const& data) {
				return data.getType() == type && data.getButton() == button && data.getId() == id && data.getInstanceId() == instanceId;
			});
			if (result != _precedeInputDataList.end()) {
				_precedeInputDataList.erase(result);
			}
		}
		// キー入力無効
		else {
			// 先行入力があるならキー入力有効として扱う
			for (auto& data : _precedeInputDataList) {
				if (data.getType() == type && data.getButton() == button && data.getId() == id && data.getInstanceId() == instanceId) {
					if (data.getFrame() < acceptFrameCount) {
						//現時点で押されている場合に有効。
						//if (isPressed(button, id)) {
						//	ret = true;
						//}
						ret = true;
					}
					data.setIsRemove(true);
				}
			}
		}
	}

	return ret;
}

bool InputManager::isReleased(int button, int id, agtk::data::ObjectInputConditionData::EnumTriggerType type, int instanceId, int acceptFrameCount)
{
	if (GameManager::getInstance()->isSceneChanging() || _ignoreInput) {
		return false;
	}

	bool ret = false;

	auto inputControllerList = this->getInputControllerList();
	if (id <= 0) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
			ret |= controller->isReleased(button);
		}
	}
	else {
		auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
		if (controller) {
			ret = controller->isReleased(button);
		}
	}

	if (acceptFrameCount > 1) {
		// 先行入力させるキーのデータがなければ登録しておく
		auto result = find_if(_precedeInputAcceptDataList.begin(), _precedeInputAcceptDataList.end(),
			[type, button, id, instanceId](auto const& data) {
			return data.getType() == type && data.getButton() == button && data.getId() == id && data.getInstanceId() == instanceId;
		});
		if (result == _precedeInputAcceptDataList.end()) {
			PrecedeInputData precedeInputData(type, button, id, instanceId);
			_precedeInputAcceptDataList.emplace_back(precedeInputData);
		}

		// キー入力有効
		if (ret) {
			// 先行入力キーを削除しておく
			auto result = find_if(_precedeInputDataList.begin(), _precedeInputDataList.end(),
				[type, button, id, instanceId](auto const& data) {
				return data.getType() == type && data.getButton() == button && data.getId() == id && data.getInstanceId() == instanceId;
			});
			if (result != _precedeInputDataList.end()) {
				_precedeInputDataList.erase(result);
			}
		}
		// キー入力無効
		else {
			// 先行入力があるならキー入力有効として扱う
			for (auto& data : _precedeInputDataList) {
				if (data.getType() == type && data.getButton() == button && data.getId() == id && data.getInstanceId() == instanceId) {
					if (data.getFrame() < acceptFrameCount) {
						//現時点で離されている場合に有効。
						//if (isReleasing(button, id)) {
						//	ret = true;
						//}
						ret = true;
					}
					data.setIsRemove(true);
				}
			}
		}
	}

	return ret;
}

bool InputManager::isReleasing(int button, int id)
{
	if (GameManager::getInstance()->isSceneChanging() || _ignoreInput) {
		return false;
	}

	auto inputControllerList = this->getInputControllerList();
	if (id <= 0) {
		//全てが離されている事を条件とする。
		bool ret = true;
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
			ret &= controller->isReleasing(button);
		}
		return ret;
	}
	auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
	if (controller) {
		return controller->isReleasing(button);
	}
	return false;
}

bool InputManager::isNoneInput(int id)
{
	auto inputControllerList = this->getInputControllerList();
	if (id <= 0) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
			if (controller->isNonePressedAll() == false) {
				return false;
			}
		}
		return true;
	}
	auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
	if (controller) {
		return controller->isNonePressedAll();
	}
	return true;
}

bool InputManager::isNoneInputWithin(cocos2d::__Array *inputKeys, int id)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(inputKeys, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto key = static_cast<cocos2d::__Integer *>(ref);
#else
		auto key = dynamic_cast<cocos2d::__Integer *>(ref);
#endif
		bool ret = this->isReleasing(key->getValue(), id);
		if (ret == false) {
			return false;
		}
	}
	return true;
}

bool InputManager::isNoneInputWithout(cocos2d::__Array *inputKeys, int id)
{
	auto gamepad = this->getInputDataRaw()->getGamepad(id);
	auto inputList = gamepad->getInputList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(inputList, el) {
		bool bKeyWithin = false;
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(inputKeys, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto key = static_cast<cocos2d::__Integer *>(ref);
#else
			auto key = dynamic_cast<cocos2d::__Integer *>(ref);
#endif
			int button = this->getInputIdByName(gamepad->getName(), key->getValue(), id);
			if (button == el->getIntKey()) {
				bKeyWithin = true;
				break;
			}
		}
		if (bKeyWithin) {
			continue;
		}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<InputGamepadData::InputData *>(el->getObject());
#else
		auto data = dynamic_cast<InputGamepadData::InputData *>(el->getObject());
#endif
		if (data->getTrigger()) return false;
		if (data->getPress()) return false;
		if (data->getRelease()) return false;
	}
	return true;
}

bool InputManager::isPressedKeyboard(int keyCode)
{
	auto keyboard = this->getInputDataRaw()->getKeyboard();
	auto key = keyboard->getInputData(keyCode);
	return key->getPress();
}

bool InputManager::isTriggeredKeyboard(int keyCode)
{
	auto keyboard = this->getInputDataRaw()->getKeyboard();
	auto key = keyboard->getInputData(keyCode);
	return key->getTrigger();
}

bool InputManager::isReleasedKeyboard(int keyCode)
{
	auto keyboard = this->getInputDataRaw()->getKeyboard();
	auto key = keyboard->getInputData(keyCode);
	return key->getRelease();
}

/**
* 先行入力受付セット(先行入力の条件を満たしていたら先行入力としてセットする)
*/
void InputManager::setPrecedeInputJudgeData()
{
	for (const auto& data : _precedeInputAcceptDataList) {
		agtk::data::ObjectInputConditionData::EnumTriggerType type = data.getType();
		int button = data.getButton();
		int id = data.getId();
		int instanceId = data.getInstanceId();

		switch (type) {
			case agtk::data::ObjectInputConditionData::kTriggerJustPressed: setPrecedeInputTriggered(button, id, type, instanceId); break;//押された瞬間
			case agtk::data::ObjectInputConditionData::kTriggerJustReleased: setPrecedeInputReleased(button, id, type, instanceId); break;//離された瞬間
			default:CC_ASSERT(0);
		}
	}
}

/**
* 押された瞬間の先行入力をセット
*/
void InputManager::setPrecedeInputTriggered(int button, int id, agtk::data::ObjectInputConditionData::EnumTriggerType type, int instanceId)
{
	bool ret = false;

	auto inputControllerList = this->getInputControllerList();
	if (id <= 0) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
			ret |= controller->isTriggered(button);
		}
	}
	else
	{
		auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
		if (controller) {
			ret = controller->isTriggered(button);
		}
	}

	if (ret) {
		PrecedeInputData precedeInputData(type, button, id, instanceId);
		_precedeInputDataList.emplace_back(precedeInputData);
	}
}

/**
* 離された瞬間の先行入力をセット
*/
void InputManager::setPrecedeInputReleased(int button, int id, agtk::data::ObjectInputConditionData::EnumTriggerType type, int instanceId)
{
	bool ret = false;

	auto inputControllerList = this->getInputControllerList();
	if (id <= 0) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
			ret |= controller->isReleased(button);
		}
	}
	else {
		auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
		if (controller) {
			ret = controller->isReleased(button);
		}
	}

	if (ret) {
		PrecedeInputData precedeInputData(type, button, id, instanceId);
		_precedeInputDataList.emplace_back(precedeInputData);
	}
}

cocos2d::__Array *InputManager::getPressedKeyboard()
{
	auto arr = cocos2d::__Array::create();
	auto keyboard = this->getInputDataRaw()->getKeyboard();
	auto inputList = keyboard->getInputList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<InputKeyboardData::InputData *>(el->getObject());
#else
		auto p = dynamic_cast<InputKeyboardData::InputData *>(el->getObject());
#endif
		if (p->getPress()) {
			arr->addObject(cocos2d::__Integer::create(el->getIntKey()));
		}
	}
	return arr;
}

cocos2d::__Array *InputManager::getTriggeredKeyboard()
{
	auto arr = cocos2d::__Array::create();
	auto keyboard = this->getInputDataRaw()->getKeyboard();
	auto inputList = keyboard->getInputList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<InputKeyboardData::InputData *>(el->getObject());
#else
		auto p = dynamic_cast<InputKeyboardData::InputData *>(el->getObject());
#endif
		if (p->getTrigger()) {
			arr->addObject(cocos2d::__Integer::create(el->getIntKey()));
		}
	}
	return arr;
}

cocos2d::__Array *InputManager::getReleasedKeyboard()
{
	auto arr = cocos2d::__Array::create();
	auto keyboard = this->getInputDataRaw()->getKeyboard();
	auto inputList = keyboard->getInputList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<InputKeyboardData::InputData *>(el->getObject());
#else
		auto p = dynamic_cast<InputKeyboardData::InputData *>(el->getObject());
#endif
		if (p->getRelease()) {
			arr->addObject(cocos2d::__Integer::create(el->getIntKey()));
		}
	}
	return arr;
}

InputManager::EnumTypeInput InputManager::getTypeInput(int id)
{
	auto gamepad = this->getInputDataRaw()->getGamepad(id);
	const char *name = gamepad->getName();
	if (name && strstr(name, "Xbox 360 Controller") != nullptr) {
		return kTypeCustom1Input;
	}
	//ps4
	else if (name && strstr(name, "Wireless Controller") != nullptr) {
		return kTypeCustom2Input;
	}
	//dinput
	return kTypeDirectInput;
}

void InputManager::reset(bool isReset/*=false*/)
{
	this->getInputDataRaw()->reset(isReset);
	if (isReset) {
		this->getSelectInputControllerList()->removeAllObjects();
	}
	_precedeInputAcceptDataList.clear();
	_precedeInputDataList.clear();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

bool InputManager::setIgnored(int id, bool bIgnored)
{
	auto gamepad = this->getGamepad(id);
	bool bOldIgnored = gamepad->getIgnored();
	gamepad->setIgnored(bIgnored);
	return bOldIgnored;
}

InputController *InputManager::getController(int id)
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(this->getInputControllerList()->objectForKey(id));
#endif
	return controller;
}

InputController *InputManager::getSelectController(int id)
{
	auto controller = dynamic_cast<InputController *>(this->getSelectInputControllerList()->objectForKey(id));
	return controller;
}

int InputManager::getPlayControllerId(int controllerId)
{
	//全入力デバイス(-2)の場合
	if (controllerId == InputManager::kControllerAll) {
		return -1;
	}

lRetry:
	auto controllerList = this->getSelectInputControllerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(controllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<InputController *>(el->getObject());
#else
		auto p = dynamic_cast<InputController *>(el->getObject());
#endif
		if (p->getId() == controllerId) {
			return el->getIntKey();
		}
	}

	// コントロールIDが指定されていて、選択されていない場合は強制的にパッド選択する。
	if (controllerId >= 0) {
		auto gm = GameManager::getInstance();
		auto playData = gm->getPlayData();
		auto selectInputControllerList = this->getSelectInputControllerList();
		unsigned int playerMax = gm->getProjectData()->getPlayerCount();
		// プレイヤー総数分チェック
		for (unsigned int idx = 0; idx < playerMax; idx++) {

			int playerId = idx + 1;														//プレイヤーID(1～)
			int id = idx + agtk::data::kProjectSystemVariable1PController;				//システム変数ID
			auto selectControllerId = playData->getCommonVariableData(id)->getValue();
			auto selectController = dynamic_cast<InputController *>(selectInputControllerList->objectForKey(playerId));
			if (selectControllerId < 0 && selectController == nullptr) {
				auto getTargetController = [this](int gamePadNo) -> InputController* {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
					auto controllerList = this->getInputControllerList();
#endif
					cocos2d::DictElement *el = nullptr;
					CCDICT_FOREACH(controllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto controller = static_cast<InputController *>(el->getObject());
#else
						auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
						if (controller->getController() != InputController::kControllerNone && controller->getGamepadNo() + 1 == gamePadNo) {
#endif
							return controller;
						}
					}
					return nullptr;
				};
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				auto controllerList = this->getInputControllerList();
#endif
				cocos2d::DictElement *el;
				// 入力された順にselectInputControllerにInputControllerを格納する。
				CCDICT_FOREACH(controllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto controller = static_cast<InputController *>(el->getObject());
#else
					auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
					if (controller->getId() == controllerId) {
						cocos2d::DictElement *el2 = nullptr;
						bool flag = false;
						for (unsigned int idx2 = 0; idx2 < playerMax; idx2++) {
							int id2 = idx2 + agtk::data::kProjectSystemVariable1PController;//システム変数ID
							if (id == id2) {
								continue;
							}
							auto selectControllerId2 = playData->getCommonVariableData(id2)->getValue();
							if (controller == getTargetController(selectControllerId2)) {
								flag = true;
								break;
							}
						}
						// 新しく入力されたデバイスを見つけた。
						if (flag == false) {
							selectInputControllerList->setObject(controller, playerId);
							playData->getCommonVariableData(id)->setValue(el->getIntKey());
							goto lRetry;
						}
					}
				}
			}
		}
	}
	return -1;
}

void InputManager::update(float delta)
{
	auto inputDataRaw = this->getInputDataRaw();
#ifdef USE_PREVIEW
    if(inputDataRaw->getPlaying()){
        _inputDataRaw->applyPlayInput();
    } else
#endif
    if(!GameManager::getInstance()->isSceneChanging()){
        //初回パッド接続で取りこぼしをここで解消する。
        auto c = Controller::getAllController();
        for (unsigned int i = 0; i < c.size(); i++) {
            auto controller = c[i];
            int id = controller->getDeviceId();
            if (id < 0 || id >= InputDataRaw::MAX_GAMEPAD) {
                continue;
            }
            auto gamepad = this->getGamepad(id);
            if (gamepad->getConnected()) {
                continue;
            }
            _inputDataRaw->registerConnected(id, controller->getDeviceName().c_str());
        }
        _lastControllerCount = c.size();
        
#ifdef USE_PREVIEW
        if(inputDataRaw->getRecording()){
            _inputDataRaw->recordInput();
        }
#endif
        inputDataRaw->applyRegisteredData();
    }
	inputDataRaw->update();

	auto controllerList = this->getInputControllerList();
	cocos2d::DictElement *el= nullptr;
	CCDICT_FOREACH(controllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto controller = static_cast<InputController *>(el->getObject());
#else
		auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
		controller->update(delta);
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	// システム変数に記録されたプレイヤー毎の使用コントローラIDを元に selectInputControllerList を更新する
	auto gm = GameManager::getInstance();
	auto playData = gm->getPlayData();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	if (playData) {
#endif
		int playerMax = gm->getProjectData()->getPlayerCount();
		auto selectInputControllerList = this->getSelectInputControllerList();

		// 選択コントローラリストの要素がプレイ人数未満の場合
		if (selectInputControllerList->count() < static_cast<unsigned int>(playerMax)) {

			// 指定gamePadNoのコントローラを取得するメソッド
			auto getTargetController = [this](int gamePadNo) -> InputController* {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				auto controllerList = this->getInputControllerList();
#endif
				cocos2d::DictElement *el = nullptr;
				CCDICT_FOREACH(controllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto controller = static_cast<InputController *>(el->getObject());
#else
					auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
					if (controller->getController() != InputController::kControllerNone && controller->getGamepadNo() + 1 == gamePadNo) {
#endif
						return controller;
					}
				}
				return nullptr;
			};

			// プレイヤー総数分チェック
			for (int idx = 0; idx < playerMax; idx++) {
				int playerId = idx + 1;														//プレイヤーID(1～)
				int id = idx + agtk::data::kProjectSystemVariable1PController;				//システム変数ID
				auto selectControllerId = playData->getCommonVariableData(id)->getValue();
				auto selectController = dynamic_cast<InputController *>(selectInputControllerList->objectForKey(playerId));

				// コントローラ選択済みの場合
				if (selectController) {
					// 選択コントローラIDが無い場合
 					if (selectControllerId < 0) {
						// 選択コントローラリストから削除
						selectInputControllerList->removeObjectForKey(playerId);
					}
					// 選択コントローラIDが異なる場合
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
					else if (selectControllerId != selectController->getGamepadNo() + 1){
#endif
						// 対象コントローラ取得
						auto controller = getTargetController(selectControllerId);

						// 対象コントローラが存在する場合
						if (controller) {
							// 選択コントローラを入れ替え
							selectInputControllerList->setObject(controller, playerId);
						}
						// 対象コントローラが存在しない場合
						else {
							// 以前選択されたコントローラをリストから削除
							selectInputControllerList->removeObjectForKey(playerId);
						}
					}
				}
				// コントローラ未選択 かつ 選択コントローラーIDが指定されている場合
				else if (selectControllerId >= 0) {
					// 対象コントローラ取得
					auto controller = getTargetController(selectControllerId);

					// 対象コントローラが存在する場合
					if (controller) {
						// 選択コントローラをリストに登録
						selectInputControllerList->setObject(controller, playerId);
					}
				}
			}
		}
	}

	//マウスポインタ
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene) {
		auto director = Director::getInstance();
		auto camera = scene->getCamera();
		auto sceneSize = scene->getSceneSize();
		auto screenSize = director->getOpenGLView()->getVisibleSize();
		auto displaySize = camera->getDisplaySize();
		auto cameraScale = camera->getZoom();
		displaySize.width *= cameraScale.x;
		displaySize.height *= cameraScale.y;
		auto mouse = this->getInputDataRaw()->getMouse();
		// ACT2-6408 ゲーム開始時マウス位置バグ対応
		if (fabs(mouse->getPoint().x) < 1e-6 && fabs(mouse->getPoint().y) < 1e-6) {
			mouse->setPoint(cocos2d::Vec2((ImGui::GetMousePos().x * displaySize.width / (sceneSize.x - (displaySize.width * 2))),
				(sceneSize.y - ImGui::GetMousePos().y) * displaySize.height / sceneSize.y));
		}
		auto point = InputMouseData::CalcTransLeftUp(mouse->getPoint(), screenSize, displaySize);
		auto layer = GameManager::getInstance()->getCurrentLayer();
		auto layerPos = layer->getPosition();
		point.x -= layerPos.x;
		point.y -= layerPos.y;
		// ACT2-6435対応
		auto cameraPos = camera->getLayerPosition();
		point.x += (cameraPos->getValue().x *-1.0f);
		point.y += (sceneSize.y - displaySize.height) - (cameraPos->getValue().y *-1.0f);
		if (point.x < 0 || point.x >= sceneSize.x || point.y < 0 || point.y >= sceneSize.y) {
			//領域外は(x,y):(-1,-1)にする。
			point.x = -1;
			point.y = -1;
		}
		auto valueData = playData->getCommonVariableData(agtk::data::kProjectSystemVariableMouseX);
		valueData->setValue(point.x);
		valueData = playData->getCommonVariableData(agtk::data::kProjectSystemVariableMouseY);
		valueData->setValue(point.y);
	}

	// 先行入力判定
	setPrecedeInputJudgeData();

	// 先行入力の削除
	if (!_precedeInputDataList.empty()) {
		// 一時停止は処理しない
		auto debugWindow = agtk::DebugManager::getInstance()->getDebugPerformanceAndSpeedSettingsWindow();
		if (debugWindow->getState() != agtk::DebugPerformanceAndSpeedSettingsWindow::kStateScenePause) {
			float frame = 0.0f;
			float frameRate = GameManager::getInstance()->getFrameRate();
			float framePerSeconds = (1.0f / frameRate) * frameRate;

			std::list<PrecedeInputData>::iterator it;
			for (it = _precedeInputDataList.begin(); it != _precedeInputDataList.end(); ) {
				if ((*it).getIsRemove()) {
					it = _precedeInputDataList.erase(it);
					continue;
				}

				// 指定インスタンスのオブジェクトを取得
				auto object = GameManager::getInstance()->getTargetObjectByInstanceId((*it).getInstanceId());
				if (object == nullptr) {
					it = _precedeInputDataList.erase(it);
					continue;
				}
				frame = (*it).getFrame() + (framePerSeconds);
				(*it).setFrame(frame);
				++it;
			}
		}
	}
}

void InputManager::afterUpdate(float delta)
{
	// 1～4PコントローラーIDで-1の値があれば入力を検知した順番に、入力機器のIDを代入し操作できるようにする。
	auto gm = GameManager::getInstance();
	auto playData = gm->getPlayData();
	if (playData == nullptr) {
		return;
	}

	unsigned int playerMax = gm->getProjectData()->getPlayerCount();
	auto selectInputControllerList = this->getSelectInputControllerList();

	// 選択コントローラリストの要素がプレイ人数未満の場合
	if (selectInputControllerList->count() < playerMax) {

		// 指定gamePadNoのコントローラを取得するメソッド
		auto getTargetController = [this](int gamePadNo) -> InputController* {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			auto controllerList = this->getInputControllerList();
#endif
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(controllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto controller = static_cast<InputController *>(el->getObject());
#else
				auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				if (controller->getController() != InputController::kControllerNone && controller->getGamepadNo() + 1 == gamePadNo) {
#endif
					return controller;
				}
			}
			return nullptr;
		};

		// プレイヤー総数分チェック
		for (unsigned int idx = 0; idx < playerMax; idx++) {

			int playerId = idx + 1;														//プレイヤーID(1～)
			int id = idx + agtk::data::kProjectSystemVariable1PController;				//システム変数ID
			auto selectControllerId = playData->getCommonVariableData(id)->getValue();
			auto selectController = dynamic_cast<InputController *>(selectInputControllerList->objectForKey(playerId));
			if (selectControllerId < 0 && selectController == nullptr) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				auto controllerList = this->getInputControllerList();
#endif
				cocos2d::DictElement *el;
				// 入力された順にselectInputControllerにInputControllerを格納する。
				CCDICT_FOREACH(controllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto controller = static_cast<InputController *>(el->getObject());
#else
					auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
					if (controller->isPressedOr(true)) {//入力あり
						cocos2d::DictElement *el2 = nullptr;
						bool flag = false;
						for (unsigned int idx2 = 0; idx2 < playerMax; idx2++) {
							int id2 = idx2 + agtk::data::kProjectSystemVariable1PController;//システム変数ID
							if (id == id2) {
								continue;
							}
							int playerId2 = idx2 + 1;
							auto selectControllerId2 = playData->getCommonVariableData(id2)->getValue();
							auto selectController2 = dynamic_cast<InputController *>(selectInputControllerList->objectForKey(playerId2));
							if (selectControllerId2 < 0 && selectController2 != nullptr) {
								//InputManager::updateで破棄する対象のため無視する。
								if(controller == selectController2) {
									flag = true;
									break;
								}
							}
							if (controller == getTargetController(selectControllerId2)) {
								flag = true;
								break;
							}
						}
						// 新しく入力されたデバイスを見つけた。
						if (flag == false) {
							playData->getCommonVariableData(id)->setValue(el->getIntKey());
						}
					}
				}
			}
		}
	}
}

void InputManager::setupController(int controllerId)
{
	// 1～4PコントローラーIDで-1の値があれば入力を検知した順番に、入力機器のIDを代入し操作できるようにする。
	auto gm = GameManager::getInstance();
	auto playData = gm->getPlayData();
	if (playData == nullptr) {
		return;
	}

	unsigned int playerMax = gm->getProjectData()->getPlayerCount();
	auto selectInputControllerList = this->getSelectInputControllerList();

	// 指定gamePadNoのコントローラを取得するメソッド
	auto getTargetController = [this](int gamePadNo) -> InputController* {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		auto controllerList = this->getInputControllerList();
#endif
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(controllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto controller = static_cast<InputController *>(el->getObject());
#else
			auto controller = dynamic_cast<InputController *>(el->getObject());
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			if (controller->getController() != InputController::kControllerNone && controller->getGamepadNo() + 1 == gamePadNo) {
#endif
				return controller;
			}
		}
		return nullptr;
	};

	auto selectControllerId = playData->getCommonVariableData(controllerId)->getValue();
	int playerId = (controllerId - agtk::data::kProjectSystemVariable1PController) + 1;
	CC_ASSERT(playerId > 0);

	auto selectController = dynamic_cast<InputController *>(selectInputControllerList->objectForKey(playerId));
	if (selectController != nullptr && selectControllerId == -1) {
		selectInputControllerList->removeObjectForKey(playerId);
	}
	if (selectControllerId >= 0) {//指定IDでデバイスが見つかった場合かつ他のプレイヤーに割り当てられていない場合。
		if (selectController != nullptr) {
			selectInputControllerList->removeObjectForKey(playerId);
		}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		auto controllerList = this->getInputControllerList();
#endif
		auto controller = dynamic_cast<InputController *>(controllerList->objectForKey(selectControllerId));
		if (controller != nullptr) {
			bool flag = false;
			for (unsigned int idx2 = 0; idx2 < playerMax; idx2++) {
				int id2 = idx2 + agtk::data::kProjectSystemVariable1PController;//システム変数ID
				if (controllerId == id2) {
					continue;
				}
				int playerId2 = idx2 + 1;
				auto selectControllerId2 = playData->getCommonVariableData(id2)->getValue();
				auto selectController2 = dynamic_cast<InputController *>(selectInputControllerList->objectForKey(playerId2));
				if (selectControllerId2 < 0 && selectController2 != nullptr) {
					//InputManager::updateで破棄する対象のため無視する。
					if (controller == selectController2) {
						flag = true;
						break;
					}
				}
				if (controller == getTargetController(selectControllerId2)) {
					flag = true;
					break;
				}
			}
			// 新しく入力されたデバイスを見つけた。
			if (flag == false) {
				playData->getCommonVariableData(controllerId)->setValue(selectControllerId);
			}
		}
	}
}

#ifdef USE_PREVIEW
bool InputManager::startRecording()
{
    return _inputDataRaw->startRecording();
}

void InputManager::stopRecording()
{
    _inputDataRaw->stopRecording();
}

bool InputManager::isRecording()
{
    return _inputDataRaw->isRecording();
}

bool InputManager::startPlaying(const char *filename)
{
    return _inputDataRaw->startPlaying(filename);
}

bool InputManager::getRestartFlg(const char *filename)
{
	return _inputDataRaw->getRestartFlg(filename);
}

bool InputManager::isReplaying()
{
    return _inputDataRaw->isReplaying();
}

#endif
