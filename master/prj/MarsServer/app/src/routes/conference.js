var express = require("express");
const { sql } = require("../db/db");
const { HTTP_STATUS_CODES, DB_FIELDS, HTTP_PATH, SOCKET_MSG_TYPE, USER_STATUS } = require("../common/constant");
const { sendJsonResponse } = require("../common/util");
const { marsLogger } = require("../common/logger");
const { sendConferenceNoti } = require("../socket/netSocket");
var router = express.Router();

const TAG = "conference";

// create conference
router.post(HTTP_PATH.POST.CREATE_DATA, function (req, result, next) {
  const body = req.body;
  const id = new Date().getTime().toString();
  const topic = body.topic;
  const start = body.start;
  const duration = body.duration;
  const participant = JSON.stringify(body.participant);
  const creator = body.creator;
  const query = `INSERT INTO ${DB_FIELDS.TABLE_CONFERENCE} (${DB_FIELDS.COL_ID},${DB_FIELDS.COL_TOPIC},${DB_FIELDS.COL_START},${DB_FIELDS.COL_DURATION},${DB_FIELDS.COL_PARTICIPANT},${DB_FIELDS.COL_CREATOR}) VALUES('${id}','${topic}','${start}','${duration}','${participant}','${creator}')`;
  sql.query(query, (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `conference [${id}] insert success`);
    sendConferenceNoti({
      type: SOCKET_MSG_TYPE.CALL_RESERVE,
      cid: id,
      participant: participant,
    });
    sendJsonResponse(result, id);
  });
});

// delete conference
router.delete(HTTP_PATH.DELETE.DELETE_DATA, function (req, result, next) {
  const id = req.params.id;
  const query = `DELETE FROM ${DB_FIELDS.TABLE_CONFERENCE} WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, id, (err, _) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `conference [${id}] delete success`);
    sendJsonResponse(result, HTTP_STATUS_CODES[200]);
  });
});

// update conference
router.put(HTTP_PATH.PUT.UPDATE_DATA, async function (req, result, next) {
  const id = req.params.id;
  const body = req.body;
  const topic = body.topic;
  const start = body.start;
  const duration = body.duration;
  const participant = body.participant;
  try {
    // update conference
    const updateQuery = `UPDATE ${DB_FIELDS.TABLE_CONFERENCE} SET ${DB_FIELDS.COL_TOPIC}=?,${DB_FIELDS.COL_START}=?,${DB_FIELDS.COL_DURATION}=?,${DB_FIELDS.COL_PARTICIPANT}=? WHERE ${DB_FIELDS.COL_ID}=?`;
    const [updateQueryRes] = await sql
      .promise()
      .query(updateQuery, [topic, start, duration, JSON.stringify(participant), id]);
    if (updateQueryRes) {
      marsLogger(TAG, `conference [${id}] update success`);
    }
    sendJsonResponse(result, HTTP_STATUS_CODES[200]);
  } catch (e) {
    if (e) {
      marsLogger(TAG, e);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
  }
});

// get conference data by conference id
router.get(HTTP_PATH.GET.GET_DATA, function (req, result, next) {
  const id = req.params.id;
  const query = `SELECT * FROM ${DB_FIELDS.TABLE_CONFERENCE} WHERE ${DB_FIELDS.COL_ID}=?`;
  sql.query(query, id, (err, res) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `conference [${id}] read success`);
    if (res.length) {
      sendJsonResponse(result, res[0]);
    } else {
      sendJsonResponse(result, null);
    }
  });
});

// get conference list by participant
router.get(HTTP_PATH.GET.GET_LIST, function (req, result, next) {
  const id = `%\"${req.params.id}\"%`;
  const query = `SELECT * FROM ${DB_FIELDS.TABLE_CONFERENCE} WHERE ${DB_FIELDS.COL_PARTICIPANT} LIKE ?`;
  sql.query(query, id, (err, res) => {
    if (err) {
      marsLogger(TAG, err);
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
      return;
    }
    marsLogger(TAG, `conference list read success`);
    if (res) {
      sendJsonResponse(result, res, false);
    } else {
      sendJsonResponse(result, null);
    }
  });
});

router.post(HTTP_PATH.POST.CALL_REQUEST, async function (req, result, next) {
  const cid = req.params.id;
  const body = req.body;
  const uid = body.userId;

  const selectQuery = `SELECT ${DB_FIELDS.COL_CREATOR} FROM ${DB_FIELDS.TABLE_CONFERENCE} WHERE ${DB_FIELDS.COL_ID} =?`;
  // get conference participant
  try {
    const [selectQueryRes] = await sql.promise().query(selectQuery, cid);
    if (selectQueryRes && selectQueryRes.length > 0) {
      const creator = selectQueryRes[0].creator;
      // call reqeust
      sendConferenceNoti({
        type: SOCKET_MSG_TYPE.CALL_REQUEST,
        uid: uid,
        cid: cid,
        creator: creator,
      });
    }
    sendJsonResponse(result, HTTP_STATUS_CODES[200]);
  } catch (e) {
    marsLogger(TAG, e);
    sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    return;
  }
});

