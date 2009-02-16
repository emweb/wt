_$_APP_CLASS_$_._p_.nextUpdateId = 0;
_$_APP_CLASS_$_._p_.userData = new Array();
_$_APP_CLASS_$_._p_.userCallback = new Array();

_$_APP_CLASS_$_._p_.updateDone = function(updateId) {
  var _Wt_ = _$_APP_CLASS_$_._p_;
  _Wt_.handleResponse("", _Wt_.userData[updateId]);
  _Wt_.userData[updateId] = null;
  var s = document.getElementById("script" + updateId);
  s.parentNode.removeChild(s);
}

_$_APP_CLASS_$_._p_.sendUpdate = function(url, data, userdata)
{
  var _Wt_ = _$_APP_CLASS_$_._p_;
  var s = document.createElement('SCRIPT');
  _Wt_.userData[_Wt_.nextUpdateId] = userdata;
  s.id = "script" + _Wt_.nextUpdateId;
  s.src = url + '&' + data + '&updateId=' + _Wt_.nextUpdateId++;
  s.type = 'text/javascript';
  var h = document.getElementsByTagName('HEAD')[0];
  h.appendChild(s);
}
