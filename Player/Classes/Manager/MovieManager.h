#ifndef __MOVIE_MANAGER_H__
#define __MOVIE_MANAGER_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "Lib/Object.h"
#include "Lib/VideoSprite.h"
#include "Data/ObjectCommandData.h"
#include "Manager/PrimitiveManager.h"

USING_NS_CC;

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief オブジェクトの「画動画を表示」で表示する動画ひとつを管理するクラス
 */
class AGTKPLAYER_API Movie : public cocos2d::Ref
{
private:
	Movie();
	virtual ~Movie();
public:
	CREATE_FUNC_PARAM2(Movie, agtk::Object *, object, agtk::data::ObjectCommandMovieShowData *, data);
	virtual void update(float delta);
	bool isEnd();
	void play();
	void stop();
	void pause();
	void resume();
	bool isPause();
	bool isClose();
private:
	virtual bool init(agtk::Object *object, agtk::data::ObjectCommandMovieShowData *data);
private:
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _object, Object);
	CC_SYNTHESIZE_RETAIN(agtk::VideoSprite *, _videoSprite, VideoSprite);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectCommandMovieShowData *, _movieShowData, MovieShowData);
	CC_SYNTHESIZE_RETAIN(agtk::data::MovieData *, _movieData, MovieData);
	CC_SYNTHESIZE_RETAIN(PrimitiveNode *, _fillBlackNode, FillBlackNode);
	CC_SYNTHESIZE(int, _layerId, LayerId);
	CC_SYNTHESIZE(cocos2d::Vec2, _startObjectPosition, StartObjectPosition);
	CC_SYNTHESIZE(cocos2d::Vec2, _startPosition, StartPosition);
	CC_SYNTHESIZE(bool, _objectBackside, ObjectBackside);
	int _objectActionId;//オブジェクトのアクションが切り替わったら表示を終了用
};
NS_AGTK_END

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief オブジェクトの「動画の再生」で表示する動画群を管理するクラス
 */
class AGTKPLAYER_API MovieManager : public cocos2d::Ref
{
private:
	MovieManager();
	static MovieManager *_movieManager;
public:
	virtual ~MovieManager();
	static MovieManager* getInstance();
	static void purge();
	bool init();
	void update(float delta);
	void addMovie(agtk::Object *object, agtk::data::ObjectCommandMovieShowData *data);
	void removeMovie(agtk::Object *object);
	cocos2d::__Array *getMovieArray(agtk::Object *object);
	bool checkGameStop();
private:
	void removeMovie(agtk::Movie *movie);
	agtk::Movie *getMovieCheckGameStop();
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _movieList, MovieList);
};

#endif	//__MOVIE_MANAGER_H__
