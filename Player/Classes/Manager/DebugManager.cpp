#include "DebugManager.h"
#include "AppMacros.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#include "ImGUI/IMGUIGLViewImpl.h"
#include "ImGUI/ImGuiLayer.h"
#include "ImGUI/imgui.h"
#include "ImGUI/CCIMGUI.h"
#include "ImGUI/imgui_ja_gryph_ranges.cpp"
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#include "Manager/GameManager.h"
#include "Manager/AudioManager.h"
#include "JavascriptManager.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"

#include "json/stringbuffer.h"
#include "json/prettywriter.h"
#include "json/document.h"

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#ifdef USE_COLLISION_MEASURE
int wallCollisionCount = 0;
int hitCollisionCount = 0;
int attackCollisionCount = 0;
int connectionCollisionCount = 0;
int woConnectionCollisionCount = 0;
int noInfoCount = 0;
int callCount = 0;
int cachedCount = 0;
int roughWallCollisionCount = 0;
int roughHitCollisionCount = 0;
#endif
NS_AGTK_BEGIN //---------------------------------------------------------------------------------//

//-------------------------------------------------------------------------------------------------------------------
#define DEBUG_LAYER_NAME "ImGUILayer"

#if defined(USE_PREVIEW) && defined(AGTK_DEBUG)
static bool show_test_window = false;
#endif
std::string DebugManager::mFontName = "fonts/ARIALUNI.TTF";

class ShaderInfo {
public:
	static void clear() {
		infoList.clear();
	}
	static void refresh() {
		clear();
		list();
	}
	static std::vector<ShaderInfo> &list() {
		if (infoList.size() == 0) {
			infoList.push_back(ShaderInfo(GameManager::tr("Retro game machine"), agtk::Shader::kShaderColorGameboy));
			infoList.push_back(ShaderInfo(GameManager::tr("Defocus"), agtk::Shader::kShaderBlur));
			infoList.push_back(ShaderInfo(GameManager::tr("Analog TV"), agtk::Shader::kShaderCRTMonitor));
		}
		return infoList;
	}
protected:
	ShaderInfo(const std::string &_name, agtk::Shader::ShaderKind _kind) {
		name = _name;
		kind = _kind;
	}
public:
	static std::vector<ShaderInfo> infoList;
	std::string name;
	agtk::Shader::ShaderKind kind;
};
std::vector<ShaderInfo> ShaderInfo::infoList;

const char *getShaderName(agtk::Shader::ShaderKind kind)
{
	for (auto &info : ShaderInfo::list()) {
		if (info.kind == kind) {
			return info.name.c_str();
		}
	}
	return nullptr;
}

static struct {
	char *localeName;
	char *name;
} gLanguageInfoList[] = {
	{ "en_US", "English" },
	{ "fr_FR", "French" },
	{ "de_DE", "German" },
	{ "es_ES", "Spanish" },
	{ "it_IT", "Italian" },
	{ "ko_KR", "Korean" },
	{ "zh_TW", "TraditionalChinese" },
	{ "zh_CN", "SimplifiedChinese" },
	{ "pt_BR", "Portuguese" },
	{ "nl_NL", "Dutch" },
	{ "ru_RU", "Russian" },
	{ "ja_JP", "Japanese" },
};

class PcInputNameList {
public:
	static void clear() {
		infoList.clear();
	}
	static void refresh() {
		clear();
		list();
	}
	static std::vector<PcInputNameList> &list() {
		if (infoList.size() == 0) {
			infoList.push_back(PcInputNameList(GameManager::tr("W"), 0));
			infoList.push_back(PcInputNameList(GameManager::tr("A"), 1));
			infoList.push_back(PcInputNameList(GameManager::tr("S"), 2));
			infoList.push_back(PcInputNameList(GameManager::tr("D"), 3));
			infoList.push_back(PcInputNameList(GameManager::tr("Left click"), 4));
			infoList.push_back(PcInputNameList(GameManager::tr("Right click"), 5));
			infoList.push_back(PcInputNameList(GameManager::tr("Up"), 10));
			infoList.push_back(PcInputNameList(GameManager::tr("Right"), 11));
			infoList.push_back(PcInputNameList(GameManager::tr("Down"), 12));
			infoList.push_back(PcInputNameList(GameManager::tr("Left"), 13));
			infoList.push_back(PcInputNameList(GameManager::tr("Middle click"), 22));
			infoList.push_back(PcInputNameList(GameManager::tr("Wheel(Up)"), 24));
			infoList.push_back(PcInputNameList(GameManager::tr("Wheel(Down)"), 26));
			infoList.push_back(PcInputNameList(GameManager::tr("Mouse"), 28));
		}
		return infoList;
	}
protected:
	PcInputNameList(const std::string &_name, int _keyCode) {
		name = _name;
		keyCode = _keyCode;
	}
public:
	static std::vector<PcInputNameList> infoList;
	std::string name;
	int keyCode;
};
std::vector<PcInputNameList> PcInputNameList::infoList;

static struct {
	char *name;
	int keyCode;
} g_dinputNameList[] = {
	{ "Button1", 0 },
	{ "Button2", 1 },
	{ "Button3", 2 },
	{ "Button4", 3 },
	{ "Button5", 4 },
	{ "Button6", 5 },
	{ "Button7", 6 },
	{ "Button8", 7 },
	{ "Button9", 8 },
	{ "Button10", 9 },
	{ "Button11", 10 },
	{ "Button12", 11 },
	{ "Button13", 12 },
	{ "Button14", 13 },
	{ "Button15", 14 },
	{ "Button16", 15 },
	{ "Button17", 16 },
	{ "Button18", 17 },
	{ "Axis1(-)", 18 },
	{ "Axis1(+)", 19 },
	{ "Axis2(-)", 20 },
	{ "Axis2(+)", 21 },
	{ "Axis3(-)", 22 },
	{ "Axis3(+)", 23 },
	{ "Axis4(-)", 24 },
	{ "Axis4(+)", 25 },
	{ "Axis5(-)", 26 },
	{ "Axis5(+)", 27 },
	{ "Axis6(-)", 28 },
	{ "Axis6(+)", 29 },
	{ "Axis7(-)", 30 },
	{ "Axis7(+)", 31 },
};

class Ps4NameList {
public:
	static void clear() {
		infoList.clear();
	}
	static void refresh() {
		clear();
		list();
	}
	static std::vector<Ps4NameList> &list() {
		if (infoList.size() == 0) {
			infoList.push_back(Ps4NameList(GameManager::tr("Square"), 0));
			infoList.push_back(Ps4NameList(GameManager::tr("Cross"), 1));
			infoList.push_back(Ps4NameList(GameManager::tr("Circle"), 2));
			infoList.push_back(Ps4NameList(GameManager::tr("Triangle"), 3));
			infoList.push_back(Ps4NameList(GameManager::tr("L1"), 4));
			infoList.push_back(Ps4NameList(GameManager::tr("R1"), 5));
			infoList.push_back(Ps4NameList(GameManager::tr("L2"), 6));
			infoList.push_back(Ps4NameList(GameManager::tr("R2"), 7));
			infoList.push_back(Ps4NameList(GameManager::tr("SHARE"), 8));
			infoList.push_back(Ps4NameList(GameManager::tr("OPTIONS"), 9));
			infoList.push_back(Ps4NameList(GameManager::tr("Left stick(Press)"), 10));
			infoList.push_back(Ps4NameList(GameManager::tr("Right stick(Press)"), 11));
			infoList.push_back(Ps4NameList(GameManager::tr("Touch pad(Press)"), 13));
			infoList.push_back(Ps4NameList(GameManager::tr("Up"), 14));
			infoList.push_back(Ps4NameList(GameManager::tr("Right"), 15));
			infoList.push_back(Ps4NameList(GameManager::tr("Down"), 16));
			infoList.push_back(Ps4NameList(GameManager::tr("Left"), 17));
			infoList.push_back(Ps4NameList(GameManager::tr("Left stick(X)"), 18));
			infoList.push_back(Ps4NameList(GameManager::tr("Left stick(Y)"), 19));
			infoList.push_back(Ps4NameList(GameManager::tr("Right stick(X)"), 20));
			infoList.push_back(Ps4NameList(GameManager::tr("Right stick(Y)"), 23));
			infoList.push_back(Ps4NameList(GameManager::tr("Left stick(Up)"), 24));
			infoList.push_back(Ps4NameList(GameManager::tr("Left stick(Right)"), 25));
			infoList.push_back(Ps4NameList(GameManager::tr("Left stick(Down)"), 26));
			infoList.push_back(Ps4NameList(GameManager::tr("Left stick(Left)"), 27));
			infoList.push_back(Ps4NameList(GameManager::tr("Right stick(Up)"), 28));
			infoList.push_back(Ps4NameList(GameManager::tr("Right stick(Right)"), 29));
			infoList.push_back(Ps4NameList(GameManager::tr("Right stick(Down)"), 30));
			infoList.push_back(Ps4NameList(GameManager::tr("Right stick(Left)"), 31));
		}
		return infoList;
	}
protected:
	Ps4NameList(const std::string &_name, int _keyCode) {
		name = _name;
		keyCode = _keyCode;
	}
public:
	static std::vector<Ps4NameList> infoList;
	std::string name;
	int keyCode;
};
std::vector<Ps4NameList> Ps4NameList::infoList;

class Xbox360NameList {
public:
	static void clear() {
		infoList.clear();
	}
	static void refresh() {
		clear();
		list();
	}
	static std::vector<Xbox360NameList> &list() {
		if (infoList.size() == 0) {
			infoList.push_back(Xbox360NameList(GameManager::tr("A"), 0));
			infoList.push_back(Xbox360NameList(GameManager::tr("B"), 1));
			infoList.push_back(Xbox360NameList(GameManager::tr("X"), 2));
			infoList.push_back(Xbox360NameList(GameManager::tr("Y"), 3));
			infoList.push_back(Xbox360NameList(GameManager::tr("LB"), 4));
			infoList.push_back(Xbox360NameList(GameManager::tr("RB"), 5));
			infoList.push_back(Xbox360NameList(GameManager::tr("SELECT"), 6));
			infoList.push_back(Xbox360NameList(GameManager::tr("START"), 7));
			infoList.push_back(Xbox360NameList(GameManager::tr("Left stick(Press)"), 8));
			infoList.push_back(Xbox360NameList(GameManager::tr("Right stick(Press)"), 9));
			infoList.push_back(Xbox360NameList(GameManager::tr("Up"), 10));
			infoList.push_back(Xbox360NameList(GameManager::tr("Right"), 11));
			infoList.push_back(Xbox360NameList(GameManager::tr("Down"), 12));
			infoList.push_back(Xbox360NameList(GameManager::tr("Left"), 13));
			infoList.push_back(Xbox360NameList(GameManager::tr("Left stick(X)"), 18));
			infoList.push_back(Xbox360NameList(GameManager::tr("Left stick(Y)"), 19));
			infoList.push_back(Xbox360NameList(GameManager::tr("Right stick(X)"), 20));
			infoList.push_back(Xbox360NameList(GameManager::tr("Right stick(Y)"), 21));
			infoList.push_back(Xbox360NameList(GameManager::tr("LT"), 22));
			infoList.push_back(Xbox360NameList(GameManager::tr("RT"), 23));
			infoList.push_back(Xbox360NameList(GameManager::tr("Left stick(Up)"), 24));
			infoList.push_back(Xbox360NameList(GameManager::tr("Left stick(Right)"), 25));
			infoList.push_back(Xbox360NameList(GameManager::tr("Left stick(Down)"), 26));
			infoList.push_back(Xbox360NameList(GameManager::tr("Left stick(Left)"), 27));
			infoList.push_back(Xbox360NameList(GameManager::tr("Right stick(Up)"), 28));
			infoList.push_back(Xbox360NameList(GameManager::tr("Right stick(Right)"), 29));
			infoList.push_back(Xbox360NameList(GameManager::tr("Right stick(Down)"), 30));
			infoList.push_back(Xbox360NameList(GameManager::tr("Right stick(Left)"), 31));
		}
		return infoList;
	}
protected:
	Xbox360NameList(const std::string &_name, int _keyCode) {
		name = _name;
		keyCode = _keyCode;
	}
public:
	static std::vector<Xbox360NameList> infoList;
	std::string name;
	int keyCode;
};
std::vector<Xbox360NameList> Xbox360NameList::infoList;

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

int getResolutionSelectId(cocos2d::Size resolutionSize);
class changeStringList {
public:
	static void clear() {
		infoList.clear();
	}
	static void refresh() {
		clear();
		list();
	}
	static std::vector<changeStringList> &list() {
		if (infoList.size() == 0) {
			infoList.push_back(changeStringList(GameManager::tr("Up"), GameManager::tr("Up", "KeyNameChange")));
			infoList.push_back(changeStringList(GameManager::tr("Down"), GameManager::tr("Down", "KeyNameChange")));
			infoList.push_back(changeStringList(GameManager::tr("X"), GameManager::tr("X", "KeyNameChange")));
			infoList.push_back(changeStringList(GameManager::tr("Y"), GameManager::tr("Y", "KeyNameChange")));
			infoList.push_back(changeStringList(GameManager::tr("Circle"), GameManager::tr("Circle", "KeyNameChange")));
			infoList.push_back(changeStringList(GameManager::tr("Cross"), GameManager::tr("Cross", "KeyNameChange")));
			infoList.push_back(changeStringList(GameManager::tr("Squarea"), GameManager::tr("Square", "KeyNameChange")));
			infoList.push_back(changeStringList(GameManager::tr("Triangle"), GameManager::tr("Triangle", "KeyNameChange")));
			/*{ "↑", "上" },
			{ "↓", "下" },
			{ "←", "左" },
			{ "→", "右" },
			{ "○", "〇" },
			{ "×", "x" },
			{ "□", "ロ" },
			{ "△", "三角" },*/

		}
		return infoList;
	}
protected:
	changeStringList(const char *_from, const char *_to) {
		from = _from;
		to = _to;
	}
public:
	static std::vector<changeStringList> infoList;
	const char *from;
	const char *to;
};
std::vector<changeStringList> changeStringList::infoList;

//------------------------------------------------------------------------------------------------
DebugDeviceController::DebugDeviceController()
{
	_deviceId = -1;
	_deviceName = nullptr;
	_name = nullptr;
	_buttonActionList = nullptr;
}

DebugDeviceController::~DebugDeviceController()
{
	CC_SAFE_RELEASE_NULL(_deviceName);
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_buttonActionList);
}

bool DebugDeviceController::init(int deviceId)
{
	this->setDeviceId(deviceId);
	if (this->isConnected()) {
		this->setup();
	}
	return true;
}

void DebugDeviceController::update(float delta)
{
	if(this->isConnected()) {
		this->setup();
	}
	else {
		this->reset();
	}
}

void DebugDeviceController::setup()
{
	if (_deviceName != nullptr) {
		return;
	}
	int deviceId = this->getDeviceId();
	auto inputManager = InputManager::getInstance();
	auto gamepad = inputManager->getGamepad(deviceId);

	auto name = gamepad->getName();
	//xbox360
	if (name && strstr(name, "Xbox 360 Controller") != nullptr) {
		this->setName(cocos2d::__String::create("Controller 01"));
	}
	//ps4
	else if (name && strstr(name, "Wireless Controller") != nullptr) {
		this->setName(cocos2d::__String::create("Controller 02"));
	}
	//etc.
	else {
		this->setName(cocos2d::__String::create(name));
	}
	this->setDeviceName(cocos2d::__String::create(name));
	this->setButtonActionList(cocos2d::__Array::create());
}

void DebugDeviceController::reset()
{
	if (_deviceName == nullptr) {
		return;
	}
	this->setDeviceName(nullptr);
	this->setButtonActionList(nullptr);
}

const char *DebugDeviceController::getDeviceName()
{
	CC_ASSERT(_deviceName);
	return _deviceName->getCString();
}

const char *DebugDeviceController::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

bool DebugDeviceController::isConnected()
{
	auto gamepad = InputManager::getInstance()->getGamepad(this->getDeviceId());
	if (gamepad) {
		return gamepad->getConnected();
	}
	return false;
}

//------------------------------------------------------------------------------------------------
int DebugExecuteLogWindow::_lineCount = 0;
DebugExecuteLogWindow::DebugExecuteLogWindow()
{
	_logList = nullptr;
	_actionLogConsoleFilterTextInputFocused = false;
}

DebugExecuteLogWindow::~DebugExecuteLogWindow()
{
	CC_SAFE_RELEASE_NULL(_logList);
}

bool DebugExecuteLogWindow::init()
{
	this->setLogList(cocos2d::__Array::create());
	memset(_filterString, 0, sizeof(_filterString));
	return true;
}

void DebugExecuteLogWindow::draw(bool *bOpenFlag)
{
	auto director = Director::getInstance();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#endif
	CC_ASSERT(glview);
	cocos2d::Size frameSize = glview->getFrameSize();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SetNextWindowSize(ImVec2(420, 200), ImGuiSetCond_Appearing);
#endif
	ImGui::Begin(GameManager::tr("Action Log Console"), bOpenFlag);

	if (ImGui::Button("Clear")) {
		this->clear();
	}

	ImGui::SameLine();
	float width = -10.0f;
	if (width != 0.0f)
		ImGui::PushItemWidth(width);
	ImGui::InputText("", (char *)_filterString, CC_ARRAYSIZE(_filterString));
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	if (ImGui::IsItemActive()) {
		if (!glview->isFocusInIme()) {
			if (!_actionLogConsoleFilterTextInputFocused) {
				_actionLogConsoleFilterTextInputFocused = true;
				glview->onFocusInIme();
			}
		}
	}
	else {
		if (glview->isFocusInIme()) {
			if (_actionLogConsoleFilterTextInputFocused) {
				_actionLogConsoleFilterTextInputFocused = false;
				glview->setImeOpenStatus(false);
				glview->onFocusOutIme();
			}
		}
	}
#endif
	if (width != 0.0f)
		ImGui::PopItemWidth();

	ImGui::Separator();
	ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	cocos2d::Ref *ref = nullptr;
	int line = 0;
	CCARRAY_FOREACH(this->getLogList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto str = static_cast<cocos2d::__String *>(ref);
#else
		auto str = dynamic_cast<cocos2d::__String *>(ref);
#endif
		if (strlen(_filterString) > 0) {
			if (strstr(str->getCString(), _filterString) != nullptr) {
				ImGui::Text(str->getCString());
				line++;
			}
		}
		else {
			ImGui::Text(str->getCString());
			line++;
		}
	}
	if (this->getLogList()) {
		static int lineMax = 0;
		if (line != lineMax) {
			ImGui::SetScrollFromPosY(ImGui::GetCursorScreenPos().y, 0.5f);
			lineMax = line;
		}
	}
	ImGui::EndChild();
	ImGui::End();
}

void DebugExecuteLogWindow::clear()
{
	this->getLogList()->removeAllObjects();
}

void DebugExecuteLogWindow::addLog(const char *fmt, ...)
{
	cocos2d::__String *logString = nullptr;

	va_list ap;
	va_start(ap, fmt);
	{
		char tmp[1024];
		vsnprintf(tmp, CC_ARRAYSIZE(tmp), fmt, ap);
		logString = cocos2d::__String::createWithFormat("%04d %s", ++_lineCount, tmp);
	}
	va_end(ap);

#if defined(AGTK_DEBUG)
	//ログ書き込み。
	GameManager::getInstance()->actionLog(1, logString->getCString());
#endif

#define LOG_LINE_MAX 1000
	if (this->getLogList()->count() > LOG_LINE_MAX) {
		auto obj = this->getLogList()->getObjectAtIndex(0);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		// _tmpString は未使用なので不要
#else
		cocos2d::__String *_tmpString = dynamic_cast<cocos2d::__String *>(obj);
#endif
		CC_ASSERT(obj);
		this->getLogList()->removeObject(obj);
	}

	this->getLogList()->addObject(logString);
}

void DebugExecuteLogWindow::addLog(const std::string log)
{
	this->addLog("%s", log.c_str());
}

