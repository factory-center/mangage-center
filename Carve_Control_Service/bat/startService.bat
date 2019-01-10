@echo off
SC QUERY gqxtestservice > NUL
IF ERRORLEVEL 1060 (
	sc create gqxtestservice binPath= "C:\Users\gqx\Desktop\testService\Debug\testService.exe"
) else (
	sc QUERY |find /i "gqxtestservice" >nul 2>nul
	if not errorlevel 1 (
		ECHO gqxtestservice is already started
	) else (
		ECHO gqxtestservice not start
		net start gqxtestservice
	)
)
pause
