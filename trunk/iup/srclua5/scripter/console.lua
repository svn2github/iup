
local console = {}

if (not loadstring) then
  loadstring = load
end

function iup.ConsoleInit(txt_cmdline, mtl_output)
  console.cmdList = {}
  console.currentListInd = 0

  console.txtCmdLine = txt_cmdline 
  console.mtlOutput = mtl_output   
  
  console.hold_caret = false

  console.mtlOutput.value = _COPYRIGHT .. "\n" ..
                            "IUP " .. iup._VERSION .. "  " .. iup._COPYRIGHT
end


--------------------- Command History ---------------------


function iup.ConsoleKeyUpCommand()
  if #console.cmdList > 0 then
    if console.currentListInd >= 1 then
      console.txtCmdLine.value = console.cmdList[console.currentListInd]
      if console.currentListInd > 1 then
        console.currentListInd = console.currentListInd - 1
      end
    end
  end
end

function iup.ConsoleKeyDownCommand()
  if #console.cmdList > 0 then
    if console.currentListInd <= #console.cmdList then
      console.txtCmdLine.value = console.cmdList[console.currentListInd]
      if console.currentListInd < #console.cmdList then
        console.currentListInd = console.currentListInd + 1
      end
    end
  end
end

function iup.ConsoleEnterCommandStr(text)
  table.insert(console.cmdList, text)
  console.currentListInd = #console.cmdList
  iup.ConsolePrint("> " .. text)
end

function iup.ConsoleEnterCommand()

  local command = console.txtCmdLine.value
  if command == nil or command == "" then
    return
  end

  iup.ConsoleEnterCommandStr(command)

  local cmd, msg = loadstring(command)
  if (not cmd) then
    cmd = loadstring("return " .. command)
  end
  if (not cmd) then
    iup.ConsolePrint("Error: ".. msg) -- the original error message
  else
    local result = {pcall(cmd)}
    if result[1] then
      for i = 2, #result do
        iup.ConsolePrintValue(result[i])
      end
    else
      iup.ConsolePrint("Error: ".. result[2])
    end
  end

  console.txtCmdLine.value = ""
end


--------------------- Print Replacement ---------------------

function iup.ConsolePrint(...)
  local param = {...}
  local str = ""
  if (#param == 0) then
    str = "nil"
  else
    for i, k in ipairs(param) do 
      if (i > 1) then str = str .."\t" end
      str = str .. tostring(k)
    end
  end
  console.mtlOutput.append = str
  if (not console.hold_caret) then
    console.mtlOutput.scrollto = "99999999:1"
  end
end

print_old = print
print = iup.ConsolePrint

write_old = io.write
io.write = function(...)
  console.mtlOutput.appendnewline="No"
  iup.ConsolePrint(...)
  console.mtlOutput.appendnewline="Yes"
end

function iup.ConsoleValueToString(v)
  if (type(v) == "string") then
    return "\"" .. v .. "\""
  else 
    return tostring(v)
  end
end

function iup.ConsolePrintTable(t)
  local str = "{\n"
  local tmp = {}
  for i, k in ipairs(t) do 
    str = str .. "  ["..tostring(i).. "] = " .. iup.ConsoleValueToString(k) .. ",\n"
    tmp[i] = true
  end
  for i, k in pairs(t) do 
    if (not tmp[i]) then
      str = str .. "  ["..tostring(i).. "] = ".. iup.ConsoleValueToString(k) .. ",\n"
    end
  end
  str = str .. "}"

  iup.ConsolePrint(str)
end

function iup.ConsolePrintFunction(f)
  local info = debug.getinfo(f, "S")
  local str = ""
  if info.what == "C" then    
    str = "   [Defined in C.]"
  else
    local s = string.sub(info.source, 1, 1)
    if s == "@" then
      local filename = string.sub(info.source, 2)
      str = "   [Defined in the file: \"" .. filename .. "\" at line " .. info.linedefined .. ".]"
    else
      str = "   [Defined in a string.]"
    end
  end

  iup.ConsolePrint(str)
end

function iup.ConsolePrintValue(v)
  if (type(v) == "table") then 
    iup.ConsolePrintTable(v)
  elseif (type(v) == "function") then 
    iup.ConsolePrintFunction(v)
  else
    iup.ConsolePrint(iup.ConsoleValueToString(v))
  end
end


--------------------- Utilities ---------------------


function iup.ConsoleListFuncs()
  console.hold_caret = true

  local global = _G
  local n,v = next(global, nil)
  while n ~= nil do
    if type(v) == "function" then
      iup.ConsolePrint(n)
    end
    n,v = next(global, n)
  end

  console.hold_caret = false
  console.mtlOutput.scrollto = "99999999:1"
end

function iup.ConsoleListVars()
  console.hold_caret = true

  local global = _G
  local n,v = next(global, nil)
  while n ~= nil do
    if type(v) ~= "function" and n ~= "_G" then
      iup.ConsolePrint(n)
    end
    n,v = next(global, n)
  end

  console.hold_caret = false
  console.mtlOutput.scrollto = "99999999:1"
end

