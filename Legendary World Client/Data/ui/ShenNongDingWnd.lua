ShenNongDingWnd={}
local XML=[[
<Window CloseBtnID = "23018" CloseBtnX = "560" CloseBtnY = "6" BackTextureIndex="6601" bSearchChildGood="1">
    <Text  PosX="300" PosY="7" Text="神农鼎" FontSize="22" Font="1" FontColor="0xFFEA9E54" FontFlag="24"/>
    <Text  PosX="116" PosY="214" Text="治疗效果" FontSize="14" Font="0" FontColor="0xFFEA9E54" FontFlag="4"/>
    <MarkView PosX="42" PosY="70" Row ="12" RowHeight="16" Font="0" Color="0xFFEA9E54" TagText="$ExplainText" Width="216" Height="124"/>
    <Image PosX="270" PosY="65"  TextID="6602"/>
    <Image PosX="391" PosY="180"  TextID="6603" EffTextID="6612" RenderMode="1"/>
    <Text  PosX="54" PosY="245" Text="病症：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="90" PosY="245" Text="$bingzheng" FontSize="12" Font="0" FontColor="$bzColor" FontFlag="4"/>
    <Text  PosX="54" PosY="270" Text="症状：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="90" PosY="270" Text="$zhengzhuang" FontSize="12" Font="0" FontColor="$zzColor" FontFlag="4"/>
    <Text  PosX="54" PosY="295" Text="药性：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="90" PosY="295" Text="$yaoxing" FontSize="12" Font="0" FontColor="$bzColor" FontFlag="4"/>
    <Text  PosX="54" PosY="320" Text="药效：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="90" PosY="320" Text="$yaoxiao" FontSize="12" Font="0" FontColor="$yxColor" FontFlag="4"/>
    <Text  PosX="54" PosY="345" Text="奖励：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="90" PosY="345" Text="$jiangli" FontSize="12" Font="0" FontColor="0xFFEA9E54" FontFlag="4"/>
    <CGoodGridNL Name="SNDGG1" PosX="346" PosY="138" BackTex="20008" iBackOffX="6" iBackOffY = "6" bDrawBackColor="1" bCheckDropGoodCanContain="1" TipsNoGood="放入草药"
        OnGridDGCheck="OnGridDGCheck1" OnGridGoodChg="OnGridGoodChg"/>
    <CGoodGridNL Name="SNDGG2" PosX="458" PosY="138" BackTex="20008" iBackOffX="6" iBackOffY = "6" bDrawBackColor="1" bCheckDropGoodCanContain="1" TipsNoGood="放入草药"
        OnGridDGCheck="OnGridDGCheck2" OnGridGoodChg="OnGridGoodChg"/>
    <Image PosX="404" PosY="240"  TextID="930" TextPack="16"/>
    <Image PosX="389" PosY="229"  TextID="6972" TextPack="17"/>
    <button PosX="289" PosY="374" Text="炼  药" NormalTexture="1210" HighlightTexture="1211" PushedTexture="1212" OnClick="OnClickExt"/>
    <button PosX="430" PosY="374" Text="一键放入" NormalTexture="1210" HighlightTexture="1211" PushedTexture="1212" OnClick="OnClickPut"/>
    <effect Name="suceft1" PosX="186" PosY="-18"  TextID="6608" PlayMode="0"/>
    <effect Name="suceft2" PosX="185" PosY="-18"  TextID="6610" PlayMode="0"/>
</Window>
]]	

