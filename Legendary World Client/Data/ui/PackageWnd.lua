function ClickGridGood(x,y)
	local GridX = x / 39;
	local GridY = y / 35;
	ClickPackageGood(math.floor(GridY) * 10 + math.floor(GridX));				
end

function UseGridGood(x,y)
	local GridX = x / 39;
    	local GridY = y / 35;
    	UsePackageGood(math.floor(GridY) * 10 + math.floor(GridX));		
end