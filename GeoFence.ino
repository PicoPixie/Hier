#include <PubNub.h>



#include <LGPRS.h>
#include <LGPRSClient.h>

#include <LGPS.h>

/* Access Point credentials for the PAYG SIM
 * in the MTKLIO Unit needed for GPRS data 
 * connectivity 'in the field'
 */
#define APN ""
#define UNAME ""
#define PASS ""

/* local client that subscribes to the privchan
 * to listen out for GeoFence breach
 * notifications coming from the sheepdog
 */
PubSubClient *subclient; 

/* local client that publishes to the sheepCast channel
 * over GPRS this unit's ID (the privchan) and current LatLng
 * 
 * Only the sheepdog/webserver script subscribes to sheepCast
 *  
 *  Many-to-1 casting
 */
LGPRSClient *c;


/* structure to hold our retrieved GPS sentence
 *  
 */
gpsSentenceInfoStruct info = {0};

/* API keys for the PubNub Real-Time Data Stream
 * which allows comms between the sheep (MTKLIO Unit) 
 * and sheepdog (webserver script calling Google Maps)
 */
char pubkey[] = "pub-YOUR-PUBLISH-KEY";
char subkey[] = "sub-YOUR-SUBSCRIBE-KEY";

/* This is the channel the 'sheep' broadcast their LatLng updates and UnitID on.
 * A script on the web server (the 'sheepdog') subscribes to this channel for updates
 * and calculates if inside a given circle. (GeoFence)
 * Location data of 'sheepdog' is obtained from web browsers supporting
 * geolocation API.
 * Uses Google Maps JS API to display a map to the browser with markers representing
 * the 'sheepdog' and all 'sheep' broadcasting on this channel.
 */
char sheepcast[] = "sheepCast"; 

/* Unique channel for this sheep (LinkIt ONE device) that the sheepdog 
 * will broadcast on in the event of a GeoFence breach.
 * The sheep needs to subscribe to this channel so it can be notified
 * if it's breached the GeoFence.
 */
char privchan[] = "Unit23"; 
int pchan = 23;

/* keep track of when this Unit last rx'd a breach
 * notification, we don't want to keep sending SMS, limit to
 * once every 5mins. But always do the first one then start the timer!
 */
unsigned long lastBreach = 0;
const long interval = 300000; //ms representing 5mins
boolean firstBreach = true;

void setup() {
  // start the serial port for help in debugging
  Serial.begin(9600);
  // wait for user to send CR/LF, else we lose data
  // this is a limitation of the LinkIt ONE
  while(Serial.available()==0) {}
  Serial.println("Serial setup OK.");
  
  PubNub.begin(pubkey,subkey);
  Serial.println("PubNub setup OK.");
  
  // start the GPS service for getting location info
  LGPS.powerOn();
  Serial.println("GPS ON.");
  
  // start the GSM service for sending SMS messages
   
  // start the GPRS data connection
  while(!LGPRS.attachGPRS(APN,UNAME,PASS)) {
    Serial.println("Attempting to connect over GPRS.");
    delay(1000);
  }
  Serial.println("GPRS setup.");
     
}

void loop() {
   
  Serial.println();
  Serial.println("Retrieving GPS data.");
  // get GPS data
  LGPS.getData(&info);
  // parse GPS & build data string to send to Cloud/DataStream
  Serial.println("Parsing GPS data, uploading to PubNub.");
  printGPGGA((char*)info.GPGGA);
    
  subclient = PubNub.subscribe(privchan);
  if(!subclient) {
     Serial.println("Not subscribed to private channel.");
     delay(1000);
     return;
  } else {
  
    Serial.println("Subscribed to private channel OK");
    String mesg;
    while(subclient->wait_for_data()) {
      char v = subclient->read();
      mesg += v;
    }
    Serial.println(mesg);
    // compare substring bc PubNub async 
    // assuming LIFO bunching
    char* indicator = "B"; // B for Breach
    char tmp[1] = {mesg.charAt(2)}; // 0=[, 1=", 2=B
    if(0==strcmp(tmp,indicator)) {
      Serial.println();
      Serial.println("Received a BREACH NOTIFICATION.");
      /* check current time if within 5mins of last
       * breach notification DONT SEND SMS
       * millis() will overflow in ~50days
       */
       unsigned long thisBreach = millis();
       if((thisBreach - lastBreach >= interval) || (firstBreach)) {
          /* need to notify of first breach since booting unit
          * and any breach outside of 5min window
          */
          lastBreach = thisBreach;
          firstBreach = false;
          
          Serial.println("Sending SMS.");
          postSMS(pchan);
          
       } else Serial.println("Breach within 5min window.");
    }
    //rx'd an [OK] no geofence breach, no SMS
    Serial.println();
    subclient->stop();
  }
  // block before next position report
  delay(5000);
}

