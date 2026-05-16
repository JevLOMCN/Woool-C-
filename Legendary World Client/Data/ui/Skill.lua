function LeftPageUp()
	local parent = this:GetParent();
	if((parent:GetSCurPage() > 3 and SELF:GetCareer() == 0) or parent:GetSCurPage() > 4) then
		return;
	end
	parent:SwitchToPage(wnd:GetSCurPage() + 1)
	parent:Update();								
end

function LeftPageDown()
	local parent = this:GetParent();
	if(parent:GetSCurPage() <= 0) then
		return;
	end
	parent:SwitchToPage(parent:GetSCurPage() - 1)
	parent:Update();					
end

function RightPageUp()
	local parent = this:GetParent();
	local scroll = parent:FindScrollByName("RightPagePos");
	if(scroll:GetPos() > 0) then
		scroll:SetPos(scroll:GetPos() - 1);
		parent:Update();
	end
end

function RightPageDown()
	local parent = this:GetParent();
	local MagicNum = GetLearnedMagicNum();
	local TotalMagicPages = math.floor((MagicNum - 1) / 8);
	local scroll = parent:FindScrollByName("RightPagePos");	
	if(scroll:GetPos() < TotalMagicPages) then
		scroll :SetPos(scroll:GetPos() + 1);
		parent:Update();
	end	
end

function GetRightSkillButton(id)
	local MagicID = GetCharacterMagicID(id);
	if(MagicID ==0) then
		this:Disable();
		this:Hide();
	end
		
end