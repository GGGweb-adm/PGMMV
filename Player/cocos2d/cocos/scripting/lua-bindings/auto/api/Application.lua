
--------------------------------
-- @module Application
-- @parent_module cc

--------------------------------
-- brief Get target platform
-- @function [parent=#Application] getTargetPlatform 
-- @param self
-- @return int#int ret (return value: int)
        
--------------------------------
-- brief Get current language config<br>
-- return Current language config
-- @function [parent=#Application] getCurrentLanguage 
-- @param self
-- @return int#int ret (return value: int)
        
--------------------------------
-- brief Get current language iso 639-1 code<br>
-- return Current language iso 639-1 code
-- @function [parent=#Application] getCurrentLanguageCode 
-- @param self
-- @return char#char ret (return value: char)
        
--------------------------------
-- brief Open url in default browser<br>
-- param String with url to open.<br>
-- return true if the resource located by the URL was successfully opened; otherwise false.
-- @function [parent=#Application] openURL 
-- @param self
-- @param #string url
-- @return bool#bool ret (return value: bool)
        
--------------------------------
-- brief Get application version.
-- @function [parent=#Application] getVersion 
-- @param self
-- @return string#string ret (return value: string)
        
--------------------------------
-- @overload self, float, int         
-- @overload self, float         
-- @function [parent=#Application] setAnimationInterval
-- @param self
-- @param #float interval
-- @param #int reason
-- @return Application#Application self (return value: cc.Application)

--------------------------------
-- brief    Get current application instance.<br>
-- return Current application instance pointer.
-- @function [parent=#Application] getInstance 
-- @param self
-- @return Application#Application ret (return value: cc.Application)
        
return nil
