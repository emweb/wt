/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_APP_MEMBER(
  1,
  JavaScriptObject,
  "WNotification",
  new (function() {
    const notifications = {};

    this.close = function(id) {
      if (notifications[id]) {
        notifications[id].close();
      }
    };

    this.create = function(id, title, params) {
      this.close(id);
      notifications[id] = new Notification(title, params);
      notifications[id].onclose = (event) => {
        delete notifications[id];
      };
    };
  })()
);
