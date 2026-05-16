

GlobalTool = {}

--打印table 表
function GlobalTool.PrintTable(table1)
	for ii,v in pairs(table1) do

		if type(v) == 'table' then 
			PrintTable(v)
		elseif type(v) == 'string' then
			Log (ii .. " :" .. v)
		elseif type(v) == 'number' then
		Log (ii .. " :" .. v)
		end
	end
end

--分割字符串
function GlobalTool.split(str, reps)
--Log(str)
	local resultStr = {}
	local i=1
	string.gsub(str,'[^'..reps..']+',function(w)
	table.insert(resultStr,i,w)
	i= i + 1
	end)
	return resultStr
end

function GlobalTool.TableSize(table1)
	local iCount = 0;
	for ii,v in pairs(table1) do
		iCount = iCount + 1;
	end
	print(123)
	return iCount;
end

function GlobalTool.GFnUseShiChangUi()
    local servertype = GetServerType()
    if servertype == 12 or servertype == 15 then
        return true
    end
    return false
end

--脚本协议 begin
function GlobalTool.Msg_UtilsActResultLua(msg)
    local strActName = msg['ActName']
    if strActName == "pet_huaxing" then
        GlobalTool.pet_huaxing(msg)
        return true
    end

    if strActName == "ShiYao2024" then
        GlobalTool.ShiYao2024(msg)
        return true
    end

	return false
end

--试药
function GlobalTool.ShiYao2024(msg)
    if msg['MsgType'] == 0 then
        OperWindow("ShenNongDingWnd", "create", 0, true)
        LuaMsgToLuaWnd("ShenNongDingWnd", msg)
    elseif msg['MsgType'] == 1 or msg['MsgType'] == 2 then
        LuaMsgToLuaWnd("ShenNongDingWnd", msg)
    end
end

--化形神炉
function GlobalTool.pet_huaxing(msg)
    if msg['MsgType'] == 0 then
        OperWindow("HuaXingShenLuWnd", "create", 0, true)
        LuaMsgToLuaWnd("HuaXingShenLuWnd", msg)
    elseif msg['MsgType'] == 1 or msg['MsgType'] == 2 then
        LuaMsgToLuaWnd("HuaXingShenLuWnd", msg)
    end
end
--脚本协议 end

return GlobalTool