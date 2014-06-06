/**
 * Serves files from the SD card, and interprets
 * other requests as commands
 */

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "WebRequest.h"
#include "WebResponse.h"

/**
 * Callback type to serve requests.
 * Takes a web request as argument.
 * Returns a web response.
 */
typedef WebResponse (*DynamicServeMethod) (WebRequest&);

class WebServer {
   
  private:
    // TCP server
    EthernetServer server;
    // Current client pointer (if any)
    EthernetClient* client;
    // Current request
    WebRequest request;
    // Dynamic server callback
    DynamicServeMethod callback;
    
  public:
    /**
     * Web server constructor.
     * Takes a port number to listen to.
     */
    WebServer(const unsigned short &port):
      server(port),
      client(NULL),
      callback(NULL)
    {
    }
    
    /**
     * Registers a callback used to serve requests.
     * The callback receives a WebRequest and returns a WebResponse.
     * If the WebResponse is a null response, the SD card will be searched for a file of this name
     */
    void registerServeMethod(DynamicServeMethod callback);
    
    /**
     * Start the server, Ethernet connection using DHCP.
     * Warning: DHCP has a cost in memory.
     * The first argument is the MAC address.
     */
    void begin(byte mac[6]);
    
    /**
     * Start the server, Ethernet connection not using DHCP.
     * The first argument is the MAC address.
     * The second argument is the IP address.
     */
    void begin(byte mac[6], const IPAddress &ip);
    
    /**
     * Get the server local IP address
     */
    IPAddress localIP();
    
    /**
     * Server one client if there is a request
     */
    void serve();
  
private:
  
  /**
   * Send a response to the client
   * The request is already parsed
   */
  void sendResponse();
  
};