//-------------------------------------------------------------------------------------------------------------------
DebugDisplayData::DebugDisplayData()
{
	_shaderList = nullptr;
	_initShaderList = nullptr;
	_tmpShaderList = nullptr;
	_controllerList = nullptr;
	_tmpControllerList = nullptr;
}

DebugDisplayData::~DebugDisplayData()
{
	CC_SAFE_RELEASE_NULL(_shaderList);
	CC_SAFE_RELEASE_NULL(_initShaderList);
	CC_SAFE_RELEASE_NULL(_tmpShaderList);
	CC_SAFE_RELEASE_NULL(_controllerList);
	CC_SAFE_RELEASE_NULL(_tmpControllerList);
}

DebugDisplayData *DebugDisplayData::create(agtk::data::ProjectData *projectData)
{
	auto p = new (std::nothrow) DebugDisplayData();
	if (p && p->init(projectData)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool DebugDisplayData::init(agtk::data::ProjectData *projectData)
{
	auto gameManager = GameManager::getInstance();
	auto scene = gameManager->getCurrentScene();
	if (scene == nullptr) {
		//シーンが存在しない。
		return false;
	}

	int windowMagnification = projectData->getWindowMagnification();
	bool magnifyWindow = projectData->getMagnifyWindow();
	bool fullScreenFlag = projectData->getScreenSettings();
	
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	auto sceneData = scene->getSceneData();
	auto layerDataList = sceneData->getLayerList();
	int layerMax = sceneData->getLayerList()->count();

	//fullScreenFlag
	this->setFullScreenFlag(fullScreenFlag);
	this->setInitFullScreenFlag(projectData->getInitScreenSettings());
	//magnifyWindow
	this->setMagnifyWindow(magnifyWindow);
	this->setInitMagnifyWindow(projectData->getInitMagnifyWindow());
	this->setWindowMagnification(windowMagnification);
	this->setInitWindowMagnification(projectData->getInitWindowMagnification());
	//language
	auto gameInformation = gameManager->getProjectData()->getGameInformation();
	this->setMainLanguageId(gameInformation->getMainLanguageId());
	this->setInitMainLanguageId(gameInformation->getInitMainLanguageId());
	//disable Screen Shake
	this->setDisableScreenShake(false);
	this->setInitDisableScreenShake(false);
	this->setTmpDisableScreenShake(false);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#ifdef USE_PREVIEW
	int layerId = data::SceneData::kTopMostWithMenuLayerId;

	//shaderList
	auto layerList = cocos2d::__Dictionary::create();
	{
		//scene topmost
		auto shaderList = cocos2d::__Dictionary::create();
		for (auto &info : ShaderInfo::list()) {
			auto kind = info.kind;
			auto data = agtk::DebugShaderData::create(layerId, kind, info.name.c_str());

			float value = 0.0f;
			bool flag = projectData->getScreenEffect(data->getId(), &value);
			data->setCheck(flag);
			data->setValue(value);

			shaderList->setObject(data, kind);
		}
		layerList->setObject(shaderList, layerId);
	}
	this->setShaderList(layerList);
	//initShaderList
	layerList = cocos2d::__Dictionary::create();
	{
		//scene topmost
		auto shaderList = cocos2d::__Dictionary::create();
		for (auto &info : ShaderInfo::list()) {
			auto kind = info.kind;
			auto data = agtk::DebugShaderData::create(layerId, kind, info.name.c_str());

			float value = 0.0f;
			bool flag = projectData->getScreenEffect(data->getId(), &value);
			data->setCheck(flag);
			data->setValue(value);

			shaderList->setObject(data, kind);
		}
		layerList->setObject(shaderList, layerId);
	}
	this->setInitShaderList(layerList);
	//tmpShaderList
	layerList = cocos2d::__Dictionary::create();
	{
		//scene topmost
		auto shaderList = cocos2d::__Dictionary::create();
		for (auto &info : ShaderInfo::list()) {
			auto kind = info.kind;
			auto data = agtk::DebugShaderData::create(layerId, kind, info.name.c_str());

			float value = 0.0f;
			bool flag = projectData->getScreenEffect(data->getId(), &value);
			data->setCheck(flag);
			data->setValue(value);

			shaderList->setObject(data, kind);
		}
		layerList->setObject(shaderList, layerId);
	}
	this->setTmpShaderList(layerList);

#endif
	return true;
}

#ifdef USE_PREVIEW
agtk::DebugShaderData *DebugDisplayData::getShaderKind(int layerId, agtk::Shader::ShaderKind kind)
{
	auto layer = dynamic_cast<cocos2d::__Dictionary *>(this->getShaderList()->objectForKey(layerId));
	CC_ASSERT(layer);
	auto value = dynamic_cast<agtk::DebugShaderData *>(layer->objectForKey(kind));
	CC_ASSERT(value);
	return value;
}

void DebugDisplayData::setShaderKind(int layerId, agtk::Shader::ShaderKind kind, float value, bool bIgnored)
{
	auto layer = dynamic_cast<cocos2d::__Dictionary *>(this->getShaderList()->objectForKey(layerId));
	CC_ASSERT(layer);
	auto p = dynamic_cast<agtk::DebugShaderData *>(layer->objectForKey(kind));
	if (p == nullptr) {
		p = agtk::DebugShaderData::create(layerId, kind, getShaderName(kind));
		layer->setObject(p, kind);
	}
	p->setValue(value);
	p->setCheck(bIgnored);
}

agtk::DebugShaderData *DebugDisplayData::getInitShaderKind(int layerId, agtk::Shader::ShaderKind kind)
{
	auto layer = dynamic_cast<cocos2d::__Dictionary *>(this->getInitShaderList()->objectForKey(layerId));
	CC_ASSERT(layer);
	auto value = dynamic_cast<agtk::DebugShaderData *>(layer->objectForKey(kind));
	CC_ASSERT(value);
	return value;
}

void DebugDisplayData::setInitShaderKind(int layerId, agtk::Shader::ShaderKind kind, float value, bool bIgnored)
{
	auto layer = dynamic_cast<cocos2d::__Dictionary *>(this->getInitShaderList()->objectForKey(layerId));
	CC_ASSERT(layer);
	auto p = dynamic_cast<agtk::DebugShaderData *>(layer->objectForKey(kind));
	if (p == nullptr) {
		p = agtk::DebugShaderData::create(layerId, kind, getShaderName(kind));
		layer->setObject(p, kind);
	}
	p->setValue(value);
	p->setCheck(bIgnored);
}

agtk::DebugShaderData *DebugDisplayData::getTmpShaderKind(int layerId, agtk::Shader::ShaderKind kind)
{
	auto layer = dynamic_cast<cocos2d::__Dictionary *>(this->getTmpShaderList()->objectForKey(layerId));
	CC_ASSERT(layer);
	auto value = dynamic_cast<agtk::DebugShaderData *>(layer->objectForKey(kind));
	CC_ASSERT(value);
	return value;
}

void DebugDisplayData::setTmpShaderKind(int layerId, agtk::Shader::ShaderKind kind, float value, bool bIgnored)
{
	auto layer = dynamic_cast<cocos2d::__Dictionary *>(this->getTmpShaderList()->objectForKey(layerId));
	CC_ASSERT(layer);
	auto p = dynamic_cast<agtk::DebugShaderData *>(layer->objectForKey(kind));
	if (p == nullptr) {
		p = agtk::DebugShaderData::create(layerId, kind, getShaderName(kind));
		layer->setObject(p, kind);
	}
	p->setValue(value);
	p->setCheck(bIgnored);
}
#endif

//-------------------------------------------------------------------------------------------------------------------
DebugSoundData::DebugSoundData()
{
}

DebugSoundData *DebugSoundData::create(float initBgmVolume, float initSeVolume, float initVoiceVolume, float bgmVolume, float seVolume, float voiceVolume)
{
	auto p = new (std::nothrow) DebugSoundData();
	if (p && p->init(initBgmVolume, initSeVolume, initVoiceVolume, bgmVolume, seVolume, voiceVolume)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool DebugSoundData::init(float initBgmVolume, float initSeVolume, float initVoiceVolume, float bgmVolume, float seVolume, float voiceVolume)
{
	CC_ASSERT(0.0f <= initBgmVolume && initBgmVolume <= 100.0f);
	CC_ASSERT(0.0f <= initSeVolume && initSeVolume <= 100.0f);
	CC_ASSERT(0.0f <= initVoiceVolume && initVoiceVolume <= 100.0f);
	this->setInitBgmVolume(initBgmVolume);
	this->setInitSeVolume(initSeVolume);
	this->setInitVoiceVolume(initVoiceVolume);
	this->setBgmVolume(bgmVolume);
	this->setSeVolume(seVolume);
	this->setVoiceVolume(voiceVolume);
	this->setTmpBgmVolume(bgmVolume);
	this->setTmpSeVolume(seVolume);
	this->setTmpVoiceVolume(voiceVolume);
	return true;
}

void DebugSoundData::reset(float initBgmVolume, float initSeVolume, float initVoiceVolume)
{
	CC_ASSERT(0.0f <= initBgmVolume && initBgmVolume <= 100.0f);
	CC_ASSERT(0.0f <= initSeVolume && initSeVolume <= 100.0f);
	CC_ASSERT(0.0f <= initVoiceVolume && initVoiceVolume <= 100.0f);
	this->setInitBgmVolume(initBgmVolume);
	this->setInitSeVolume(initSeVolume);
	this->setInitVoiceVolume(initVoiceVolume);
	this->setBgmVolume(initBgmVolume);
	this->setSeVolume(initSeVolume);
	this->setVoiceVolume(initVoiceVolume);
	this->setTmpBgmVolume(initBgmVolume);
	this->setTmpSeVolume(initSeVolume);
	this->setTmpVoiceVolume(initVoiceVolume);
}

//-------------------------------------------------------------------------------------------------------------------
DebugObjectInfoWindow::DebugObjectInfoWindow()
{
	_object = nullptr;
	_switchList = nullptr;
	_switchListCount = 0;
	_variableList = nullptr;
	_variableListCount = 0;
	_position = cocos2d::Vec2::ZERO;
}

DebugObjectInfoWindow::~DebugObjectInfoWindow()
{
	CC_SAFE_RELEASE_NULL(_object);
	if (_switchList) {
		delete[] _switchList;
		_switchList = nullptr;
	}
	if (_variableList) {
		delete[] _variableList;
		_variableList = nullptr;
	}
}

bool DebugObjectInfoWindow::init(agtk::Object *object, cocos2d::Vec2 pos)
{
	auto playObjectData = object->getPlayObjectData();

	this->setObject(object);
	this->setDisplayFlag(true);

	//switch
	_switchListCount = playObjectData->getSwitchList()->count();
	_switchList = new bool[_switchListCount];
	memset(_switchList, 0, sizeof(*_switchList) * _switchListCount);
	this->setSwitchListFromPlaySwitchData();

	//variable
	_variableListCount = playObjectData->getVariableList()->count();
	_variableList = new double[_variableListCount];
	memset(_variableList, 0, sizeof(*_variableList) * _variableListCount);
	this->setVariableListFromPlayVariableData();

	this->setPosition(pos);
	return true;
}

void DebugObjectInfoWindow::setSwitchListFromPlaySwitchData()
{
	cocos2d::DictElement *el = nullptr;
	auto playObjectData = this->getObject()->getPlayObjectData();
	auto switchDataList = playObjectData->getSwitchList();
	int count = 0;
	CCDICT_FOREACH(switchDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
		_switchList[count++] = p->getValue();
	}
	CC_ASSERT(count == _switchListCount);
}

void DebugObjectInfoWindow::setPlaySwitchDataFromSwitchList()
{
	cocos2d::DictElement *el = nullptr;
	auto playObjectData = this->getObject()->getPlayObjectData();
	auto switchDataList = playObjectData->getSwitchList();
	int count = 0;
	CCDICT_FOREACH(switchDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
		if (_switchList[count] != p->getValue()) {
			p->setValue(_switchList[count]);
		}
		count++;
	}
	CC_ASSERT(count == _switchListCount);
	//データを調整する。
	playObjectData->adjustData();
	//スイッチ・変数更新時のオブジェクト更新処理。
	GameManager::getInstance()->updateObjectVariableAndSwitch();
}

void DebugObjectInfoWindow::setVariableListFromPlayVariableData()
{
	cocos2d::DictElement *el = nullptr;
	auto variableDataList = this->getObject()->getPlayObjectData()->getVariableList();
	int count = 0;
	CCDICT_FOREACH(variableDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
		if (p->getId() == agtk::data::kObjectSystemVariableSingleInstanceID || p->getId() == agtk::data::kObjectSystemVariableInstanceCount) {
			p = this->getObject()->getPlayObjectData()->getVariableData(p->getId());
		}
		_variableList[count++] = p->getValue();
	}
	CC_ASSERT(count == _variableListCount);
}

void DebugObjectInfoWindow::setPlayVariableDataFromVariableList()
{
	cocos2d::DictElement *el = nullptr;
	auto playObjectData = this->getObject()->getPlayObjectData();
	auto variableDataList = playObjectData->getVariableList();
	int count = 0;
	CCDICT_FOREACH(variableDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
		if (p->getId() == agtk::data::kObjectSystemVariableSingleInstanceID || p->getId() == agtk::data::kObjectSystemVariableInstanceCount) {
			p = this->getObject()->getPlayObjectData()->getVariableData(p->getId());
		}
		if (_variableList[count] != p->getValue()) {
			p->setValue(_variableList[count]);
		}
		count++;
	}
	CC_ASSERT(count == _variableListCount);
	//データを調整する。
	playObjectData->adjustData();
	//スイッチ・変数更新時のオブジェクト更新処理。
	GameManager::getInstance()->updateObjectVariableAndSwitch();
}

void DebugObjectInfoWindow::draw()
{
	if (_displayFlag == false) {
		return;
	}

	this->setSwitchListFromPlaySwitchData();
	this->setVariableListFromPlayVariableData();

	auto object = this->getObject();
	auto objectName = object->getObjectData()->getName();
	std::string windowName;
	if (object->getScenePartObjectData()) {
		auto scenePartName = object->getScenePartObjectData()->getName();
		windowName = cocos2d::String::createWithFormat("%s(%s)", scenePartName, objectName)->getCString();
	}
	else {
		windowName = objectName;
	}
	windowName += std::string("##") + std::to_string(object->getInstanceId());

	if (_position != cocos2d::Vec2::ZERO) {
		ImGui::SetNextWindowPos(ImVec2(_position.x, _position.y));
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiSetCond_Appearing);
#endif
	ImGui::Begin(windowName.c_str(), &_displayFlag);

	ImVec2 v = ImGui::GetWindowPos();
	if (this->getPosition() != cocos2d::Vec2(v.x, v.y)) {
		this->setPosition(cocos2d::Vec2(v.x, v.y));
	}

	auto pos = this->getObject()->getDispPosition();
	if (object->getCurrentObjectAction()) {
		ImGui::Text(GameManager::tr("Action Name: %s"), object->getCurrentObjectAction()->getObjectActionData()->getName());
	}
	ImGui::Text(GameManager::tr("Position: (%4.2f, %4.2f)"), pos.x, pos.y);
	auto layerData = object->getSceneLayer()->getLayerData();
	ImGui::Text(GameManager::tr("Layer: %s"), layerData->getName());
	ImGui::Text(GameManager::tr("Instance ID: %d"), object->getInstanceId());
	ImGui::Separator();
	ImGui::Text(GameManager::tr("Switch"));
	ImGui::BeginChild("##Switch", ImVec2(0, 160));
	{
		auto switchList = object->getPlayObjectData()->getSwitchList();
		cocos2d::DictElement *el = nullptr;
		int count = 0;
		CCDICT_FOREACH(switchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
			ImGui::Checkbox(p->getSwitchData()->getName(), &_switchList[count++]);
		}
	}
	ImGui::EndChild();
	ImGui::Separator();
	ImGui::Text(GameManager::tr("Variable"));
	ImGui::BeginChild("##Variable", ImVec2(0, 180));
	{
		auto variableList = object->getPlayObjectData()->getVariableList();
		cocos2d::DictElement *el = nullptr;
		int count = 0;
		CCDICT_FOREACH(variableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
			if (p->getId() == agtk::data::kObjectSystemVariableSingleInstanceID || p->getId() == agtk::data::kObjectSystemVariableInstanceCount) {
				p = object->getPlayObjectData()->getVariableData(p->getId());
			}
			ImGui::InputDouble(p->getVariableData()->getName(), &_variableList[count++], 1.0, 1.0, 2);
		}
	}
	ImGui::EndChild();
	ImGui::Separator();
	if (ImGui::Button(GameManager::tr("Close"))) {
		_displayFlag = false;
//		this->setSwitchListFromPlaySwitchData();
//		this->setVariableListFromPlayVariableData();
	}
//	ImGui::SameLine();
//	if (ImGui::Button("変更を反映する")) {
		this->setPlaySwitchDataFromSwitchList();
		this->setPlayVariableDataFromVariableList();
//	}
	ImGui::End();
}

//-------------------------------------------------------------------------------------------------------------------
DebugResourcesCommonWindow::DebugResourcesCommonWindow()
{
	_displayFlag = false;
	_switchList = nullptr;
	_switchListCount = 0;
	_variableList = nullptr;
	_variableListCount = 0;
}

DebugResourcesCommonWindow::~DebugResourcesCommonWindow()
{
	if (_switchList) {
		delete[] _switchList;
		_switchList = nullptr;
	}
	if (_variableList) {
		delete[] _variableList;
		_variableList = nullptr;
	}
}

bool DebugResourcesCommonWindow::init()
{
	auto projectPlayData = GameManager::getInstance()->getPlayData();

	//switch
	_switchListCount = projectPlayData->getCommonSwitchList()->count();
	_switchList = new bool[_switchListCount];
	memset(_switchList, 0, sizeof(*_switchList) * _switchListCount);
	this->setSwitchListFromPlaySwitchData();

	//variable
	_variableListCount = projectPlayData->getCommonVariableList()->count();
	_variableList = new double[_variableListCount];
	memset(_variableList, 0, sizeof(*_variableList) * _variableListCount);
	this->setVariableListFromPlayVariableData();
	return true;
}

void DebugResourcesCommonWindow::draw()
{
	if (_displayFlag == false) {
		return;
	}
	this->setSwitchListFromPlaySwitchData();
	this->setVariableListFromPlayVariableData();

	auto projectPlayData = GameManager::getInstance()->getPlayData();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SetNextWindowSize(ImVec2(480, 0), ImGuiSetCond_Appearing);
#endif
	ImGui::Begin(GameManager::tr("Data of Common Variables and Switches"), &_displayFlag);
	{
		ImGui::Text(GameManager::tr("Switch"));
		ImGui::BeginChild("##Switch", ImVec2(0, 160));
		if (projectPlayData) {
			auto switchList = projectPlayData->getCommonSwitchList();
			cocos2d::DictElement *el = nullptr;
			int count = 0;
			CCDICT_FOREACH(switchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
				auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
				ImGui::Checkbox(p->getSwitchData()->getName(), &_switchList[count++]);
			}
		}
		ImGui::EndChild();
		ImGui::Separator();
		ImGui::Text(GameManager::tr("Variable"));
		ImGui::BeginChild("##Variable", ImVec2(0, 180));
		if (projectPlayData) {
			auto variableList = projectPlayData->getCommonVariableList();
			cocos2d::DictElement *el = nullptr;
			int count = 0;
			CCDICT_FOREACH(variableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
				auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
				ImGui::InputDouble(p->getVariableData()->getName(), &_variableList[count++], 1.0, 0, 2);
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	this->updatePlayVariableSwitchDataList();
}

void DebugResourcesCommonWindow::setSwitchListFromPlaySwitchData()
{
	cocos2d::DictElement *el = nullptr;
	auto playData = GameManager::getInstance()->getPlayData();
	if (playData) {
		auto switchDataList = playData->getCommonSwitchList();
		int count = 0;
		CCDICT_FOREACH(switchDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
			_switchList[count++] = p->getValue();
		}
		CC_ASSERT(count == _switchListCount);
	}
}

bool DebugResourcesCommonWindow::setPlaySwitchDataFromSwitchList()
{
	bool bUpdate = false;
	cocos2d::DictElement *el = nullptr;
	auto playData = GameManager::getInstance()->getPlayData();
	if (playData) {
		auto switchDataList = playData->getCommonSwitchList();
		int count = 0;
		CCDICT_FOREACH(switchDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
			if (p->getValue() != _switchList[count]) {
				bUpdate = true;
				p->setValue(_switchList[count]);
			}
			count++;
		}
		CC_ASSERT(count == _switchListCount);
	}
	return bUpdate;
}

void DebugResourcesCommonWindow::setVariableListFromPlayVariableData()
{
	cocos2d::DictElement *el = nullptr;
	auto playData = GameManager::getInstance()->getPlayData();
	if (playData) {
		auto variableDataList = playData->getCommonVariableList();
		int count = 0;
		CCDICT_FOREACH(variableDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
			_variableList[count++] = p->getValue();
		}
		CC_ASSERT(count == _variableListCount);
	}
}

bool DebugResourcesCommonWindow::setPlayVariableDataFromVariableList()
{
	bool bUpdate = false;
	cocos2d::DictElement *el = nullptr;
	auto playData = GameManager::getInstance()->getPlayData();
	if (playData) {
		auto variableDataList = playData->getCommonVariableList();
		int count = 0;
		CCDICT_FOREACH(variableDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
			if (p->getValue() != _variableList[count]) {
				bUpdate = true;
				p->setValue(_variableList[count]);
			}
			count++;
		}
		CC_ASSERT(count == _variableListCount);
	}
	return bUpdate;
}

void DebugResourcesCommonWindow::updatePlayVariableSwitchDataList()
{
	bool ret1 = setPlaySwitchDataFromSwitchList();
	bool ret2 = setPlayVariableDataFromVariableList();
	if (ret1 || ret2) {
		//データを調整する。
		auto playData = GameManager::getInstance()->getPlayData();
		if (playData) {
			playData->adjustData();
		}
		//スイッチ・変数更新時のシステム・オブジェクト更新処理。
		GameManager::getInstance()->updateSystemVariableAndSwitch();
		GameManager::getInstance()->updateObjectVariableAndSwitch();
	}
}

//-------------------------------------------------------------------------------------------------------------------
bool DebugPerformanceAndSpeedSettingsWindow::init()
{
	_frameRate = GameManager::getInstance()->getFrameRate();
	_state = kStateScenePlay;
	_displayFlag = false;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	return true;
}

void DebugPerformanceAndSpeedSettingsWindow::draw()
{
	if (!_displayFlag) {
		return;
	}
	auto gameManager = GameManager::getInstance();
	auto director = Director::getInstance();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#endif
	CC_ASSERT(glview);
	cocos2d::Size frameSize = glview->getFrameSize();
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
	float zoom = glview->getFrameZoomFactor();
	frameSize.width = frameSize.width * zoom;
	frameSize.height = frameSize.height * zoom;
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto windowSize = cocos2d::Size(220, 40);
	ImGui::SetNextWindowPos(ImVec2((frameSize.width - windowSize.width) * 0.5f, frameSize.height - windowSize.height - 25));
	ImGui::SetNextWindowSize(ImVec2(windowSize.width, windowSize.height));

	if (!ImGui::Begin(GameManager::tr("Performance/Speed Settings"), &_displayFlag, ImVec2(0, 0), 0.5f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
// #AGTK-WIN
#ifdef USE_30FPS
		if (gameManager->getProjectData()->getMode30Fps()) {
			gameManager->setFrameRate(FRAME30_RATE);
			director->setAnimationInterval(1.0f / FRAME30_RATE);
		}
		else {
			gameManager->setFrameRate(FRAME60_RATE);
			director->setAnimationInterval(1.0f / FRAME60_RATE);
		}
#else
		gameManager->setFrameRate(FRAME60_RATE);
		director->setAnimationInterval(FRAME_PER_SECONDS);
#endif
#endif
		ImGui::End();
		return;
	}

	// 一時停止
	static char *icon[] = { "||", ">" };
	int iconId = _state == kStateScenePlay ? 0 : 1;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	if (ImGui::Button(icon[iconId], ImVec2(32, 24))) {
#endif
		if (_state == kStateScenePlay) {
			_state = kStateScenePause;
		}
		else if (_state == kStateScenePause) {
			_state = kStateScenePlay;
		}
	}
	//コマ送り
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SameLine(0, 4);
	if (ImGui::Button("|>", ImVec2(32, 24))) {
#endif
		_state = kStateSceneFrameByFrame;
	}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	//set frame rate
	ImGui::SameLine(0, 16);
	int frameRate = gameManager->getFrameRate();
	int tmpFrameRate = frameRate;
	std::string str = std::to_string(frameRate);
	ImGui::Text("FPS");
	ImGui::SameLine(0, 8);
	ImGui::PushItemWidth(100);
	ImGui::InputInt("", &tmpFrameRate);
	if (frameRate != tmpFrameRate && tmpFrameRate <= FRAME60_RATE * 2) {
		gameManager->setFrameRate(tmpFrameRate);
		director->setAnimationInterval(1.0f / tmpFrameRate);
	}
#endif

	ImGui::End();
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

//------------------------------------------------------------------------------------------------
DebugManager* DebugManager::_debugManager = nullptr;
DebugManager::DebugManager()
{
	_layer = nullptr;
	_soundData = nullptr;
	_displayData = nullptr;
#ifdef USE_PREVIEW
	_renderTextureDebug = nullptr;
#endif

	_bForcedClosePopupModal = false;
	_bShowMenuBar = false;
	_bShowSoundWindow = false;
	_bShowMethodOfOperationWindow = false;
	_bShowEditOperationWindow = false;
	_bShowGameDisplayWindow = false;
	_bShowGameInformationWindow = false;
	_bShowControllerWindow = false;
#if !defined(USE_RUNTIME)
	_bShowChangeSceneWindow = false;
#endif
	_bShowDebugFrameRate = false;
#if !defined(USE_RUNTIME)
	_fixFramePerSecondFlag = true;
#endif
	_showLoadingScene = false;
#if defined(AGTK_DEBUG)
	_bShowWebSocketWindow = false;
	_bShowMovieWindow = false;
#endif
	_showDebugObjectInfoWindow = false;
	_showDebugNormalSceneObjectInfoFlag = true;
	_showDebugMenuSceneObjectInfoFlag = true;
	memset(_showDebugObjectName, 0, sizeof(_showDebugObjectName));
	_showDebugObjectNameFocused = false;
	_bShowDebugDispGrid = false;
	_running = false;
	_deviceControllerList = nullptr;
#if defined(USE_PREVIEW)
	_bReloadProjectData = false;
#endif
#if defined(AGTK_DEBUG)
	_logList = nullptr;
	_folderList = nullptr;
	onMovieState = nullptr;
#endif
	_debugExecuteLogWindow = nullptr;
	_pause = false;

	_collisionWallEnabled = false;
	_collisionHitEnabled = false;
	_collisionAttackEnabled = false;
	_collisionConnectionEnabled = false;
	_logConsoleDisplayEnabled = false;
	_freeMovingEnabled = false;
	_invincibleModeEnabled = false;
	_frameRateDisplayEnabled = false;
	_skipOneFrameWhenSceneCreatedIgnored = false;
	_debugForDevelopment = false;
	_showPhysicsBoxEnabled = false;
	_showPortalFlag = false;
	_showTileWallFlag = false;
	_showPartOthersFlag = false;
	_showLimitAreaFlag = false;
	_showLimitCameraFlag = false;

	_selectSceneId = 0;

	_selectedDeviceId = 0;//0:選択なし、n>=1:デバイスID
	_selectedEditOperationType = 0;//0:テンプレートから選択 1:コントローラー入力
	_selectedDeviceKeyCodeId = -1;
	_initInputMappingList = nullptr;
	_inputMappingList = nullptr;
	_debugGridLineList = nullptr;
	_objectInfoWindowList = nullptr;
	_debugResourcesCommonWindow = nullptr;

	_selectPhysicsIterationNormal = false;
	_selectPhysicsIterationTwice = false;
	_selectPhysicsIterationThreeTimes = true;
#ifdef USE_PREVIEW
	_appNameVersion = nullptr;
#endif
	_debugPerformanceAndSpeedSettingsWindow = nullptr;
	_debugDisableScreenShake = false;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

DebugManager::~DebugManager()
{
#ifdef USE_PREVIEW
	saveDebugSetting();
#endif
#ifdef USE_RUNTIME
	saveDebugSettingForRuntime();
#endif

	CC_SAFE_RELEASE_NULL(_layer);
	CC_SAFE_RELEASE_NULL(_soundData);
	CC_SAFE_RELEASE_NULL(_displayData);
#ifdef USE_PREVIEW
	CC_SAFE_RELEASE_NULL(_renderTextureDebug);
#endif
	CC_SAFE_RELEASE_NULL(_deviceControllerList);
	CC_SAFE_RELEASE_NULL(_initInputMappingList);
	CC_SAFE_RELEASE_NULL(_inputMappingList);
#if defined(AGTK_DEBUG)
	CC_SAFE_RELEASE_NULL(_logList);
	CC_SAFE_RELEASE_NULL(_folderList);
#endif
	CC_SAFE_RELEASE_NULL(_debugExecuteLogWindow);
	CC_SAFE_RELEASE_NULL(_debugGridLineList);
	CC_SAFE_RELEASE_NULL(_objectInfoWindowList);
	CC_SAFE_RELEASE_NULL(_debugResourcesCommonWindow);
	CC_SAFE_RELEASE_NULL(_debugPerformanceAndSpeedSettingsWindow);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

DebugManager* DebugManager::getInstance()
{
	if (_debugManager == nullptr) {
		_debugManager = new DebugManager();
		bool ret = _debugManager->init();
		CC_ASSERT(ret);
	}
	return _debugManager;
}

void DebugManager::purge()
{
	ShaderInfo::clear();
	PcInputNameList::clear();
	Ps4NameList::clear();
	Xbox360NameList::clear();
	changeStringList::clear();
	if (_debugManager == nullptr) {
		return;
	}
	DebugManager *p = _debugManager;
	_debugManager = nullptr;
	p->release();
}

bool DebugManager::init()
{
	//font
	if (initFont() == false) {
		return false;
	}
	auto director = Director::getInstance();

	//audio
	float bgmVolume = 1.0f;
	float seVolume = 1.0f;
	float voiceVolume = 1.0f;
	auto audioManager = AudioManager::getInstance();
	if (audioManager != nullptr) {
		bgmVolume = audioManager->getBgmVolume();
		seVolume = audioManager->getSeVolume();
		voiceVolume = audioManager->getVoiceVolume();
	}
	auto const * const ssd = GameManager::getInstance()->getProjectData()->getSoundSetting();
	auto soundData = agtk::DebugSoundData::create(ssd->getInitBgmVolume(), ssd->getInitSeVolume(), ssd->getInitVoiceVolume(), bgmVolume * 100.0f, seVolume * 100.0f, voiceVolume * 100.0f);
	if (soundData == nullptr) {
		return false;
	}
	this->setSoundData(soundData);

	auto arr = cocos2d::__Array::create();
	for (int i = 0; i < InputDataRaw::MAX_GAMEPAD; i++) {
		auto data = DebugDeviceController::create(i);
		arr->addObject(data);
	}
	this->setDeviceControllerList(arr);

	//input
	auto initInputMappingList = cocos2d::__Dictionary::create();
	auto inputMapping = GameManager::getInstance()->getProjectData()->getInitInputMapping();
	auto allKeys = inputMapping->getInputMappingList()->allKeys();
	for (int i = 0, max = allKeys->count(); i < max; i++) {
		auto obj = allKeys->getObjectAtIndex(i);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ikey = static_cast<cocos2d::Integer *>(obj);
#else
		auto ikey = dynamic_cast<cocos2d::Integer *>(obj);
#endif
		auto id = ikey->getValue();
		auto p = inputMapping->getInputMapping(id);
		auto data = agtk::data::InputMapping::create(p);
		initInputMappingList->setObject(data, id);
	}
	this->setInitInputMappingList(initInputMappingList);
	this->setInputMappingList(cocos2d::__Dictionary::create());

#if defined(AGTK_DEBUG)
	director->setDisplayStats(true);
#endif
	_bShowDebugFrameRate = director->isDisplayStats();//cocos2d用デバッグ表示

	this->setObjectInfoWindowList(cocos2d::__Array::create());
#if defined(AGTK_DEBUG)
	this->setLogList(cocos2d::__Array::create());
#endif
	this->setDebugExecuteLogWindow(DebugExecuteLogWindow::create());
	_selectSceneId = this->findSelectSceneId("１面");
//	_selectSceneId = this->findSelectSceneId("２面");
//	_selectSceneId = this->findSelectSceneId("基本シーントップビュー外");
//	_freeMovingEnabled = true;
//	this->setShowMenuBar(true);
	auto debugGridLineList = cocos2d::__Array::create();
	if (debugGridLineList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setDebugGridLineList(debugGridLineList);
	this->setDebugResourcesCommonWindow(agtk::DebugResourcesCommonWindow::create());
	this->setDebugPerformanceAndSpeedSettingsWindow(agtk::DebugPerformanceAndSpeedSettingsWindow::create());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	//メニューバー表示有無。
	auto gameManager = GameManager::getInstance();
	auto projectData = gameManager->getProjectData();
	if (projectData) {
		this->setShowMenuBar(projectData->getDisplayMenuBar());
	}
#ifdef USE_PREVIEW
	loadDebugSetting();
#endif
#ifdef USE_RUNTIME
	loadDebugSettingForRuntime();
#endif

	return true;
}

bool DebugManager::initFont()
{
	ImFontConfig config;
	config.OversampleH = 1;
	config.OversampleV = 1;
	config.PixelSnapH = 1;
	ImGuiIO& io = ImGui::GetIO();
	const ImWchar *glyphRanges = glyphRangesJapanese;
	if (GameManager::getInstance()->getEditorLocale() == "zh_CN") {
#ifdef IMGUI_NAV_SUPPORT
		glyphRanges = io.Fonts->GetGlyphRangesChineseFull();
#else
		glyphRanges = io.Fonts->GetGlyphRangesChinese();
#endif
	}
	auto fileUtils = FileUtils::getInstance();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	io.Fonts->AddFontFromFileTTF(fileUtils->getSuitableFOpen(fileUtils->fullPathForFilename(mFontName)).c_str(), 16.0f, &config, glyphRanges);
#endif
	io.Fonts->Build();
	return true;
}

void DebugManager::createDisplayData()
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto windowMagnification = projectData->getWindowMagnification();
	auto magnifyWindow = projectData->getMagnifyWindow();
	auto screenSettings = projectData->getScreenSettings();
	auto displayData = agtk::DebugDisplayData::create(projectData);
	displayData->setDisableScreenShake(_debugDisableScreenShake);
	displayData->setInitDisableScreenShake(_debugDisableScreenShake);
	displayData->setTmpDisableScreenShake(_debugDisableScreenShake);
	this->setDisplayData(displayData);

	auto screenSize = projectData->getScreenSize();
#ifdef USE_PREVIEW
	auto renderTexture = cocos2d::RenderTexture::create(screenSize.width, screenSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
	renderTexture->setContentSize(screenSize);
	this->setRenderTextureDebug(renderTexture);
	renderTexture->getSprite()->getTexture()->setAliasTexParameters();
#endif
}

void DebugManager::removeDisplayData()
{
	auto displayData = this->getDisplayData();
	if (displayData) {
		this->setDebugDisableScreenShake(displayData->getDisableScreenShake());
	}
	this->setDisplayData(nullptr);
#ifdef USE_PREVIEW
	this->setRenderTextureDebug(nullptr);
#endif
}

void DebugManager::showMainMenuBar()
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu(GameManager::tr("Settings"))) {
			if (ImGui::MenuItem(GameManager::tr("Sound"), nullptr, &_bShowSoundWindow)) {
				auto soundData = this->getSoundData();
				if (_bShowSoundWindow) {
					soundData->setTmpBgmVolume(soundData->getBgmVolume());
					soundData->setTmpSeVolume(soundData->getSeVolume());
					soundData->setTmpVoiceVolume(soundData->getVoiceVolume());
				}
				else {
					auto audioManager = AudioManager::getInstance();
					audioManager->setBgmVolume(soundData->getBgmVolume() * 0.01f);
					audioManager->setSeVolume(soundData->getSeVolume() * 0.01f);
					audioManager->setVoiceVolume(soundData->getVoiceVolume() * 0.01f);
				}
			}
			if (ImGui::MenuItem(GameManager::tr("Operation settings"), nullptr)) {
				_bShowMethodOfOperationWindow = true;
				_selectedDeviceId = 0;//0:選択なし、n>=1:デバイスID
				this->getInputMappingList()->removeAllObjects();
				auto inputMapping = GameManager::getInstance()->getProjectData()->getInputMapping();
				auto inputMappingList = inputMapping->getInputMappingList();
				cocos2d::DictElement *el;
				CCDICT_FOREACH(inputMappingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<agtk::data::InputMapping *>(el->getObject());
#else
					auto p = dynamic_cast<agtk::data::InputMapping *>(el->getObject());
#endif
					auto data = agtk::data::InputMapping::create(p);
					this->getInputMappingList()->setObject(data, el->getIntKey());
				}
			}
			if (ImGui::MenuItem(GameManager::tr("Game Screen"), nullptr, &_bShowGameDisplayWindow)) {
				if (_bShowGameDisplayWindow) {
					auto sceneData = GameManager::getInstance()->getCurrentScene()->getSceneData();
					auto layerDataList = sceneData->getLayerList();
					auto displayData = this->getDisplayData();
					displayData->setTmpFullScreenFlag(displayData->getFullScreenFlag());
					displayData->setTmpMagnifyWindow(displayData->getMagnifyWindow());
					displayData->setTmpWindowMagnification(displayData->getWindowMagnification());
					displayData->setTmpMainLanguageId(displayData->getMainLanguageId());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#ifdef USE_PREVIEW
					//scene topmost
					int layerId = data::SceneData::kTopMostWithMenuLayerId;
					for (auto &info : ShaderInfo::list()) {
						auto value = displayData->getShaderKind(layerId, info.kind);
						displayData->setTmpShaderKind(layerId, info.kind, value->getValue(), value->getCheck());
					}
#endif
				}
				else {
#ifdef USE_PREVIEW
					this->resetChangeShader();
#endif
				}
			}

#if defined(AGTK_DEBUG)//websocket
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)

			if (ImGui::MenuItem(GameManager::tr("WebSocket"), nullptr)) {
				_bShowWebSocketWindow = true;
			}
#endif
#endif
#if defined(AGTK_DEBUG)//一時停止
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
			if (ImGui::MenuItem(GameManager::tr("Pause"), nullptr, &_pause)) {
				auto scene = GameManager::getInstance()->getCurrentScene();
				if (scene == nullptr) {
					auto parent = scene->getParent();
					if (this->getPause()) {
						//一時停止。
						this->pause(parent);
					}
					else {
						//再開
						this->resume(parent);
					}
				}
			}
#endif
#endif
#if 0
			if (ImGui::MenuItem("ムービー", nullptr)) {
				_bShowMovieWindow = true;
			}
#endif
#if 0
			if (ImGui::MenuItem("終了", nullptr)) {
				Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
				exit(0);
#endif
			}
#endif
			// ACT2-6469 コントローラー設定メニュー
			if (ImGui::MenuItem(GameManager::tr("Controller settings"), nullptr, &_bShowControllerWindow)) {
				if (_bShowControllerWindow) {
					auto displayData = this->getDisplayData();
					// 現在のコントローラー情報をリストにする
					auto controllerList = cocos2d::__Array::create();
					auto projectPlayData = GameManager::getInstance()->getPlayData();
					for (int id = agtk::data::kProjectSystemVariable1PController; id <= agtk::data::kProjectSystemVariable4PController; id++) {
						auto selectControllerId = projectPlayData->getCommonVariableData(id)->getValue();
						auto p = cocos2d::Integer::create(selectControllerId);
						controllerList->insertObject(p, id-agtk::data::kProjectSystemVariable1PController);
					}
					displayData->setControllerList(controllerList);
					// 同じものをTmpに保存
					displayData->setTmpControllerList(controllerList);
				}
				else {
#ifdef USE_PREVIEW
					this->resetChangeShader();
#endif
				}
			}
			ImGui::EndMenu();
		}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#ifdef USE_PREVIEW
		if (ImGui::BeginMenu(GameManager::tr("Debugging"))) {
			if (ImGui::MenuItem(GameManager::tr("Show TileWall"), nullptr, &_showTileWallFlag)) {
				this->showTileWallView(_showTileWallFlag);
			}
			if (ImGui::MenuItem(GameManager::tr("Show Wall Detection"), nullptr, &_collisionWallEnabled)) {
				this->setVisibleCollisionArea(agtk::data::TimelineInfoData::kTimelineWall, this->getCollisionWallEnabled());
			}
			if (ImGui::MenuItem(GameManager::tr("Show Collision Detection"), nullptr, &_collisionHitEnabled)) {
				this->setVisibleCollisionArea(agtk::data::TimelineInfoData::kTimelineHit, this->getCollisionHitEnabled());
			}
			if (ImGui::MenuItem(GameManager::tr("Show Attack Detection"), nullptr, &_collisionAttackEnabled)) {
				this->setVisibleCollisionArea(agtk::data::TimelineInfoData::kTimelineAttack, this->getCollisionAttackEnabled());
			}
			if (ImGui::MenuItem(GameManager::tr("Show Connection Point"), nullptr, &_collisionConnectionEnabled)) {
				this->setVisibleCollisionArea(agtk::data::TimelineInfoData::kTimelineConnection, this->getCollisionConnectionEnabled());
			}
			if (ImGui::MenuItem(GameManager::tr("Show Runtime Log Console"), nullptr, &_logConsoleDisplayEnabled)) {
				//※ログコンソールを表示する。タイトルバーを消す。
				CCLOG("open execlog");
			}
			if (ImGui::MenuItem(GameManager::tr("Free Movement Mode"), nullptr, &_freeMovingEnabled)) {
				//プレイヤーキャラクターを自由に移動させる事ができる。
				//重力や判定の影響を受けない。
				CCLOG("open free moving");
			}
			if (ImGui::MenuItem(GameManager::tr("Invincible Mode"), nullptr, &_invincibleModeEnabled)) {
				//プレイヤーがダメージを受けない状態で進める事が出来る。
				CCLOG("open Invincible Mode");
			}
			if (ImGui::MenuItem(GameManager::tr("Frame Rate"), nullptr, &_frameRateDisplayEnabled)) {
				CCLOG("open framerate");
			}
			bool bPerformanceAndSpeedSettingsWindow = this->getDebugPerformanceAndSpeedSettingsWindow()->getDisplayFlag();
			if (ImGui::MenuItem(GameManager::tr("Performance/Speed Settings"), nullptr, &bPerformanceAndSpeedSettingsWindow)) {
				this->getDebugPerformanceAndSpeedSettingsWindow()->setDisplayFlag(bPerformanceAndSpeedSettingsWindow);
				if (bPerformanceAndSpeedSettingsWindow == false) {
// #AGTK-WIN
#ifdef USE_30FPS
					if (GameManager::getInstance()->getProjectData()->getMode30Fps()) {
						GameManager::getInstance()->setFrameRate(FRAME30_RATE);
						Director::getInstance()->setAnimationInterval(1.0f / FRAME30_RATE);
					}
					else {
						GameManager::getInstance()->setFrameRate(FRAME60_RATE);
						Director::getInstance()->setAnimationInterval(1.0f / FRAME60_RATE);
					}
#else
					GameManager::getInstance()->setFrameRate(FRAME60_RATE);
					Director::getInstance()->setAnimationInterval(FRAME_PER_SECONDS);
#endif
				}
			}
			if (ImGui::MenuItem(GameManager::tr("Show Debugging for Development"), nullptr, &_debugForDevelopment)) {
				CCLOG("debug for development (on/off): %d", _debugForDevelopment);
			}
			if (ImGui::MenuItem(GameManager::tr("Show physics debug"), nullptr, &_showPhysicsBoxEnabled)) {
				GameManager::getInstance()->setPhysicsDebugVisible(_showPhysicsBoxEnabled);
			}
			if (ImGui::MenuItem(GameManager::tr("Show Debugging for Other Parts"), nullptr, &_showPartOthersFlag)) {
				this->showPartOthersView(_showPartOthersFlag);
			}
			if (ImGui::MenuItem(GameManager::tr("Show portal debug"), nullptr, &_showPortalFlag)) {
				this->showPortalView(_showPortalFlag);
			}
			if (ImGui::MenuItem(GameManager::tr("Show Debugging for Player Move Range Restriction"), nullptr, &_showLimitAreaFlag)) {
				this->showLimitArea(_showLimitAreaFlag);
			}
			if (ImGui::MenuItem(GameManager::tr("Show Debugging for Camera Range Restriction"), nullptr, &_showLimitCameraFlag)) {
				this->showLimitCamera(_showLimitCameraFlag);
			}
			if (ImGui::MenuItem(GameManager::tr("Disable Skip During Scene Generation"), nullptr, &_skipOneFrameWhenSceneCreatedIgnored)) {
				CCLOG("skip 1 frame when the scene created. (%d)", _skipOneFrameWhenSceneCreatedIgnored);
			}
#if !defined(USE_RUNTIME)
			if (ImGui::MenuItem(GameManager::tr("Fixed FPS"), nullptr, &_fixFramePerSecondFlag)) {
				CCLOG(" frame per second fiexed.(%d)", _fixFramePerSecondFlag);
			}
			//シーン切り替え
			if (ImGui::MenuItem(GameManager::tr("Change Scene"), nullptr, &_bShowChangeSceneWindow)) {
				CCLOG("change scene.(%d)", _bShowChangeSceneWindow);
			}
#endif
			if (ImGui::MenuItem(GameManager::tr("Loading Scene"), nullptr, &_showLoadingScene)) {
				CCLOG(" show loading scene (%d)", _showLoadingScene);
			}
			
			ImGui::EndMenu(); 
		}
#endif
		if (ImGui::BeginMenu(GameManager::tr("Game Data"))) {
			if (ImGui::MenuItem(GameManager::tr("Game Data"), nullptr, &_bShowGameInformationWindow)) {}
			ImGui::EndMenu();
		}
#ifdef USE_PREVIEW
		if (ImGui::BeginMenu(GameManager::tr("Debug"))) {
			//デバッグ表示（cocos2d用のデバッグ表示）
			bool bShowFrameRate = _bShowDebugFrameRate;
			ImGui::MenuItem(GameManager::tr("Show Simple Draw Load"), nullptr, &_bShowDebugFrameRate);
			if (bShowFrameRate != _bShowDebugFrameRate) {
				auto director = Director::getInstance();
				director->setDisplayStats(_bShowDebugFrameRate);
			}
			if (ImGui::MenuItem(GameManager::tr("Object Data"), nullptr, &_showDebugObjectInfoWindow)) {
				CCLOG("object information");
			}
			bool bResourcesCommonWindow = this->getDebugResourcesCommonWindow()->getDisplayFlag();
			if (ImGui::MenuItem(GameManager::tr("Data of Common Variables and Switches"), nullptr, &bResourcesCommonWindow)){
				CCLOG("resources common information%d", bResourcesCommonWindow);
				this->getDebugResourcesCommonWindow()->setDisplayFlag(bResourcesCommonWindow);
			}
			if (ImGui::MenuItem(GameManager::tr("Show Grid"), nullptr, &_bShowDebugDispGrid)) {
				CCLOG("display grid");
			}
#ifdef USE_PREVIEW
			if (getAppNameVersion() == nullptr) {
				auto gameManager = GameManager::getInstance();
				setAppNameVersion(cocos2d::String::createWithFormat("%s %s\n%s", gameManager->getAppName()->getCString(), gameManager->getAppVersion()->getCString(), "PREVIEW_PLAYER_VERSION"));
			}
			ImGui::MenuItem(getAppNameVersion()->getCString(), nullptr, nullptr, true);
#endif
			if (ImGui::BeginMenu(GameManager::tr("Iteration Count of Physics Calculation"))) {
				if (ImGui::MenuItem(GameManager::tr("Normal"), nullptr, &_selectPhysicsIterationNormal)) {
					_selectPhysicsIterationTwice = false;
					_selectPhysicsIterationThreeTimes = false;
					GameManager::getInstance()->setPhysicsIterations(DEFAULT_PHYSICS_ITERATION);
				}
				else if (ImGui::MenuItem(GameManager::tr("x2"), nullptr, &_selectPhysicsIterationTwice)) {
					_selectPhysicsIterationNormal = false;
					_selectPhysicsIterationThreeTimes = false;
					GameManager::getInstance()->setPhysicsIterations(DEFAULT_PHYSICS_ITERATION * 2);
				}
				else if (ImGui::MenuItem(GameManager::tr("x3"), nullptr, &_selectPhysicsIterationThreeTimes)) {
					_selectPhysicsIterationNormal = false;
					_selectPhysicsIterationTwice = false;
					GameManager::getInstance()->setPhysicsIterations(DEFAULT_PHYSICS_ITERATION * 3);
				}
				ImGui::EndMenu();
			}
			//プロジェクトデータを再読み込み。
			if (ImGui::MenuItem(GameManager::tr("Reload Project Data"), nullptr, &_bReloadProjectData)) {
				GameManager::getInstance()->restartScene();
				_bReloadProjectData = false;
			}
			ImGui::EndMenu();
		}
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		ImGui::EndMainMenuBar();
	}
}

void DebugManager::showSoundWindow()
{
	if (!_bShowSoundWindow) {
		return;
	}
	//static char *windowName = "サウンド設定";
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiSetCond_Appearing);//※サイズの設定できます。無くても自動補正してくれるようです。
#endif
	ImGui::Begin(GameManager::tr("Sound"), &_bShowSoundWindow);

	auto audioManager = AudioManager::getInstance();
	auto soundData = this->getSoundData();
	float bgmVolume = soundData->getTmpBgmVolume();
	float seVolume = soundData->getTmpSeVolume();
	float voiceVolume = soundData->getTmpVoiceVolume();

	ImGui::Text(GameManager::tr("BGM Volume: "));
	ImGui::SliderFloat("", &bgmVolume, 0.0f, 100.0f, "%.1f%%");
	if (bgmVolume != soundData->getTmpBgmVolume()) {
		soundData->setTmpBgmVolume(bgmVolume);
		audioManager->setBgmVolume(bgmVolume * 0.01f);
	}
	ImGui::Text(GameManager::tr("SE Volume: "));
	ImGui::SliderFloat(" ", &seVolume, 0.0f, 100.0f, "%.1f%%");
	if (seVolume != soundData->getTmpSeVolume()) {
		soundData->setTmpSeVolume(seVolume);
		audioManager->setSeVolume(seVolume * 0.01f);
	}
	ImGui::Text(GameManager::tr("Voice Volume: "));
	ImGui::SliderFloat("  ", &voiceVolume, 0.0f, 100.0f, "%.1f%%");
	if (voiceVolume != soundData->getTmpVoiceVolume()) {
		soundData->setTmpVoiceVolume(voiceVolume);
		audioManager->setSeVolume(voiceVolume * 0.01f);
	}
	ImGui::Separator();
	bool bInitSetting = ImGui::Button(GameManager::tr("Reset to Default"));
	if (bInitSetting) {
		soundData->setTmpBgmVolume(soundData->getInitBgmVolume());
		soundData->setTmpSeVolume(soundData->getInitSeVolume());
		soundData->setTmpVoiceVolume(soundData->getInitVoiceVolume());
		audioManager->setBgmVolume(soundData->getTmpBgmVolume() * 0.01f);
		audioManager->setSeVolume(soundData->getTmpSeVolume() * 0.01f);
		audioManager->setVoiceVolume(soundData->getTmpVoiceVolume() * 0.01f);
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SameLine(280);
#endif
	bool bOk = ImGui::Button(GameManager::tr("OK"));
	if (bOk) {
		soundData->setBgmVolume(soundData->getTmpBgmVolume());
		soundData->setSeVolume(soundData->getTmpSeVolume());
		soundData->setVoiceVolume(soundData->getTmpVoiceVolume());
		audioManager->setBgmVolume(soundData->getBgmVolume() * 0.01f);
		audioManager->setSeVolume(soundData->getSeVolume() * 0.01f);
		audioManager->setVoiceVolume(soundData->getVoiceVolume() * 0.01f);

		auto * const gm = GameManager::getInstance();
		auto * const ss = gm->getProjectData()->getSoundSetting();
		ss->setBgmVolume(soundData->getBgmVolume());
		ss->setSeVolume(soundData->getSeVolume());
		ss->setVoiceVolume(soundData->getVoiceVolume());
		gm->saveConfig();// 設定を保存
	}
	ImGui::SameLine();
	bool bCancel = ImGui::Button(GameManager::tr("Cancel"));
	if (bCancel || _bShowSoundWindow == false) {
		audioManager->setBgmVolume(soundData->getBgmVolume() * 0.01f);
		audioManager->setSeVolume(soundData->getSeVolume() * 0.01f);
		audioManager->setVoiceVolume(soundData->getVoiceVolume() * 0.01f);
	}
	if (bOk || bCancel) {
		_bShowSoundWindow = false;
	}
	ImGui::End();
}

void DebugManager::showMethodOfOperationWindow()
{
	if (!_bShowMethodOfOperationWindow) {
		return;
	}
	auto im = InputManager::getInstance();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SetNextWindowSize(ImVec2(450, 310), ImGuiSetCond_Appearing);
#endif
	ImGui::Begin(GameManager::tr("Operation settings"), &_bShowMethodOfOperationWindow);
	ImGui::Text(GameManager::tr("Control Method Settings"));

	int listCount = 1;
	const char *padName[InputDataRaw::MAX_GAMEPAD + 2];
	padName[0] = GameManager::tr("Select Input Device");
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	//キーボード・マウス
	padName[listCount++] = GameManager::tr("Keyboard/mouse");
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getDeviceControllerList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<DebugDeviceController *>(ref);
#else
		auto data = dynamic_cast<DebugDeviceController *>(ref);
#endif
		CC_ASSERT(data);
		if (data->isConnected() == false) {
			continue;
		}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		padName[listCount++] = (char *)data->getName();
	}
	ImGui::Combo("", &_selectedDeviceId, (const char **)padName, listCount);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	int deviceId = _selectedDeviceId - 2;
#endif
	if (deviceId >= 0) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<DebugDeviceController *>(this->getDeviceControllerList()->getObjectAtIndex(deviceId));
#else
		auto data = dynamic_cast<DebugDeviceController *>(this->getDeviceControllerList()->getObjectAtIndex(deviceId));
#endif
		if (data->isConnected() == false) {
			_selectedDeviceId = 0;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		}
	}

	ImGui::Separator();

	if(_selectedDeviceId > 0) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		ImGui::BeginChild("##scrollingregion", ImVec2(0, 160));
		static int selected_button = -1;
#endif
		auto projectData = GameManager::getInstance()->getProjectData();
		auto inputMapping = projectData->getInputMapping();
		auto inputMappingKeys = inputMapping->getInputMappingList()->allKeys();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(inputMappingKeys, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto id = static_cast<cocos2d::__Integer *>(ref);
#else
			auto id = dynamic_cast<cocos2d::__Integer *>(ref);
#endif
			auto p = inputMapping->getInputMapping(id->getValue());
			auto operationKey = inputMapping->getOperationKey(p->getKeyId());
			if (!operationKey) {
				//プロジェクトデータから削除された操作キー。
				continue;
			}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			ImGui::BeginChild(id->getValue(), ImVec2(120, 20));
			ImGui::Text(operationKey->getName());
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			ImGui::EndChild();
#endif

			if (deviceId >= 0) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				ImGui::SameLine();
#endif
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto data = static_cast<DebugDeviceController *>(this->getDeviceControllerList()->getObjectAtIndex(deviceId));
#else
				auto data = dynamic_cast<DebugDeviceController *>(this->getDeviceControllerList()->getObjectAtIndex(deviceId));
#endif
				const char *name = data->getDeviceName();
				const char *btnName = nullptr;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				//xbox360
				if (name && strstr(name, "Xbox 360 Controller") != nullptr) {
					auto &infoList = Xbox360NameList::list();
					for (unsigned int i = 0; i < infoList.size(); i++) {
						if (infoList[i].keyCode == p->getCustom1Input()) {
							btnName = infoList[i].name.c_str();
							break;
						}
					}
				}
				//ps4
				else if (name && strstr(name, "Wireless Controller") != nullptr) {
					auto &infoList = Ps4NameList::list();
					for (unsigned int i = 0; i < infoList.size(); i++) {
						if (infoList[i].keyCode == p->getCustom2Input()) {
							btnName = infoList[i].name.c_str();
							break;
						}
					}
				}
				//dinput
				else {
					for (int i = 0; i < CC_ARRAYSIZE(g_dinputNameList); i++) {
						if (g_dinputNameList[i].keyCode == p->getDiInput()) {
							btnName = g_dinputNameList[i].name;
							break;
						}
					}
				}
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				ImGui::BeginChild(id->getValue() + inputMapping->getInputMappingCount(), ImVec2(200, 22));
#endif

				//文字列がない場合
				char *emptyStr = "";
				if (btnName == nullptr) btnName = emptyStr;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				if (ImGui::Button(btnName, ImVec2(200, 20))) {
#endif
					selected_button = id->getValue();
					_bShowEditOperationWindow = true;
					_selectedEditOperationType = 1;//0:テンプレートから選択 1:コントローラー入力

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto data = static_cast<DebugDeviceController *>(this->getDeviceControllerList()->getObjectAtIndex(deviceId));
#else
					auto data = dynamic_cast<DebugDeviceController *>(this->getDeviceControllerList()->getObjectAtIndex(deviceId));
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
					const char *name = data->getDeviceName();
					//xbox360
					if (name && strstr(name, "Xbox 360 Controller") != nullptr) {
						auto &infoList = Xbox360NameList::list();
						for (unsigned int i = 0; i < infoList.size(); i++) {
							if (infoList[i].keyCode == p->getCustom1Input()) {
								_selectedDeviceKeyCodeId = i;
								break;
							}
						}
					}
					//ps4
					else if (name && strstr(name, "Wireless Controller") != nullptr) {
						auto &infoList = Ps4NameList::list();
						for (unsigned int i = 0; i < infoList.size(); i++) {
							if (infoList[i].keyCode == p->getCustom2Input()) {
								_selectedDeviceKeyCodeId = i;
								break;
							}
						}
					}
					//dinput
					else {
						for (int i = 0; i < CC_ARRAYSIZE(g_dinputNameList); i++) {
							if (g_dinputNameList[i].keyCode == p->getDiInput()) {
								_selectedDeviceKeyCodeId = i;
								break;
							}
						}
					}
#endif
				}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				ImGui::EndChild();
#endif
			}
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
			//キーボード・マウス
			else {
				ImGui::SameLine();
				auto name = InputController::getKeyCodeNamePc(p->getPcInput());
				ImGui::BeginChild(id->getValue() + inputMapping->getInputMappingCount(), ImVec2(200, 22));
				if (ImGui::Button(name, ImVec2(200, 20))) {
					selected_button = id->getValue();
					_bShowEditOperationWindow = true;
					_selectedEditOperationType = 1;//0:テンプレートから選択 1:コントローラー入力
					_selectedDeviceKeyCodeId = 0;
					auto &infoList = PcInputNameList::list();
					for (unsigned int i = 0; i < infoList.size(); i++) {
						if (infoList[i].keyCode == p->getPcInput()) {
							_selectedDeviceKeyCodeId = i;
							break;
						}
					}
				}
				ImGui::EndChild();
			}
#endif
		}
		if (selected_button >= 0) {
			bool ret = this->showEditOperationWindow(selected_button);
			if (ret == false) selected_button = -1;
		}
		ImGui::EndChild();
	}
	else {
		ImGui::BeginChild("##empty", ImVec2(0, 160));
		ImGui::EndChild();
	}

	ImGui::Separator();
	if(ImGui::Button(GameManager::tr("Reset to Default"))){
		auto inputMapping = GameManager::getInstance()->getProjectData()->getInputMapping();
		auto inputMappingKeys = inputMapping->getInputMappingList()->allKeys();
		auto initInputMappingList = this->getInitInputMappingList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(inputMappingKeys, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto id = static_cast<cocos2d::__Integer *>(ref);
#else
			auto id = dynamic_cast<cocos2d::__Integer *>(ref);
#endif
			auto dstInputMapping = inputMapping->getInputMapping(id->getValue());
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto srcInputMapping = static_cast<agtk::data::InputMapping *>(initInputMappingList->objectForKey(id->getValue()));
#else
			auto srcInputMapping = dynamic_cast<agtk::data::InputMapping *>(initInputMappingList->objectForKey(id->getValue()));
#endif
			dstInputMapping->copy(srcInputMapping);
		}
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SameLine(320);
#endif
	bool bOk = ImGui::Button(GameManager::tr("OK"));
	if (bOk) {
		GameManager::getInstance()->saveConfig();// 設定を保存
	}
	ImGui::SameLine();
	bool bCancel = ImGui::Button(GameManager::tr("Cancel"));
	if (bCancel) {
		auto inputMapping = GameManager::getInstance()->getProjectData()->getInputMapping();
		auto inputMappingKeys = inputMapping->getInputMappingList()->allKeys();
		auto inputMappingList = this->getInputMappingList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(inputMappingKeys, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto id = static_cast<cocos2d::__Integer *>(ref);
#else
			auto id = dynamic_cast<cocos2d::__Integer *>(ref);
#endif
			auto dstInputMapping = inputMapping->getInputMapping(id->getValue());
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto srcInputMapping = static_cast<agtk::data::InputMapping *>(inputMappingList->objectForKey(id->getValue()));
#else
			auto srcInputMapping = dynamic_cast<agtk::data::InputMapping *>(inputMappingList->objectForKey(id->getValue()));
#endif
			dstInputMapping->copy(srcInputMapping);
		}
	}
	if (bOk || bCancel) {
		_bShowMethodOfOperationWindow = false;
	}
	ImGui::End();
}

bool DebugManager::showEditOperationWindow(int buttonId)
{
	if (!_bShowEditOperationWindow) {
		return false;
	}
	auto inputManager = InputManager::getInstance();
	inputManager->setIgnoreInput(true);

	static char *deviceName[] = {
		"Custom1 Controller",
		"Custom2 Controller",
		"Direct Input",
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		"Keyboard & Mouse",
	};
	enum EnumDeviceType {
		kDeviceTypeNone = -1,//none
		kDeviceTypeXbox360 = 0,//xbox360
		kDeviceTypePs4,//ps4
		kDeviceTypeDirectInput,//direct input
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		kDeviceTypePcInput,//pc input(keyboard & mouse)
	};
	char nameList[32][128];
	for (int i = 0; i < CC_ARRAYSIZE(nameList); i++) {
		memset(nameList[i], 0x00, sizeof(nameList[i]));
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	int deviceId = _selectedDeviceId - 2;
#endif
	char *_nameList[32];
	int nameListCount = 0;
	EnumDeviceType deviceType = kDeviceTypeNone;
	DebugDeviceController *data = nullptr;
	if (deviceId >= 0) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		data = static_cast<DebugDeviceController *>(this->getDeviceControllerList()->getObjectAtIndex(deviceId));
#else
		data = dynamic_cast<DebugDeviceController *>(this->getDeviceControllerList()->getObjectAtIndex(deviceId));
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		const char *name = data->getDeviceName();
		//xbox360
		if (name && strstr(name, "Xbox 360 Controller") != nullptr) {
			deviceType = kDeviceTypeXbox360;
		}
		//ps4
		else if (name && strstr(name, "Wireless Controller") != nullptr) {
			deviceType = kDeviceTypePs4;
		}
		//direct input
		else {
			deviceType = kDeviceTypeDirectInput;
		}
#endif
	}
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
	else {
		//pc input(keyboard & mouse)
		deviceType = kDeviceTypePcInput;
	}
#endif

	ImGui::OpenPopup(deviceName[deviceType]);
	if (ImGui::BeginPopupModal(deviceName[deviceType], &_bShowEditOperationWindow)) {

// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
		static int deviceKeyCode = -1;
		static const char *deviceKeyCodeName = nullptr;
#endif
		ImGui::Text(GameManager::tr("Key settings"));
		if (ImGui::RadioButton(GameManager::tr("Select a template"), &_selectedEditOperationType, 0)) {
			deviceKeyCode = _selectedDeviceKeyCodeId;
			deviceKeyCodeName = nullptr;
		}
		ImGui::SameLine();
		//テンプレートから選択
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		switch (deviceType) {
		case kDeviceTypeXbox360: {
			auto &infoList = Xbox360NameList::list();
			for (unsigned int i = 0; i < infoList.size(); i++) {
				_nameList[i] = (char *)infoList[i].name.c_str();
				nameListCount++;
			}
			break; }
		case kDeviceTypePs4: {
			auto &infoList = Ps4NameList::list();
			for (unsigned int i = 0; i < infoList.size(); i++) {
				_nameList[i] = (char *)infoList[i].name.c_str();
				nameListCount++;
			}
			break; }
		case kDeviceTypeDirectInput: {
			for (int i = 0; i < CC_ARRAYSIZE(g_dinputNameList); i++) {
				_nameList[i] = (char *)g_dinputNameList[i].name;
				nameListCount++;
			}
			break; }
		case kDeviceTypePcInput: {
			auto reservedKeyCodePcList = inputManager->getController(0)->getReservedKeyCodePcBase();
			cocos2d::DictElement *el;
			CCDICT_FOREACH(reservedKeyCodePcList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto data = static_cast<InputController::ReservedKeyCodeData *>(el->getObject());
#else
				auto data = dynamic_cast<InputController::ReservedKeyCodeData *>(el->getObject());
#endif
				auto name = InputController::getKeyCodeNamePc(data->getId());
				_nameList[nameListCount] = (char *)name;
				nameListCount++;
			}
			break; }
		default:
			CC_ASSERT(0);
		}
#endif
		if (ImGui::Combo("", &_selectedDeviceKeyCodeId, (const char **)_nameList, nameListCount) && _selectedEditOperationType == 0) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			switch (deviceType) {
			case kDeviceTypeXbox360://xbox360
				deviceKeyCode = Xbox360NameList::list()[_selectedDeviceKeyCodeId].keyCode;
				break;
			case kDeviceTypePs4://ps4
				deviceKeyCode = Ps4NameList::list()[_selectedDeviceKeyCodeId].keyCode;
				break;
			case kDeviceTypeDirectInput://direct input
				deviceKeyCode = g_dinputNameList[_selectedDeviceKeyCodeId].keyCode;
				break;
			case kDeviceTypePcInput: {//pc input
				auto reservedKeyCodePcList = inputManager->getController(0)->getReservedKeyCodePcBase();
				cocos2d::DictElement *el;
				int count = 0;
				CCDICT_FOREACH(reservedKeyCodePcList, el) {
					if (count == _selectedDeviceKeyCodeId) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto data = static_cast<InputController::ReservedKeyCodeData *>(el->getObject());
#else
						auto data = dynamic_cast<InputController::ReservedKeyCodeData *>(el->getObject());
#endif
						deviceKeyCode = data->getId();
						break;
					}
					count++;
				}
				break; }
			default:CC_ASSERT(0);
			}
#endif
		}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		if (ImGui::RadioButton((deviceType == 0 || deviceType == 1) ? GameManager::tr("Controller input") : GameManager::tr("Key input"), &_selectedEditOperationType, 1)) {
#endif
			deviceKeyCode = -1;
			deviceKeyCodeName = nullptr;
		}
		deviceKeyCodeName = nullptr;
		if (data) {
			ImGui::SameLine();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			auto gamepad = inputManager->getGamepad(data->getDeviceId());
			auto inputList = gamepad->getInputList();
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(inputList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<InputGamepadData::InputData *>(el->getObject());
#else
				auto p = dynamic_cast<InputGamepadData::InputData *>(el->getObject());
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				if (p->getTrigger() || p->getPress()) {
#endif
					int key = el->getIntKey();
					//入力ID
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
					if (deviceType == kDeviceTypeXbox360) {
						if (key == 18 || key == 19 || key == 20 || key == 21) {//右スティック（X,Y） or 左スティック（X,Y）
							continue;
						}
					}
					if (deviceType == kDeviceTypePs4) {
						if (key == 18 || key == 19 || key == 20 || key == 23) {//右スティック（X,Y） or 左スティック（X,Y）
							continue;
						}
					}
#endif
					deviceKeyCode = key;
					break;
				}
			}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

			//入力受け付け
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			switch (deviceType) {
			case kDeviceTypeXbox360: {//xbox360
				auto &infoList = Xbox360NameList::list();
				for (unsigned int i = 0; i < infoList.size(); i++) {
					if (infoList[i].keyCode == deviceKeyCode) {
						deviceKeyCodeName = infoList[i].name.c_str();
						break;
					}
				}
				if (deviceKeyCodeName == nullptr) {
					deviceKeyCode = -1;
				}
 				break; }
			case kDeviceTypePs4: {//ps4
				auto &infoList = Ps4NameList::list();
				for (unsigned int i = 0; i < infoList.size(); i++) {
					if (infoList[i].keyCode == deviceKeyCode) {
						deviceKeyCodeName = infoList[i].name.c_str();
						break;
					}
				}
				if (deviceKeyCodeName == nullptr) {
					deviceKeyCode = -1;
				}
				break; }
			case kDeviceTypeDirectInput: {//dinput
				for (int i = 0; i < CC_ARRAYSIZE(g_dinputNameList); i++) {
					if (g_dinputNameList[i].keyCode == deviceKeyCode) {
						deviceKeyCodeName = g_dinputNameList[i].name;
						break;
					}
				}
				if (deviceKeyCodeName == nullptr) {
					deviceKeyCode = -1;
				}
				break; }
			default: CC_ASSERT(0);
			}
#endif

			if (deviceKeyCode < 0 || _selectedEditOperationType == false) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				if (ImGui::Button((deviceType == 0 || deviceType == 1) ? GameManager::tr("Press a controller button...") : GameManager::tr("Press a key..."))) {
				}
#endif
			}
			else {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				if (ImGui::Button(deviceKeyCodeName, ImVec2(200, 18))) {
#endif
					deviceKeyCode = -1;
					deviceKeyCodeName = nullptr;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
				}
			}
			if (ImGui::RadioButton(GameManager::tr("None"), &_selectedEditOperationType, 2)) {
				deviceKeyCode = -1;
				deviceKeyCodeName = nullptr;
			}
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), GameManager::tr("Click to reassign"));
#endif
		}
		else {
			ImGui::SameLine();
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
			auto inputDataList = inputManager->getController(0)->getInputKeyboardData()->getInputList();
			cocos2d::DictElement *el;
			CCDICT_FOREACH(inputDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto inputData = static_cast<InputKeyboardData::InputData *>(el->getObject());
#else
				auto inputData = dynamic_cast<InputKeyboardData::InputData *>(el->getObject());
#endif
				if (inputData->getPress() && inputData->getKeyCode() > 0) {
					int const kc = InputKeyboardData::upperKeyCode( inputData->getKeyCode() ) + InputController::kReservedKeyCodePc_Max;
					deviceKeyCodeName = (char *)InputController::getKeyCodeNamePc( kc );
					deviceKeyCode = kc ;
					break;
				}
			}
			if (deviceKeyCode != -1) {
				deviceKeyCodeName = (char *)InputController::getKeyCodeNamePc(deviceKeyCode);
			}
			else {
				auto reservedKeyCodePc = inputManager->getController(0)->getReservedKeyCodePc();
				cocos2d::DictElement *el;
				CCDICT_FOREACH(reservedKeyCodePc, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto data = static_cast<InputController::ReservedKeyCodeData *>(el->getObject());
#else
					auto data = dynamic_cast<InputController::ReservedKeyCodeData *>(el->getObject());
#endif
					if (deviceKeyCode >= 0 && data->getKeyCode() == deviceKeyCode) {
						deviceKeyCodeName = (char *)InputController::getKeyCodeNamePc(data->getKeyCode());
						break;
					}
				}
			}
			if (deviceKeyCode < 0 || _selectedEditOperationType == false) {
				if (ImGui::Button(GameManager::tr("Press a key..."))) {
				}
			}
			else {
				if (deviceKeyCodeName != nullptr) {
					if (ImGui::Button(deviceKeyCodeName, ImVec2(200, 18))) {
						deviceKeyCode = -1;
						deviceKeyCodeName = nullptr;
					}
				}
			}
#endif
			if (ImGui::RadioButton(GameManager::tr("None"), &_selectedEditOperationType, 2)) {
				deviceKeyCode = -1;
				deviceKeyCodeName = nullptr;
			}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), GameManager::tr("Press a key to reassign"));
#endif
		}
		ImGui::Separator();

		bool bOk = ImGui::Button(GameManager::tr("OK"));
		if (bOk) {
			auto inputMapping = GameManager::getInstance()->getProjectData()->getInputMapping();
			auto inputMappingData = inputMapping->getInputMapping(buttonId);
			switch (deviceType) {
			case kDeviceTypeXbox360:
				inputMappingData->setCustom1Input(deviceKeyCode);
				break;
			case kDeviceTypePs4:
				inputMappingData->setCustom2Input(deviceKeyCode);
				break;
			case kDeviceTypeDirectInput:
				inputMappingData->setDiInput(deviceKeyCode);
				break;
			case kDeviceTypePcInput:
				inputMappingData->setPcInput(deviceKeyCode);
				break;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			default:CC_ASSERT(0);
			}
		}
		ImGui::SameLine();
		bool bCancel = ImGui::Button(GameManager::tr("Cancel"));
		if (bOk || bCancel) {
			_bShowEditOperationWindow = false;
			deviceKeyCode = -1;
			deviceKeyCodeName = nullptr;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	//ImGui::End();

	inputManager->setIgnoreInput(_bShowEditOperationWindow);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	return _bShowEditOperationWindow;
}

