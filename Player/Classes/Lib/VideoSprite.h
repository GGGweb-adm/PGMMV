#ifndef __VIDEO_SPRITE_H__
#define	__VIDEO_SPRITE_H__

#include "Lib/Macros.h"
#include "Lib/Common.h"
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
#include "vlc/vlc.h"
#else
#endif

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------
class AGTKPLAYER_API VideoSprite : public cocos2d::Sprite
{
public:
	enum EnumState {
		kStateIdle,
		kStatePlay,
		kStateStop,
		kStatePause,
		kStateResume,
		kStateMax,
	};
private:
	VideoSprite();
	virtual ~VideoSprite();
public:
	static VideoSprite *createWithFilename(std::string path, cocos2d::Size size = cocos2d::Size::ZERO, bool bLoop = false);
	const char *getFilename();
	void play(float volume = 1.0f);
	void stop();
	void pause();
	void resume();
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;
	virtual void update(float dt) override;
	bool isEndReached() { return _endReached; }
	bool isPause() { return _pause; }
	void setVolume(float volume, float seconds = 0.0f);
	float getVolume();
	libvlc_state_t getVlcState();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
private:
	virtual bool initWithFilename(std::string path, cocos2d::Size size, bool bLoop);
	static void *lock(void *data, void **p_pixels);
	static void unlock(void *data, void *id, void *const *p_pixels);
	static void display(void *data, void *id);
	static void endReached(const struct libvlc_event_t *event, void *data);
	static unsigned videoSetup(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);
	CC_SYNTHESIZE(EnumState, _state, State);
	agtk::ValueTimer<float> *_volume;
	libvlc_instance_t *_vlc;
	libvlc_media_player_t *_vlcPlayer;
	//image
	unsigned char *_buffer;
	CC_SYNTHESIZE(unsigned int, _texWidth, TexWidth);
	CC_SYNTHESIZE(unsigned int, _texHeight, TexHeight);
	bool _endReached;
	bool _pause;
};

NS_AGTK_END

#endif	//__VIDEO_SPRITE_H__
