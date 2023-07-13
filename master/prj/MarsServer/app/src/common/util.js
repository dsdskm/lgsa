const { marsLogger } = require("./logger");
const TAG = "response";
function sendJsonResponse(res, val, print = true) {
  res.send(
    JSON.stringify({
      ret: val,
    })
  );
  if (print) {
    marsLogger(TAG, val);
  }
}

module.exports = { sendJsonResponse };
