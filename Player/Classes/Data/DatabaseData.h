#ifndef __AGTK_DATABASE_DATA_H__
#define	__AGTK_DATABASE_DATA_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Lib/Macros.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API DatabaseData : public cocos2d::Ref
{
public:
	enum DatabaseType {
		kResourceSet,			// 素材セット
		kMotion,				// モーション
		kVariable,				// 変数
	};
private:
	DatabaseData();
	virtual ~DatabaseData();
public:
	CREATE_FUNC_PARAM(DatabaseData, const rapidjson::Value&, json);

public:
#if defined(AGTK_DEBUG)
	void dump();
#endif
	const std::vector<string> &getRowList() const { return _rowList; }
	const std::vector<string> &getColumnList() const { return _columnList; }
	string getDatabaseValue(int columnIndex, int rowIndex);
	string getDatabaseColumnName(int columnIndex);
	int getDatabaseTemplateType();
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
	CC_SYNTHESIZE(int, _templateType, TemplateType); // データベースのテンプレートタイプ (0:デフォルト, 1:オブジェクト用)

	vector<string> _columnList;
	vector<string> _rowList;
	//map<pair<int, int>, cocos2d::__String *> _databaseList;
	map<pair<int, int>, string> _databaseList;
};

NS_DATA_END
NS_AGTK_END

#endif	//__AGTK_DATABASE_DATA_H__
