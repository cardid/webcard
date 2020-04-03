// webcard.js
'use strict';

var nativeCallMap = new Map();

function Reader(name, atr) {
  this.name = name;
  this.atr = atr;
  
  this.connect = function(shared) {
    return new Promise(function(resolve, reject) {
      let uid = Date.now().toString(36) + Math.random().toString(36).substr(2,5);
      nativeCallMap.set(uid, {c: 2, resolve: resolve, reject: reject});
      window.postMessage({ webcard: "request", i: uid, c: 2, r: 0, p: shared ? 2 : 1 }, "*");
    });
  }
  
  this.disconnect = function() {
    return new Promise(function(resolve, reject) {
      let uid = Date.now().toString(36) + Math.random().toString(36).substr(2,5);
      nativeCallMap.set(uid, {c: 3, resolve: resolve, reject: reject});
      window.postMessage({ webcard: "request", i: uid, c: 3, r: 0 }, "*");
    });
  }

  this.transcieve = function(apdu) {
    return new Promise(function(resolve, reject) {
      let uid = Date.now().toString(36) + Math.random().toString(36).substr(2,5);
      nativeCallMap.set(uid, {c: 4, resolve: resolve, reject: reject});
      window.postMessage({ webcard: "request", i: uid, c: 4, r: 0, a: apdu }, "*");  
    });
  }
}

function WebCard() {
  console.log('Starting WebCard');
  var _readers;

  var banner = document.getElementById('webcard-install-banner');

  if (banner == null)
  {
    banner = document.createElement('div');
    banner.id = "webcard-install-banner";
    banner.style = "width:100%;top:0;left:0;position:fixed;"
      + "background-color:rgba(250,250,210,0.75);border-bottom:1px solid gold;z-index:1030;"
    var html = '<p style="margin:8pt;font:10pt Helvetica;color:black;">';
    html += 'In order to provide a better login experience, this page uses your smart card.<br \\>Please install the latest ';

    if (navigator.platform == "Win32")
      html += '<a href="' + WebCard.remoteURL + '/webcard.msi">';
    else
      html += '<a href="' + WebCard.remoteURL + '/webcard.dmg">';
    
    html += 'Smart Card Browser Extension</a> and restart your browser.</p>';
    banner.innerHTML = html;
    document.body.appendChild(banner);
  }

  this.readers = function() {
    return new Promise(function(resolve, reject) {
      let uid = Date.now().toString(36) + Math.random().toString(36).substr(2,5);
      nativeCallMap.set(uid, {c: 1, resolve: resolve, reject: reject});
      window.postMessage({ webcard: "request", i: uid, c: 1 }, "*");
    });
  }

  this.response = function(msg) {
    if (msg.e) {
      switch(msg.e) {
        case 1: // Card inserted
          if (navigator.webcard.cardinserted !== undefined)
            navigator.webcard.cardinserted(_readers[msg.r]);
          break;
        case 2: // Card removed
          if (navigator.webcard.cardremoved !== undefined)
            navigator.webcard.cardremoved(_readers[msg.r]);
          break;
        }
      return;
    }
    var pending = nativeCallMap.get(msg.i);
    switch (pending.c) {
      case 1:  // List readers 
        _readers = [];
        msg.d.forEach(elm => {
          var rdr = new Reader(elm.n, elm.a);
          _readers.push(rdr);
        });
        let readers = _readers;
        pending.resolve(readers);
        break;
      case 2: // Connect
        if (msg.d != "")
          pending.resolve(msg.d);
        else
          pending.reject();
        break;
      case 3: // Disconnect
        pending.resolve();
        break;
      case 4: // Transcieve
        pending.resolve(msg.d);
        break;
    }
    nativeCallMap.delete(msg.i);
  }

};

if (typeof navigator.webcard === 'undefined') {
  navigator.webcard = new WebCard();
}

window.addEventListener("message", function(event) {
  // We only accept messages from ourselves
  if (event.source != window)
    return;

  if (event.data.webcard && (event.data.webcard == "response")) {
    navigator.webcard.response(event.data);
  }
}, false);