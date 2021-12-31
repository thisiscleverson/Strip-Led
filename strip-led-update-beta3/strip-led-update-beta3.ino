/*
      Strip led 2.0 

      to: Cleverson Emanuel Silva -- 12/23/2021

*/
//library

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

///////////////////////////////////

#define ssid "YOURSSID"
#define password "YOURPASSWORD"

WiFiServer server(80); // set web port number to 80
String header; // variable to store the HTTP req uest

IPAddress local_IP(192, 168, 0, 123);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

/////////////////////////////////////

#define pin 3        // pin strip led
#define NumPixels 30 // number pixels of strip led

Adafruit_NeoPixel pixels(NumPixels, pin, NEO_GRB + NEO_KHZ400);

/////////////////////////////////////

int red   = 0; 
int green = 0;
int blue  = 0;
////////////////////////////////////
bool RainbowTurnON = false;
bool endRainbow =  false;

void setup() {
  Serial.begin(115200); // serial 
  pixels.begin();        // strip led  
  
  // turn on strip led in color blue
  for(int i=0; i<NumPixels; i++){
      pixels.setPixelColor(i,pixels.Color(0,230,255));
      pixels.show();
      delay(500);
  }
  
  /*/ turn on strip led with the color of rainbow  
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
        for(int i=0; i < pixels.numPixels(); i++) {
          int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
          pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
          pixels.show();
          delay(500);          
    }
  } */

  ///////////////////////////////////////////

  connect_WiFi();
}


void connect_WiFi(){
  // Print where it will connct
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  }

  Serial.println("\n\n");
  Serial.print("Connecting to: "+String(ssid)+" ");
  
  ////////////////////////////////
  WiFi.begin(ssid,password); // connect in WiFi

  while(WiFi.status() != WL_CONNECTED){
       delay(500);
       Serial.print(".");
    }
    
  //print local IP address and start web server
  Serial.print("\n\nWiFi connected in: "+String(ssid)+"\n\nIP address: ");
  Serial.print(WiFi.localIP());

    server.begin(); 
}

void loop() {
  wifi_communication();

  if(RainbowTurnON == true){
    rainbowStripLed(10);
  }
}

void wifi_communication(){
    WiFiClient client = server.available(); // client wifi

    /////////////////////////////////////
      static unsigned long currentTime  = millis();
      static unsigned long previousTime = 0;
      static const long    timeoutTime  = 2000; //(2000ms = 2s)
    /////////////////////////////////////
    

    if(client){
        currentTime  = millis();
        previousTime = currentTime;
        Serial.println("New Client connecting.");
        String currentLine = "";
                          
        while(client.connected() && currentTime - previousTime <= timeoutTime){
            currentTime = millis();

            if(client.available()){
               char c = client.read();
                Serial.write(c);
               header += c;

               if(c == '\n'){
                   if(currentLine.length() == 0){
                      client.println("HTTP/1.1 200 OK");
                      client.println("Content-type:text/html");
                      client.println("Connection: close");
                      client.println();
                   
                      // Display the HTML web page                      
                      client.println("<!DOCTYPE html><html>");
                      client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                      client.println("<link rel=\"icon\" href=\"data:,\">");
                      client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">");
                      client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js\"></script>");
                      client.println("</head><body><div class=\"container\"><div class=\"row\"><h1 style=\"color: blue; margin-left: 15px;\">ESP Color</h1></div>");
                      client.println("<a class=\"btn btn-primary btn-lg\" href=\"#\" id=\"change_color\" role=\"button\">Change Color</a> <a href='/all_RGB?RainbowON&'><button style=\"margin-left: 15px; background: #069cc2; border-radius: 6px; padding: 10px; cursor: pointer; color: #fff; border: none; font-size: 16px;\"> rainbow </button></a> ");
                      client.println("<p>  </p>");
                      client.println("<input class=\"jscolor {onFineChange:'update(this)'}\" id=\"rgb\"></div>");
                      client.println("<script>function update(picker) {document.getElementById('rgb').innerHTML = Math.round(picker.rgb[0]) + ', ' +  Math.round(picker.rgb[1]) + ', ' + Math.round(picker.rgb[2]);");
                      client.println("document.getElementById(\"change_color\").href=\"?r\" + Math.round(picker.rgb[0]) + \"g\" +  Math.round(picker.rgb[1]) + \"b\" + Math.round(picker.rgb[2]) + \"&\";}</script> </body> </html>");
                      // The HTTP response ends with another blank line
                      client.println(); 
                      
                      if(header.indexOf("GET /all_RGB?r") >= 0){
                          int pos1 = header.indexOf('r'); //red
                          int pos2 = header.indexOf('g'); // green
                          int pos3 = header.indexOf('b'); // blue
                          int pos4 = header.indexOf('&');

                          red   = header.substring(pos1+1, pos2).toInt();
                          green = header.substring(pos2+1, pos3).toInt();
                          blue  = header.substring(pos3+1, pos4).toInt();

                          setColor(red,green,blue,0);


                      }//enf if header indexOF
                        else if(header.indexOf("GET /all_RGB?") >= 0){
                           int pos = header.indexOf('?');
                           int finally = header.indexOf('&');
                           String command = header.substring(pos+1,finally);

                           if(command == "RainbowON"){
                             setColor(0,0,0,1);
                           }
                        }                  
                       break; // break out of the while loop 
                                                                                         
                   } //end if currentLine length
                    else{
                      currentLine = "";
                    }// end else
               } // end if c
                else if(c != '\r'){
                    currentLine += c;                
                }//end else                  

            } //end if client available

        }//end while 
        
        header = ""; // clear the header variable
        //Close the connection
        client.stop();
        Serial.println("Client Disconnected\n\n"); 
    }//end if Client

}//end void


void setColor(byte R,byte G,byte B,bool WM){
    switch(WM){
      case 0:
          RainbowTurnON = false;
          endRainbow = true;
         for(int  i=0; i < NumPixels; i++){
                pixels.setPixelColor(i,pixels.Color(R,G,B));
                pixels.show();
              }
      break;

      case 1:
          RainbowTurnON = true;
          endRainbow = false;
      break;
    }
}

void rainbowStripLed(int wait){
      for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
        for(int i=0; i < pixels.numPixels(); i++) {
          int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
          pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    } 
      wifi_communication();
      if(endRainbow == true){
        endRainbow = false;
        break;
      }      
      pixels.show(); // Update strip with new contents
      delay(wait); 
  }  
}



