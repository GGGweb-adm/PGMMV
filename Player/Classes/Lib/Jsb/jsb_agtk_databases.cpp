#include "Lib/Macros.h"
#include "GameManager.h"

#include "jsapi.h"
#include "jsfriendapi.h"
#include "scripting/js-bindings/manual/js_bindings_config.h"
#include "scripting/js-bindings/manual/js_bindings_core.h"
#include "scripting/js-bindings/manual/spidermonkey_specifics.h"
#include "scripting/js-bindings/manual/js_manual_conversions.h"
#include "mozilla/Maybe.h"
#include "scripting/js-bindings/manual/js-BindingsExport.h"
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
#include "scripting/js-bindings/manual/ScriptingCore.h"
#endif

static bool register_agtk_databases(JSContext *_cx,JS::HandleObject object);

//databases
static bool js_databases_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_databases_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_databases_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//database
static bool js_database_getColumnList(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_database_getRowList(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_database_getColumnByIndex(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_database_getColumnByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_database_getRowByIndex(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_database_getRowByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_database_getFieldByIndex(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_databases(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_databases(cx, object);
	CCASSERT(ret, "Failed to register_agtk_databases");
}

bool register_agtk_databases(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass databases_class = {
		"databases",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	auto databases = JS_DefineObject(cx, robj, "databases", &databases_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(databases, "databases should not be nullptr");
	JS::RootedObject rtexts(cx, databases);
	static JSFunctionSpec databases_methods[] = {
		{ "get", js_databases_get, 0, 0, 0 },
		{ "getIdList", js_databases_getIdList, 0, 0, 0 },
		{ "getIdByName", js_databases_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rtexts, databases_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*databases);
	JS_SetProperty(cx, robj, "databases", rval);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	static JSClass databaseBase_class = {
		"databaseBase",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	auto databaseBase = JS_DefineObject(cx, robj, "databaseBase", &databaseBase_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(databaseBase, "databaseBase should not be nullptr");
	JS::RootedObject rdatabaseBase(cx, databaseBase);
	static JSFunctionSpec database_methods[] = {
		{ "getColumnList", js_database_getColumnList, 0, 0, 0 },        // カラム名の配列を取得
		{ "getRowList", js_database_getRowList, 0, 0, 0 },              // レコード名の配列を取得
		{ "getColumnByIndex", js_database_getColumnByIndex, 0, 0, 0 },  // カラム値の配列を番号から取得
		{ "getColumnByName", js_database_getColumnByName, 0, 0, 0 },    // カラム値の配列をカラム名から取得
		{ "getRowByIndex", js_database_getRowByIndex, 0, 0, 0 },        // レコード値の配列を番号から取得
		{ "getRowByName", js_database_getRowByName, 0, 0, 0 },          // レコード値の配列をレコード名から取得
		{ "getFieldByIndex", js_database_getFieldByIndex, 0, 0, 0 },    // フィールド値を番号から取得
		{ NULL },
	};
	JS_DefineFunctions(cx, rdatabaseBase, database_methods);
	rval.setObject(*databaseBase);
	JS_SetProperty(cx, robj, "databaseBase", rval);
#endif
	return true;
}

// databases
enum {
	ObjectOperatable,
};
bool js_databases_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
    //ret: <object : Agtk.database>、見つからなければnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto databaseId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);
	if (!databaseData) {
		return true;
	}
	static JSClass database_class = {
		"database",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto sc = ScriptingCore::getInstance();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, "Agtk", &nsval);
	JS::RootedValue rv(cx);
	bool ret = false;
	if (nsval == JSVAL_VOID) {
		return false;
	}
	jsObj.set(nsval.toObjectOrNull());
	JS::RootedValue rdatabaseBaseVal(cx);
	JS_GetProperty(cx, jsObj, "databaseBase", &rdatabaseBaseVal);
	if (!rdatabaseBaseVal.isObject()) {
		return false;
	}

	JSObject* databaseBase = rdatabaseBaseVal.get().toObjectOrNull();
	JS::RootedObject rdatabaseBase(cx, databaseBase);
	auto database = JS_DefineObject(cx, jsthis, "database", &database_class, rdatabaseBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(database, "database should not be nullptr");
	JS::RootedObject rdatabase(cx, database);
#else
	auto database = JS_DefineObject(cx, jsthis, "database", &database_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(database, "database should not be nullptr");
	JS::RootedObject rdatabase(cx, database);
	static JSFunctionSpec database_methods[] = {
		{ "getColumnList", js_database_getColumnList, 0, 0, 0 },        // カラム名の配列を取得
		{ "getRowList", js_database_getRowList, 0, 0, 0 },              // レコード名の配列を取得
		{ "getColumnByIndex", js_database_getColumnByIndex, 0, 0, 0 },  // カラム値の配列を番号から取得
		{ "getColumnByName", js_database_getColumnByName, 0, 0, 0 },    // カラム値の配列をカラム名から取得
		{ "getRowByIndex", js_database_getRowByIndex, 0, 0, 0 },        // レコード値の配列を番号から取得
		{ "getRowByName", js_database_getRowByName, 0, 0, 0 },          // レコード値の配列をレコード名から取得
		{ "getFieldByIndex", js_database_getFieldByIndex, 0, 0, 0 },    // フィールド値を番号から取得
		{ NULL },
	};
	JS_DefineFunctions(cx, rdatabase, database_methods);
#endif
	JS::RootedValue rdatabaseId(cx, JS::Int32Value(databaseId));
	JS_DefineProperty(cx, rdatabase, "id", rdatabaseId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rdatabaseName(cx, std_string_to_jsval(cx, databaseData->getName()->getCString()));
	JS_DefineProperty(cx, rdatabase, "name", rdatabaseName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rdatabaseType(cx, JS::Int32Value(databaseData->getDatabaseTemplateType()));
	JS_DefineProperty(cx, rdatabase, "type", rdatabaseType, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rval(cx);
	rval.setInt32(databaseId);
	JS_SetProperty(cx, rdatabase, "databaseId", rval);
	args.rval().set(OBJECT_TO_JSVAL(database));
	return true;
}

bool js_databases_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : データベースIDの配列>、見つからなければnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	auto projectData = GameManager::getInstance()->getProjectData();
	std::list<int> idList;
	std::function<void(cocos2d::Dictionary *)> scanRecur = [&scanRecur, &idList, projectData](cocos2d::Dictionary *children){
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
			auto p = dynamic_cast<agtk::data::DatabaseData *>(el->getObject());
			if (!p->getFolder()) {
				idList.push_back(p->getId());
			}
			if (p->getChildren()) {
				scanRecur(p->getChildren());
			}
		}
		return;
	};
	scanRecur(projectData->getDatabaseList());

	if (idList.size() < 1) {
		return true;
	}

	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, idList.size()));
	uint32_t i = 0;
	for(auto id: idList){
		JS::RootedValue element(cx, JS::Int32Value(id));
		JS_SetElement(cx, jsarr, i, element);
		i++;
	}

	args.rval().set(OBJECT_TO_JSVAL(jsarr));
	return true;
}

bool js_databases_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: データベース名>
	//ret: <int : データベースID>、見つからなければ-1
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string databaseName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &databaseName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseDataByName(databaseName.c_str());

	if(databaseData){
		value = databaseData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

//database
bool js_database_getColumnList(JSContext *cx, unsigned argc, JS::Value *vp) {
	//ret: <array : カラム名の配列>、見つからなければnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "databaseId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto databaseId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);
	if (!databaseData) {
		return false;
	}

	auto columnList = databaseData->getColumnList();
	if (columnList.size() < 1) {
		return true;
	}
	args.rval().set(std_vector_string_to_jsval(cx, columnList));
	return true;
}

bool js_database_getRowList(JSContext *cx, unsigned argc, JS::Value *vp) {
	//ret: <array : レコード名の配列>、見つからなければnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "databaseId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto databaseId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);
	if (!databaseData) {
		return false;
	}

	auto rowList = databaseData->getRowList();
	if (rowList.size() < 1) {
		return true;
	}
	args.rval().set(std_vector_string_to_jsval(cx, rowList));
	return true;
}

bool js_database_getColumnByIndex(JSContext *cx, unsigned argc, JS::Value *vp) {
	//arg: <int : カラム番号(0から)>
	//ret: <array: カラム値の配列>、見つからなければnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	auto len = args.length();
	if (len < 1) {
		return false;
	}

// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isNumber()) {
		return false;
	}
	auto columnIndex = JavascriptManager::getInt32(v2);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "databaseId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto databaseId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);
	if (!databaseData) {
		return false;
	}
	auto columnList = databaseData->getColumnList();
	auto rowList = databaseData->getRowList();
	if (columnIndex > columnList.size() || columnIndex < 0) {
		return true;
	}

	string columnValue;
	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, rowList.size()));
	for (int i = 0; i < rowList.size(); i++) {
		columnValue = databaseData->getDatabaseValue(columnIndex, i);
		JS::RootedValue element(cx, std_string_to_jsval(cx, columnValue));
		JS_SetElement(cx, jsarr, i, element);
	}

	args.rval().set(OBJECT_TO_JSVAL(jsarr));
	return true;
}

bool js_database_getColumnByName(JSContext *cx, unsigned argc, JS::Value *vp) {
	//arg: <string : カラム名(0から)>
	//ret: <array: カラム値の配列>、見つからなければnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	auto len = args.length();
	if (len < 1) {
		return false;
	}

	std::string columnName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &columnName)) {
			return false;
		}
	}

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "databaseId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto databaseId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);
	if (!databaseData) {
		return false;
	}

	int columnIndex = -1;
	auto columnList = databaseData->getColumnList();
	auto rowList = databaseData->getRowList();
	for (int i = 0; i < columnList.size(); i++) {
		if (columnList[i] == columnName) {
			columnIndex = i;
			break;
		}
	}

	if (columnIndex == -1) {
		return true;
	}

	string columnValue;
	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, rowList.size()));
	for (int i = 0; i < rowList.size(); i++) {
		columnValue = databaseData->getDatabaseValue(columnIndex, i);
		JS::RootedValue element(cx, std_string_to_jsval(cx, columnValue));
		JS_SetElement(cx, jsarr, i, element);
	}
	args.rval().set(OBJECT_TO_JSVAL(jsarr));

	return true;
}

bool js_database_getRowByIndex(JSContext *cx, unsigned argc, JS::Value *vp) {
	//arg: <int : レコード番号(0から)>
	//ret: <array: レコード値の配列>、見つからなければnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	auto len = args.length();
	if (len < 1) {
		return false;
	}

// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isNumber()) {
		return false;
	}
	auto rowIndex = JavascriptManager::getInt32(v2);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "databaseId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto databaseId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);
	if (!databaseData) {
		return false;
	}

	auto columnList = databaseData->getColumnList();
	auto rowList = databaseData->getRowList();
	if (rowIndex > rowList.size() || rowIndex < 0) {
		return true;
	}

	string rowValue;
	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, columnList.size()));
	for (int i = 0; i < columnList.size(); i++) {
		rowValue = databaseData->getDatabaseValue(i, rowIndex);
		JS::RootedValue element(cx, std_string_to_jsval(cx, rowValue));
		JS_SetElement(cx, jsarr, i, element);
	}

	args.rval().set(OBJECT_TO_JSVAL(jsarr));
	return true;
}