bool IsFullScreen()
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#endif
	CC_ASSERT(glview);
	return glview->isFullScreen();
}

void ChangeScreen(bool bFullScreen)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#endif
	CC_ASSERT(glview);
	auto project = GameManager::getInstance()->getProjectData();
	auto screenSize = project->getScreenSize();
	glview->setFullScreen(bFullScreen, screenSize.width, screenSize.height);
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

void RestoreScreen()
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#endif
	CC_ASSERT(glview);
	glview->restoreScreen();
}

void FocusWindow()
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#endif
	CC_ASSERT(glview);
	glview->focusWindow();
}

cocos2d::Size GetScreenResolutionSize()
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#endif
	CC_ASSERT(glview);
	return glview->getDesignResolutionSize();
}

cocos2d::Size GetFrameSize()
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#endif
	CC_ASSERT(glview);
	return glview->getFrameSize();
}

void SetCursorVisible(bool bVisible)
{
	//cursor
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(Director::getInstance()->getOpenGLView());
#endif
	CC_ASSERT(glview);
	glview->setCursorVisible(bVisible);
}

void ChangeScreenResolutionSize(cocos2d::Size designResolutionSize, float magnifyWindow)
{
	auto director = Director::getInstance();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#endif
	CC_ASSERT(glview);
	cocos2d::Size resolutionSize = glview->getDesignResolutionSize();
//	cocos2d::Size designResolutionSize = cocos2d::Size(g_resolution[_selectedResolutionId].width, g_resolution[_selectedResolutionId].height);
	cocos2d::Size frameSize = glview->getFrameSize();

	//サイズと拡大率を同時に変える。
	glview->setFrameSizeAndZoomFactor(designResolutionSize.width, designResolutionSize.height, magnifyWindow);
	glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::SHOW_ALL);

	float frameAspect = frameSize.width / frameSize.height;
	float designAspect = designResolutionSize.width / designResolutionSize.height;
	float scaleFactor = 1.0f;
	//※デザインサイズを変更すると表示がおかしくなる。
	//glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::SHOW_ALL);
	director->setContentScaleFactor(scaleFactor);

	CCLOG("frameSize            ( %f, %f)", frameSize.width, frameSize.height);
	CCLOG("frameAspect            %f", frameAspect);
	CCLOG("designResolutionSize ( %f, %f )", designResolutionSize.width, designResolutionSize.height);
	CCLOG("designAspect           %f", designAspect);
	CCLOG("setContentScaleFactor( %f )", scaleFactor);
}