local XMLSC=[[
<Window CloseBtnID = "33018" CloseBtnX = "531" CloseBtnY = "7" BackTextureIndex="36398" bSearchChildGood="1">
    <Text  PosX="281" PosY="20" Text="神农鼎" FontSize="24" Font="1" FontColor="0xFFE6B260" FontFlag="24"/>
    <Text  PosX="113" PosY="56" Text="提示" FontSize="14" Font="0" FontColor="0xFFE6C800" FontFlag="4"/>
    <Text  PosX="99" PosY="256" Text="治疗效果" FontSize="14" Font="0" FontColor="0xFFE6C800" FontFlag="4"/>
    <MarkView PosX="20" PosY="82" Row ="12" RowHeight="16" Font="0" Color="0xFFEA9E54" TagText="$ExplainText" Width="216" Height="166"/>
    <Text  PosX="24" PosY="285" Text="病症：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="60" PosY="285" Text="$bingzheng" FontSize="12" Font="0" FontColor="$bzColor" FontFlag="4"/>
    <Text  PosX="24" PosY="307" Text="症状：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="60" PosY="307" Text="$zhengzhuang" FontSize="12" Font="0" FontColor="$zzColor" FontFlag="4"/>
    <Text  PosX="24" PosY="329" Text="药性：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="60" PosY="329" Text="$yaoxing" FontSize="12" Font="0" FontColor="$bzColor" FontFlag="4"/>
    <Text  PosX="24" PosY="351" Text="药效：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="60" PosY="351" Text="$yaoxiao" FontSize="12" Font="0" FontColor="$yxColor" FontFlag="4"/>
    <Text  PosX="24" PosY="373" Text="奖励：" FontSize="12" Font="0" FontColor="0xFF9E8B78" FontFlag="4"/>
    <Text  PosX="60" PosY="373" Text="$jiangli" FontSize="12" Font="0" FontColor="0xFFEA9E54" FontFlag="4"/>
    <CGoodGridNL Name="SNDGG1" PosX="326" PosY="110" BackTex="33012" iBackOffX="4" iBackOffY = "4" bDrawBackColor="1" bCheckDropGoodCanContain="1" TipsNoGood="放入草药"
        OnGridDGCheck="OnGridDGCheck1" OnGridGoodChg="OnGridGoodChg"/>
    <CGoodGridNL Name="SNDGG2" PosX="426" PosY="110" BackTex="33012" iBackOffX="4" iBackOffY = "4" bDrawBackColor="1" bCheckDropGoodCanContain="1" TipsNoGood="放入草药"
        OnGridDGCheck="OnGridDGCheck2" OnGridGoodChg="OnGridGoodChg"/>
    <Image PosX="377" PosY="199"  TextID="930" TextPack="16"/>
    <Image PosX="362" PosY="188"  TextID="6972" TextPack="17"/>
    <button PosX="285" PosY="374" Text="炼  药" NormalTexture="33024" HighlightTexture="33025" PushedTexture="33024" OnClick="OnClickExt"/>
    <button PosX="415" PosY="374" Text="一键放入" NormalTexture="33024" HighlightTexture="33025" PushedTexture="33024" OnClick="OnClickPut"/>
    <effect Name="suceft1" PosX="160" PosY="-60"  TextID="6608" PlayMode="0"/>
    <effect Name="suceft2" PosX="160" PosY="-60"  TextID="6610" PlayMode="0"/>
</Window>
]]	


local LocalWndName = 'ShenNongDingWnd';

local DATA={
    ['ExplainText']="",
    ['bingzheng']="",
    ['bzColor']="0xFFEA9E54",
    ['bingzhengidx']=0,
    ['zhengzhuang']="",
    ['zzColor']="0xFFEA9E54",
    ['yaoxing']="",
    ['yaoxiao']="",
    ['yxColor']="0xFFEA9E54",
    ['jiangli']="",
};


function ShenNongDingWnd.GetRenderXml()
    if GlobalTool.GFnUseShiChangUi() then
        return XMLSC
    end
    return XML
end

function ShenNongDingWnd.GetRenderData()
    return DATA
end

