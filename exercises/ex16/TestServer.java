// A simple HTTP server to help test your 

import java.io.IOException;
import java.io.OutputStream;
import java.time.LocalDateTime;
import java.net.InetSocketAddress;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.Headers;

/** Server for testing our simple HTTP 1.0 client. */
public class TestServer {
  /** Global counter for the number of requests processed. */
  static int requestCount = 0;

  /** Wapper object for the function that responds to requests. */
  static class MyHandler implements HttpHandler {
    public void handle( HttpExchange ex ) throws IOException {
      // To the console, report some information about the request.
      Headers hdr = ex.getRequestHeaders();
      System.out.println( "Path: " + ex.getRequestURI() );
      System.out.println( "Host: " + hdr.getFirst( "Host" ) );
      System.out.println( "User-Agent: " + hdr.getFirst( "User-Agent" ) );
      System.out.println( "Accept: " + hdr.getFirst( "Accept" ) );
      System.out.println();

      // Fill in the body of the response with some information about
      // the request.
      StringBuilder response = new StringBuilder();
      response.append( "Request URI: " + ex.getRequestURI() + "\n" );
      response.append( "Request addr: " +
                       ex.getRemoteAddress().getAddress() +
                       "\n" );
      // Increment the request count and include that in the response.
      // Do we need to worry about synchronization here?
      requestCount++;
      response.append( "Request count: " + requestCount + "\n" );
      
      // Turn the response into an array of bytes and send it back.
      byte[] body = response.toString().getBytes();
      ex.sendResponseHeaders( 200, body.length );

      // Write out the body as a sequence of bytes.
      OutputStream out = ex.getResponseBody();
      out.write( body );
      out.close();
    }
  }
  public static void main(String[] args) throws Exception {
    // Run the test server with the port number as an argument.
    if ( args.length != 1 ) {
      System.err.println( "usage: TestServer <port>" );
      System.exit( 1 );
    }
    
    // Object containing our address and chosen port number.
    InetSocketAddress addr =
      new InetSocketAddress( Integer.parseInt( args[ 0 ] ) );

    // Server to get http requests at this address and dispatch them
    // to a handler.
    HttpServer server = HttpServer.create( addr, 0 );

    // Send all URLs to an isntance of MyHandler.
    server.createContext( "/", new MyHandler() );

    // Start receiving and and responding to requests.
    server.start();
  }
}
