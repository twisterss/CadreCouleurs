#include "WebResponse.h"

WebResponse WebResponse::createText(const char* body) {
  return WebResponse(200, "text/plain", body);
}

WebResponse WebResponse::createHTML(const char* body) {
  return WebResponse(200, "text/html", body);
}

WebResponse WebResponse::createNotFound(const char* body) {
  return WebResponse(404, "text/html", body);
}

WebResponse WebResponse::createNull() {
  return WebResponse(0, "", "");
}

void WebResponse::setAllowAllOrigins(bool allow) {
  allowAllOrigins = allow;
}

const char& WebResponse::getCode() {
  return code;
}

bool WebResponse::isNull() {
  return code == 0;
}

void WebResponse::send(EthernetClient* client)
{
  // Send nothing if null
  if (isNull())
    return;
  // Send the header
  client->print("HTTP/1.1 ");
  char codeMessage[25] = "500 SERVER ERROR";
  switch(code) {
    case 200:
      strcpy(codeMessage, "200 OK");
      break;
    case 404:
      strcpy(codeMessage, "404 NOT FOUND");
      break;
  }
  client->println(codeMessage);
  client->print("Content-Type: ");
  client->println(type);
  if (allowAllOrigins)
    client->println("Access-Control-Allow-Origin: *");
  client->println("Connection: close");
  client->println();
  // Send the content
  client->print(body);
}
