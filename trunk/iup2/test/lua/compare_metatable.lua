
l = iuplist{}
b = iupbutton{title="xpto"}
d = iupdialog{iuphbox{l, b}}
d:show()
x = IupGetFocus()
print("x=", x, "b=", b)
if x == b then
  print("� igual")
else
  print("� diferente")
end

