WooolFarmWnd={}
local XML=[[
<Window CloseBtnID = "23018" CloseBtnX = "720" CloseBtnY = "5" BackTextureIndex="32632">
</Window>
]]	

local LocalWndName = 'WooolFarmWnd';

local DATA={
};

function WooolFarmWnd.GetRenderXml()
    return XML
end

function WooolFarmWnd.GetRenderData()
    return DATA
end