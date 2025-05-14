@echo off

set split=hen_splitter.exe
set sver=0

:start

cls
echo Split [R]elease or [D]ebug Bins?
echo.
set /p sver=

if %sver%==r goto rls
if %sver%==R goto rls
if %sver%==d goto dbg
if %sver%==D goto dbg

goto start

:rls
%split% /unpack PS3HEN.BIN_CEX_480 /out "480C"
%split% /unpack PS3HEN.BIN_CEX_481 /out "481C"
%split% /unpack PS3HEN.BIN_CEX_482 /out "482C"
%split% /unpack PS3HEN.BIN_CEX_483 /out "483C"
%split% /unpack PS3HEN.BIN_CEX_484 /out "484C"
%split% /unpack PS3HEN.BIN_CEX_485 /out "485C"
%split% /unpack PS3HEN.BIN_CEX_486 /out "486C"
%split% /unpack PS3HEN.BIN_CEX_487 /out "487C"
%split% /unpack PS3HEN.BIN_CEX_488 /out "488C"
%split% /unpack PS3HEN.BIN_CEX_489 /out "489C"
%split% /unpack PS3HEN.BIN_CEX_490 /out "490C"
%split% /unpack PS3HEN.BIN_CEX_491 /out "491C"
%split% /unpack PS3HEN.BIN_CEX_492 /out "492C"
goto end

:dbg
%split% /unpack PS3HEN.BIN_CEX_480_DEBUG /out "480C"
%split% /unpack PS3HEN.BIN_CEX_481_DEBUG /out "481C"
%split% /unpack PS3HEN.BIN_CEX_482_DEBUG /out "482C"
%split% /unpack PS3HEN.BIN_CEX_483_DEBUG /out "483C"
%split% /unpack PS3HEN.BIN_CEX_484_DEBUG /out "484C"
%split% /unpack PS3HEN.BIN_CEX_485_DEBUG /out "485C"
%split% /unpack PS3HEN.BIN_CEX_486_DEBUG /out "486C"
%split% /unpack PS3HEN.BIN_CEX_487_DEBUG /out "487C"
%split% /unpack PS3HEN.BIN_CEX_488_DEBUG /out "488C"
%split% /unpack PS3HEN.BIN_CEX_489_DEBUG /out "489C"
%split% /unpack PS3HEN.BIN_CEX_490_DEBUG /out "490C"
%split% /unpack PS3HEN.BIN_CEX_491_DEBUG /out "491C"
%split% /unpack PS3HEN.BIN_CEX_492_DEBUG /out "492C"
%split% /unpack PS3HEN.BIN_DEX_482_DEBUG /out "482D"
%split% /unpack PS3HEN.BIN_DEX_484_DEBUG /out "484D"
goto end

:end
exit
