/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/


#ifndef __PLAYER_MESSAGEBOX_SERVICE_PROTOCOL_H
#define __PLAYER_MESSAGEBOX_SERVICE_PROTOCOL_H

#include <string>

#include "PlayerMacros.h"
#include "PlayerServiceProtocol.h"

PLAYER_NS_BEGIN

class PlayerMessageBoxServiceProtocol : public PlayerServiceProtocol
{
public:
    static const int BUTTONS_OK = 0;
    static const int BUTTONS_OK_CANCEL = 1;
    static const int BUTTONS_YES_NO = 2;
    static const int BUTTONS_YES_NO_CANCEL = 3;

    static const int BUTTON_OK = 0;
    static const int BUTTON_CANCEL = 1;
    static const int BUTTON_YES = 2;
    static const int BUTTON_NO = 3;

    // Show a message box, return index of user clicked button
    //
    // @return int first button index is 0
    virtual int showMessageBox(const std::string &title,
                               const std::string &message,
                               int buttonsType = BUTTONS_OK) = 0;
};

PLAYER_NS_END

#endif // __PLAYER_MESSAGEBOX_SERVICE_PROTOCOL_H
