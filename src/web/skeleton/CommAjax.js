/*
 * Workaround non equivalence in short circuit evaluation of google minifier
 * for IE
 */
_$_WT_CLASS_$_.condCall = function(o, f, a) {
  if (o[f])
    o[f](a);
};

_$_APP_CLASS_$_._p_.comm = new (function(handleResponse) {
    var handler = handleResponse;

    function Request(url, data, userData, id, timeout) {
      var request = false;
      var timer = null;
      var handled = false;

      function recvCallback() {
	if (request.readyState == 4) {
	  if (handled)
	    return;

	  // console.log("recvCallback " + request.status);
	  clearTimeout(timer);

	  if (request.status == 200
	      && request.getResponseHeader("Content-Type")
	      && request.getResponseHeader("Content-Type")
	      .indexOf("text/javascript") == 0)
	    handler(0, request.responseText, userData);
	  else
	    handler(1, null, userData);

	  request.onreadystatechange = new Function;
	  request = null;

	  handled = true;
	}
      }

      function handleTimeout() {
	request.onreadystatechange = new Function;
	request = null;
	handled = true;
	handler(2, null, userData);
      };

      this.abort = function() {
	request.onreadystatechange = new Function;
	handled = true;
	request.abort();
	request = null;
      };

      if (window.XMLHttpRequest)
	request = new XMLHttpRequest();
      else if (window.ActiveXObject)
	try {
	  request = new ActiveXObject("Msxml2.XMLHTTP");
	} catch (err) {
	  try {
	    request = new ActiveXObject("Microsoft.XMLHTTP");
	  } catch (err2) {
	  }
	}

      if (!request)
	return;

      request.open('POST', url, true);
      request.setRequestHeader("Content-type",
			       "application/x-www-form-urlencoded;");
      if (_$_CLOSE_CONNECTION_$_)
	request.setRequestHeader("Connection","close");

      if (timeout > 0)
	timer = setTimeout(handleTimeout, timeout);
      request.onreadystatechange = recvCallback;
      request.send(data);
    }

    this.responseReceived = function(updateId) { };

    this.sendUpdate = function(url, data, userData, id, timeout) {
      return new Request(url, data, userData, id, timeout);
    };
  })(_$_APP_CLASS_$_._p_.handleResponse);
