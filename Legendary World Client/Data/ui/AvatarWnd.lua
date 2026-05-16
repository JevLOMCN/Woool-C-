function GetBackgroundColor(type)
	local good = GetDropGood();
	if(good:GetID() == 0) then
		this:SetBack(0);
	elseif(IsCanContain(type,good:GetStdMode()) 
		and ((SELF:GetLevel() >= good:GetNeedLevel() and good:GetNeed() == 0) 
			or good:GetNeed() > 0)) 
	then
		this:SetBack(2147548928);
	else	
		this:SetBack(2164195328);
	end
end

function ShowTip(iX,iY)	
	local tip = this:FindTipByName("DarkExpTip");
	tip:Clear();
	tip:Show();
	if(iX >=28 and iX <= 82 and iY >= 199 and iY <= 392) then
		if(iY >= 199 and iY <= 208 ) 
		then
			tip:SetText("上限加成："..SelfDemonAttr:Get_EXP_AC2().." ",false);
		elseif(iY >= 209 and iY <= 219)	then
			tip:SetText("下限加成："..SelfDemonAttr:Get_EXP_AC1().." ",false);
		elseif(iY >= 245 and iY <= 253)	then
			tip:SetText("上限加成："..SelfDemonAttr:Get_EXP_MAC2().." ",false);
		elseif(iY >= 254 and iY <= 263)	then
			tip:SetText("下限加成："..SelfDemonAttr:Get_EXP_MAC1().." ",false);
		elseif(iY >= 286 and iY <= 296)	then
			tip:SetText("上限加成："..SelfDemonAttr:Get_EXP_DC2().." ",false);
		elseif(iY >= 297 and iY <= 306)	then
			tip:SetText("下限加成："..SelfDemonAttr:Get_EXP_DC1().." ",false);
		elseif(iY >= 329 and iY <= 339)	then
			tip:SetText("上限加成："..SelfDemonAttr:Get_EXP_MC2().." ",false);
		elseif(iY >= 340 and iY <= 348)	then
			tip:SetText("下限加成："..SelfDemonAttr:Get_EXP_MC1().." ",false);
		elseif(iY >= 370 and iY <= 380)	then
			tip:SetText("上限加成："..SelfDemonAttr:Get_EXP_SC2().." ",false);
		elseif(iY >= 381 and iY <= 389)	then
			tip:SetText("下限加成："..SelfDemonAttr:Get_EXP_SC1().." ",false);
		else 
			tip:Hide();
		end
	else
		tip:Hide();		
	end	
	
	tip:Move(iX,iY - 20);			
end