bool js_database_getRowByName(JSContext *cx, unsigned argc, JS::Value *vp) {
	//arg: <string : レコード名>
	//ret: <array: レコード値の配列>、見つからなければnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	auto len = args.length();
	if (len < 1) {
		return false;
	}

	std::string rowName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &rowName)) {
			return false;
		}
	}

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "databaseId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto databaseId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);
	if (!databaseData) {
		return false;
	}

	int rowIndex = -1;
	auto columnList = databaseData->getColumnList();
	auto rowList = databaseData->getRowList();
	for (int i = 0; i < rowList.size(); i++) {
		if (rowList[i] == rowName) {
			rowIndex = i;
			break;
		}
	}

	if (rowIndex == -1) {
		return true;
	}

	string rowValue;
	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, columnList.size()));
	for (int i = 0; i < columnList.size(); i++) {
		rowValue = databaseData->getDatabaseValue(i, rowIndex);
		JS::RootedValue element(cx, std_string_to_jsval(cx, rowValue));
		JS_SetElement(cx, jsarr, i, element);
	}
	args.rval().set(OBJECT_TO_JSVAL(jsarr));

	return true;
}

bool js_database_getFieldByIndex(JSContext *cx, unsigned argc, JS::Value *vp) {
	//args: <int : カラム番号>, <int : レコード番号>
	//ret: <string : フィールドの値>、見つからない場合はnull
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	auto len = args.length();
	if (len < 2) {
		return false;
	}

// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isNumber()) {
		return false;
	}
	auto columnIndex = JavascriptManager::getInt32(v2);

// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v3 = args[1];
#else
#endif
	if (!v3.isNumber()) {
		return false;
	}
	auto rowIndex = JavascriptManager::getInt32(v3);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "databaseId", &v);
	if (!v.isNumber()) {
		return false;
	}

	auto databaseId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);
	if (!databaseData) {
		return false;
	}

	auto columnList = databaseData->getColumnList();
	auto rowList = databaseData->getRowList();

	if (columnIndex > columnList.size() || columnIndex < 0) {
		return true;
	}
	if (rowIndex > rowList.size() || rowIndex < 0) {
		return true;
	}

	auto val = databaseData->getDatabaseValue(columnIndex, rowIndex);
	args.rval().set(std_string_to_jsval(cx, val));
	return true;
}

//===============================================================================================//
