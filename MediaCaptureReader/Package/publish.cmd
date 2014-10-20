@echo off
setlocal

set VERSION=0.0.1

set OUTPUT=c:\NuGet\

nuget push %OUTPUT%Packages\MMaitre.MediaCaptureReader.%VERSION%.nupkg
nuget push %OUTPUT%Symbols\MMaitre.MediaCaptureReader.Symbols.%VERSION%.nupkg -Source http://nuget.gw.symbolsource.org/Public/NuGet 