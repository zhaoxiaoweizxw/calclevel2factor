
@ECHO OFF
ECHO upzip %1 ...
set datapath=%2
set destpath=%3
set tdate=%datapath%%1.zip
echo starting on %tdate%  ...

set exe7z="C:\Program Files\7-Zip\7z.exe"
if not exist %tdate% goto filenotexist
if not exist %exe7z% goto no7z

%exe7z% x %tdate% -o%destpath% -aoa
if %errorlevel% GTR 0 goto unziperror

goto end

:filenotexist
echo %tdate% not exist
exit /b 1

:no7z
echo system have not 7z.exe, please install first
exit /b 2

:unziperror
echo extract zipfile failed!!
exit /b 3

:end
echo unzip success
exit /b 0
