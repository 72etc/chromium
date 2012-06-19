// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var assertEq = chrome.test.assertEq;
var assertTrue = chrome.test.assertTrue;
var fail = chrome.test.fail;
var succeed = chrome.test.succeed;

function assertThrowsError(method, opt_expectedError) {
  try {
    method();
    fail('error not thrown');
  } catch (e) {
    var message = e.message || e;
    if (opt_expectedError) {
      assertEq(opt_expectedError, message);
    } else {
      assertTrue(
          message.indexOf('is not available in packaged apps') != -1,
          'Unexpected message ' + message);
    }
  }
}

chrome.test.runTests([
  function testDocument() {
    assertThrowsError(document.open);
    assertThrowsError(document.clear);
    assertThrowsError(document.close);
    assertThrowsError(document.write);
    assertThrowsError(document.writeln);

    assertThrowsError(function() {document.all;});
    assertThrowsError(function() {document.bgColor;});
    assertThrowsError(function() {document.fgColor;});
    assertThrowsError(function() {document.alinkColor;});
    assertThrowsError(function() {document.linkColor;});
    assertThrowsError(function() {document.vlinkColor;});
    succeed();
  },

  function testHistory() {
    assertThrowsError(history.back);
    assertThrowsError(history.forward);
    assertThrowsError(history.pushState);
    assertThrowsError(history.replaceState);
    assertThrowsError(function() {history.length;});
    assertThrowsError(function() {history.state;});
    succeed();
  },

  function testWindowFind() {
    assertThrowsError(Window.prototype.find);
    assertThrowsError(window.find);
    assertThrowsError(find);
    succeed();
  },

  function testWindowAlert() {
    assertThrowsError(Window.prototype.alert);
    assertThrowsError(window.alert);
    assertThrowsError(alert);
    succeed();
  },

  function testWindowConfirm() {
    assertThrowsError(Window.prototype.confirm);
    assertThrowsError(window.confirm);
    assertThrowsError(confirm);
    succeed();
  },

  function testWindowPrompt() {
    assertThrowsError(Window.prototype.prompt);
    assertThrowsError(window.prompt);
    assertThrowsError(prompt);
    succeed();
  },

  function testBars() {
    var bars = ['locationbar', 'menubar', 'personalbar',
                'scrollbars', 'statusbar', 'toolbar'];
    for (var x = 0; x < bars.length; x++) {
      assertThrowsError(function() {
        var visible = this[bars[x]].visible;
        visible = window[bars[x]].visible;
      });
    }
    succeed();
  },

  function testBlockedEvents() {
    var eventHandler = function() { fail('event handled'); };
    var blockedEvents = ['unload', 'beforeunload'];

    for (var i = 0; i < blockedEvents.length; ++i) {
      assertThrowsError(function() {
        window['on' + blockedEvents[i]] = eventHandler;
      });
      assertThrowsError(function() {
        window.addEventListener(blockedEvents[i], eventHandler);
      });
      assertThrowsError(function() {
        Window.prototype.addEventListener.apply(window,
            [blockedEvents[i], eventHandler]);
      });
    }

    succeed();
  },

  function testSyncXhr() {
    var xhr = new XMLHttpRequest();
    assertThrowsError(function() {
      xhr.open('GET', 'data:should not load', false);
    }, 'INVALID_ACCESS_ERR: DOM Exception 15');
    succeed();
  }
]);
