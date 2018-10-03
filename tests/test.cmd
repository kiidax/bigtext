del result.txt
..\x64\Release\boar.exe sample -s -c 2 test.txt test2.txt -o result.txt

exit /b

..\x64\Release\boar.exe vocab -c ..\boar\test2.txt
..\x64\Release\boar.exe vocab ..\boar\test2.txt
..\x64\Release\boar.exe vocab -c ..\boar\test2.txt
..\x64\Release\boar.exe vocab ..\boar\test2.txt

del result.txt
..\x64\Release\boar.exe sample -c ..\boar\test2.txt - 0.1 result.txt
del result.txt
..\x64\Release\boar.exe sample ..\boar\test2.txt - 0.1 result.txt
del result.txt
..\x64\Release\boar.exe sample -c ..\boar\test2.txt - 0.1 result.txt
del result.txt
..\x64\Release\boar.exe sample ..\boar\test2.txt - 0.1 result.txt




python gentest.py verify