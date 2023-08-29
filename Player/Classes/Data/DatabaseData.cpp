/**
* @brief データベースデータ
*/
#include "DatabaseData.h"
#include "Manager/GameManager.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN


DatabaseData::DatabaseData()
{
	_name = nullptr;
	_children = nullptr;
}

DatabaseData::~DatabaseData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_children);
}

bool DatabaseData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	if (json.HasMember("folder")) {
		this->setFolder(json["folder"].GetBool());
		if (this->getFolder()) {
			auto dic = cocos2d::__Dictionary::create();
			if (json.HasMember("children")) {
				for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
					auto p = DatabaseData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
					CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
					dic->setObject(p, p->getId());
				}
			}
			this->setChildren(dic);
			return true;
		}
	}

	CC_ASSERT(json.HasMember("type"));
	this->setTemplateType(json["type"].GetInt());

	if (json.HasMember("columnList")) {
		auto &list = json["columnList"];
		for (auto i = 0; i < list.Size(); i++) {
			_columnList.push_back(list[i].GetString());
		}
	}
	if (json.HasMember("rowList")) {
		auto &list = json["rowList"];
		for (auto i = 0; i < list.Size(); i++) {
			_rowList.push_back(list[i].GetString());
		}
	}
	if (json.HasMember("databaseValueList")) {
		auto &list = json["databaseValueList"];
		for (auto i = 0; i < list.Size(); i++) {
			_databaseList.insert({ { list[i]["column"].GetInt(), list[i]["row"].GetInt() }, list[i]["val"].GetString() });
		}
	}
	return true;
}

string DatabaseData::getDatabaseValue(int columnIndex, int rowIndex)
{
	map<pair<int, int>, string>::iterator it = _databaseList.find(make_pair(columnIndex, rowIndex));
	if (it != _databaseList.end()) {
		return it->second;
	}
	return "";
}

string DatabaseData::getDatabaseColumnName(int columnIndex)
{
	if(columnIndex < _columnList.size()) {
		return _columnList[columnIndex];
	}
	return "";
}

int DatabaseData::getDatabaseTemplateType()
{
	return _templateType;
}


#if defined(AGTK_DEBUG)
void DatabaseData::dump()
{
	CCLOG("id:%d", this->getId());
}
#endif


NS_DATA_END
NS_AGTK_END
