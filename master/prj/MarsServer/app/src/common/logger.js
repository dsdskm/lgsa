const fs = require("fs");
const logFileName = getFileName(new Date());

function getLogFilePath(filename) {
  return `../logfiles/${filename}.txt`;
}
if (!fs.existsSync(`${getLogFilePath(logFileName)}`)) {
  fs.writeFile(`${getLogFilePath(logFileName)}`, "", function (err) {
    if (err) {
      console.log(err);
      return;
    }
    console.log(`created log file ${logFileName}`);
  });
}

function marsLogger(tag, val, print = true, write = true) {
  const d = new Date();
  const time = d.toLocaleString();
  const log = `[${time}][${tag}] ${val}`;
  if (print) {
    console.log(log);
  }
  if (write) {
    const filename = getFileName(d);
    fs.appendFile(`${getLogFilePath(filename)}`, log + "\n", function (err) {
      if (err) {
        console.log(`err ${err}`);
        return;
      }
    });
  }
}

function getFileName(d) {
  const year = d.getFullYear();
  const month = String(d.getMonth() + 1).padStart(2, "0");
  const date = String(d.getDate()).padStart(2, "0");
  const filename = `${year}_${month}_${date}`;
  return filename;
}

module.exports = { marsLogger };
