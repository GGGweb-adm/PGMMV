#ifndef __CC_IMGUI_H__
#define __CC_IMGUI_H__

#include "cocos2d.h"
#include "imgui.h"

USING_NS_CC;

class CC_DLL CCIMGUI_Value
{
public:
	void* value = nullptr;
	bool getBool() { return (bool)value; };
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	int getInt() { return (int)value; };
#endif
};

class CC_DLL CCIMGUI
{
private:
	//-------------------------------------------------------
	GLFWwindow* _window = nullptr;
	ImVec4 _clearColor = ImColor(114, 144, 154);
	//-------------------------------------------------------
	std::map<std::string, std::function<void()>> _callPiplines;
	std::map<std::string, CCIMGUI_Value*> _values;
	//-------------------------------------------------------
	bool isShowSetupStyle = false;
	void displaySetupStyle();
public:
	static CCIMGUI* getInstance();
	//-------------------------------------------------------
	GLFWwindow* getWindow() { return _window; };
	void setWindow(GLFWwindow* window) { _window = window; };
	ImVec4 getClearColor() { return _clearColor; };
	void setClearColor(ImColor color) { _clearColor = color; };
	//-------------------------------------------------------
	void init();
	void updateImGUI();
	void addImGUI(std::function<void()> imGUICall, std::string name) {_callPiplines[name] = imGUICall;};
	void removeImGUI(std::string name);
	//-------------------------------------------------------
	void setValue(bool value, std::string uid);
	void setValue(int value, std::string uid);
	CCIMGUI_Value* getValue(std::string uid);
	void removeValue(std::string uid);
	//-------------------------------------------------------
	void setShowStyleEditor(bool show) { isShowSetupStyle = show; };
};

#endif // __IMGUILAYER_H__