void DebugManager::showGameDisplayWindow()
{
	if (!_bShowGameDisplayWindow) {
		return;
	}
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
#if defined(USE_RUNTIME)
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SetNextWindowSize(ImVec2(320, 200), ImGuiSetCond_Appearing);
#endif
#else
#ifdef IMGUI_NAV_SUPPORT
	ImGui::SetNextWindowSize(ImVec2(650, 460), ImGuiCond_Appearing);
#else
	ImGui::SetNextWindowSize(ImVec2(650, 460), ImGuiSetCond_Appearing);
#endif
#endif
	ImGui::Begin(GameManager::tr("Game screen settings"), &_bShowGameDisplayWindow);
	auto displayData = this->getDisplayData();
	auto sceneData = scene->getSceneData();
	int layerMax = sceneData->getLayerList()->count();
	auto layerList = sceneData->getLayerList();

	std::function<void(bool, int)> ChangeMagnifyWindow = [&](bool magnifyWindow, int windowMagnification) {
		auto projectData = GameManager::getInstance()->getProjectData();
		auto screenSize = projectData->getScreenSize();
		float magnify = magnifyWindow ? windowMagnification : 1.0f;
		auto view = Director::getInstance()->getOpenGLView();
		if (view->getFrameZoomFactor() != magnify) {
			agtk::ChangeScreenResolutionSize(screenSize, magnify);
		}
	};

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	//ウインドウ表示・フルスクリーン
	bool bReplaceFullScreenForWindow = false;
	int fullScreen = (int)displayData->getTmpFullScreenFlag();
	ImGui::RadioButton(GameManager::tr("Window"), &fullScreen, 0);
	ImGui::RadioButton(GameManager::tr("Full screen"), &fullScreen, 1);
	if (displayData->getTmpFullScreenFlag() != (bool)fullScreen) {
		ChangeScreen(fullScreen);
		displayData->setTmpFullScreenFlag(fullScreen);
		bReplaceFullScreenForWindow = !fullScreen;
	}
	//拡大表示
	int windowMagnification = displayData->getTmpWindowMagnification() - 2;
	static char *magnifyWindowSizeName[] = { "x2", "x3", "x4", "x5", "x6", "x7", "x8" };
	bool magnifyWindow = displayData->getTmpMagnifyWindow();
	bool bCheckbox = ImGui::Checkbox(GameManager::tr("Magnify"), &magnifyWindow); ImGui::SameLine();
	if (magnifyWindow != displayData->getTmpMagnifyWindow()) {
		displayData->setTmpMagnifyWindow(magnifyWindow);
	}
	ImGui::BeginChild("MagnifyWindow", ImVec2(200, 28));
	char *magnifyWindowNameList[CC_ARRAYSIZE(magnifyWindowSizeName)];
	for (int i = 0; i < CC_ARRAYSIZE(magnifyWindowSizeName); i++) {
		magnifyWindowNameList[i] = magnifyWindowSizeName[i];
	}
	bool bCombo = ImGui::Combo(" ", &windowMagnification, (const char **)magnifyWindowNameList, CC_ARRAYSIZE(magnifyWindowNameList));
	ImGui::EndChild();
#endif

	//画面振動を無効
	bool disableScreenShake = displayData->getTmpDisableScreenShake();
	bool bCheckbox2 = ImGui::Checkbox(GameManager::tr("Disable Screen Shake"), &disableScreenShake);
	if (displayData->getTmpDisableScreenShake() != disableScreenShake) {
		displayData->setTmpDisableScreenShake(disableScreenShake);
	}

	ImGui::Separator();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	if (bCheckbox || bCheckbox2 || bCombo || bReplaceFullScreenForWindow) {
		displayData->setTmpWindowMagnification(windowMagnification + 2);
		if (displayData->getTmpFullScreenFlag() == false) {//フルスクリーンではない場合。
			ChangeMagnifyWindow(displayData->getTmpMagnifyWindow(), displayData->getTmpWindowMagnification());
		}
	}
#endif

#ifdef USE_PREVIEW
	//画面エフェクト
	ImGui::BeginGroup();
	{
		ImGui::Text(GameManager::tr("Screen effect"));
		ImGui::BeginGroup();
		{
			int checkboxWidth = 130;
			int sliderWidth = 160;
			int layerId = data::SceneData::kTopMostWithMenuLayerId;

			//layer1 base
			auto shaderList = dynamic_cast<cocos2d::__Dictionary *>(displayData->getTmpShaderList()->objectForKey(layerId));
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(shaderList, el) {
				auto shaderData = dynamic_cast<agtk::DebugShaderData *>(el->getObject());
				auto kind = shaderData->getKind();
				auto value = shaderData->getValue();
				bool bCheck = shaderData->getCheck();
				bool ret = ImGui::Checkbox(shaderData->getName(), &bCheck);
				if (ret) {
					//scene topmost
					auto _value = displayData->getTmpShaderKind(layerId, kind);
					_value->setCheck(bCheck);
				}
				ImGui::SameLine(checkboxWidth);
				ImGui::PushItemWidth(sliderWidth);
				char tmpName[16];
				memset(tmpName, 0, sizeof(tmpName));
				for (int j = 0; j < kind; j++) {
					tmpName[j] = ' ';
				}
				ImGui::SliderFloat(tmpName, &value, 0.0f, 100.0f);
				if (value != shaderData->getValue()) {
					//scene topmost
					auto _value = displayData->getTmpShaderKind(layerId, kind);
					_value->setValue(value);
				}
				ImGui::PopItemWidth();
			}
		}
		ImGui::EndGroup();

		ImGui::SameLine();
		//render
		float renderWidth = 280;
		ImGui::BeginGroup();
		auto renderTextureDebug = this->getRenderTextureDebug();
		if (renderTextureDebug) {
			cocos2d::Size sz = renderTextureDebug->getContentSize();
			float r = renderWidth / sz.width;
			ImGui::Image(
				(ImTextureID)renderTextureDebug->getSprite()->getTexture()->getName(),
				ImVec2(sz.width * r, sz.height * r),
				ImVec2(0, 1), ImVec2(1, 0),
				ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128)
			);
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();
#endif

	ImGui::Separator();
	ImGui::BeginGroup();
	//language
	{
		auto projectData = GameManager::getInstance()->getProjectData();
		auto gameInformation = projectData->getGameInformation();
		ImGui::Text(GameManager::tr("Language settings"));
		ImGui::BeginGroup();

		ImGui::Text(GameManager::tr("Language:"));
		ImGui::SameLine();

		int languageCount = gameInformation->getLanguageCount();
		char **languageNameList = new char *[languageCount];
		memset(languageNameList, 0, sizeof(char *) * languageCount);
		for (int i = 0; i < languageCount; i++) {
			auto name = gameInformation->getLanguage(i);
			std::function<char *(const char *)> getLanguageName = [&](const char *localeName)->char *{
				for (int j = 0; j < CC_ARRAYSIZE(gLanguageInfoList); j++) {
					if (strcmp(name, gLanguageInfoList[j].localeName) == 0) {
						return gLanguageInfoList[j].name;
					}
				}
				return nullptr;
			};
			languageNameList[i] = getLanguageName(name);
		}
		int mainLanguageId = displayData->getTmpMainLanguageId();
		bool ret = ImGui::Combo("  ", &mainLanguageId, (const char **)languageNameList, languageCount);
		if (ret) {
			if (mainLanguageId != displayData->getTmpMainLanguageId()) {
				gameInformation->setMainLanguage(cocos2d::__String::create(gameInformation->getLanguage(mainLanguageId)));
				displayData->setTmpMainLanguageId(mainLanguageId);
			}
			JavascriptManager::setLocalePlugins(gameInformation->getMainLanguage());
		}
		delete languageNameList;
		ImGui::EndGroup();
	}
	ImGui::EndGroup();

	ImGui::Separator();
	bool bInitSetting = ImGui::Button(GameManager::tr("Reset to Default"));
	if (bInitSetting) {
		displayData->setTmpFullScreenFlag(displayData->getInitFullScreenFlag());
		displayData->setTmpMagnifyWindow(displayData->getInitMagnifyWindow());
		displayData->setTmpWindowMagnification(displayData->getInitWindowMagnification());
		displayData->setTmpMainLanguageId(displayData->getInitMainLanguageId());
		displayData->setTmpDisableScreenShake(displayData->getInitDisableScreenShake());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#ifdef USE_PREVIEW
		//scene topost
		int layerId = data::SceneData::kTopMostWithMenuLayerId;
		for (auto &info : ShaderInfo::list()) {
			auto kind = info.kind;
			auto value = displayData->getInitShaderKind(layerId, kind);
			displayData->setTmpShaderKind(layerId, kind, value->getValue(), value->getCheck());
		}
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		//screen
		ChangeMagnifyWindow(displayData->getMagnifyWindow(), displayData->getTmpWindowMagnification());
		ChangeScreen(displayData->getTmpFullScreenFlag());
#endif
		//language
		auto gameInformation = GameManager::getInstance()->getProjectData()->getGameInformation();
		gameInformation->setMainLanguage(cocos2d::__String::create(gameInformation->getLanguage(displayData->getTmpMainLanguageId())));
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
#if defined(USE_RUNTIME)
	ImGui::SameLine(190);
#else
	ImGui::SameLine(520);
#endif
#endif
	bool bOk = ImGui::Button(GameManager::tr("OK"));
	if (bOk) {
		displayData->setFullScreenFlag(displayData->getTmpFullScreenFlag());
		displayData->setMagnifyWindow(displayData->getTmpMagnifyWindow());
		displayData->setWindowMagnification(displayData->getTmpWindowMagnification());
		displayData->setMainLanguageId(displayData->getTmpMainLanguageId());
		displayData->setDisableScreenShake(displayData->getTmpDisableScreenShake());
		setDebugDisableScreenShake(displayData->getDisableScreenShake());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#ifdef USE_PREVIEW
		//scene topmost
		int layerId = data::SceneData::kTopMostWithMenuLayerId;
		for (auto &info : ShaderInfo::list()) {
			auto kind = info.kind;
			auto value = displayData->getTmpShaderKind(layerId, kind);
			displayData->setShaderKind(layerId, kind, value->getValue(), value->getCheck());
		}
#endif

		auto gm = GameManager::getInstance();
		auto pd = gm->getProjectData();
		pd->setScreenSettings(displayData->getFullScreenFlag() ? 1 : 0 );
		pd->setMagnifyWindow(displayData->getMagnifyWindow());
		pd->setWindowMagnification(displayData->getWindowMagnification());
		pd->getGameInformation()->setMainLanguage(cocos2d::__String::create(pd->getGameInformation()->getLanguage(displayData->getMainLanguageId())));
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

		gm->saveConfig();// 設定を保存
	}
	ImGui::SameLine();
	bool bCancel = ImGui::Button(GameManager::tr("Cancel"));
	if (bCancel || _bShowGameDisplayWindow == false) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		//screen
		ChangeMagnifyWindow(displayData->getMagnifyWindow(), displayData->getWindowMagnification());
		ChangeScreen(displayData->getFullScreenFlag());
#endif
		//language
		auto gameInformation = GameManager::getInstance()->getProjectData()->getGameInformation();
		gameInformation->setMainLanguage(cocos2d::__String::create(gameInformation->getLanguage(displayData->getMainLanguageId())));

#ifdef USE_PREVIEW
		this->resetChangeShader();
#endif
	}
	if (bOk || bCancel) {
		_bShowGameDisplayWindow = false;
		ImGui::CloseCurrentPopup();
	}
	ImGui::End();
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

void DebugManager::showGameInformationWindow()
{
	if (!_bShowGameInformationWindow) {
		return;
	}
	const char *windowName = GameManager::tr("Game Data");
	ImGui::OpenPopup(windowName);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Appearing);
#endif
	if (ImGui::BeginPopupModal(windowName, &_bShowGameInformationWindow)) {
		auto projectData = GameManager::getInstance()->getProjectData();
		auto gameInformation = projectData->getGameInformation();
		ImGui::Text(GameManager::tr("Title: %s"), gameInformation->getTitle());
		ImGui::Text(GameManager::tr("Creator: %s"), gameInformation->getAuthor());
		ImGui::Text(GameManager::tr("Genre: %s"), gameInformation->getGenre());
		ImGui::Text(GameManager::tr("Content: %s"), gameInformation->getDescription());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		ImGui::EndPopup();
	}
}

#if defined(USE_PREVIEW) && defined(AGTK_DEBUG)
void DebugManager::showWebSocketWindow()
{
	if (!_bShowWebSocketWindow) {
		return;
	}
#ifdef IMGUI_NAV_SUPPORT
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_Appearing);
#else
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiSetCond_Appearing);
#endif
	ImGui::Begin("WebSocket", &_bShowWebSocketWindow);

	auto gameManager = GameManager::getInstance();
	ImGui::InputText("hostname", GameManager::getHostnamePtr(), GameManager::getHostnameSize());
	ImGui::InputInt("port", GameManager::getPortPtr());
	static char buf[64] = "";
	if (ImGui::Button("connect")) {
		gameManager->createWebsocket(GameManager::getHostnamePtr(), *GameManager::getPortPtr());
	}
	ImGui::SameLine();
	if (ImGui::Button("disconnect")) {
		gameManager->removeWebsocket();
	}
	ImGui::SameLine();
	if (ImGui::Button("state")) {
		if (gameManager->getWebSocket()) {
			int state = gameManager->getWebSocket()->getState();
			static char* stateName[] = {
				"CONNECTING",  /** &lt; value 0 */
				"OPEN",        /** &lt; value 1 */
				"CLOSING",     /** &lt; value 2 */
				"CLOSED",      /** &lt; value 3 */
			};
			sprintf(buf, "Websocket state = %d (%s)", state, stateName[state]);
			this->getLogList()->addObject(cocos2d::__String::create(buf));
			memset(buf, 0, sizeof(buf));
		}
		else {
			this->getLogList()->addObject(cocos2d::__String::create("Websocket is null"));
		}
	}
	ImGui::Separator();
