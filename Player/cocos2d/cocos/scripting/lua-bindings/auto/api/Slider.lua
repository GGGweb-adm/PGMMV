
--------------------------------
-- @module Slider
-- @extend Widget
-- @parent_module ccui

--------------------------------
-- Changes the progress direction of slider.<br>
-- param percent  Percent value from 1 to 100.
-- @function [parent=#Slider] setPercent 
-- @param self
-- @param #int percent
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- Query the maximum percent of Slider. The default value is 100.<br>
-- since v3.7<br>
-- return The maximum percent of the Slider.
-- @function [parent=#Slider] getMaxPercent 
-- @param self
-- @return int#int ret (return value: int)
        
--------------------------------
-- Load normal state texture for slider ball.<br>
-- param normal    Normal state texture.<br>
-- param resType    @see TextureResType .
-- @function [parent=#Slider] loadSlidBallTextureNormal 
-- @param self
-- @param #string normal
-- @param #int resType
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- Load dark state texture for slider progress bar.<br>
-- param fileName   File path of texture.<br>
-- param resType    @see TextureResType .
-- @function [parent=#Slider] loadProgressBarTexture 
-- @param self
-- @param #string fileName
-- @param #int resType
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- 
-- @function [parent=#Slider] getBallNormalFile 
-- @param self
-- @return ResourceData#ResourceData ret (return value: cc.ResourceData)
        
--------------------------------
-- 
-- @function [parent=#Slider] getSlidBallDisabledRenderer 
-- @param self
-- @return Sprite#Sprite ret (return value: cc.Sprite)
        
--------------------------------
-- Sets if slider is using scale9 renderer.<br>
-- param able True that using scale9 renderer, false otherwise.
-- @function [parent=#Slider] setScale9Enabled 
-- @param self
-- @param #bool able
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- 
-- @function [parent=#Slider] getBallPressedFile 
-- @param self
-- @return ResourceData#ResourceData ret (return value: cc.ResourceData)
        
--------------------------------
-- brief Return a zoom scale<br>
-- since v3.3
-- @function [parent=#Slider] getZoomScale 
-- @param self
-- @return float#float ret (return value: float)
        
--------------------------------
-- Sets capinsets for progress bar slider, if slider is using scale9 renderer.<br>
-- param capInsets Capinsets for progress bar slider.<br>
-- js NA
-- @function [parent=#Slider] setCapInsetProgressBarRenderer 
-- @param self
-- @param #rect_table capInsets
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- Load textures for slider ball.<br>
-- param normal     Normal state texture.<br>
-- param pressed    Pressed state texture.<br>
-- param disabled    Disabled state texture.<br>
-- param texType    @see TextureResType .
-- @function [parent=#Slider] loadSlidBallTextures 
-- @param self
-- @param #string normal
-- @param #string pressed
-- @param #string disabled
-- @param #int texType
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- 
-- @function [parent=#Slider] getSlidBallRenderer 
-- @param self
-- @return Node#Node ret (return value: cc.Node)
        
--------------------------------
-- Add call back function called when slider's percent has changed to slider.<br>
-- param callback An given call back function called when slider's percent has changed to slider.
-- @function [parent=#Slider] addEventListener 
-- @param self
-- @param #function callback
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- Set a large value could give more control to the precision.<br>
-- since v3.7<br>
-- param percent The max percent of Slider.
-- @function [parent=#Slider] setMaxPercent 
-- @param self
-- @param #int percent
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- Load texture for slider bar.<br>
-- param fileName   File name of texture.<br>
-- param resType    @see TextureResType .
-- @function [parent=#Slider] loadBarTexture 
-- @param self
-- @param #string fileName
-- @param #int resType
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- 
-- @function [parent=#Slider] getProgressBarFile 
-- @param self
-- @return ResourceData#ResourceData ret (return value: cc.ResourceData)
        
--------------------------------
-- Gets capinsets for bar slider, if slider is using scale9 renderer.<br>
-- return capInsets Capinsets for bar slider.
-- @function [parent=#Slider] getCapInsetsBarRenderer 
-- @param self
-- @return rect_table#rect_table ret (return value: rect_table)
        
--------------------------------
-- Gets capinsets for progress bar slider, if slider is using scale9 renderer.<br>
-- return Capinsets for progress bar slider.<br>
-- js NA
-- @function [parent=#Slider] getCapInsetsProgressBarRenderer 
-- @param self
-- @return rect_table#rect_table ret (return value: rect_table)
        
--------------------------------
-- 
-- @function [parent=#Slider] getSlidBallPressedRenderer 
-- @param self
-- @return Sprite#Sprite ret (return value: cc.Sprite)
        
--------------------------------
-- Load pressed state texture for slider ball.<br>
-- param pressed    Pressed state texture.<br>
-- param resType    @see TextureResType .
-- @function [parent=#Slider] loadSlidBallTexturePressed 
-- @param self
-- @param #string pressed
-- @param #int resType
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- 
-- @function [parent=#Slider] getBackFile 
-- @param self
-- @return ResourceData#ResourceData ret (return value: cc.ResourceData)
        
--------------------------------
-- Gets If slider is using scale9 renderer.<br>
-- return True that using scale9 renderer, false otherwise.
-- @function [parent=#Slider] isScale9Enabled 
-- @param self
-- @return bool#bool ret (return value: bool)
        
--------------------------------
-- 
-- @function [parent=#Slider] getBallDisabledFile 
-- @param self
-- @return ResourceData#ResourceData ret (return value: cc.ResourceData)
        
--------------------------------
-- Sets capinsets for bar slider, if slider is using scale9 renderer.<br>
-- param capInsets Capinsets for bar slider.
-- @function [parent=#Slider] setCapInsetsBarRenderer 
-- @param self
-- @param #rect_table capInsets
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- Gets the progress direction of slider.<br>
-- return percent Percent value from 1 to 100.
-- @function [parent=#Slider] getPercent 
-- @param self
-- @return int#int ret (return value: int)
        
--------------------------------
-- Sets capinsets for slider, if slider is using scale9 renderer.<br>
-- param capInsets Capinsets for slider.
-- @function [parent=#Slider] setCapInsets 
-- @param self
-- @param #rect_table capInsets
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- Load disabled state texture for slider ball.<br>
-- param disabled   Disabled state texture.<br>
-- param resType    @see TextureResType .
-- @function [parent=#Slider] loadSlidBallTextureDisabled 
-- @param self
-- @param #string disabled
-- @param #int resType
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- 
-- @function [parent=#Slider] getSlidBallNormalRenderer 
-- @param self
-- @return Sprite#Sprite ret (return value: cc.Sprite)
        
--------------------------------
--  When user pressed the button, the button will zoom to a scale.<br>
-- The final scale of the button  equals (button original scale + _zoomScale)<br>
-- since v3.3
-- @function [parent=#Slider] setZoomScale 
-- @param self
-- @param #float scale
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- @overload self, string, string, int         
-- @overload self         
-- @function [parent=#Slider] create
-- @param self
-- @param #string barTextureName
-- @param #string normalBallTextureName
-- @param #int resType
-- @return Slider#Slider ret (return value: ccui.Slider)

--------------------------------
-- 
-- @function [parent=#Slider] createInstance 
-- @param self
-- @return Ref#Ref ret (return value: cc.Ref)
        
--------------------------------
-- 
-- @function [parent=#Slider] getVirtualRenderer 
-- @param self
-- @return Node#Node ret (return value: cc.Node)
        
--------------------------------
-- 
-- @function [parent=#Slider] ignoreContentAdaptWithSize 
-- @param self
-- @param #bool ignore
-- @return Slider#Slider self (return value: ccui.Slider)
        
--------------------------------
-- Returns the "class name" of widget.
-- @function [parent=#Slider] getDescription 
-- @param self
-- @return string#string ret (return value: string)
        
--------------------------------
-- 
-- @function [parent=#Slider] hitTest 
-- @param self
-- @param #vec2_table pt
-- @param #cc.Camera camera
-- @param #vec3_table p
-- @return bool#bool ret (return value: bool)
        
--------------------------------
-- 
-- @function [parent=#Slider] init 
-- @param self
-- @return bool#bool ret (return value: bool)
        
--------------------------------
-- 
-- @function [parent=#Slider] getVirtualRendererSize 
-- @param self
-- @return size_table#size_table ret (return value: size_table)
        
--------------------------------
-- Default constructor.<br>
-- js ctor<br>
-- lua new
-- @function [parent=#Slider] Slider 
-- @param self
-- @return Slider#Slider self (return value: ccui.Slider)
        
return nil
