var createError = require("http-errors");
var express = require("express");
var path = require("path");
var cookieParser = require("cookie-parser");
var logger = require("morgan");

var indexRouter = require("./routes/index");
var userRouter = require("./routes/user");
var conferenceRouter = require("./routes/conference");
var contactRouter = require("./routes/contact");
var devRouter = require("./routes/dev");
const requestIP = require("request-ip");
const IP = require("ip");
const { marsLogger } = require("./common/logger");
const TAG = "app";
var app = express();

// view engine setup
app.set("views", path.join(__dirname, "views"));
app.set("view engine", "pug");

app.use(logger("dev"));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, "public")));

var monitor = function (req, res, next) {
  const ipAddress = IP.address();
  const apiUrl = req.protocol + "://" + req.get("host") + req.originalUrl;
  marsLogger(TAG, `apiUrl=${apiUrl}, caller ip=${ipAddress}, body=${JSON.stringify(req.body)}`, false, true);
  marsLogger(TAG, `apiUrl=${apiUrl}, caller ip=${ipAddress}`, true, false);
  next();
};
app.use(monitor);
app.use("/", indexRouter);
app.use("/user", userRouter);
app.use("/conference", conferenceRouter);
app.use("/contact", contactRouter);
app.use("/dev", devRouter);

// catch 404 and forward to error handler
app.use(function (req, res, next) {
  next(createError(404));
});

// error handler
app.use(function (err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get("env") === "development" ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render("error");
});

module.exports = app;
