// webcard.js
'use strict';

var nativeCallMap = new Map();

function Reader(index, name, atr) {
  this.index = index;
  this.name = name;
  this.atr = atr;
  
  this.connect = function(shared) {
    let ind = this.index;
    return new Promise(function(resolve, reject) {
      let uid = Date.now().toString(36) + Math.random().toString(36).substr(2,5);
      nativeCallMap.set(uid, {c: 2, resolve: resolve, reject: reject});
      window.postMessage({ webcard: "request", i: uid, c: 2, r: ind, p: shared ? 2 : 1 }, window.location.href);
    });
  }
  
  this.disconnect = function() {
    let ind = this.index;
    return new Promise(function(resolve, reject) {
      let uid = Date.now().toString(36) + Math.random().toString(36).substr(2,5);
      nativeCallMap.set(uid, {c: 3, resolve: resolve, reject: reject});
      window.postMessage({ webcard: "request", i: uid, c: 3, r: ind }, window.location.href);
    });
  }

  this.transcieve = function(apdu) {
    let ind = this.index;
    return new Promise(function(resolve, reject) {
      let uid = Date.now().toString(36) + Math.random().toString(36).substr(2,5);
      nativeCallMap.set(uid, {c: 4, resolve: resolve, reject: reject});
      window.postMessage({ webcard: "request", i: uid, c: 4, r: ind, a: apdu }, window.location.href);  
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
      + "background-color:rgba(250,250,210,0.85);border-bottom:1px solid gold;z-index:1030;"
    var html = '<p style="margin:8pt;font:11pt Helvetica;color:black;">';
    html += 'This page needs you to install the ';

    if (navigator.platform == "Win32")
      html += '<a href="https://webcard.cardid.org/webcard.msi">';
    else
      html += '<a href="https://webcard.cardid.org/webcard.dmg">';
    
    html += 'Smart Card Extension</a> to access your smart card readers.</p>';
    banner.innerHTML = html;
    document.body.appendChild(banner);
  }

  this.readers = function() {
    return new Promise(function(resolve, reject) {
      let uid = Date.now().toString(36) + Math.random().toString(36).substr(2,5);
      nativeCallMap.set(uid, {c: 1, resolve: resolve, reject: reject});
      window.postMessage({ webcard: "request", i: uid, c: 1 }, window.location.href);
    });
  }

  this.response = function(msg) {
    if (msg.e) {
      switch(msg.e) {
        case 1: // Card inserted
          if (navigator.webcard.cardInserted !== undefined)
            navigator.webcard.cardInserted(_readers[msg.r]);
          break;
        case 2: // Card removed
          if (navigator.webcard.cardRemoved !== undefined)
            navigator.webcard.cardRemoved(_readers[msg.r]);
          break;
        }
      return;
    }
    var pending = nativeCallMap.get(msg.i);
    switch (pending.c) {
      case 1:  // List readers 
        _readers = [];
        msg.d.forEach((elm, index) => {
          var rdr = new Reader(index, elm.n, elm.a);
          _readers.push(rdr);
        });
        pending.resolve(_readers);
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
}

if (typeof navigator.webcard === 'undefined') {
  navigator.webcard = new WebCard();
}

window.addEventListener("message", function(event) {
  // We only accept messages from ourselves
  if (!Object.is(event.source, this.window) || !window.location.href.startsWith(event.origin))
    return;

  if (event.data.webcard && (event.data.webcard == "response")) {
    navigator.webcard.response(event.data);
  }
}, false);