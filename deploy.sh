echo "Installing example ${APPBIN} to ${DEPLOYROOT}/${APPNAME}/ ..."
mkdir -p ${DEPLOYROOT}/${APPNAME}
install ${APPBIN} ${DEPLOYROOT}/${APPNAME}
if [ "${APPRESOURCES}" ]; then
  cp -R ${APPRESOURCES} ${DEPLOYROOT}/${APPNAME}
fi
cp -R ${WT_SOURCE_DIR}/resources ${DEPLOYROOT}/${APPNAME}
echo "Done."

