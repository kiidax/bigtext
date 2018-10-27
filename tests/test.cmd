del result.txt

..\x64\Release\bigtext.exe sample -q test.txt -r 0.5 result.txt
..\x64\Release\bigtext.exe count -c result.txt

exit /b

..\x64\Release\bigtext.exe sample -s -c 2 test.txt test2.txt -o result.txt
..\x64\Release\bigtext.exe sample ..\bigtext\test2.txt test.txt -n 1000 result.txt
..\x64\Release\bigtext count result.txt
..\x64\Release\bigtext.exe vocab ..\bigtext\test2.txt -c 1 result.txt
..\x64\Release\bigtext.exe sample -r0.1 test.txt result.txt
..\x64\Release\bigtext.exe sample -s -c 3 test.txt test2.txt -o result.txt
..\x64\Release\bigtext.exe vocab -c ..\bigtext\test2.txt
..\x64\Release\bigtext.exe vocab ..\bigtext\test2.txt
..\x64\Release\bigtext.exe vocab -c ..\bigtext\test2.txt
..\x64\Release\bigtext.exe vocab ..\bigtext\test2.txt

del result.txt
..\x64\Release\bigtext.exe sample -c ..\bigtext\test2.txt - 0.1 result.txt
del result.txt
..\x64\Release\bigtext.exe sample ..\bigtext\test2.txt - 0.1 result.txt
del result.txt
..\x64\Release\bigtext.exe sample -c ..\bigtext\test2.txt - 0.1 result.txt
del result.txt
..\x64\Release\bigtext.exe sample ..\bigtext\test2.txt - 0.1 result.txt

python gentest.py verify