--消息处理
function ShenNongDingWnd.Msg(msg)
	if msg['MsgType'] == 0 then
        local var = msg['StrList']
        if var[1] ~= nil then
            DATA['bingzheng']=var[1]
        end
        if var[2] ~= nil then
            DATA['zhengzhuang']=var[2]
        end
        if var[3] ~= nil then
            DATA['yaoxing']=var[3]
        end
        if var[4] ~= nil then
            DATA['ExplainText']=var[4]
        end

        if DATA['bingzheng'] == "风症" then
            DATA['bzColor'] = "0xffdba077"
        elseif DATA['bingzheng'] == "寒症" then
            DATA['bzColor'] = "0xff00ffff"
        elseif DATA['bingzheng'] == "毒症" then
            DATA['bzColor'] = "0xff00ff00"
        elseif DATA['bingzheng'] == "虚症" then
            DATA['bzColor'] = "0xffa5a5a5"
        end

        if DATA['zhengzhuang'] == "普通" then
            DATA['zzColor'] = "0xff92d050"
        elseif DATA['zhengzhuang'] == "严重" then
            DATA['zzColor'] = "0xffffff00"
        elseif DATA['zhengzhuang'] == "危急" then
            DATA['zzColor'] = "0xffff0000"
        end

        --yao0~yaon 药材名 jie0~jie0放的是接触病症0~n所需要的药材
        local jj = 5
        while var[jj] ~= nil do 
            local result = GlobalTool.split(var[jj], '#')

            local idx = 1
            --服务器发过来第一个数字表示药材数量
            local yaonum = result[1]
            idx = idx + 1

            local yao = "yao" .. (jj - 5)
            if DATA[yao] == nil then
                DATA[yao] = {}
            end
            for ii = 1, yaonum do
                if result[idx] ~= nil then
                    table.insert(DATA[yao],result[idx])
                end
                idx = idx + 1
            end

            local jienum = table.getn(result) - yaonum - 1
            for ii = 1, jienum do
                local jie = "jie"..result[idx]
                if DATA[jie] == nil then
                        DATA[jie] = {}
                    end
                    table.insert(DATA[jie],yao)
                idx = idx + 1
            end
            jj = jj + 1
        end

        local vNum = msg['NumList']
        if vNum[1] then
            DATA['bingzhengidx'] = vNum[1]
        end
        UpdateWnd(LocalWndName);
    elseif msg['MsgType'] == 1 then
        local vNum = msg['NumList']
        local vStr = msg['StrList']
        if vStr[1] ~= nil then
            DATA['yaoxiao']=vStr[1]
        end

        if DATA['yaoxiao'] == "杯水车薪" then
            DATA['yxColor'] = "0xffffffff"
        elseif DATA['yaoxiao'] == "药到病除" then
            DATA['yxColor'] = "0xff139c00"
        elseif DATA['yaoxiao'] == "灵丹妙药" then
            DATA['yxColor'] = "0xff1064ef"
        elseif DATA['yaoxiao'] == "起死回生" then
            DATA['yxColor'] = "0xffd879ec"
        end

        if vStr[2] ~= nil and vNum[1] ~= nil then
            DATA['jiangli']=vStr[2]..vNum[1]
        end
        UpdateWnd(LocalWndName);
    elseif msg['MsgType'] == 2 then
        local msgto = {
            ['MsgType']=3,
            ['WndName']=LocalWndName,
            ['ControlName']="suceft1",
            ['VecInt']={1}
        }
        MsgToWnd(msgto)
        msgto['ControlName']="suceft2"
        MsgToWnd(msgto)

        local vStr = msg['StrList']
        if vStr[1] ~= nil then
            local str = "恭喜您，已完成了炼药！\n"
            str = str.."病人服用了您炼制的丹药，已经得到了治疗。\n"
            str = str.."本次炼药的效果为："..vStr[1].."，请查收您的奖励。\n"
            str = str.."高品质草药，炼制出的丹药疗效越强，获得的奖励也越多。\n"
            str = str.."望您在下次治疗中再接再厉！\n"
            MsgBox_PopSimpleMsgBox(1, str, 1, LocalWndName)
        end
	end
end

function ShenNongDingWnd.OnGridDGCheck1()
    local good = Good_GetDropGood()
    local strName = Good_GetGoodAttrStr(good, "name", "", "")

    local ii = 0
    while true do 
        local str = "yao"..ii
        if DATA[str] ~= nil then
            local num = table.getn(DATA[str])
            for jj = 1, num do
                if DATA[str][jj] == strName then
                    return true
                end
            end
        else
            break
        end
        ii = ii + 1
    end
    return false
end

function ShenNongDingWnd.OnGridDGCheck2()
    local good = Good_GetDropGood()
    local strName = Good_GetGoodAttrStr(good, "name", "", "")

    local ii = 0
    while true do 
        local str = "yao"..ii
        if DATA[str] ~= nil then
            local num = table.getn(DATA[str])
            for jj = 1, num do
                if DATA[str][jj] == strName then
                    return true
                end
            end
        else
            break
        end
        ii = ii + 1
    end
    return false
end

function ShenNongDingWnd.OnGridGoodChg()
    local good1 = Good_GetSingleGoodGG(LocalWndName, "SNDGG1")
    local id1 = Good_GetGoodAttrInt(good1, "id", LocalWndName, "SNDGG1")
    local good2 = Good_GetSingleGoodGG(LocalWndName, "SNDGG2")
    local id2 = Good_GetGoodAttrInt(good2, "id", LocalWndName, "SNDGG2")
    if id1 > 0 and id2 > 0 then
        local Msg = {
                ['ActName'] = "ShiYao2024",
                ['ActType'] = 1,
                ['ListInt'] = {id1, id2},
                }
		SendMsg(Msg)
    else
        DATA['yaoxiao']=""
        DATA['jiangli']=""
        UpdateWnd(LocalWndName);
    end
