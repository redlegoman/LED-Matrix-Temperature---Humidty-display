"use strict";
//2012-06-23 http://alexandre.alapetite.fr

function escapeHtml(text)
{
	return text.replace(/&/g, "&amp;")
		.replace(/</g, "&lt;")
		.replace(/>/g, "&gt;")
		.replace(/"/g, "&quot;")
		.replace(/'/g, "&#039;");
}

var fs = require('fs');
var os = require('os');
var path = require('path');
var util = require('util');

var serverSignature = 'Node.js / Debian ' + os.type() + ' ' + os.release() + ' ' + os.arch() + ' / Raspberry Pi';

function done(request, response)
{
	util.log(request.connection.remoteAddress + '\t' + response.statusCode + '\t"' + request.method + ' ' + request.url + '"\t"' +
		request.headers['user-agent'] + '"\t"' + request.headers['accept-language'] + '"\t"' + request.headers['referer'] + '"');
}

function serve404(request, response, requestUrl)
{//When a static file is not found
	response.writeHead(404,
	{
		'Content-Type': 'text/html; charset=UTF-8',
		'Date': (new Date()).toUTCString(),
		'Server': serverSignature
	});
	response.end('<!DOCTYPE html>\n\
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en-GB" lang="en-GB">\n\
<head>\n\
<meta charset="UTF-8" />\n\
<title>404 Not Found</title>\n\
</head>\n\
<body>\n\
<h1>Not Found</h1>\n\
<p>The requested <abbr title="Uniform Resource Locator">URL</abbr> <kbd>' +
	escapeHtml(requestUrl.pathname) + '</kbd> was not found on this server.</p>\n\
</body>\n\
</html>\n\
');
	done(request, response);
}

function serveStaticFile(request, response, requestUrl)
{
	var myPath = '.' + requestUrl.pathname;
	if (myPath && (/^\.\/[a-z0-9_-]+\.[a-z]{2,4}$/i).test(myPath) && (!(/\.\./).test(myPath)))
		fs.stat(myPath, function (err, stats)
		{
			if ((!err) && stats.isFile())
			{
				var ext = path.extname(myPath);
				var mimes = { '.css': 'text/css', '.html': 'text/html', '.ico': 'image/x-icon', '.jpg': 'image/jpeg',
					'.js': 'application/javascript', '.png': 'image/png', '.txt': 'text/plain', '.xml': 'application/xml' };
				var modifiedDate = new Date(stats.mtime).toUTCString();
				if (modifiedDate === request.headers['if-modified-since'])
				{
					response.writeHead(304,
					{
						'Content-Type': ext && mimes[ext] ? mimes[ext] : 'application/octet-stream',
						'Date': (new Date()).toUTCString()
					});
					response.end();
				}
				else
				{
					response.writeHead(200,
					{
						'Content-Type': ext && mimes[ext] ? mimes[ext] : 'application/octet-stream',
						'Content-Length': stats.size,
						'Cache-Control': 'public, max-age=86400',
						'Date': (new Date()).toUTCString(),
						'Last-Modified': modifiedDate,
						'Server': serverSignature
					});
					fs.createReadStream(myPath).pipe(response);
				}
				done(request, response);
			}
			else serve404(request, response, requestUrl);
		});
	else serve404(request, response, requestUrl);
}

function serveHome(request, response, requestUrl)
{
	var now = new Date();
	response.writeHead(200,
	{
		'Content-Type': 'text/html; charset=UTF-8',
		'Date': now.toUTCString(),
		'Server': serverSignature
	});
	response.end('<!DOCTYPE html>\n\
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en-GB" lang="en-GB">\n\
<head>\n\
<meta charset="UTF-8" />\n\
<title>Test of Node.js on Raspberry Pi</title>\n\
<meta name="robots" content="noindex" />\n\
<meta name="viewport" content="initial-scale=1.0,width=device-width" />\n\
</head>\n\
<body>\n\
<pre>\n\
Hello ' + request.connection.remoteAddress + '!\n\
This is <a href="http://nodejs.org/">Node.js</a> on <a href="http://www.raspberrypi.org/">Raspberry Pi</a> :-)\n\
It is now ' + now.toISOString() + '.\n\
</pre>\n\
<ul>\n\
<li><a href="./temperature">Temperature</a></li>\n\
<li><a href="index.js">Source code</a></li>\n\
</ul>\n\
</body>\n\
</html>\n\
');
	done(request, response);
}

var arduino = require('./arduinoTemperature.js');	//Connection with Arduino

function serveTemperature(request, response, requestUrl)
{
	var now = new Date();
	response.writeHead(200,
	{
		'Content-Type': 'text/html; charset=UTF-8',
		'Date': now.toUTCString(),
		'Server': serverSignature
	});
	response.end(arduino.htmlTemperature());
	done(request, response);
}

var http = require('http');
var url = require('url');

var server = http.createServer(function (request, response)
{
	var requestUrl = url.parse(request.url);
	switch (requestUrl.pathname)
	{
		case '/': serveHome(request, response, requestUrl); break;
		case '/temperature': serveTemperature(request, response, requestUrl); break;
		default: serveStaticFile(request, response, requestUrl); break;
	}
}).listen(8080);

console.log('Node.js server running at %j', server.address());
