set PATH=%~p0;%PATH%
set OLDPWD=%CD%
REM Required until wthttp accepts a "--approot path" parameter
cd ..\lib\Wt\examples\@EXAMPLESUBDIR@
set DOCROOT=%CD%

%~p0\..\lib\Wt\examples\@EXAMPLESUBDIR@\@EXAMPLENAME@.wt  --docroot %DOCROOT% --http-port 8080 --http-addr 0.0.0.0 %*
cd %OLDPWD%
