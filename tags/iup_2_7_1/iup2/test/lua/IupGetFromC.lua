
b = iupbutton{title="nen"}
IupSetHandle("seila", b)
a = IupGetFromC{"seila"}
IupMessage("veio", tostring(a))
if a == b then
  IupMessage('isso', '� s� dar commit')
end
--IupGetFromC("sdfdf")
IupGetFromC{}
