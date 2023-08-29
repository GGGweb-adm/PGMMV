#ifndef __IMAGE_MANAGER_H__
#define __IMAGE_MANAGER_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Lib/Object.h"
#include "Data/ObjectCommandData.h"

USING_NS_CC;

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
/**
* @brief オブジェクトの「画像を表示」で表示する画像ひとつを管理するクラス
*/
class AGTKPLAYER_API Image : public cocos2d::Ref
{
private:
	Image();
	virtual ~Image();
public:
	static Image *create(agtk::Object *object, agtk::data::ObjectCommandImageShowData *data);
	virtual void update(float delta);
	bool isStop();
	bool isClose();
	bool isCloseByOk();
	bool isCloseByChangeAction();
private:
	virtual bool init(agtk::Object *object, agtk::data::ObjectCommandImageShowData *data);
private:
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _object, Object);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectCommandImageShowData *, _imageShowData, ImageShowData);
	CC_SYNTHESIZE_RETAIN(agtk::data::ImageData *, _imageData, ImageData);
	CC_SYNTHESIZE_RETAIN(cocos2d::Sprite *, _sprite, Sprite);
	CC_SYNTHESIZE_RETAIN(agtk::EventTimer *, _eventTimer, EventTimer);
	CC_SYNTHESIZE(int, _layerId, LayerId);
	CC_SYNTHESIZE(cocos2d::Vec2, _startObjectPosition, StartObjectPosition);
	CC_SYNTHESIZE(cocos2d::Vec2, _startPosition, StartPosition);
	CC_SYNTHESIZE(bool, _objectBackside, ObjectBackside);

	bool _waitOneFrame;	// 1フレーム待機するフラグ(生成後に更新が呼ばれる為、キー入力判定が即座に行われてしまうのを回避する用)
	int _objectActionId;//オブジェクトのアクションが切り替わったら表示を終了用
};
NS_AGTK_END

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief オブジェクトの「画像を表示」で表示する画像群を管理するクラス
 */
class AGTKPLAYER_API ImageManager : public cocos2d::Ref
{
private:
	ImageManager();
	static ImageManager *_imageManager;
public:
	virtual ~ImageManager();
	static ImageManager* getInstance();
	static void purge();
	bool init();
	void update(float delta);
	void addImage(agtk::Object *object, agtk::data::ObjectCommandImageShowData *data);
	void removeImage(agtk::Object *object);
	cocos2d::__Array *getImageArray(agtk::Object *object);
	bool checkGameStop();
private:
	void removeImage(agtk::Image *image);
	agtk::Image *getImageCheckGameStop();
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _imageList, ImageList);
};

#endif	//__IMAGE_MANAGER_H__
