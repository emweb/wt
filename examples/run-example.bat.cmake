set PATH=%~p0;%PATH%
set OLDPWD=%CD%
cd ..\lib\Wt\examples\@EXAMPLESUBDIR@
set DOCROOT=%CD%
cd %OLDPWD%

%~p0\..\lib\Wt\examples\@EXAMPLESUBDIR@\@EXAMPLENAME@.wt  --docroot %DOCROOT%\@DOCROOTSUBFOLDER@ --approot %DOCROOT%\@APPROOTSUBFOLDER@ --http-port 8080 --http-addr 0.0.0.0 %*
