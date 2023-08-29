#ifndef __GIF_PLAYER_H__
#define	__GIF_PLAYER_H__

#include "Lib/Macros.h"
#include "Lib/Player/BasePlayer.h"
#include "Lib/Animation.h"
#include "Data/ProjectData.h"
#include "Manager/PrimitiveManager.h"

#include "External/gif/gif_lib.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------
/**
 * @brief GIFアニメの1フレーム分を管理
 */
class GifFrame : public cocos2d::Ref
{
public:
	/**
	 * @brief GIFアニメの1枚分のイメージデータを管理
	 */
	class Bitmap : public cocos2d::Ref
	{
	private:
		Bitmap();
		virtual ~Bitmap();
	public:
		CREATE_FUNC_PARAM2(Bitmap, unsigned int, width, unsigned int, height);
		unsigned char *getAddr(unsigned int pos = 0);
		void copy(Bitmap *bitmap);
	private:
		virtual bool init(unsigned int width, unsigned int height);
	private:
		unsigned char* _buffer;
		CC_SYNTHESIZE(unsigned int, _width, Width);
		CC_SYNTHESIZE(unsigned int, _height, Height);
		CC_SYNTHESIZE(unsigned int, _length, Length);
	};
private:
	GifFrame();
	virtual ~GifFrame();
public:
	CREATE_FUNC_PARAM2(GifFrame, GifFileType *, fileType, int, frameIndex);
	CREATE_FUNC_PARAM3(GifFrame, GifFileType *, fileType, int, frameIndex, GifFrame *, prevGifFrame);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(GifFileType *fileType, int frameIndex);
	virtual bool init(GifFileType *fileType, int frameIndex, GifFrame *prevGifFrame);
	unsigned int getDuration(const SavedImage *image);
	const ColorMapObject *getColorMap(const GifFileType *fileType, const SavedImage *image);
	int getTransparent(const SavedImage *image);
	int getDisposalMethod(const SavedImage *image);
private:
	CC_SYNTHESIZE(int, _duration, Duration);
	CC_SYNTHESIZE(int, _durationEnd, DurationEnd);
	CC_SYNTHESIZE(int, _frameIndex, FrameIndex);
	CC_SYNTHESIZE_RETAIN(Bitmap *, _bitmap, Bitmap);
};

//-------------------------------------------------------------------------------------------------
/**
* @brief シーン背景画像やタイル画像に指定されたGIFアニメの表示を管理するクラス。
*/
class GifAnimation : public cocos2d::Ref
{
private:
	GifAnimation();
	virtual ~GifAnimation();
public:
	CREATE_FUNC_PARAM(GifAnimation, const char *, filename);
	void play(int frames = 0);
	void stop();
	void pause() { _pause = true; };
	void resume() { _pause = false; };
	bool update(float dt);
	const char *getFilename();
	GifFrame::Bitmap *getBitmap();
	int setFrameNo(int frameNo);
	int getFrameMax();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const char *filename);
	static int callbackDecode(GifFileType *gif, GifByteType *bytes, int size);
private:
	GifFrame *getGifFrame(unsigned int id);
	unsigned int getGifFrameCount();
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);
	CC_SYNTHESIZE(unsigned int, _width, Width);
	CC_SYNTHESIZE(unsigned int, _height, Height);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _gifFrameList, GifFrameList);
	CC_SYNTHESIZE_READONLY(int, _frameNo, FrameNo);
	CC_SYNTHESIZE(float, _frameCount, FrameCount);
	CC_SYNTHESIZE(int, _maxDuration, MaxDuration);
	GifFileType *_fileType;
	bool _pause;
};

//-------------------------------------------------------------------------------------------------
/**
 * @brief GIFアニメの表示を管理するクラス。
 */
class GifPlayer : public cocos2d::Sprite, public agtk::BasePlayer
{
private:
	GifPlayer();
	virtual ~GifPlayer();
public:
	static GifPlayer *createWithFilename(std::string filename);
	static GifPlayer *createWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData);
	virtual void update(float dt);
	virtual void play(int actionNo, int actionDirectNo, float seconds = 0.0f, bool bIgnoredSound = false, bool bReverse = false);
	void stop();
public:
	virtual void setPosition(const cocos2d::Vec2& pos) override;
	virtual void setPosition(float x, float y) override;
	virtual const cocos2d::Vec2& getPosition() const;
	virtual void getPosition(float *x, float *y) const;
public:
	virtual void setScale(const cocos2d::Vec2& pos);
	virtual void setScale(float scaleX, float scaleY) override;
	virtual float getScaleX() const;
	virtual float getScaleY() const;
public:
	virtual void setRotation(float rotation) override;
	virtual float getRotation() const;
#ifdef AGTK_DEBUG
public:
	void setVisibleDebugDisplay(bool bVisible);
	PrimitiveNode *getDebugNode();
#endif
private:
	virtual bool initWithFilename(std::string filename);
	virtual bool initWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData);
	void updateGif(float dt);
private:
	CC_SYNTHESIZE_RETAIN(agtk::GifAnimation *, _gifAnimation, GifAnimation);
	bool _updateGifFirstFlag;//初期GIF更新フラグ
};

NS_AGTK_END

#endif	//__GIF_PLAYER_H__
