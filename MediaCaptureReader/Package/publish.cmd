@echo off
setlocal

set VERSION=2.0.0

set OUTPUT=c:\NuGet\

%OUTPUT%nuget push %OUTPUT%Packages\MMaitre.MediaCaptureReader.%VERSION%.nupkg
%OUTPUT%nuget push %OUTPUT%Symbols\MMaitre.MediaCaptureReader.Symbols.%VERSION%.nupkg -Source http://nuget.gw.symbolsource.org/Public/NuGet 