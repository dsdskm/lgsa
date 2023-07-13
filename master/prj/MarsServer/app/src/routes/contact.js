var express = require("express");
const { sql } = require("../db/db.js");
const { HTTP_STATUS_CODES, DB_FIELDS, HTTP_PATH } = require("../common/constant.js");
const { sendJsonResponse } = require("../common/util.js");
const { marsLogger } = require("../common/logger.js");
var router = express.Router();
const TAG = "contact";

// create contact
router.post(HTTP_PATH.POST.CREATE_DATA, function (req, result, next) {
  const body = req.body;
  const id = body.id;
  const users = JSON.stringify(body.users);
  const query = `INSERT INTO ${DB_FIELDS.TABLE_CONTACT} (${DB_FIELDS.COL_ID},${DB_FIELDS.COL_USERS}) VALUES('${id}','${users}')`;
  sql.query(query, (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `contact ${id} insert success`);
    sendJsonResponse(result, HTTP_STATUS_CODES[201]);
  });
});

// delete contact
router.delete(HTTP_PATH.DELETE.DELETE_DATA, function (req, result, next) {
  const id = req.params.id;
  const query = `DELETE FROM ${DB_FIELDS.TABLE_CONTACT} WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, id, (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `contact ${id} delete success`);
    sendJsonResponse(result, HTTP_STATUS_CODES[200]);
  });
});

// update contact
router.put(HTTP_PATH.PUT.UPDATE_DATA, function (req, result, next) {
  const body = req.body;
  const id = body.id;
  const users = JSON.stringify(body.users);
  const query = `UPDATE ${DB_FIELDS.TABLE_CONTACT} SET ${DB_FIELDS.COL_USERS}=? WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, [users, id], (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `contact ${id} update success`);
    sendJsonResponse(result, HTTP_STATUS_CODES[200]);
  });
});

// get contact list
router.get(HTTP_PATH.GET.GET_LIST, async function (req, result, next) {
  const id = req.params.id;

  try {
    const contactSelectQuery = `SELECT * FROM ${DB_FIELDS.TABLE_CONTACT} WHERE ${DB_FIELDS.COL_ID}=?`;
    const [contactSelectQueryRes] = await sql.promise().query(contactSelectQuery, id);
    if (contactSelectQueryRes && contactSelectQueryRes.length > 0) {
      // get users
      const users = contactSelectQueryRes[0].users;
      if (users && users.length > 0) {
        var usersText = "";
        for (var i = 0; i < users.length; i++) {
          usersText += `'${users[i]}'`;
          if (i < users.length - 1) {
            usersText += ",";
          }
        }
        const userSelectQuery = `SELECT * FROM ${DB_FIELDS.TABLE_USER} WHERE ${DB_FIELDS.COL_ID} IN (${usersText})`;
        const [userSelectQueryRes] = await sql.promise().query(userSelectQuery);
        if (userSelectQueryRes) {
          marsLogger(TAG, `contact list read success`);
          sendJsonResponse(result, userSelectQueryRes);
        }
      } else {
        sendJsonResponse(result, []);
      }
    } else {
      sendJsonResponse(result, []);
    }
  } catch (e) {
    marsLogger(TAG, e);
    sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    return;
  }
});

module.exports = router;
