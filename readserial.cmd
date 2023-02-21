@echo off
title Reading Serial data
:start
cls
ncat -l -p 4555 
goto start