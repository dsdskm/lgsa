var express = require("express");
const mailer = require("../api/emailApi.js");
const { generateRandomString } = require("../api/util.js");
const { HTTP_STATUS_CODES, DB_FIELDS, USER_STATUS, HTTP_PATH } = require("../common/constant.js");
const { sql } = require("../db/db.js");
const url = require("url");
const { sendJsonResponse } = require("../common/util.js");
const { marsLogger } = require("../common/logger.js");
const { connectSocketToClient } = require("../socket/netSocket.js");
const TAG = "user";
var router = express.Router();

// create user
router.post(HTTP_PATH.POST.CREATE_DATA, async function (req, result, next) {
  const body = req.body;
  const id = body.id;
  const email = body.email;
  const password = body.password;
  const firstname = body.firstname;
  const lastname = body.lastname;
  const address = body.address;
  const ipAddress = body.ipAddress;
  const time = new Date().getTime().toString();
  const query = `INSERT INTO ${DB_FIELDS.TABLE_USER} (${DB_FIELDS.COL_ID},${DB_FIELDS.COL_EMAIL},${
    DB_FIELDS.COL_PASSWORD
  },${DB_FIELDS.COL_FIRSTNAME},${DB_FIELDS.COL_LASTNAME},${DB_FIELDS.COL_ADDRESS},${DB_FIELDS.COL_IS_ENABLE},${
    DB_FIELDS.COL_IP_ADDRESS
  },${DB_FIELDS.COL_LAST_LOGIN_TIME},${
    DB_FIELDS.COL_STATUS
  }) VALUES('${id}','${email}','${password}','${firstname}','${lastname}','${address}',${true},'${ipAddress}','${time}','${
    USER_STATUS.CREATED
  }')`;
  try {
    const [queryRes] = await sql.promise().query(query);
    if (queryRes) {
      marsLogger(TAG, `user [${id}] create success`);

      // create contact db
      const users = JSON.stringify([]);
      const contactQuery = `INSERT INTO ${DB_FIELDS.TABLE_CONTACT} (${DB_FIELDS.COL_ID},${DB_FIELDS.COL_USERS}) VALUES('${id}','${users}')`;
      const [contactQueryRes] = await sql.promise().query(contactQuery);
      if (contactQueryRes) {
        marsLogger(TAG, `created ${id} contact db`);
      }
    }
    sendJsonResponse(result, HTTP_STATUS_CODES[201]);
  } catch (e) {
    marsLogger(TAG, `err.code = ${e.code}`);
    sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    return;
  }
});

// delete user
router.delete(HTTP_PATH.DELETE.DELETE_DATA, function (req, result, next) {
  const id = req.params.id;
  const query = `DELETE FROM ${DB_FIELDS.TABLE_USER} WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, id, (err, res) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `user ${id} delete success`);
    sendJsonResponse(result, HTTP_STATUS_CODES[200]);
  });
});

// update user
router.put(HTTP_PATH.PUT.UPDATE_DATA, function (req, result, next) {
  const id = req.params.id;
  const body = req.body;
  const email = body.email;
  const password = body.password;
  const firstname = body.firstname;
  const lastname = body.lastname;
  const address = body.address;
  const query = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_EMAIL}=?,${DB_FIELDS.COL_PASSWORD}=?,${DB_FIELDS.COL_FIRSTNAME}=?,${DB_FIELDS.COL_LASTNAME}=?,${DB_FIELDS.COL_ADDRESS}=? WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, [id, email, password, firstname, lastname, address], (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `user [${id}] update success`);
  });
  sendJsonResponse(result, HTTP_STATUS_CODES[200]);
});

// update user password
router.put(HTTP_PATH.PUT.UPDATE_PASSWORD, function (req, result, next) {
  const id = req.params.id;
  const body = req.body;
  const password = body.password;
  const query = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_PASSWORD}=? WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, [password, id], (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `user [${id}] password change success`);
  });
  sendJsonResponse(result, HTTP_STATUS_CODES[200]);
});

// update user email
router.put(HTTP_PATH.PUT.UPDATE_EMAIL, function (req, result, next) {
  const id = req.params.id;
  const body = req.body;
  const email = body.email;
  const query = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_EMAIL}=? WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, [email, id], (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `user [${id}] email change success`);
  });
  sendJsonResponse(result, HTTP_STATUS_CODES[200]);
});

// update user enable
router.put(HTTP_PATH.PUT.UPDATE_ENABLE, function (req, result, next) {
  const id = req.params.id;
  const body = req.body;
  const enable = body.enable;
  const query = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_IS_ENABLE}=? WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, [enable, id], (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `user [${id}] enable change success`);
  });
  sendJsonResponse(result, HTTP_STATUS_CODES[200]);
});

// reset password
router.delete(HTTP_PATH.DELETE.RESET_PASSWORD, async function (req, result, next) {
  const id = req.params.id;
  const body = req.body;
  const email = body.email;
  const encryptedPassword = body.encryptedPassword;
  const decryptedPassword = body.decryptedPassword;
  try {
    const validationQuery = `SELECT * FROM ${DB_FIELDS.TABLE_USER} WHERE ${DB_FIELDS.COL_ID}=? AND ${DB_FIELDS.COL_EMAIL}=?`;
    const [validationQueryRes] = await sql.promise().query(validationQuery, [id, email]);

    if (validationQueryRes && validationQueryRes.length > 0) {
      // send email with new password
      // do not use
      //const generatedPassword = generateRandomString(8);
      const updateQuery = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_PASSWORD}=? WHERE ${DB_FIELDS.COL_ID}=?`;
      await sql.promise().query(updateQuery, [encryptedPassword, id]);
      marsLogger(TAG, `user [${id}] password reset success`);
      const imagePath = `\"https://datalabs.co.kr/images/aboutus/friends01_01.png\"`;
      // send email
      mailer.sendGmail({
        toEmail: email,
        subject: "[MARS] Password guide mail",
        text: "text",
        html: `<!doctype html>
        <html>
        <body style="margin: 0; padding: 0;">
          <p>Hello, Please check your new password
          <h2>${decryptedPassword}</h2>
          <p>If you forgot your id, please contact to our email</p>
          <br/>
          <br/>
          <br/>
          <img src=${imagePath}/>
          <br/>
          <br/>
          <p>Thank you</p>
        </body>
        </html>`,
      });
      sendJsonResponse(result, HTTP_STATUS_CODES[200]);
    } else {
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    }
  } catch (e) {
    marsLogger(TAG, e);
    sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    return;
  }
});

