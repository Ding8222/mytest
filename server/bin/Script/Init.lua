package.path = "./Script/?.lua;"..package.path

function test()
	local n = 0
	for i=0,10000000 do
		n = n + 1
	end
end

local n = 0
function test2()
	n = n + 1
end

SetScriptInitFlag(true)
