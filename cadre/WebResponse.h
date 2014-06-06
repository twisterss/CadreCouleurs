/*
  Represents any response to an HTTP request
 */

#include <Arduino.h>
#include <Ethernet.h>

class WebResponse {
   
  private:
    // Response code
    unsigned int code;
    // Content type
    const char* type;
    // Response body
    const char* body;
    // Header field: allow all origins for XHR or not
    bool allowAllOrigins;
    
  public:
    /**
     * Raw constructor for a string body
     */
    WebResponse(unsigned int code, const char* type, const char* body):
      code(code),
      type(type),
      body(body),
      allowAllOrigins(false)
    {
    }
    
    /**
     * Shortcut for an OK response of type text
     */
    static WebResponse createText(const char* body);
    
    /**
     * Shortcut for an OK response of type HTML
     */
    static WebResponse createHTML(const char* body);
    
    /**
     * Shortcut for a NOT FOUND response
     */
    static WebResponse createNotFound(const char* body);
    
    /**
     * Shortcut for a fake null response (no response sent)
     */
    static WebResponse createNull();
    
    /**
     * Set if the header to allow all origins should be set
     */
    void setAllowAllOrigins(bool allow = true);
    
    /**
     * Returns the status code
     */
    const char& getCode();
    
    /**
     * Is it a fake null response?
     */
    bool isNull();
    
    /**
     * Send the response to the client
     */
    void send(EthernetClient* client);
};
