"use strict";
//2012-06-23 http://alexandre.alapetite.fr

var arduinoSerialPort = '/dev/ttyUSB0';	//Serial port over USB connection between the Raspberry Pi and the Arduino

var os = require('os');
var serverSignature = 'Node.js / Debian ' + os.type() + ' ' + os.release() + ' ' + os.arch() + ' / Raspberry Pi B + Arduino Uno R3';

var postOptions =
{
	host: 'posttestserver.com',	//Change to your own server
	path: '/post.php',
	method: 'POST',
	headers:
	{
		'Content-Type': 'application/x-www-form-urlencoded',
		'Connection': 'close',
		'User-Agent': serverSignature
	}
};

var http = require('http');
function postData(s)
{//Standard HTTP POST request
	var myOptions = postOptions;
	postOptions.headers['Content-Length'] = s.length;

	var requestPost = http.request(myOptions, function(res)
	{
		res.setEncoding('utf8');
		/*res.on('data', function (chunk)
		{
			console.log(chunk);
		});*/
	});

	requestPost.on('error', function(e)
	{
		console.warn(e);
	});

	requestPost.write(s);
	requestPost.end();
}


var serialport = require('serialport');
var serialPort = new serialport.SerialPort(arduinoSerialPort,
{//Listening on the serial port for data coming from Arduino over USB
baudrate:9600,
parser: serialport.parsers.readline('\n')
});

var lastTemperatureIndoor = 0;
var lastTemperatureOutoor = 0;
var lastHumidityIndoor = 0;
var lastHumidityOutdoor = 0;
var dateLastInfo = new Date(0);

var querystring = require('querystring');

var util = require("util"), repl = require("repl");
serialPort.write("t");
//serialPort.write("t");
serialPort.on('data', function (data)
{//When a new line of text is received from Arduino over USB
	try
	{
		var j = JSON.parse(data);
		lastTemperatureIndoor = j.celsius;
		lastTemperatureOutoor = j.celsius2;
		lastHumidityIndoor = j.humidty;
		lastHumidityOutdoor = j.humidity2;
		dateLastInfo = new Date();
		postData(querystring.stringify(j));	//Forward the Arduino information to another Web server
//
var fs = require('fs');
var log = fs.createWriteStream('/home/pi/www/log.txt', {'flags': 'a'});
log.write(dateLastInfo)
log.write(",");
log.write(lastTemperatureIndoor);
log.write(",");
log.write(lastHumidityIndoor);
log.write(",");
log.write(lastTemperatureOutoor);
log.write(",");
log.write(lastHumidityOutdoor);
log.end("\n");



//
	}
	catch (ex)
	{
		console.warn(ex);
	}
});


function colourScale(t)
{//Generate an HTML colour in function of the temperature
	if (t <= -25.5) return '0,0,255';
	if (t <= 0) return Math.round(255 + (t * 10)) + ',' + Math.round(255 + (t * 10)) + ',255';
	if (t <= 12.75) return Math.round(255 - (t * 20)) + ',255,' + Math.round(255 - (t * 20));
	if (t <= 25.5) return Math.round((t - 12.75) * 20) + ',255,0';
	if (t <= 38.25) return '255,' + Math.round(255 - (t - 25.5) * 20) + ',0';
	return '255,0,0';
}
function HcolourScale(t)
{//Generate an HTML colour in function of the temperature
	if (t <= -25.5) return '0,0,255';
	if (t <= 0) return Math.round(255 + (t * 10)) + ',' + Math.round(255 + (t * 10)) + ',255';
	if (t <= 60.75) return Math.round(255 - (t * 20)) + ',255,' + Math.round(255 - (t * 20));
	if (t <= 75.5) return Math.round((t - 12.75) * 20) + ',255,0';
	if (t <= 98.25) return '255,' + Math.round(255 - (t - 25.5) * 20) + ',0';
	return '255,0,0';
}

function htmlTemperature()
{
	return '<!DOCTYPE html>\n\
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en-GB" lang="en-GB">\n\
<head>\n\
<meta charset="UTF-8" />\n\
<meta http-equiv="refresh" content="10" />\n\
<title>Temperature - Arduino - Raspberry Pi</title>\n\
<meta name="keywords" content="Temperature, Arduino, Raspberry Pi" />\n\
<meta name="viewport" content="initial-scale=1.0,width=device-width" />\n\
<meta name="geo.position" content="55.6793;12.4864" />\n\
<meta name="geo.region" content="DK-84" />\n\
<meta name="geo.placename" content="Vanløse" />\n\
<meta name="robots" content="noindex" />\n\
<style type="text/css">\n\
html, body {background:black; color:white; font-family:sans-serif; text-align:center}\n\
.out {font-size:48pt}\n\
.in {font-size:36pt}\n\
.r, .sb {bottom:0; color:#AAA; position:absolute}\n\
.r {left:0.5em; margin-right:5em; text-align:left}\n\
.sb {right:0.5em}\n\
a {color:#AAA; text-decoration:none}\n\
a:hover {border-bottom:1px dashed}\n\
</style>\n\
</head>\n\
<body>\n\
<p>Outdoor Temperature <strong class="out" style="color:rgb(' + colourScale(lastTemperatureOutoor) + ')">' + (Math.round(lastTemperatureOutoor * 10) / 10.0) + '°C</strong></p>\n\
<p>Outdoor Humidity <strong class="out" style="color:rgb(' + HcolourScale(lastHumidityOutdoor) + ')">' + (Math.round(lastHumidityOutdoor * 10) / 10.0) + '%</strong></p>\n\
<p>Indoor Temperature <strong class="in" style="color:rgb(' + colourScale(lastTemperatureIndoor) + ')">' + (Math.round(lastTemperatureIndoor * 10) / 10.0) + '°C</strong></p>\n\
<p>Indoor Humidity <strong class="in" style="color:rgb(' + HcolourScale(lastHumidityIndoor) + ')">' + (Math.round(lastHumidityIndoor * 10) / 10.0) + '%</strong></p>\n\
</body>\n\
</html>\n\
';
}

module.exports.htmlTemperature = htmlTemperature;
