/*
 *
 * From orbited-0.0.2
 *
 * LICENSE: see orbited_LICENSE.txt
 */
/*
Orbited.connect(got_event) // Tries to find the best option
Orbited.connect(got_event, "htmlfile") // Useses ie htmlfile
Orbited.connect(got_Event, "longpoll") //
*/

Orbited = {
  connection: null,

    utf8decode : function (utftext) {
        var string = "";
        var i = 0;
        var c = c1 = c2 = 0;

        while ( i < utftext.length ) {

            c = utftext.charCodeAt(i);

            if (c < 128) {
                string += String.fromCharCode(c);
                i++;
            }
            else if((c > 191) && (c < 224)) {
                c2 = utftext.charCodeAt(i+1);
                string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
                i += 2;
            }
            else {
                c2 = utftext.charCodeAt(i+1);
                c3 = utftext.charCodeAt(i+2);
                string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
                i += 3;
            }

        }

        return string;
    },
  
  connect: function(event_cb, user, location, session, transport) {
    this.user = user;
    this.location = location;
    this.transport = transport;
    this.session = session;
    this.event_cb = event_cb;
    document.domain = this.extract_xss_domain(document.domain);
    if (typeof transport == 'undefined') {
      this.find_best_transport();
    }
    this.connection = this['connect_' + this.transport]();
  },
  
  find_best_transport: function() {
    // IF we're on IE 5.01/5.5/6/7 we want to use an htmlfile
    try {
      var test = ActiveXObject;
      this.transport = "htmlfile";
      return;
    }
    catch (e) {}
    
    // If the browser supports server-sent events, we should use those
    if ((typeof window.addEventStream) == "function") {
      this.transport = "server_sent_events";
      return;
    }
    
    // // Otherwise use the iframe
    this.transport = "iframe";
    return;
    
    // otherwise use xhr streaming
    //this.transport = "xhr_stream"; // UTF8 broken on firefox
    //return;
  },
  
  connect_iframe: function() {
    var url = this.location + '&user=' + this.user;
    url += "&session=" + this.session + "&transport=iframe";
    var ifr = document.createElement("iframe");
    ifr.setAttribute("id", "orbited_event_source");
    ifr.setAttribute("src", url);
    ifr.setAttribute("style", "border: 0px; height: 0px; width: 0px;");
    document.body.appendChild(ifr);
    this.kill_load_bar();
    var event_cb = this.event_cb;
    var kill_load = this.kill_load_bar;
    this.event_cb = function(data) {
      event_cb(data);
      kill_load();
    }
  },
  
  connect_htmlfile: function() {  
    var url = this.location + '&user=' + this.user;
    url += "&session=" + this.session + "&transport=iframe";
    var transferDoc = new ActiveXObject("htmlfile"); // !?!
    transferDoc.open();
    transferDoc.write("<html>");
    transferDoc.write("<script>document.domain='" + document.domain + "';</script>");
    transferDoc.write("</html>");
    transferDoc.parentWindow.Orbited = this;
    transferDoc.close();
    var ifrDiv = transferDoc.createElement("div");
    transferDoc.body.appendChild(ifrDiv);
    ifrDiv.innerHTML = "<iframe src='"+url+"'></iframe>";
    this.data_queue = [];
    dq = this.data_queue;
    event_cb = this.event_cb;

    /* Support IE 5.01 */
    if (typeof Array.prototype.shift == "undefined") {
        
      Array.prototype.shift = function () {
	var A_s = 0;
	var response = this[0];
        for (A_s = 0; A_s < this.length-1; A_s++) {
	  this[A_s] = this[A_s + 1];
        }
        this.length--;
        return response;
      }
    }
    /* End IE 5.01 Hack */
    
    
    function s() {
        while (dq.length > 0) {
            event_cb(dq.shift());
        }
    }
    this.timer = window.setInterval(s, 50);
  },
  
  connect_xhr_stream: function() {
    var url = this.location + '&user=' + this.user +
              "&session=" + this.session + "&transport=xhr_stream";
    var offset = 0;
    var length_seen = 0;
    var boundary = "\r\n|O|\r\n";
    var event_cb = this.event_cb;
    var xhr = this.create_xhr();
    
    xhr.onreadystatechange = function() {
      try {
        if (xhr.status != 200) {
          Orbited.log("failed");
          return false;
        }
      }
      catch(e) {
        Orbited.log(String(e));
        return false;
      }

      // We have some new data
      if (xhr.readyState == 3) {
        
        var handle_event = function () {
        
          var response_stream = xhr.responseText;
          var rs_length = response_stream.length;
        
          // If there's no new text, bail out.
          if (rs_length == offset || rs_length == length_seen) {
            return;
          }
        
          // At the very start of the file, skip our bogus padding, or if
          // we haven't gotten through it yet, bail out--reset offset so
          // this gets tried again later
          if (offset === 0) {
            offset = response_stream.indexOf('\r\n\r\n');
            if (offset === -1) {
              offset = 0;
              length_seen = response_stream.length;
              return;
            }
          }
        
          // find the next boundary, starting at our offset
          var next_boundary = response_stream.indexOf(boundary, offset);
    
          // if we can't find any end boundaries, bail out
          if (next_boundary == -1) {
            length_seen = rs_length;
            return;
          }
      
          // if we made it this far, between offset and next_boundary lies
          // our payload
          var data = response_stream.slice(offset, next_boundary);
          offset = next_boundary + boundary.length;
          
          // If two boundaries come in a row, that implies we might be getting
          // a ping.  If so, do nothing, and move the offset
          if (data === "") {
            if (response_stream.indexOf('ping' + boundary, offset) == offset) {
              offset += ('ping' + boundary).length;
            }
          }
          else {
            data = eval(data);
            if (typeof data != 'undefined') {
              event_cb(data);          
            }
          }          
          // try again; we may have gotten multiple events at once
          handle_event();
        }
        
        handle_event();
      }
      // Connection is finished.
      if (xhr.readyState == 4) {
        // Orbited.log(xhr.responseText);
      }
    }
    xhr.open("GET", url, true);
    xhr.send(null);
  },
  
  connect_xhr_multipart: function() {
    var url = this.location + '&user=' + this.user +
              "&session=" + this.session + "&transport=xhr_multipart";
    
    var xhr = this.create_xhr();
    var event_cb = this.event_cb;
    xhr.onreadystatechange = function() {
      try {
        if (xhr.status != 200) {
          Orbited.log("failed");
          return false;
        }
      }
      catch(e) {
        Orbited.log(String(e));
        return false;
      }
      // We have a new event
      if (xhr.readyState == 4) {
        var data = eval(xhr.responseText);
        if (typeof data != 'undefined') {
          event_cb(data);          
        }
        return true;
      }
    }
    xhr.multipart = true;
    xhr.open("GET", url, true);
    xhr.send(null);
  },
  
  connect_server_sent_events: function() {
    var url = this.location + '&user=' + this.user +
              "&session=" + this.session + "&transport=server_sent_events";
    var es = document.createElement('event-source');
    var event_cb = this.event_cb;
    es.setAttribute('src', url);
    document.body.appendChild(es);
    
    var callback = function(event) {
      var data = eval(event.data);
      if (typeof data != 'undefined') {
        Orbited.log(data);
        event_cb(data);          
      }
    };
    es.addEventListener('orbited', callback, false);
  },
  
  extract_xss_domain: function(old_domain) {
    domain_pieces = old_domain.split('.');
    if (domain_pieces.length == 4) {
      var is_ip = true;
      for (var i = 0; i < 4; ++i) {
        var n = Number(domain_pieces[i]);
        if (isNaN(n)) {
          is_ip = false;
        }
      }
      if (is_ip) {
        return old_domain;
      }
    }
    return domain_pieces.slice(-2, domain_pieces.length).join(".");
  },
  
  attach_iframe: function(ifr) {
    if (typeof this.data_queue != 'undefined') {
        dq = this.data_queue;
        ifr.e = function(data) {
            dq[dq.length] = data;
        }
    }
    else {
        ifr.e = this.event_cb;
    }
  },
  
  create_xhr: function() {
    // try {
    //   return window.ActiveXObject ?
    //     new ActiveXObject("Microsoft.XMLHTTP") :
    //     new XMLHttpRequest();
    // }
    // catch(e) {}
    try { return new ActiveXObject("Msxml2.XMLHTTP"); } catch (e) {}
    try { return new ActiveXObject("Microsoft.XMLHTTP"); } catch (e) {}
    try { return new XMLHttpRequest(); } catch(e) {}
    return null;
  },
  
  log: function(arg) {
    if (window.console) {
      window.console.log(arg);
    }
    else if (window.opera) {
      window.opera.postError(arg); 
    }
  },

  kill_load_bar: function () {
    if (this.load_kill_ifr == null) {
      this.load_kill_ifr = document.createElement('iframe');
    }
    document.body.appendChild(this.load_kill_ifr);
    document.body.removeChild(this.load_kill_ifr);
  }
  
}
