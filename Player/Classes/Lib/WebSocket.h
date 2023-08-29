#ifndef __WEBSOCKET_H__
#define	__WEBSOCKET_H__

#include "Lib/Macros.h"
#include "network/WebSocket.h"

NS_AGTK_BEGIN
class WebSocket : public cocos2d::Ref, public cocos2d::network::WebSocket::Delegate
{
public:
	WebSocket();
	virtual ~WebSocket();
	CREATE_FUNC(WebSocket);

public:
	std::function<void()> onConnectionOpened;
	std::function<void(std::string message)> onMessageReceived;
	std::function<void(char *binaryMessage, unsigned int len)> onBinaryMessageReceived;
	std::function<void()> onConnectionClosed;
	std::function<void(const cocos2d::network::WebSocket::ErrorCode &error)> onErrorOccurred;

private:
	virtual bool init();
	virtual void onOpen(cocos2d::network::WebSocket *ws);
	virtual void onMessage(cocos2d::network::WebSocket *ws, const cocos2d::network::WebSocket::Data& data);
	virtual void onClose(cocos2d::network::WebSocket *ws);
	virtual void onError(cocos2d::network::WebSocket *ws, const cocos2d::network::WebSocket::ErrorCode& error);

public:
	void connect(const std::string hostname, int port);
	void disconnect();
	bool send(const std::string msg);
	bool send(const unsigned char *binaryMsg, unsigned int len);
	int getState();
private:
	cocos2d::network::WebSocket *_websocket;
};
NS_AGTK_END

#endif	//__WEBSOCKET_H__
