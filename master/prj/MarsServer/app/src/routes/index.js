var express = require("express");
const { HTTP_STATUS_CODES } = require("../common/constant");
const { sendJsonResponse } = require("../common/util");
var router = express.Router();

router.get("/", function (req, result, next) {
  sendJsonResponse(result, HTTP_STATUS_CODES[200]);
});

module.exports = router;
