_$_APP_CLASS_$_._p_.lastId = 0;
_$_APP_CLASS_$_._p_.userData = new Array();
_$_APP_CLASS_$_._p_.userCallback = new Array();

_$_APP_CLASS_$_._p_.commResponseReceived = function(updateId) {
  var _Wt_ = _$_APP_CLASS_$_._p_;
  for (i = _Wt_.lastId; i < updateId; ++i) {
    _Wt_.handleResponse("", _Wt_.userData[i]);
    _Wt_.userData[i] = null;
    var s = document.getElementById("script" + i);
    if (s != null)
      s.parentNode.removeChild(s);
  }
  _Wt_.lastId = updateId - 1;
}

_$_APP_CLASS_$_._p_.sendUpdate = function(url, data, userdata, updateId)
{
  var _Wt_ = _$_APP_CLASS_$_._p_;
  var s = document.createElement('SCRIPT');
  _Wt_.userData[updateId] = userdata;
  s.id = "script" + updateId;
  s.src = url + '&' + data;
  s.type = 'text/javascript';
  var h = document.getElementsByTagName('HEAD')[0];
  h.appendChild(s);
}
