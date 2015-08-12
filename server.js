/*

This is a simple server router that runs with node.js (http://nodejs.org/)
and processes my incoming posts and other requests.

Based on code originally written by Noah Veltman, veltman.com

Also includes Flickr API module example code from 
https://github.com/Pomax/node-flickrapi

John Keefe
http://johnkeefe.net
March 2015

*/

// establish all of the dependency modules
// install these modules with "npm" the node packet manager
//    npm install express request body-parser twilio

var express = require('express'),
    fs = require('fs'),
    request = require('request'),
    bodyParser = require('body-parser'),
    cookieParser = require('cookie-parser'),
    twilio = require('twilio'),
    app = express(),
    sparkfunTemptexterKeys = require('../api_keys/sparkfun_temptexter_keys'),
    twilioKeys = require('../api_keys/twilio_keys');
    
    // The last two lines read in js files containing my secret keys,
    // mainly so I don't accidentally post them in github.
    // An example of the files is here: https://gist.github.com/jkeefe/f77e0002d8f202371bff
    // Invoked in this program with sparkfunTemtexterKeys.PRIVATE_KEY

// this is needed to read the body!
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
  extended: true
}));

// Accept incoming texts with temp, humidity & battery level
// sent via Twilio
// and then post them to a data.sparkfun.com table.

//  Note: I am confirming that the post request came from twilio
//  using the 'twilio.webhook()' paramerter in the next line.
//  That checks the TWILIO_AUTH_TOKEN, which I have stored in a 
//  file outside of this directory. It can also be set using: 
//    export TWILIO_AUTH_TOKEN=[my auth token string]
//  ... but that doesn't work with forever, which I use to run the server.
//  More the webhook authorization here:
//  https://www.twilio.com/blog/2014/01/secure-your-nodejs-webhooks-with-middleware-for-express-middleware.html
app.post(/^\/?incoming-temperature-texts/i, twilio.webhook(twilioKeys.TWILIO_AUTH_TOKEN), function(req, response){
    
  // Receive Text //
    
	// is there a body of information
	if (!req.body) {
		req.send("Posting error");
		return true;
	}
  
  var text_message = req.body.Body;
  
  // strip out spaces (the arduino code pads numbers with spaces)
  text_message = text_message.replace(/ /g,'');
  
  // text messages sent in the format:
  // device_id,temperature,humidity,battery
  // ie: john's_arduino,78.5,51.2,91
  
  // make an array out of the comma-separated values
  var values = text_message.split(',');
  
  // Post to data.sparkfun.com page //
  
  // Format for "posting" is:  http://data.sparkfun.com/input/[publicKey]?private_key=[privateKey]&battery=[value]&device_id=[value]&humidity=[value]&temperature=[value]
  // Public URL is: https://data.sparkfun.com/streams/yAY9jDmE7VSZ96LRGYJd
  
  // build the url to hit sparfun with
  var data_url = "http://data.sparkfun.com/input/yAY9jDmE7VSZ96LRGYJd?private_key=" + sparkfunTemptexterKeys.PRIVATE_KEY +
  "&device_id=" + values[0] + 
  "&temperature=" + values[1] + 
  "&humidity=" + values[2] + 
  "&battery=" + values[3];
  
  // compose the request
  var options = {
    url: data_url,
    method: 'GET'
  };
  
  // start the request/get to data.sparkfun.com
  request(options, function(error, response, body){
    
    if (error || response.statusCode != 200) {
      console.log("Error hitting Sparkfun site: ",error);
    
    } else {
      
      // everything went okay
      
    }
  });  
});

app.use(express.static(__dirname));
 
app.listen(80);
console.log('running!');