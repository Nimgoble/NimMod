cd /D "C:\Program Files (x86)\MSBuild\12.0\Bin\"
MSBuild "E:\Programming\GitHub\NimMod\NimMod.sln" /p:configuration=DebugGame /p:platform=Windows
MSBuild "E:\Programming\GitHub\NimMod\NimMod.sln" /p:configuration="DebugGame Editor" /p:platform=Windows
MSBuild "E:\Programming\GitHub\NimMod\NimMod.sln" /p:configuration=Development /p:platform=Windows
MSBuild "E:\Programming\GitHub\NimMod\NimMod.sln" /p:configuration="Development Editor" /p:platform=Windows

pause