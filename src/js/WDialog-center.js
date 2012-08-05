(function(id, centerX, centerY) {
  function windowSize() {
    var x, y;

    if (typeof (window.innerWidth) === 'number') {
       x = window.innerWidth;
       y = window.innerHeight;
    } else {
      x = document.documentElement.clientWidth;
      y = document.documentElement.clientHeight;
    }

    return { x: x, y: y};
  }

  function centerDialog() {
    var el = document.getElementById(id);
    if ((el.style.display != 'none') && (el.style.visibility != 'hidden')) {
      var ws = windowSize();
      var w = el.offsetWidth, h = el.offsetHeight;

      if (centerX) {
	el.style.left = Math.round((ws.x - w)/2) + 'px';
	el.style.marginLeft = '0px';
      }

      if (centerY) {
	el.style.top = Math.round((ws.y - h)/2) + 'px';
	el.style.marginTop = '0px';
      }

      el.style.visibility = 'visible';
    }
  }

  if (window.addEventListener) {
    window.addEventListener('DOMContentLoaded', centerDialog, false);
  } else {
    document.attachEvent('onDOMContentLoaded', centerDialog);
  }
})($el, $centerX, $centerY);
