/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_APP_MEMBER(
  2,
  JavaScriptFunction,
  "WNotificationSetup",
  function(APP) {
    if (typeof APP.WNotification === "undefined" && window.Notification) {
      new (function() {
        APP.WNotification = this;
        const notifications = {};

        function removeSignals(id) {
          const notif = notifications[id];
          if (notif) {
            notif.onclose = () => {};
            notif.onclick = () => {};
            notif.onshow = () => {};
            notif.onerror = () => {};
          }
        }

        this.close = function(id) {
          if (notifications[id]) {
            notifications[id].close();
          }
        };

        this.create = function(id, title, params) {
          removeSignals(id);
          notifications[id] = new Notification(title, params);
          const notif = notifications[id];

          notif.onclose = () => {
            APP.emit(id, "closed");
            delete notifications[id];
          };

          notif.onclick = () => {
            APP.emit(id, "clicked");
          };

          notif.onshow = () => {
            APP.emit(id, "shown");
          };

          notif.onerror = () => {
            APP.emit(id, "error");
          };
        };
      })();
    }
  }
);
