/*
 * Represents an HTTP request
 */

#include <Arduino.h>

// Request parsing state indicators
typedef enum {
  SERV_REQ_METHOD,
  SERV_REQ_RESOURCE,
  SERV_REQ_PARAMS,
  SERV_REQ_EMPTY_LINE,
  SERV_REQ_MISC,
  SERV_REQ_REPLIED
} WebRequestState;

class WebRequest {
   
  public:
    // State of the processing of the request
    WebRequestState state;
    // Data received in the request
    String method, resource, params;
    
    /**
     * Initial empty request constructor
     */
    WebRequest():
      state(SERV_REQ_METHOD)
    {
    }
    
    /**
     * Clear request data (new request)
     */
    void clear();
};

