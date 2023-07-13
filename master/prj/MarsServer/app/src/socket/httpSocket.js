const app = require("../app");
const { DB_FIELDS, USER_STATUS } = require("../common/constant");
const { marsLogger } = require("../common/logger");
const { sql } = require("../db/db");
var http = require("http").createServer(app);
var io = require("socket.io")(http);
const TAG = "serverSocket";
const EVENT_NAME_PING_ECHO = "pingEcho";
const EVENT_NAME_CALL_NOTIFICATION = "callNotification";

const PORT_CALL_PING_ECHO = 3001;
const PORT_CALL_NOTIFICATION = 3002;

http.listen(PORT_CALL_PING_ECHO, "127.0.0.1", function (socket) {
  // for ping echo
  marsLogger(TAG, `listen ${PORT_CALL_PING_ECHO} for ${EVENT_NAME_PING_ECHO}`);
});

http.listen(PORT_CALL_NOTIFICATION, "127.0.0.1", function (socket) {
  // for call notification
  marsLogger(TAG, `listen ${PORT_CALL_NOTIFICATION} for ${EVENT_NAME_CALL_NOTIFICATION}`);
});

io.on("connection", function (socket) {
  const time = new Date().toLocaleString();
  const id = socket.handshake.query.id;
  const ipAddress = socket.handshake.address;
  marsLogger(TAG, `connection id ${id} ip ${ipAddress}`);
  socket.on(EVENT_NAME_PING_ECHO, function () {
    if (!id) {
      sendEcho(socket.id, "please input a id(userId)");
    } else {
      checkUserStatus(id, ipAddress);
      sendEcho(socket.id, "ok");
    }
  });
});

var sendEcho = function (socketId, message) {
  io.to(socketId).emit(EVENT_NAME_PING_ECHO, message);
};

var sendCallNotification = function (conferenceId) {
  io.sockets.emit(EVENT_NAME_CALL_NOTIFICATION, conferenceId);
};
const PING_ECHO_LIMIT_TIME = 5 * 1000; // sec
var timeoutList = {};

async function checkUserStatus(id, ipAddress) {
  const time = new Date().getTime();
  updateUserStatus(id, ipAddress, USER_STATUS.LOGIN, time);
  if (timeoutList[id] !== null) {
    marsLogger(TAG, `login id ${id}`);
    clearTimeout(timeoutList[id]);
  }
  timeoutList[id] = setTimeout(() => {
    marsLogger(TAG, `logout id ${id}`);
    updateUserStatus(id, ipAddress, USER_STATUS.LOGOUT, time);
  }, PING_ECHO_LIMIT_TIME);
}
async function updateUserStatus(id, ipAddress, status, time) {
  try {
    // update last login time
    const updateQuery = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_STATUS}=?,${DB_FIELDS.COL_IP_ADDRESS}=?,${DB_FIELDS.COL_LAST_LOGIN_TIME}=? WHERE ${DB_FIELDS.COL_ID}=?`;
    await sql.promise().query(updateQuery, [status, ipAddress, time, id]);
  } catch (e) {
    marsLogger(TAG, e);
  }
}
module.exports = {
  sendCallNotification: sendCallNotification,
};
