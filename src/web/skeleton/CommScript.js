/*
 * Workaround non equivalence in short circuit evaluation of google minifier
 * for IE
 */
_$_WT_CLASS_$_.condCall = function(o, f, a) {
  if (o[f])
    o[f](a);
}

_$_APP_CLASS_$_._p_.comm = new (function(handleResponse) {
    var Wt = _$_WT_CLASS_$_;

    var handler = handleResponse;
    var lastId = 0;
    var requests = new Array();

    function Request(url, data, userData, id, timeout) {
      var self = this;

      this.script = document.createElement('SCRIPT');
      this.script.id = "script" + id;
      this.script.src = url + '&' + data;
      this.script.type = 'text/javascript';

      var h = document.getElementsByTagName('HEAD')[0];
      h.appendChild(this.script);

      this.userData = userData;

      this.abort = function() {
	self.script.parentNode.removeChild(script);
      }
    }

    this.responseReceived = function(updateId) {
      for (i = lastId; i < updateId; ++i) {
	var request = requests[i];

	if (request) {
	  handler(0, "", request.userData);
	  request.script.parentNode.removeChild(request.script);
	}

	Wt.arrayRemove(requests, i);
      }

      lastId = updateId;
    };

    this.sendUpdate = function(url, data, userData, id, timeout) {
      var request = new Request(url, data, userData, id, timeout);
      requests[id] = request;
      return request;
    };

  })(_$_APP_CLASS_$_._p_.handleResponse);