#if 1
	ImGui::BeginGroup();
	ImGui::Text("log");// %s", i == 0 ? "Top" : i == 1 ? "25 % " : i == 2 ? "Center" : i == 3 ? "75 % " : "Bottom");
	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)0), ImVec2(ImGui::GetWindowWidth() * 0.8f, 200.0f), true);
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getLogList(), ref) {
		auto str = dynamic_cast<cocos2d::__String *>(ref);
		ImGui::Text(str->getCString());
	}
	static int line = 0;
	if (this->getLogList()) {
		if (line != this->getLogList()->count()) {
			line = this->getLogList()->count();
			ImGui::SetScrollFromPosY(ImGui::GetCursorScreenPos().y, 0.5f);
		}
	}
	ImGui::EndChild();
	ImGui::EndGroup();
#endif
	if (ImGui::Button("clear")) {
		this->getLogList()->removeAllObjects();
	}
	ImGui::Separator();
	ImGui::InputText("message", buf, 64);

	if (ImGui::Button("send text")) {
		if (strlen(buf) > 0) {
			gameManager->sendMessage(buf);
		}
		else {
			ImGui::OpenPopup("Warning");
		}
	}
	if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text(GameManager::tr("No input, so input something."));
		ImGui::Separator();
		if (ImGui::Button(GameManager::tr("OK"), ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::Separator();

	//send binary file.
	static int selected_filename = -1;
	ImGui::Text("binary data");
	if (ImGui::Button("select.."))
		ImGui::OpenPopup("select");
	ImGui::SameLine();
	ImGui::Text(selected_filename == -1 ? "<None>" : dynamic_cast<cocos2d::__String *>(this->getFolderList()->getObjectAtIndex(selected_filename))->getCString());
	if (ImGui::BeginPopup("select")) {
		//int resolution = selected_filename;
		cocos2d::Ref *ref = nullptr;
		for (int i = 0; i < this->getFolderList()->count(); i++) {
			auto str = dynamic_cast<cocos2d::__String *>(this->getFolderList()->getObjectAtIndex(i));
			if (ImGui::Selectable(str->getCString())) {
				selected_filename = i;
			}
		}
		ImGui::EndPopup();
	}
	if (ImGui::Button("send binary")) {
		if (selected_filename > -1) {
			auto str = dynamic_cast<cocos2d::__String *>(this->getFolderList()->getObjectAtIndex(selected_filename));
			std::string tmp = str->getCString();
			int offset = tmp.find(":");
			tmp = tmp.substr(0, offset);
			//				auto data = cocos2d::FileUtils::getInstance()->getDataFromFile(std::string("test_png\\") + std::string(str->getCString()));
			auto data = cocos2d::FileUtils::getInstance()->getDataFromFile(std::string("test_png\\") + std::string(tmp));
			gameManager->sendBinaryMessage(data.getBytes(), data.getSize());
			char tmp_char[128];
			sprintf(tmp_char, "SEND BINARY --------------");
			this->getLogList()->addObject(cocos2d::__String::create(tmp_char));
			this->getLogList()->addObject(cocos2d::__String::create(str->getCString()));
		}
	}

	ImGui::End();
}
#endif

void DebugManager::showExecuteLogWindow()
{
	if (!_logConsoleDisplayEnabled) {
		return;
	}
	this->getDebugExecuteLogWindow()->draw(&_logConsoleDisplayEnabled);
}

void DebugManager::showFramerateWindow()
{
#ifdef USE_COLLISION_MEASURE
	//壁判定回数、当たり判定回数を表示する。
	{
		auto director = Director::getInstance();

		auto glview = dynamic_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
		CC_ASSERT(glview);
		cocos2d::Size frameSize = glview->getFrameSize();
		auto windowSize = cocos2d::Size(800, 30);
		ImGui::SetNextWindowPos(ImVec2(0*(frameSize.width - windowSize.width) * 0.5f, 25));
		ImGui::SetNextWindowSize(ImVec2(windowSize.width, windowSize.height));

		static bool enabled = true;
		if (!ImGui::Begin("Metrics", &enabled, ImVec2(0, 0), 0.5f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::End();
			return;
		}

		//ImGui::SameLine(24);
		//ImGui::Text("Wall:%5d, Hit: %5d, Atk: %5d, Conn: %5d, woConn: %5d", wallCollisionCount, hitCollisionCount, attackCollisionCount, connectionCollisionCount, woConnectionCollisionCount);
		ImGui::Text("Wall:%5d, Hit:%5d, Atk:%5d, Conn:%5d, woConn:%5d, call:%5d, noInfo:%d, cached:%d, rwall:%d, rhit:%d", wallCollisionCount, hitCollisionCount, attackCollisionCount, connectionCollisionCount, woConnectionCollisionCount, callCount, noInfoCount, cachedCount, roughWallCollisionCount, roughHitCollisionCount);
		ImGui::End();
	}
#endif
	auto director = Director::getInstance();
	static float prevDeltaTime = 0.016f;
	static const float FPS_FILTER = 0.10f;

	float deltaTime = director->getDeltaTime();
	float dt = deltaTime * FPS_FILTER + (1 - FPS_FILTER) * prevDeltaTime;
	prevDeltaTime = dt;
	float frameRate = 1.0f / dt;

	if (!_frameRateDisplayEnabled) {
		return;
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto glview = static_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#else
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#endif
	CC_ASSERT(glview);
	cocos2d::Size frameSize = glview->getFrameSize();
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
	float zoom = glview->getFrameZoomFactor();
	frameSize.width *= zoom;
	frameSize.height *= zoom;
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto windowSize = cocos2d::Size(80, 30);
	ImGui::SetNextWindowPos(ImVec2((frameSize.width - windowSize.width) * 0.5f, 25));
#endif
	ImGui::SetNextWindowSize(ImVec2(windowSize.width, windowSize.height));

#ifdef IMGUI_NAV_SUPPORT
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	if (!ImGui::Begin(GameManager::tr("Frame Rate"), &_frameRateDisplayEnabled, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
#endif
#else
	if (!ImGui::Begin(GameManager::tr("Frame Rate"), &_frameRateDisplayEnabled, ImVec2(0, 0), 0.5f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
#endif
		ImGui::End();
		return;
	}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	ImGui::SameLine(24);
#endif
	ImGui::Text("%.1f fps", frameRate);
	ImGui::End();
}

#ifdef USE_PREVIEW
void DebugManager::showInputState()
{
	auto inputManager = InputManager::getInstance();
	const char *label = nullptr;
	if (inputManager->isRecording()) {
		label = "Rec.";
	}
	else if (inputManager->isReplaying()) {
		label = "Replay";
	}
	else {
		return;
	}

	auto director = Director::getInstance();
	auto glview = dynamic_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
	CC_ASSERT(glview);
	cocos2d::Size frameSize = glview->getFrameSize();
	auto windowSize = cocos2d::Size(80, 30);
	auto frameZoomFactor = glview->getFrameZoomFactor();
	ImGui::SetNextWindowPos(ImVec2((frameSize.width * frameZoomFactor - windowSize.width) - 4, (this->getShowMenuBar() ? 22  : 0) + 4));
	ImGui::SetNextWindowSize(ImVec2(windowSize.width, windowSize.height));

	static bool opened = true;
#ifdef IMGUI_NAV_SUPPORT
	if (!ImGui::Begin("Input State", &opened, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
#else
	if (!ImGui::Begin("Input State", &opened, ImVec2(0, 0), 0.5f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
#endif
		ImGui::End();
		return;
	}

	ImGui::SameLine(24);
	ImGui::Text(label);
	ImGui::End();
}
#endif


#if !defined(USE_RUNTIME)
void DebugManager::showChangeSceneWindow()
{
	if (_bShowChangeSceneWindow == false) {
		return;
	}

	auto projectData = GameManager::getInstance()->getProjectData();
	ImGui::Begin(GameManager::tr("Scene Selection"), &_bShowChangeSceneWindow);
	auto sceneList = projectData->getSceneList();
	if (nullptr != sceneList) {
		std::function<int(cocos2d::__Dictionary *)> func = [&](cocos2d::__Dictionary *children) {
			cocos2d::DictElement *el;
			int selectId = -1;
			CCDICT_FOREACH(children, el) {
				auto p = dynamic_cast<agtk::data::SceneData *>(el->getObject());
				if (p->getFolder()) {
					if (ImGui::BeginMenu(p->getName())) {
						if (p->getChildren()) {
							auto id = func(p->getChildren());
							if (id >= 0) {
								ImGui::EndMenu();
								return id;
							}
						}
						ImGui::EndMenu();
					}
				}
				else {
					if (ImGui::MenuItem(p->getName())) {
						selectId = p->getId();
					}
				}
			}
			return selectId;
		};
		auto selectId = func(sceneList);
		if (selectId >= 0) {
			GameManager::getInstance()->startScene(selectId);
		}
	}
	ImGui::End();
}
#endif

#if defined(USE_PREVIEW) && defined(AGTK_DEBUG)
void DebugManager::showMovieWindow()
{
	if (!_bShowMovieWindow) {
		return;
	}
#ifdef IMGUI_NAV_SUPPORT
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Appearing);
#else
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Appearing);
#endif
	ImGui::Begin("Movie", &_bShowMovieWindow);
	{
		ImGui::Text(GameManager::tr("Movie test"));
		if (ImGui::Button(GameManager::tr("Play"))) {
			onMovieState(MovieState::Play);
		}
		ImGui::SameLine();
		if (ImGui::Button(GameManager::tr("Stop"))) {
			onMovieState(MovieState::Stop);
		}
		ImGui::SameLine();
		if (ImGui::Button(GameManager::tr("Pause"))) {
			onMovieState(MovieState::Pause);
		}
	}
	ImGui::End();
}
#endif

void DebugManager::showDebugObjectInfoWindow()
{
	if (!_showDebugObjectInfoWindow) {
		return;
	}
#ifdef IMGUI_NAV_SUPPORT
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Appearing);
#else
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiSetCond_Appearing);
#endif
	ImGui::Begin(GameManager::tr("Object Information"), &_showDebugObjectInfoWindow);
	{
		auto gameManager = GameManager::getInstance();
		auto scene = gameManager->getCurrentScene();
		if (scene == nullptr) {
			ImGui::End();
			return;
		}
		auto placeholderText = GameManager::tr("Input Search Text");
		// ImGui::InputTextWithHint()が使えない。
		ImGui::InputText("##search text", _showDebugObjectName, sizeof(_showDebugObjectName), ImGuiInputTextFlags_AutoSelectAll);
		auto director = Director::getInstance();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto glview = static_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#else
		auto glview = dynamic_cast<IMGUIGLViewImpl *>(director->getOpenGLView());
#endif
		CC_ASSERT(glview);
// #ATGK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
		if (ImGui::IsItemActive()) {
			if (!glview->isFocusInIme()) {
				if (!_showDebugObjectNameFocused) {
					_showDebugObjectNameFocused = true;
					glview->onFocusInIme();
				}
			}
		}
		else {
			if (glview->isFocusInIme()) {
				if (_showDebugObjectNameFocused) {
					_showDebugObjectNameFocused = false;
					glview->setImeOpenStatus(false);
					glview->onFocusOutIme();
				}
			}
		}
#endif
		if (strlen(_showDebugObjectName) == 0) {
			ImGui::SameLine(-1, 16);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			ImGui::Text(placeholderText);
			ImGui::PopStyleVar();
		}
		ImGui::Checkbox(GameManager::tr("Normal Scene"), &_showDebugNormalSceneObjectInfoFlag);
		ImGui::Checkbox(GameManager::tr("Menu Scene"), &_showDebugMenuSceneObjectInfoFlag);
		cocos2d::Array *normalSceneObjectList = nullptr;
		cocos2d::Array *menuSceneObjectList = nullptr;
		if (_showDebugNormalSceneObjectInfoFlag) {
			// シーンレイヤーの動作がOFFの場合は、表示対象としない。
			auto arr = cocos2d::__Array::create();
			cocos2d::DictElement *el = nullptr;
			auto sceneLayerList = scene->getSceneLayerList();
			CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif			
				if (sceneLayer->getObjectList() == nullptr) {
					continue;
				}
				if (sceneLayer->getActiveFlg() == false) {
					continue;
				}
				arr->addObjectsFromArray(sceneLayer->getObjectList());
			}
			normalSceneObjectList = arr;
		}
		if (_showDebugMenuSceneObjectInfoFlag) {
			//メニューレイヤーが非表示であっても破棄されていなければ表示対象とする。
			auto arr = cocos2d::__Array::create();
			cocos2d::DictElement *el = nullptr;
			auto sceneLayerList = scene->getMenuLayerList();
			CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif			
				if (sceneLayer->isMenuObjectStop()) {
					continue;
				}
				if (sceneLayer->getObjectList() == nullptr) {
					continue;
				}
				arr->addObjectsFromArray(sceneLayer->getObjectList());
			}
			menuSceneObjectList = arr;
		}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference();
#else
		auto objectList = scene->getObjectAll();
#endif
		int objectCount = objectList->count();
		ImGui::Text(GameManager::tr("Object Count: %d"), (normalSceneObjectList ? normalSceneObjectList->count() : 0) + (menuSceneObjectList ? menuSceneObjectList->count() : 0));
		ImGui::Separator();
		ImGui::BeginChild("##object list");
		auto objectText = (strlen(_showDebugObjectName) > 0) ? _showDebugObjectName : nullptr;
		auto addImguiChild = [this, scene, objectText](cocos2d::Array *objectList) {
			cocos2d::Ref *ref = nullptr;
			int cnt = 0;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objectData = object->getObjectData();
				auto scenePartData = object->getScenePartObjectData();
				auto name = objectData->getName();
				auto scenePartName = scenePartData != nullptr ? scenePartData->getName() : nullptr;
				if (objectText){
					if (strstr(name, objectText) == nullptr && (scenePartName == nullptr || strstr(scenePartName, objectText) == nullptr)) {
						continue;
					}
				}
				auto physicsNode = object->getphysicsNode();
				auto isFollowPhyisicsNode = physicsNode && objectData->getPhysicsSettingFlag() && objectData->getPhysicsSetting()->getFollowConnectedPhysics();
				cocos2d::Vec2 pos = (isFollowPhyisicsNode ? Scene::getPositionSceneFromCocos2d(physicsNode->getPosition(), scene) : object->getDispPosition());
				if (scenePartName) {
					ImGui::Text(GameManager::tr("Name: %s"), scenePartName);
				}
				ImGui::Text(GameManager::tr("Object Name: %s"), name);
				if (object->getCurrentObjectAction()) {
					ImGui::Text(GameManager::tr("Action Name: %s"), object->getCurrentObjectAction()->getObjectActionData()->getName());
				}
				ImGui::Text(GameManager::tr("Position: (%4.2f, %4.2f)"), pos.x, pos.y);
				auto layerData = object->getSceneLayer()->getLayerData();
				ImGui::Text(GameManager::tr("Layer: %s"), layerData->getName());
				ImGui::Text(GameManager::tr("Instance ID: %d"), object->getInstanceId());
				std::string buttonName = std::string(GameManager::tr("Details")) + std::string("##") + std::to_string(cnt);
				if (ImGui::Button(buttonName.c_str())) {
					this->createObjectInfoWindow(object);
				}
				ImGui::Separator();
				cnt++;
			}
		};
		if (normalSceneObjectList) {
			addImguiChild(normalSceneObjectList);
		}
		if (menuSceneObjectList) {
			addImguiChild(menuSceneObjectList);
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void DebugManager::showResourcesCommonWindow()
{
	this->getDebugResourcesCommonWindow()->draw();
}

void DebugManager::showPerformanceAndSpeedSettingsWindow()
{
	this->getDebugPerformanceAndSpeedSettingsWindow()->draw();
}

void DebugManager::start(cocos2d::Layer *layer)
{
	if (_running) {
		//※とりあえず、シーンを再開した時にendを通す。
		this->end();
	}

	CCIMGUI::getInstance()->addImGUI([&]() {
		this->draw();
	}, "DebugWindowId");

	// スケジューラーを登録する。
	static const char *scheduleName = "checkImGUI";
	cocos2d::Director::getInstance()->getScheduler()->schedule([&](float delta)
	{
		auto director = Director::getInstance();
		if (!director->getRunningScene()->getChildByName(DEBUG_LAYER_NAME))
		{
			director->getRunningScene()->addChild(ImGuiLayer::create(), INT_MAX, DEBUG_LAYER_NAME);
			director->getScheduler()->unschedule(scheduleName, this);
		}
	}, this, 0, false, scheduleName);

	Scheduler *s = Director::getInstance()->getScheduler();
	s->scheduleUpdate(_debugManager, kSchedulePriorityDebugManager, false);

	this->setLayer(layer);
	_running = true;
}

void DebugManager::end()
{
	if (!_running) {
		return;
	}
	auto director = Director::getInstance();

	// スケジューラーを停止する。
	if (director->getRunningScene()->getChildByName(DEBUG_LAYER_NAME)) {
		director->getRunningScene()->removeChildByName(DEBUG_LAYER_NAME, false);
	}
	CCIMGUI::getInstance()->removeImGUI("DebugWindowId");

	Scheduler *s = Director::getInstance()->getScheduler();
	s->unscheduleUpdate(_debugManager);

	_running = false;
}

void DebugManager::reset()
{
	this->getObjectInfoWindowList()->removeAllObjects();

	//ポップアップモーダルが開いている場合は強制的に閉じる。
	if (_bShowGameInformationWindow//ゲーム情報ウインドウ（ポップアップモーダル）が開いていた場合閉じる。
	|| _bShowEditOperationWindow) {//操作変更ウインドウ（ポップアップモーダル）が開いている場合閉じる。
		_bForcedClosePopupModal = true;
	}
	_bShowSoundWindow = false;
	_bShowMethodOfOperationWindow = false;
	_bShowEditOperationWindow = false;
	_bShowGameDisplayWindow = false;
	_bShowGameInformationWindow = false;
	_bShowControllerWindow = false;
#if !defined(USE_RUNTIME)
	_bShowChangeSceneWindow = false;
#endif
#if defined(AGTK_DEBUG)
	_bShowWebSocketWindow = false;
	_bShowMovieWindow = false;
#endif
	_running = false;
#if defined(USE_PREVIEW)
	_bReloadProjectData = false;
#endif
	_pause = false;

	_selectSceneId = 0;

	_selectedDeviceId = 0;//0:選択なし、n>=1:デバイスID
	_selectedEditOperationType = 0;//0:テンプレートから選択 1:コントローラー入力
	_selectedDeviceKeyCodeId = -1;
}

void DebugManager::pause(cocos2d::Node *node)
{
	auto children = node->getChildren();
	for (int i = 0; i < node->getChildrenCount(); i++) {
		auto n = children.at(i);
		n->pause();
		this->pause(n);
	}
}

void DebugManager::resume(cocos2d::Node *node)
{
	auto children = node->getChildren();
	for (int i = 0; i < node->getChildrenCount(); i++) {
		auto n = children.at(i);
		n->resume();
		this->resume(n);
	}
}

void DebugManager::draw()
{
#ifdef AGTK_DEBUG
	//debug
	if (InputManager::getInstance()->isTriggeredKeyboard((int)EventKeyboard::KeyCode::KEY_F4)) {
		if (true) {
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto node = dynamic_cast<cocos2d::Node *>(scene);
			while (node) {
				CCLOG("node: %p", node);
				node = node->getParent();
			}
		}
#if 0
		auto scene = GameManager::getInstance()->getCurrentScene();
		if (scene) {
			// スプライトを生成
			auto sprite = Sprite::create("img/basetile.png");

			// アンカーポイントを設定
			sprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);

			scene->addChild(sprite);
	}
#endif
#if 1
		ScriptingCore::getInstance()->evalString("console.log('Agtk2?: ' + Agtk)");
		auto jsData = FileUtils::getInstance()->getStringFromFile("plugins/test.js");
		//CCLOG("jsData: %s", jsData.c_str());
		ScriptingCore::getInstance()->evalString(jsData.c_str());
#else
		auto jsData = FileUtils::getInstance()->getStringFromFile("C:\\tmp\\20171018\\test.js");
		CCLOG("jsData: %s", jsData.c_str());
		JavascriptManager::getInstance()->evaluateJavaScript(jsData.c_str());
#endif
	}
#endif

#ifdef USE_PREVIEW
	this->showInputState();
#endif
#ifdef IMGUI_NAV_SUPPORT
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.80f;
#else
	ImGui::GetStyle().WindowFillAlphaDefault = 0.80f;
#endif
	if (_bForcedClosePopupModal) {
		ImGui::ForcedClosePopup();
		_bForcedClosePopupModal = false;
	}
	if (this->getShowMenuBar()) this->showMainMenuBar();
	if (this->getShowMenuBar()) this->showSoundWindow();
	if (this->getShowMenuBar()) this->showMethodOfOperationWindow();
	if (this->getShowMenuBar()) this->showGameDisplayWindow();
	if (this->getShowMenuBar()) this->showControllerWindow();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	this->showGameInformationWindow();
	this->showExecuteLogWindow();
	this->showFramerateWindow();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	this->showPerformanceAndSpeedSettingsWindow();
#endif
#if !defined(USE_RUNTIME)
	this->showChangeSceneWindow();
#endif
#if defined(USE_PREVIEW) && defined(AGTK_DEBUG)
	if (this->getShowMenuBar()) this->showWebSocketWindow();
	if (this->getShowMenuBar()) this->showMovieWindow();
	if (show_test_window) {
#ifdef IMGUI_NAV_SUPPORT
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow(&show_test_window);
#else
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
#endif
	}
#endif
#if 0
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow(&show_test_window);
	}
#endif
	this->showDebugObjectInfoWindow();
	this->showObjectInfoWindows();
	this->showResourcesCommonWindow();
#ifdef IMGUI_NAV_SUPPORT
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 1.00f;
#else
	ImGui::GetStyle().WindowFillAlphaDefault = 1.00f;
#endif
}

void DebugManager::update(float delta)
{
	auto inputManager = InputManager::getInstance();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto menuDisplayPressed = inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyMenuDisplay1OperationKeyId)
		&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyMenuDisplay2OperationKeyId)
		&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyMenuDisplay3OperationKeyId)
		&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyMenuDisplay4OperationKeyId);
	auto menuDisplayTriggered = menuDisplayPressed && (
		inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyMenuDisplay1OperationKeyId)
		|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyMenuDisplay2OperationKeyId)
		|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyMenuDisplay3OperationKeyId)
		|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyMenuDisplay4OperationKeyId)
		);
	if (menuDisplayTriggered) {
		this->setShowMenuBar(!this->getShowMenuBar());
	}
