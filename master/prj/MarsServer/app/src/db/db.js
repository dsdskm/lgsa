require("dotenv").config();

const mysql = require("mysql2");
const { DB_FIELDS } = require("../common/constant");
const { marsLogger } = require("../common/logger");

const sql = mysql.createConnection({
  host: process.env.DB_HOST,
  user: process.env.DB_USER,
  password: process.env.DB_PASSWORD,
  database: process.env.DB_NAME,
  port: process.env.DB_PORT,
});

const TAG = "db";

sql.connect((error) => {
  if (error) {
    marsLogger(TAG, `mysql connect error ${error}`);
  } else {
    marsLogger(TAG, `mysql connected`);
    initTables();
  }
});

function initTables() {
  // user table
  tableQuery(getUserTableQuery(), DB_FIELDS.TABLE_USER);

  // conference table
  tableQuery(getConferenceTableQuery(), DB_FIELDS.TABLE_CONFERENCE);

  // contact table
  tableQuery(getContactTableQuery(), DB_FIELDS.TABLE_CONTACT);
}

function getUserTableQuery() {
  var sqlQuery = `CREATE TABLE IF NOT EXISTS ${DB_FIELDS.TABLE_USER}` + `(`;
  sqlQuery += DB_FIELDS.COL_ID + ` VARCHAR(50) PRIMARY KEY` + `,`;
  sqlQuery += DB_FIELDS.COL_EMAIL + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_PASSWORD + ` VARCHAR(100)` + `,`;
  sqlQuery += DB_FIELDS.COL_FIRSTNAME + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_LASTNAME + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_ADDRESS + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_IP_ADDRESS + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_IS_ENABLE + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_STATUS + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_LAST_LOGIN_TIME + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_TOKEN + ` VARCHAR(50)` + `)`;
  return sqlQuery;
}
function getConferenceTableQuery() {
  var sqlQuery = `CREATE TABLE IF NOT EXISTS ${DB_FIELDS.TABLE_CONFERENCE}` + `(`;
  sqlQuery += DB_FIELDS.COL_ID + ` VARCHAR(50) PRIMARY KEY` + `,`;
  sqlQuery += DB_FIELDS.COL_TOPIC + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_START + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_DURATION + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_CREATOR + ` VARCHAR(50)` + `,`;
  sqlQuery += DB_FIELDS.COL_PARTICIPANT + ` JSON` + `,`;
  sqlQuery += DB_FIELDS.COL_JOIN_USERS + ` JSON` + `)`;
  return sqlQuery;
}

function getContactTableQuery() {
  var sqlQuery = `CREATE TABLE IF NOT EXISTS ${DB_FIELDS.TABLE_CONTACT}` + `(`;
  sqlQuery += DB_FIELDS.COL_ID + ` VARCHAR(50) PRIMARY KEY` + `,`;
  sqlQuery += DB_FIELDS.COL_USERS + ` JSON` + `)`;
  return sqlQuery;
}

function tableQuery(sqlQuery, tableName) {
  sql.query(sqlQuery, (err, res) => {
    if (err) {
      marsLogger(TAG, err);
      return;
    }
    if (res.warningStatus === 0) {
      marsLogger(TAG, `init ${tableName} table`);
    }
  });
}

module.exports = {
  sql,
};
