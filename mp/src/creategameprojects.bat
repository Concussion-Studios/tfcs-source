@echo off
cls
pushd %~dp0
	devtools\bin\vpc.exe /sdk +game /mksln tfcsource.sln /2013
popd
@pause