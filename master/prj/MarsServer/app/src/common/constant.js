const HTTP_STATUS_CODES = {
  200: "200", // success
  201: "201", // successful resource creation
  202: "202", // successful start of a request that will be handled asynchronously
  204: "204", // success but the response body is empty (e.g., zero rows for a query)
  400: "400", // data in the request is not valid
  401: "401", // invalid user credentials provided for authentication
  403: "403", // access denied (authorization failure)
  404: "404", // the requested resource was not found
  415: "415", // the request Content-Type header is an unsupported media type
  422: "422", // business logic or business validation error
  500: "500", // unexpected error processing the request (e.g., out of memory, NPE)
};

const DB_FIELDS = {
  TABLE_USER: "user",
  TABLE_CONFERENCE: "conference",
  TABLE_CONTACT: "contact",
  COL_ID: "id",
  COL_EMAIL: "email",
  COL_PASSWORD: "password",
  COL_FIRSTNAME: "firstname",
  COL_LASTNAME: "lastname",
  COL_ADDRESS: "address",
  COL_IP_ADDRESS: "ipAddress",
  COL_IS_ENABLE: "isEnable",
  COL_STATUS: "status",
  COL_LAST_LOGIN_TIME: "lastLoginTime",
  COL_TOKEN: "token",
  COL_TOPIC: "topic",
  COL_START: "start",
  COL_DURATION: "duration",
  COL_USERS: "users",
  COL_PARTICIPANT: "participant",
  COL_JOIN_USERS: "joinUsers",
  COL_CREATOR: "creator",
};

const USER_STATUS = {
  LOGIN: "login",
  LOGOUT: "logout",
  BUSY: "busy",
  CREATED: "created",
};
const HTTP_PATH = {
  POST: {
    CREATE_DATA: "/",
    DEV_SQL: "/sql",
    CALL_REQUEST: "/:id/callRequest",
    CALL_REJECT: "/:id/callReject",
  },
  PUT: {
    UPDATE_DATA: "/:id/update",
    UPDATE_PASSWORD: "/:id/password",
    UPDATE_EMAIL: "/:id/email",
    UPDATE_ENABLE: "/:id/enable",
    LOGIN: "/:id",
    LOGOUT: "/:id/logout",
    UPDATE_JOIN_USERS: "/:id/join",
  },
  GET: {
    GET_DATA: "/:id",
    GET_LIST: "/:id/list",
    SEARCH: "/:id/search",
  },
  DELETE: {
    DELETE_DATA: "/:id/delete",
    RESET_PASSWORD: "/:id/password",
  },
};

const SOCKET_MSG_TYPE = {
  PING: "ping",
  CALL_RESERVE: "callReserve",
  CALL_REQUEST: "callRequest",
  CALL_JOINED: "callJoined",
  CALL_LEFT: "callLeft",
  CALL_REJECT: "callReject",
  USER_UPDATE: "userUpdate",
  CID: "cid",
  UID: "uid",
};

module.exports = {
  HTTP_STATUS_CODES,
  DB_FIELDS,
  USER_STATUS,
  HTTP_PATH,
  SOCKET_MSG_TYPE,
};
