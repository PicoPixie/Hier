#include <LGPRS.h>
#include <LGPRSClient.h>

LGPRSClient u;


/* UbiDots IoT data stream 
 * values 
 */
char action[] = "POST ";
char server[] = "things.ubidots.com";
char path[] = "/api/v1.6/variables/YOUR_VAR_ID_HERE/values";
char token[] = "YOUR_API_KEY_HERE";
int port = 80;

void postSMS(int channel) {
  String jsn;
  jsn = "{\"value\": ";
  jsn += (String)channel;
  jsn += "}";

  if(u.connect(server,port)) {
/*
    Serial.print(action);
    Serial.print(path);
    Serial.println(" HTTP/1.1");
    Serial.println("Content-Type: application/json");
    Serial.print("X-Auth-Token: ");
    Serial.println(token);
    Serial.print("Host: ");
    Serial.print(server);
    Serial.println(":80");
    Serial.println();
    Serial.println(jsn);
    Serial.println();
*/  
    u.print(action);
    u.print(path);
    u.println(" HTTP/1.1");
    u.println("Content-Type: application/json");
    u.print("X-Auth-Token: ");
    u.println(token);
    u.print("Host: ");
    u.print(server);
    u.println(":80");
    u.println();
    u.println(jsn);
    u.println();
    
    while(u.connected()) {
      while(u.connected() && !u.available()) {} //wait
      char v = u.read();
      if (v < 0) break;
      String response;
      response += (String)v;
      Serial.print(response);
    }
    u.stop();
    Serial.println();
  } else Serial.println("Couldn't access UbiDots");
}
