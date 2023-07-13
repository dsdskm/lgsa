require("dotenv").config();
const nodemailer = require("nodemailer");
const { marsLogger } = require("../common/logger");
const TAG = "emailApi";
const mailSender = {
  sendGmail: function (param) {
    var transporter = nodemailer.createTransport({
      service: "gmail",
      prot: 587,
      host: "smtp.gmlail.com",
      secure: false,
      auth: {
        user: process.env.GMAIL_ID_MARS,
        pass: process.env.GMAIL_ID_MARS_PASSWORD,
      },
    });
    var mailOptions = {
      from: process.env.GMAIL_ID_MARS,
      sender:process.env.GMAIL_ID_MARS_PASSWORD,
      to: param.toEmail,
      subject: param.subject,
      text: param.text,
      html:param.html
    };

    transporter.sendMail(mailOptions, function (error, info) {
      if (error) {
        marsLogger(TAG, error);
      } else {
        marsLogger(TAG, "Email sent: " + info.response);
      }
    });
  },
};

module.exports = mailSender;