#endif

	//-----------------------------------------------------------------------------------------------------------
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
	// escape キー
	auto displaySwitchPressed = inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyDisplaySwitch1OperationKeyId)
		&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyDisplaySwitch2OperationKeyId)
		&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyDisplaySwitch3OperationKeyId)
		&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyDisplaySwitch4OperationKeyId);
	auto displaySwitchTriggered = displaySwitchPressed && (
		inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyDisplaySwitch1OperationKeyId)
		|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyDisplaySwitch2OperationKeyId)
		|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyDisplaySwitch3OperationKeyId)
		|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyDisplaySwitch4OperationKeyId)
		);
	if (displaySwitchTriggered) {
		bool bFullScreen = IsFullScreen();
		ChangeScreen(!bFullScreen);

		//ウインドウ表示の場合。
		auto projectData = GameManager::getInstance()->getProjectData();
		if (!agtk::IsFullScreen()) {
			//解像度を変更。
			auto screenSize = projectData->getScreenSize();
			auto nowScreenSize = agtk::GetScreenResolutionSize();
			float magnifyWindow = projectData->getMagnifyWindow() ? projectData->getWindowMagnification() : 1.0f;
			auto windowSize = screenSize * magnifyWindow;
			auto nowWindowSize = agtk::GetFrameSize();
			if ((screenSize.width != nowScreenSize.width || screenSize.height != nowScreenSize.height)
				|| (windowSize.width != nowWindowSize.width || windowSize.height != nowWindowSize.height)) {
				agtk::ChangeScreenResolutionSize(screenSize, magnifyWindow);
			}
		}
	}