// get user list data
router.get(HTTP_PATH.GET.GET_LIST, function (req, result, next) {
  const query = `SELECT * FROM ${DB_FIELDS.TABLE_USER}`;
  sql.query(query, (err, res) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `all user list read success`);
    if (res.length) {
      sendJsonResponse(result, res);
    } else {
      sendJsonResponse(result, null);
    }
  });
});

// get user data
router.get(HTTP_PATH.GET.GET_DATA, function (req, result, next) {
  const id = req.params.id;
  const query = `SELECT * FROM ${DB_FIELDS.TABLE_USER} WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, id, (err, res) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `user [${id}] read success`);
    if (res.length) {
      sendJsonResponse(result, res[0]);
    } else {
      sendJsonResponse(result, null);
    }
  });
});

// login
router.put(HTTP_PATH.PUT.LOGIN, async function (req, result, next) {
  const body = req.body;
  const id = req.params.id;
  const password = body.password;

  try {
    // login check
    const loginCheckQuery = `SELECT * FROM ${DB_FIELDS.TABLE_USER} WHERE ${DB_FIELDS.COL_ID}=? AND ${DB_FIELDS.COL_PASSWORD}=?`;
    const [loginCheckQueryRes] = await sql.promise().query(loginCheckQuery, [id, password]);
    if (loginCheckQueryRes && loginCheckQueryRes.length > 0) {
      // update ipAddress, token, status
      const ipAddress = body.ipAddress;
      const token = generateRandomString(16);
      const time = new Date().getTime().toString();
      const updateQuery = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_IP_ADDRESS}=?,${DB_FIELDS.COL_TOKEN}=?,${DB_FIELDS.COL_STATUS}=?,${DB_FIELDS.COL_LAST_LOGIN_TIME}=? WHERE ${DB_FIELDS.COL_ID}=?`;
      await sql.promise().query(updateQuery, [ipAddress, token, USER_STATUS.LOGIN, time, id]);
      sendJsonResponse(result, token);
      setTimeout(function () {
        connectSocketToClient(id, ipAddress);
      }, 1000);
    } else {
      sendJsonResponse(result, HTTP_STATUS_CODES[403]);
    }
  } catch (e) {
    marsLogger(TAG, e);
    sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    return;
  }
});

// logout
router.put(HTTP_PATH.PUT.LOGOUT, async function (req, result, next) {
  const body = req.body;
  const id = req.params.id;
  const token = body.token;

  try {
    // user id check
    const loginCheckQuery = `SELECT * FROM ${DB_FIELDS.TABLE_USER} WHERE ${DB_FIELDS.COL_ID}=? AND ${DB_FIELDS.COL_TOKEN}=?`;
    const [loginCheckQueryRes] = await sql.promise().query(loginCheckQuery, [id, token]);
    if (loginCheckQueryRes && loginCheckQueryRes.length > 0) {
      // login check success
      // update status, ipAddress, token
      const updateQuery = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_STATUS}=?,${DB_FIELDS.COL_IP_ADDRESS}=?,${DB_FIELDS.COL_TOKEN}=? WHERE ${DB_FIELDS.COL_ID}=?`;
      await sql.promise().query(updateQuery, [USER_STATUS.LOGOUT, "", "", id]);
      sendJsonResponse(result, HTTP_STATUS_CODES[200]);
    } else {
      // login check failed
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    }
  } catch (e) {
    marsLogger(TAG, e);
    sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    return;
  }
});

// search user data
router.get(HTTP_PATH.GET.SEARCH, function (req, result, next) {
  const queryData = url.parse(req.url, true).query;
  const key = "%" + queryData.key + "%";
  const query = `SELECT * FROM ${DB_FIELDS.TABLE_USER} WHERE ${DB_FIELDS.COL_LASTNAME} LIKE ? OR ${DB_FIELDS.COL_FIRSTNAME} LIKE ? OR ${DB_FIELDS.COL_ADDRESS} LIKE ? OR ${DB_FIELDS.COL_EMAIL} LIKE ? OR ${DB_FIELDS.COL_ID} LIKE ?`;
  sql.query(query, [key, key, key, key, key], (err, res) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    sendJsonResponse(result, res);
  });
});

module.exports = router;
