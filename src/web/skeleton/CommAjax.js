_$_APP_CLASS_$_._p_.recvCallback = function(request, userData) {
  if (request.readyState == 4) {
    if (request.status) {
      _$_APP_CLASS_$_._p_.handleResponse
        (request.status == 200 ? request.responseText : "", userData);
    }
    request.onreadystatechange = new Function;
  }
};

_$_APP_CLASS_$_._p_.commResponseReceived = function(updateId) {
}

_$_APP_CLASS_$_._p_.sendUpdate = function(url, data, userData, id) {
  var xmlHttpReq = false;
  if (window.XMLHttpRequest) {
    xmlHttpReq = new XMLHttpRequest();
  } else if (window.ActiveXObject) {
    try {
      xmlHttpReq = new ActiveXObject("Msxml2.XMLHTTP");
    } catch (err) {
      try {
	xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
      } catch (err2) {
      }
    }
  }

  xmlHttpReq.open('POST', url, true);
  xmlHttpReq.setRequestHeader("Content-type",
			      "application/x-www-form-urlencoded;");
  if (_$_CLOSE_CONNECTION_$_)
    xmlHttpReq.setRequestHeader("Connection","close");
  xmlHttpReq.onreadystatechange
  = function() { _$_APP_CLASS_$_._p_.recvCallback(xmlHttpReq, userData); };
  xmlHttpReq.send(data);

  return xmlHttpReq;
};