router.post(HTTP_PATH.POST.CALL_REJECT, async function (req, result, next) {
  const cid = req.params.id;
  const body = req.body;
  const uid = body.userId;

  try {
    // get current participant
    const selectQuery = `SELECT ${DB_FIELDS.COL_PARTICIPANT} FROM ${DB_FIELDS.TABLE_CONFERENCE} WHERE ${DB_FIELDS.COL_ID}=?`;
    const [selectQueryRes] = await sql.promise().query(selectQuery, cid);
    if (selectQueryRes) {
      const cParticipant = selectQueryRes[0].participant;

      const nParticipant = Object.values(cParticipant).filter((p) => {
        return p !== uid;
      });
      const updateQuery = `UPDATE ${DB_FIELDS.TABLE_CONFERENCE} SET ${DB_FIELDS.COL_PARTICIPANT}=? WHERE ${DB_FIELDS.COL_ID}=?`;
      await sql.promise().query(updateQuery, [JSON.stringify(nParticipant), cid]);
      marsLogger(TAG, `call reject [${uid}]`);
      // call reqeust
      sendConferenceNoti({
        type: SOCKET_MSG_TYPE.CALL_REJECT,
        uid: uid,
        cid: cid,
      });
    }
    sendJsonResponse(result, HTTP_STATUS_CODES[200]);
  } catch (e) {
    marsLogger(TAG, e);
    sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    return;
  }
});

// update join users
router.put(HTTP_PATH.PUT.UPDATE_JOIN_USERS, async function (req, result, next) {
  const id = req.params.id;
  const body = req.body;
  const userId = body.userId;
  const isJoin = body.isJoin;
  const selectQuery = `SELECT * FROM ${DB_FIELDS.TABLE_CONFERENCE} WHERE ${DB_FIELDS.COL_ID} =?`;
  // get conference participant
  try {
    const [selectQueryRes] = await sql.promise().query(selectQuery, id);
    if (selectQueryRes && selectQueryRes.length > 0) {
      var participant = selectQueryRes[0].participant;
      var joinUsers = selectQueryRes[0].joinUsers;
      if (isJoin) {
        // join users is null
        if (!joinUsers) {
          joinUsers = [];
          joinUsers.push(userId);
        }

        // not in join users
        if (!joinUsers.includes(userId)) {
          joinUsers.push(userId);
        }

        // in join users, but not in participant
        if (!participant.includes(userId)) {
          participant.push(userId);
        }
      } else {
        // out join
        if (joinUsers) {
          const _joinUsers = joinUsers.filter((value) => {
            return value !== userId;
          });
          joinUsers = _joinUsers;
        }
      }

      participant = JSON.stringify(participant);
      joinUsers = JSON.stringify(joinUsers);

      const updateQuery = `UPDATE ${DB_FIELDS.TABLE_CONFERENCE} SET ${DB_FIELDS.COL_PARTICIPANT}=?,${DB_FIELDS.COL_JOIN_USERS}=? WHERE ${DB_FIELDS.COL_ID}=?`;
      await sql.promise().query(updateQuery, [participant, joinUsers, id]);
      marsLogger(TAG, `conference [${id}] join user update success`);
      sendJsonResponse(result, HTTP_STATUS_CODES[200]);
      const userUpdateQuery = `UPDATE ${DB_FIELDS.TABLE_USER} SET ${DB_FIELDS.COL_STATUS}=? WHERE ${DB_FIELDS.COL_ID}=?`;
      let status;
      if (isJoin) {
        // join
        sendConferenceNoti({
          type: SOCKET_MSG_TYPE.CALL_JOINED,
          cid: id,
          uid: userId,
        });
        status = USER_STATUS.BUSY;
      } else {
        // left
        sendConferenceNoti({
          type: SOCKET_MSG_TYPE.CALL_LEFT,
          cid: id,
          uid: userId,
        });
        status = USER_STATUS.LOGIN;
      }
      await sql.promise().query(userUpdateQuery, [status, userId]);
    } else {
      sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    }
  } catch (e) {
    marsLogger(TAG, e);
    sendJsonResponse(result, HTTP_STATUS_CODES[500]);
    return;
  }
});

module.exports = router;