end

function ShenNongDingWnd.OnClickExt()
    local good1 = Good_GetSingleGoodGG(LocalWndName, "SNDGG1")
    local id1 = Good_GetGoodAttrInt(good1, "id", LocalWndName, "SNDGG1")
    local good2 = Good_GetSingleGoodGG(LocalWndName, "SNDGG2")
    local id2 = Good_GetGoodAttrInt(good2, "id", LocalWndName, "SNDGG2")
    if id1 > 0 and id2 > 0 then
        local Msg = {
                ['ActName'] = "ShiYao2024",
                ['ActType'] = 2,
                ['ListInt'] = {id1, id2},
                }
		SendMsg(Msg)
    end
end

--一键放入
function ShenNongDingWnd.OnClickPut()
    Good_SingleGoodBackToPack(LocalWndName, "SNDGG1")
    Good_SingleGoodBackToPack(LocalWndName, "SNDGG2")

    --遍历背包 找到最高等级每种药
    local packagesize = SELF_GetSelfAttrInt("packagesize2")
    local allGood = {}
    local allGoodLv = {}
    for ii = 1, packagesize do
        local good = Good_GetPackageGood(ii-1, false)
        local strName = Good_GetGoodAttrStr(good, "name", "", "")

        local ii = 0
        while true do 
            local str = "yao"..ii
            if DATA[str] ~= nil then
                local num = table.getn(DATA[str])
                for jj = 1, num do
                    if DATA[str][jj] == strName then
                        if allGoodLv[str] == nil then
                            allGood[str]=good
                            allGoodLv[str]=jj
                        elseif allGoodLv[str] > jj then
                            allGood[str]=good
                            allGoodLv[str]=jj
                        end
                        break
                    end
                end
            else
                break
            end
            ii = ii + 1
        end
    end

    --记录哪个yao已经被放入
    local strTemp = ""
    --2个格子放入药材 先找对症的 
    local bzStr = "jie"..DATA['bingzhengidx']
    for ii=1,2 do 
        if DATA[bzStr] ~= nil and DATA[bzStr][ii] ~= nil then
            local yao = DATA[bzStr][ii]
            if allGood[yao] ~= nil then
                local ctrlName = "SNDGG"..ii
                strTemp = yao
                Good_SetSingleGoodGG(allGood[yao],LocalWndName, ctrlName)
            end
        end
    end

    --再找不对症中品质最高的
    local good1 = Good_GetSingleGoodGG(LocalWndName, "SNDGG1")
    local good2 = Good_GetSingleGoodGG(LocalWndName, "SNDGG2")
    local id1 = Good_GetGoodAttrInt(good1, "id", LocalWndName, "SNDGG1")
    local id2 = Good_GetGoodAttrInt(good2, "id", LocalWndName, "SNDGG2")
    local vId = {id1, id2}
    
    for ii=1,2 do 
        if vId[ii] == 0 then
            local lvDef = 1000
            local goodDef
            local bFind = false
            for v in pairs(allGoodLv) do 
                if allGoodLv[v] < lvDef and v ~= strTemp then
                    goodDef=allGood[v]
                    lvDef = allGoodLv[v]
                    strTemp = v
                    bFind = true
                end
            end
            if bFind then
                local ctrlName = "SNDGG"..ii
                Good_SetSingleGoodGG(goodDef,LocalWndName, ctrlName)
            end
        end
    end

    id1 = Good_GetGoodAttrInt(good1, "id", LocalWndName, "SNDGG1")
    id2 = Good_GetGoodAttrInt(good2, "id", LocalWndName, "SNDGG2")
    if id1 > 0 and id2 > 0 then
        local Msg = {
                ['ActName'] = "ShiYao2024",
                ['ActType'] = 1,
                ['ListInt'] = {id1, id2},
                }
		SendMsg(Msg)
    else
        DATA['yaoxiao']=""
        DATA['jiangli']=""
        UpdateWnd(LocalWndName);
    end
end
-- SetAttr(good, propKery, propval)
-- SetAttrMulti(good, map)