#endif	
	//-----------------------------------------------------------------------------------------------------------
	auto scene = GameManager::getInstance()->getCurrentScene();
#ifdef USE_PREVIEW
	if (this->getRenderTextureDebug() && scene) {
		//update shader
		if (_bShowGameDisplayWindow) {
			this->changeShader();

			auto scene = GameManager::getInstance()->getCurrentScene();
			auto renderer = Director::getInstance()->getRenderer();
			auto renderTexture = this->getRenderTextureDebug();
			GameManager::visitScene(renderer, scene, renderTexture);
		}
	}
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getDeviceControllerList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto device = static_cast<DebugDeviceController *>(ref);
#else
		auto device = dynamic_cast<DebugDeviceController *>(ref);
#endif
		device->update(delta);
	}

	//グリッド表示。
	if (_bShowDebugDispGrid) {
		if (this->getDebugGridLineList()->count() == 0) {
			this->createGridLine();
		}
	}
	else {
		if (this->getDebugGridLineList()->count() > 0) {
			this->removeGridLine();
		}
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
void DebugManager::setVisibleCollisionArea(agtk::data::TimelineInfoData::EnumTimelineType type, bool bVisible)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneLayer = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(sceneLayer, el) {
		auto layer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		CC_ASSERT(layer);
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(layer->getObjectList(), ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			CC_ASSERT(object);
			object->setVisibleDebugDisplayArea(type, bVisible);
		}
	}
}
#endif

void DebugManager::createGridLine()
{
	removeGridLine();

	auto primitiveManager = PrimitiveManager::getInstance();
	auto gameManager = GameManager::getInstance();
	auto projectData = gameManager->getProjectData();
	auto scene = gameManager->getCurrentScene();
	if (scene == nullptr) {
		//シーンが存在しない。
		return;
	}
	auto sceneData = scene->getSceneData();
	auto horzScreenCount = sceneData->getHorzScreenCount();
	auto vertScreenCount = sceneData->getVertScreenCount();
	auto layer = gameManager->getCurrentLayer();
	auto gridLineList = this->getDebugGridLineList();

	cocos2d::Color4F blue(0, 0, 1.0f, 0.7f);
	int horzLineCount = (projectData->getScreenWidth() * horzScreenCount) / projectData->getTileWidth();
	for (int i = 0; i <= horzLineCount; i++) {
		auto p = primitiveManager->createLine(i * projectData->getTileWidth(), 0, i * projectData->getTileWidth(), scene->getSceneSize().y, blue);
		layer->addChild(p, BaseLayer::ZOrder::Debug);
		gridLineList->addObject(p);
	}
	int vertLineCount = (projectData->getScreenHeight() * vertScreenCount) / projectData->getTileHeight();
	for (int i = 0; i <= vertLineCount; i++) {
		float y = scene->getSceneSize().y - i * projectData->getTileHeight();
		auto p = primitiveManager->createLine(0, y, scene->getSceneSize().x, y, blue);
		layer->addChild(p, BaseLayer::ZOrder::Debug);
		gridLineList->addObject(p);
	}
}

void DebugManager::removeGridLine()
{
	auto primitiveManager = PrimitiveManager::getInstance();
	auto layer = GameManager::getInstance()->getCurrentLayer();
	auto gridLineList = this->getDebugGridLineList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(gridLineList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<PrimitiveNode *>(ref);
#else
		auto p = dynamic_cast<PrimitiveNode *>(ref);
#endif
		layer->removeChild(p);
		primitiveManager->remove(p);
	}
	gridLineList->removeAllObjects();
}

/**
* ポータルのデバッグ表示ON/OFF
* @param	isShow	表示フラグ
*/
void DebugManager::showPortalView(bool isShow)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneLayer = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(sceneLayer, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto layer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		CC_ASSERT(layer);
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(layer->getPortalObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto portal = static_cast<agtk::Portal *>(ref);
#else
			auto portal = dynamic_cast<agtk::Portal *>(ref);
#endif
			CC_ASSERT(portal);
			portal->showDebugVisible(isShow);
		}
	}
}

/**
* タイルの壁のデバッグ表示ON/OFF
* @param	isShow	表示フラグ
*/
void DebugManager::showTileWallView(bool isShow)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneLayer = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(sceneLayer, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto layer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		CC_ASSERT(layer);
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(layer->getTileMapList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto tileMap = static_cast<agtk::TileMap *>(ref);
#else
			auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
#endif
			CC_ASSERT(tileMap);
			for (int j = 0; j < tileMap->getTileMapHeiht(); j++) {
				for (int i = 0; i < tileMap->getTileMapWidth(); i++) {
					auto tile = tileMap->getTile(i, j);
					if (tile != nullptr) {
						tile->showDebugVisible(isShow);
					}
				}
			}
		}
	}
}

#ifdef USE_PREVIEW
/**
* その他パーツのデバッグ表示ON/OFF
* @param	isShow	表示フラグ
*/
void DebugManager::showPartOthersView(bool isShow)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}

	// コースの表示
	auto courseList = scene->getCourseList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(scene->getCourseList(), ref) {
		auto course = dynamic_cast<agtk::OthersCourse *>(ref);
		CC_ASSERT(course);
		course->showDebugVisible(isShow);
	}

	// 坂、360度ループの表示
	auto sceneLayer = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(sceneLayer, el) {
		auto layer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		CC_ASSERT(layer);
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(layer->getSlopeList(), ref) {
			auto slope = dynamic_cast<agtk::Slope *>(ref);
			CC_ASSERT(slope);

			slope->showDebugVisible(isShow);
		}

		CCARRAY_FOREACH(layer->getLoopCourseList(), ref) {
			auto course = dynamic_cast<agtk::OthersCourse *>(ref);
			CC_ASSERT(course);

			course->showDebugVisible(isShow);
		}
	}
}
#endif

