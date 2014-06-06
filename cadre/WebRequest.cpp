#include "WebRequest.h"

void WebRequest::clear()
{
  this->state = SERV_REQ_METHOD;
  this->method = "";
  this->resource = "";
  this->params = "";
}
