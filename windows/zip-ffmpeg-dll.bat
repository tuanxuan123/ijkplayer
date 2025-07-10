@echo off
REM 压缩build目录
set "buildsourceDir=%cd%\FFmpeg-ijkplayer\build"
set "buildzipFile=%cd%\FFmpeg-ijkplayer\pixvideo_win_build.zip"
set zip="C:\Program Files\7-Zip\7z.exe"
%zip% a -tzip "%buildzipFile%" "%buildsourceDir%"
exit 0