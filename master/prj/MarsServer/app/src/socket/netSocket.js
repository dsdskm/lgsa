var net = require("net");
const { DB_FIELDS, USER_STATUS, SOCKET_MSG_TYPE } = require("../common/constant");
const { marsLogger } = require("../common/logger");
const { sql } = require("../db/db");
const TAG = "socket";
var socketArr = {};
var timeoutList = {};
const PING_ECHO_LIMIT_TIME = 5 * 1000; // sec
var connectSocketToClient = function (id, ipAddress) {
  marsLogger(TAG, `connectSocketToClient id ${id} ipAddress ${ipAddress}`);
  socketArr[ipAddress] = net.connect({ host: ipAddress, port: 3001 }, function () {
    marsLogger(TAG, `connected to ${ipAddress}`);
    sendPingMessage(ipAddress);
  });

  socketArr[ipAddress].on("data", function (msg) {
    marsLogger(TAG, `${msg}`, false, true);
    sendPingMessage(ipAddress);
    checkUserStatus(id, ipAddress);
  });
  socketArr[ipAddress].on("error", (err) => {
    marsLogger(TAG, `socket error ${err}`);
    socketArr[ipAddress].end();
  });
  socketArr[ipAddress].on("timeout", () => {
    marsLogger(TAG, `socket timeout`);
  });
  socketArr[ipAddress].on("close", () => {
    marsLogger(TAG, `socket close`);
  });

  function sendPingMessage(ipAddress) {
    setTimeout(function () {
      const time = new Date().getTime();
      const data = `{\"type":\"${SOCKET_MSG_TYPE.PING}\",\"trial\":\"${time}\"}\0`;
      socketArr[ipAddress].write(data);
    }, 1000);
  }
};

async function checkUserStatus(id, ipAddress) {
  const time = new Date().getTime();
  updateUserStatus(id, ipAddress, USER_STATUS.LOGIN, time);
  if (timeoutList[id] !== null) {
    marsLogger(TAG, `login id ${id}`, false, true);
    clearTimeout(timeoutList[id]);
  }
  timeoutList[id] = setTimeout(() => {
    marsLogger(TAG, `logout id ${id}`, false, true);
    updateUserStatus(id, ipAddress, USER_STATUS.LOGOUT, time);
    sendConferenceNoti({
      type: SOCKET_MSG_TYPE.USER_UPDATE,
      uid: id,
    });
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

var sendConferenceNoti = function (data) {
  let msg = "";
  if (data.type === SOCKET_MSG_TYPE.USER_UPDATE) {
    msg = `{\"type":\"${data.type}\",\"uid\":\"${data.uid}\"`;
  } else {
    msg = `{\"type":\"${data.type}\",\"cid\":\"${data.cid}\"`;
    switch (data.type) {
      case SOCKET_MSG_TYPE.CALL_RESERVE:
        msg += `,\"participant\":${data.participant}`;
        break;
      case SOCKET_MSG_TYPE.CALL_JOINED:
      case SOCKET_MSG_TYPE.CALL_LEFT:
      case SOCKET_MSG_TYPE.CALL_REJECT:
        msg += `,\"uid\":\"${data.uid}\"`;
        break;
      case SOCKET_MSG_TYPE.CALL_REQUEST:
      default:
        break;
    }
  }
  msg += "}\0";

  marsLogger(TAG, msg);
  Object.entries(socketArr).forEach(([k, v]) => {
    const writeRes = v.write(msg, function (err) {
      if (err) {
        marsLogger(TAG, `socket write err ${k} ${err}`);
      }
    });
    marsLogger(TAG, `writeRes=${writeRes} address=${k}`);
    
  });
};

module.exports = {
  sendConferenceNoti: sendConferenceNoti,
  connectSocketToClient: connectSocketToClient,
};