#ifdef USE_PREVIEW
void DebugManager::showLimitArea(bool bShow)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene) {
		scene->showDebugLimitArea(bShow);
	}
}

void DebugManager::showLimitCamera(bool bShow)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene) {
		scene->showDebugLimitCamera(bShow);
	}
}
#endif
#ifdef USE_PREVIEW
void DebugManager::resetChangeShader()
{
	auto displayData = this->getDisplayData();

	// リセット
	{
		//scene topmost
		int layerId = data::SceneData::kTopMostWithMenuLayerId;
		for (auto &info : ShaderInfo::list()) {
			auto value = displayData->getShaderKind(layerId, info.kind);
			displayData->setTmpShaderKind(layerId, info.kind, value->getValue(), value->getCheck());
		}
	}

	// 更新
	{
		//scene topMost(メニュー含む)
		this->changeShader(agtk::data::SceneData::kTopMostWithMenuLayerId, this->getDisplayData());
	}
}

void DebugManager::changeShader()
{
	//scene topMost(メニュー含む)
	this->changeShader(agtk::data::SceneData::kTopMostWithMenuLayerId, this->getDisplayData());
}

void DebugManager::changeShader(int layerId, agtk::DebugDisplayData *displayData)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	agtk::RenderTextureCtrl *renderTextureCtrl = nullptr;
	if (layerId == agtk::data::SceneData::kBackgroundLayerId) {
		renderTextureCtrl = scene->getSceneBackground()->getRenderTexture();
	}
	else if (layerId == agtk::data::SceneData::kTopMostLayerId) {
		renderTextureCtrl = scene->getSceneTopMost()->getRenderTexture();
	}
	else if (layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {
		SceneTopMost* sceneTopMost = scene->getSceneTopMost();
		renderTextureCtrl = sceneTopMost->getWithMenuRenderTexture();
		if (renderTextureCtrl == nullptr) {
			sceneTopMost->createObjectCommandWithMenuRenderTexture();
			renderTextureCtrl = sceneTopMost->getWithMenuRenderTexture();
		}
	}
	else {
		renderTextureCtrl = scene->getRenderTextureCtrl(layerId);
	}
	if (renderTextureCtrl == nullptr) {
		return;
	}
	auto shaderList = dynamic_cast<cocos2d::__Dictionary *>(displayData->getTmpShaderList()->objectForKey(layerId));
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(shaderList, el) {
		agtk::Shader::ShaderKind kind = (agtk::Shader::ShaderKind)el->getIntKey();
		auto checkValue = dynamic_cast<DebugShaderData *>(el->getObject());
		bool bCheck = checkValue->getCheck();
		float value = checkValue->getValue() * 0.01f;

		auto shader = renderTextureCtrl->getShader(kind);
		if (shader == nullptr && bCheck) {
			shader = renderTextureCtrl->addShader(kind, value);
		}
		if (bCheck) {
			shader->setIntensity(value);
		}
		else {
			if (renderTextureCtrl->isShader(kind)) {
				renderTextureCtrl->removeShader(kind);
			}
		}
	}
}

#endif
int DebugManager::getSceneId(int selectSceneId)
{
	if (selectSceneId < 0) {
		selectSceneId = _selectSceneId;
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	cocos2d::DictElement *el = nullptr;
	auto sceneList = projectData->getSceneList();
	int sceneId = 0;
	CCDICT_FOREACH(sceneList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneData = static_cast<agtk::data::SceneData *>(el->getObject());
#else
		auto sceneData = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif
		if (selectSceneId == sceneId) {
			return sceneData->getId();
		}
		sceneId++;
	}
	return -1;
}

int DebugManager::findSelectSceneId(std::string name)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	cocos2d::DictElement *el = nullptr;
	auto sceneList = projectData->getSceneList();
	int selectSceneId = 0;
	CCDICT_FOREACH(sceneList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneData = static_cast<agtk::data::SceneData *>(el->getObject());
#else
		auto sceneData = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif
		if (name == sceneData->getName()){
			return selectSceneId;
		}
		selectSceneId++;
	}
	return -1;
}

int DebugManager::findSelectSceneId(int sceneId)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	cocos2d::DictElement *el = nullptr;
	auto sceneList = projectData->getSceneList();
	int selectSceneId = 0;
	CCDICT_FOREACH(sceneList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneData = static_cast<agtk::data::SceneData *>(el->getObject());
#else
		auto sceneData = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif
		if (sceneId == sceneData->getId()) {
			return selectSceneId;
		}
		selectSceneId++;
	}
	return -1;
}

void DebugManager::createObjectInfoWindow(agtk::Object *object, cocos2d::Vec2 pos)
{
	auto objectInfoWindowList = this->getObjectInfoWindowList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectInfoWindowList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto objectInfoWindow = static_cast<agtk::DebugObjectInfoWindow *>(ref);
#else
		auto objectInfoWindow = dynamic_cast<agtk::DebugObjectInfoWindow *>(ref);
#endif
		if (objectInfoWindow->getObject() == object) {
			objectInfoWindow->setDisplayFlag(true);
			return;
		}
	}
	auto objectInfoWindow = agtk::DebugObjectInfoWindow::create(object, pos);
	objectInfoWindowList->addObject(objectInfoWindow);
}

void DebugManager::removeObjectInfoWindow(agtk::Object *object)
{
	auto objectInfoWindowList = this->getObjectInfoWindowList();
	bool bRemove;
	do {
		bRemove = false;
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectInfoWindowList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto objectInfoWindow = static_cast<agtk::DebugObjectInfoWindow *>(ref);
#else
			auto objectInfoWindow = dynamic_cast<agtk::DebugObjectInfoWindow *>(ref);
#endif
			if (objectInfoWindow->getObject() == object) {
				objectInfoWindowList->removeObject(objectInfoWindow);
				bRemove = true;
				break;
			}
		}
	} while (bRemove);
}

DebugObjectInfoWindow *DebugManager::getObjectInfoWindow(agtk::Object *object)
{
	auto objectInfoWindowList = this->getObjectInfoWindowList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectInfoWindowList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto objectInfoWindow = static_cast<agtk::DebugObjectInfoWindow *>(ref);
#else
		auto objectInfoWindow = dynamic_cast<agtk::DebugObjectInfoWindow *>(ref);
#endif
		if (objectInfoWindow->getObject() == object) {
			return objectInfoWindow;
		}
	}
	return nullptr;
}

void DebugManager::showObjectInfoWindows()
{
	auto objectInfoWindowList = this->getObjectInfoWindowList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectInfoWindowList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto objectInfoWindow = static_cast<agtk::DebugObjectInfoWindow *>(ref);
#else
		auto objectInfoWindow = dynamic_cast<agtk::DebugObjectInfoWindow *>(ref);
#endif
		objectInfoWindow->draw();
	}
}

// ACT2-6469 コントローラー設定
void DebugManager::showControllerWindow()
{
	if (!_bShowControllerWindow) {
		return;
	}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
	ImGui::SetNextWindowSize(ImVec2(400 * 2, 250 * 2), ImGuiCond_Appearing);//※サイズの設定できます。無くても自動補正してくれるようです。
#else
	ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiSetCond_Appearing);//※サイズの設定できます。無くても自動補正してくれるようです。
#endif
	ImGui::Begin(GameManager::tr("Controller settings"), &_bShowControllerWindow);

	//////////////////////////////////
	// ドロップダウン
	//////////////////////////////////
	int listCount = 1;
	const char* padName[InputDataRaw::MAX_GAMEPAD + 2] = { nullptr };
	padName[0] = GameManager::tr("None");

	// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
	int deviceIdList[InputDataRaw::MAX_GAMEPAD + 2];
	for (int i = 0; i < InputDataRaw::MAX_GAMEPAD + 2; i++) deviceIdList[i] = -1;
	static int selected_button = -1;
#else
	//キーボード・マウス
	padName[listCount++] = GameManager::tr("Keyboard/mouse");
#endif
	cocos2d::Ref* ref = nullptr;
	CCARRAY_FOREACH(this->getDeviceControllerList(), ref) {
		// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<DebugDeviceController*>(ref);
#else
		auto data = dynamic_cast<DebugDeviceController*>(ref);
#endif
		CC_ASSERT(data);
		if (data->isConnected() == false) {
			continue;
		}
		// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
		deviceIdList[listCount - 1] = data->getDeviceId();
#endif
		padName[listCount++] = (char*)data->getName();
	}

	//////////////////////////////////
	// コントローラー数分表示
	//////////////////////////////////
	auto displayData = this->getDisplayData();
	auto controllerList = displayData->getTmpControllerList();
	for (int conNum = 1; conNum <= 4; conNum++) {
		// 現在の値を取得する
		auto val = dynamic_cast<cocos2d::Integer*>(controllerList->getObjectAtIndex(conNum - 1));
		// ドロップダウンリストの値に変換する
		auto listNum = val->getValue() < 0 ? 0 : val->getValue() +1;
		auto str = ":" + to_string(conNum) + "P Controller";
		bool isChange = ImGui::Combo(GameManager::tr(str.c_str()), &listNum, (const char**)padName, listCount);

		// 変更したら値を格納する
		if (isChange) {
			auto changeVal = listNum == 0 ? -1 : listNum-1;
			auto p = cocos2d::Integer::create(changeVal);
			controllerList->setObject(p, conNum - 1);
		}
	}
	
	bool bOk = ImGui::Button(GameManager::tr("OK"));
	if (bOk) {
		// 値を入れる
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		for (int id = agtk::data::kProjectSystemVariable1PController; id <= agtk::data::kProjectSystemVariable4PController; id++) {
			auto val = dynamic_cast<cocos2d::Integer*>(controllerList->getObjectAtIndex(id - agtk::data::kProjectSystemVariable1PController));
			projectPlayData->getCommonVariableData(id)->setValue(val->getValue());
		}
		GameManager::getInstance()->updateSystemVariableAndSwitch();
	}
	ImGui::SameLine();
	bool bCancel = ImGui::Button(GameManager::tr("Cancel"));
	if (bOk || bCancel) {
		_bShowControllerWindow = false;
	}
	
	ImGui::End();
}

void DebugManager::setFontName(const std::string &fontName)
{
	mFontName = std::string("fonts/") + fontName + ".ttf";
}
//iniファイルにデバッグ機能設定を保存
void DebugManager::saveDebugSetting()
{
	auto iniFilePath = FileUtils::getInstance()->getWritablePath() + std::string("player.ini");
	auto jsonData = FileUtils::getInstance()->getStringFromFile(iniFilePath);
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		//CCASSERT(0, "Error: Json Parse.");
		doc.SetObject();
	}
	else {
		if (doc.HasMember("debugSetting")) {
			// 一度削除してから設定を保存
			doc.RemoveMember("debugSetting");
		}
	}
	rapidjson::Value saveDebug(rapidjson::kObjectType);
	saveDebug.AddMember("Show TileWall", _showTileWallFlag, allocator);
	saveDebug.AddMember("Show Wall Detection", _collisionWallEnabled, allocator);
	saveDebug.AddMember("Show Collision Detection", _collisionHitEnabled, allocator);
	saveDebug.AddMember("Show Attack Detection", _collisionAttackEnabled, allocator);
	saveDebug.AddMember("Show Connection Point", _collisionConnectionEnabled, allocator);
	saveDebug.AddMember("Show Runtime Log Console", _logConsoleDisplayEnabled, allocator);
	saveDebug.AddMember("Free Movement Mode", _freeMovingEnabled, allocator);
	saveDebug.AddMember("Invincible Mode", _invincibleModeEnabled, allocator);
	saveDebug.AddMember("Frame Rate", _frameRateDisplayEnabled, allocator);
	saveDebug.AddMember("Performance / Speed Settings", this->getDebugPerformanceAndSpeedSettingsWindow()->getDisplayFlag(), allocator);
	saveDebug.AddMember("Show Debugging for Development", _debugForDevelopment, allocator);
	saveDebug.AddMember("Show physics debug", _showPhysicsBoxEnabled, allocator);
	saveDebug.AddMember("Show Debugging for Other Parts", _showPartOthersFlag, allocator);
	saveDebug.AddMember("Show portal debug", _showPortalFlag, allocator);
	saveDebug.AddMember("Show Debugging for Player Move Range Restriction", _showLimitAreaFlag, allocator);
	saveDebug.AddMember("Show Debugging for Camera Range Restriction", _showLimitCameraFlag, allocator);
	saveDebug.AddMember("Disable Skip During Scene Generation", _skipOneFrameWhenSceneCreatedIgnored, allocator);
#if !defined(USE_RUNTIME)
	saveDebug.AddMember("Fixed FPS", _fixFramePerSecondFlag, allocator);
//	saveDebug.AddMember("Change Scene", _bShowChangeSceneWindow, allocator);	// シーン切り替え時に閉じられるので保存しない
#endif
	saveDebug.AddMember("Loading Scene", _showLoadingScene, allocator);
	saveDebug.AddMember("Object Data", _showDebugObjectInfoWindow, allocator);
	saveDebug.AddMember("Normal Scene Object Data", _showDebugNormalSceneObjectInfoFlag, allocator);
	saveDebug.AddMember("Menu Scene Object Data", _showDebugMenuSceneObjectInfoFlag, allocator);
	saveDebug.AddMember("Data of Common Variables and Switches", this->getDebugResourcesCommonWindow()->getDisplayFlag(), allocator);
	saveDebug.AddMember("Show Grid", _bShowDebugDispGrid, allocator);
	if (this->getDisplayData()) {
		_debugDisableScreenShake = this->getDisplayData()->getDisableScreenShake();
	}
	saveDebug.AddMember("Disable Screen Shake", _debugDisableScreenShake, allocator);

	doc.AddMember("debugSetting", saveDebug, allocator);
	// 書き込み
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	FileUtils::getInstance()->writeStringToFile(buffer.GetString(), iniFilePath);
}

#ifdef USE_RUNTIME
void DebugManager::saveDebugSettingForRuntime()
{
	auto iniFilePath = FileUtils::getInstance()->getWritablePath() + std::string("player.ini");
	auto jsonData = FileUtils::getInstance()->getStringFromFile(iniFilePath);
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		//CCASSERT(0, "Error: Json Parse.");
		doc.SetObject();
	}
	else {
		if (doc.HasMember("debugSetting")) {
			// 一度削除してから設定を保存
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			//doc.RemoveMember("debugSetting");
#endif
		}
	}

	//セーブデータ。
	rapidjson::Value saveDebug(rapidjson::kObjectType);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	_debugDisableScreenShake = this->getDisplayData()->getDisableScreenShake();

	if (doc["debugSetting"].HasMember("Disable Screen Shake")) {
		doc["debugSetting"]["Disable Screen Shake"] = _debugDisableScreenShake;
	}
	else {
		rapidjson::Value saveDebug(rapidjson::kObjectType);
		{
			saveDebug.AddMember("Disable Screen Shake", _debugDisableScreenShake, allocator);
		}
		doc.AddMember("debugSetting", saveDebug, allocator);
	}
#endif

	// 書き込み
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	FileUtils::getInstance()->writeStringToFile(buffer.GetString(), iniFilePath);
}
#endif

//iniファイルからデバッグ機能設定を読み込み
void DebugManager::loadDebugSetting()
{
	auto iniFilePath = FileUtils::getInstance()->getWritablePath() + std::string("player.ini");
	auto jsonData = FileUtils::getInstance()->getStringFromFile(iniFilePath);
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
	}
	else {
		if (doc["debugSetting"].HasMember("Show TileWall")) {
			_showTileWallFlag = doc["debugSetting"]["Show TileWall"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show Wall Detection")) {
			_collisionWallEnabled = doc["debugSetting"]["Show Wall Detection"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show Collision Detection")) {
			_collisionHitEnabled = doc["debugSetting"]["Show Collision Detection"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show Attack Detection")) {
			_collisionAttackEnabled = doc["debugSetting"]["Show Attack Detection"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show Connection Point")) {
			_collisionConnectionEnabled = doc["debugSetting"]["Show Connection Point"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show Runtime Log Console")) {
			_logConsoleDisplayEnabled = doc["debugSetting"]["Show Runtime Log Console"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Free Movement Mode")) {
			_freeMovingEnabled = doc["debugSetting"]["Free Movement Mode"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Invincible Mode")) {
			_invincibleModeEnabled = doc["debugSetting"]["Invincible Mode"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Frame Rate")) {
			_frameRateDisplayEnabled = doc["debugSetting"]["Frame Rate"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Performance / Speed Settings")) {
			bool bPerformanceAndSpeedSettingsWindow = doc["debugSetting"]["Performance / Speed Settings"].GetBool();
			this->getDebugPerformanceAndSpeedSettingsWindow()->setDisplayFlag(bPerformanceAndSpeedSettingsWindow);
		}
		if (doc["debugSetting"].HasMember("Show Debugging for Development")) {
			_debugForDevelopment = doc["debugSetting"]["Show Debugging for Development"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show physics debug")) {
			_showPhysicsBoxEnabled = doc["debugSetting"]["Show physics debug"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show Debugging for Other Parts")) {
			_showPartOthersFlag = doc["debugSetting"]["Show Debugging for Other Parts"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show portal debug")) {
			_showPortalFlag = doc["debugSetting"]["Show portal debug"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show Debugging for Player Move Range Restriction")) {
			_showLimitAreaFlag = doc["debugSetting"]["Show Debugging for Player Move Range Restriction"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Show Debugging for Camera Range Restriction")) {
			_showLimitCameraFlag = doc["debugSetting"]["Show Debugging for Camera Range Restriction"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Disable Skip During Scene Generation")) {
			_skipOneFrameWhenSceneCreatedIgnored = doc["debugSetting"]["Disable Skip During Scene Generation"].GetBool();
		}
#if !defined(USE_RUNTIME)
		if (doc["debugSetting"].HasMember("Fixed FPS")) {
			_fixFramePerSecondFlag = doc["debugSetting"]["Fixed FPS"].GetBool();
		}
//		if (doc["debugSetting"].HasMember("Change Scene")) {
//			_bShowChangeSceneWindow = doc["debugSetting"]["Change Scene"].GetBool();
//		}
#endif
		if (doc["debugSetting"].HasMember("Loading Scene")) {
			_showLoadingScene = doc["debugSetting"]["Loading Scene"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Object Data")) {
			_showDebugObjectInfoWindow = doc["debugSetting"]["Object Data"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Normal Scene Object Data")) {
			_showDebugNormalSceneObjectInfoFlag = doc["debugSetting"]["Normal Scene Object Data"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Menu Scene Object Data")) {
			_showDebugMenuSceneObjectInfoFlag = doc["debugSetting"]["Menu Scene Object Data"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Data of Common Variables and Switches")) {
			bool bResourcesCommonWindow = doc["debugSetting"]["Data of Common Variables and Switches"].GetBool();
			this->getDebugResourcesCommonWindow()->setDisplayFlag(bResourcesCommonWindow);
		}
		if (doc["debugSetting"].HasMember("Show Grid")) {
			_bShowDebugDispGrid = doc["debugSetting"]["Show Grid"].GetBool();
		}
		if (doc["debugSetting"].HasMember("Disable Screen Shake")) {
			_debugDisableScreenShake = doc["debugSetting"]["Disable Screen Shake"].GetBool();
		}
	}
}

#ifdef USE_RUNTIME
void DebugManager::loadDebugSettingForRuntime()
{
	auto iniFilePath = FileUtils::getInstance()->getWritablePath() + std::string("player.ini");
	auto jsonData = FileUtils::getInstance()->getStringFromFile(iniFilePath);
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
	}
	else {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		if (doc["debugSetting"].HasMember("Disable Screen Shake")) {
			_debugDisableScreenShake = doc["debugSetting"]["Disable Screen Shake"].GetBool();
		}
	}
}
#endif

NS_AGTK_END //-----------------------------------------------------------------------------------//
