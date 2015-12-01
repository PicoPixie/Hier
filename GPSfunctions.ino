

extern LGPRSClient *c;
extern char sheepcast[];
extern char privchan[];



void uploadData(char* dataString) {

  //publish to PubNub DataStream for subscribers to collect
  c = PubNub.publish(sheepcast, dataString);
  if(!c) {
     Serial.println("Publishing error.");
     delay(1000);
     return; // goes back to loop() gets fresh coords, tries again with those
  }
  while(c->connected()) {
    while(c->connected() && !c->available()) {} // wait until available

     char v = c->read();
     Serial.print(v);
  }
  c->stop();
  Serial.println();
}

float arrayToFloat(const char* str) {
  
  char tmp_str[20];
  float converted;
  
  strcpy(tmp_str,str);  
  converted = atof(tmp_str);
  return converted;
}

const char* nextToken(const char* src, char* buf) {
  
  int i = 0;
  
  while(src[i] != 0 && src[i] != ',') i++;
  if(buf) {
    strncpy(buf,src,i);
    buf[i] = 0; 
  }
  if(src[i]) i++;

  return src+i;
}

void convertCoords(float lat, float lon, const char* ns, const char* ew, float &lat_return, float &lon_return) {
  
  int lat_deg_int = int(lat/100); // first 2 chars latitudinal degree
  float lat_float = lat - (lat_deg_int*100); // remove degree part
  lat_return = lat_deg_int + (lat_float/60);  

  if(*ns == 'S') {
    lat_return *= -1;
  }

  int lon_deg_int = int(lon/100);
  float lon_float = lon - (lon_deg_int*100);
  lon_return = lon_deg_int + (lon_float/60);

  if(*ew == 'W') {
    lon_return *= -1;
  }
}

void printGPGGA(const char* sentence) {
  
  char lat[20] = {0};
  char lon[20] = {0};
  char buf[20] = {0};
  char ns[1] = {0};
  char ew[1] = {0};
  char fix[1] = {0};
  char sat[20] = {0};
  char hdop[10] = {0};
  char alt[20] = {0};
  const char* p = sentence;
  
  p = nextToken(p,0); // $GPGGA
  p = nextToken(p,0); // Timestamp
  p = nextToken(p,lat); // latitude
  p = nextToken(p,ns); // N or S 
  p = nextToken(p,lon); // longitude
  p = nextToken(p,ew); // E or W 
  p = nextToken(p,fix); // fix quality, 1 = GPS fix
  p = nextToken(p,sat); // satellites tracked
  p = nextToken(p,hdop); // horiz. dilution -- UNUSED
  p = nextToken(p,alt); // altitude in m above MSL -- UNUSED

  if(fix[0] == '1') {
    Serial.print(atoi(sat));
    Serial.println(" satellite(s) fixed");
    
    // convert str coords to floats
    float f_lat = arrayToFloat(lat);
    float f_lon = arrayToFloat(lon);
    // convert to decimal coords for Cloud service
    float f_lat_fixed, f_lon_fixed;
    convertCoords(f_lat, f_lon, ns, ew, f_lat_fixed, f_lon_fixed);
    // now build the upload object
    char GPSdata[180] = {0};
    char str_lat_fixed[20];
    char str_lon_fixed[20];
    strcat(GPSdata,"{\"id\":\"");
    strcat(GPSdata,privchan);
    strcat(GPSdata,"\",\"lat\":");
    sprintf(str_lat_fixed,"%.4f",f_lat_fixed);
    strcat(GPSdata,str_lat_fixed); 
    strcat(GPSdata,",\"lng\":");
    sprintf(str_lon_fixed,"%.4f",f_lon_fixed);
    strcat(GPSdata,str_lon_fixed); 
    strcat(GPSdata,"}");
    
    
    Serial.print("GPS data string: ");
    Serial.println(GPSdata);
    // POST it to Cloud
    uploadData(GPSdata);
  } else {
    Serial.println("No GPS fix...");
  }
}

