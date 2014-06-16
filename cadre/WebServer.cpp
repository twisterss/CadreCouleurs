#include "WebServer.h"

void WebServer::registerServeMethod(DynamicServeMethod callback) {
  this->callback = callback;
}

void WebServer::begin(byte mac[6]) {
  if (started)
    return;
  Ethernet.begin(mac);
  this->server.begin();
  started = true;
}


void WebServer::begin(byte mac[6], const IPAddress &ip) {
  if (started)
    return;
  Ethernet.begin(mac, ip);
  this->server.begin();
}

bool WebServer::isConnected() {
  return localIP() != IPAddress(0, 0, 0, 0);
}

IPAddress WebServer::localIP() {
  if (!started)
    return IPAddress(0, 0, 0, 0);
  return Ethernet.localIP();
}

void WebServer::serve() {
  if (!started)
    return;
  // listen for incoming clients
  EthernetClient client = this->server.available();
  this->client = &client;
  if (client) {
    // Parse received request
    this->request.clear();
    while (client.connected()) {
      if (client.available()) {
	char c = client.read();
	switch (this->request.state) {
	  case SERV_REQ_METHOD:
	    // Receiving the method part
	    if (c == ' ')
	      this->request.state = SERV_REQ_RESOURCE;
	    else
	      this->request.method+= c;
	    break;
	  case SERV_REQ_RESOURCE:
	    // Receiving the resource part
	    if (c == '?')
	      this->request.state = SERV_REQ_PARAMS;
	    else if (c == ' ')
	      this->request.state = SERV_REQ_MISC;
	    else if(c != '/' || this->request.resource != "")
	      this->request.resource+= c;
	    break;
	  case SERV_REQ_PARAMS:
	    // Receiving the parameters
	    if (c == ' ')
	      this->request.state = SERV_REQ_MISC;
	    else
	      this->request.params+= c;
	    break;
	  case SERV_REQ_MISC:
	    // Receiving something after the interesting part
	    if (c == '\n')
	      this->request.state = SERV_REQ_EMPTY_LINE;
	    break;
	  case SERV_REQ_EMPTY_LINE:
	    // Receiving an empty line for now
	    // An empty line means that the request is over
	    // In this case, send the response
	    if (c == '\n') {
	      this->sendResponse();
	      this->request.state = SERV_REQ_REPLIED;
	    } else if (c != '\r') {
	      this->request.state = SERV_REQ_MISC;
	    }
	    break;
	}
	// Stop listening if full request received
	if (this->request.state == SERV_REQ_REPLIED)
	  break;
      }
    }
    // Give the web browser time to receive data
    delay(1);
    // Close the connection
    client.stop();
    this->client = NULL;
  }
}

void WebServer::sendResponse() {
  if (callback != NULL) {
    // Ask the callback for a dynamic response
    WebResponse response = callback(this->request);
    if (!response.isNull()) {
      response.send(this->client);
      return;
    }
  }
  // 404 error
  WebResponse response = WebResponse::createNotFound("NOT FOUND");
  response.send(this->client);
}
