var express = require("express");
const { sql } = require("../db/db");
const { HTTP_PATH } = require("../common/constant");
const { sendJsonResponse } = require("../common/util");
var router = express.Router();

// do sql query
router.post(HTTP_PATH.POST.DEV_SQL, function (req, result, next) {
  const body = req.body;
  const query = body.query;
  sql.query(query, (err, queryRes) => {
    if (err) {
      marsLogger(TAG,err);
      sendJsonResponse(result, err);
      return;
    }
    sendJsonResponse(result, queryRes);
  });
});

module.exports = router;
