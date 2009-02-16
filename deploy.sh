echo "Installing example ${APPBIN} to ${DEPLOYROOT}/${APPNAME}/ ..."
mkdir -p ${DEPLOYROOT}/${APPNAME}
install ${APPBIN} ${DEPLOYROOT}/${APPNAME}
if [ "${APPRESOURCES}" ]; then
  cp -R ${APPRESOURCES} ${DEPLOYROOT}/${APPNAME}
fi
echo "Done."

