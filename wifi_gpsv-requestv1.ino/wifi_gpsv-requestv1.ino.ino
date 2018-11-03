
#include <SoftwareSerial.h>
#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial SerialESP8266(10, 11); // RX, TX
SoftwareSerial SerialGPS(9,8);


int year;
byte month, day, hour, minute, second, hundredths;
unsigned long chars;
unsigned short sentences, failed_checksum;
bool isReadGPS = false;
bool isRequestOK = false;
const  String ssid     = "Fibertel WiFi872 2.4GHz";         
const  String password = "0043637393";   
const String server = "192.168.0.235";
const String port = "8888";

void setup() { 
  Serial.begin(9600);
  SerialESP8266.begin(9600);

  bool isOk = isOn();
  while(!isOk){
    isOk = isOn();
  }

  //ESP8266 en modo estación (nos conectaremos a una red existente)
    SerialESP8266.println("AT+CWMODE=1");
    if(SerialESP8266.find((char*) "OK"))
          Serial.println("ClientMode");

  
  bool isConnect = connectWifi();
  while(!isConnect){
    isConnect = connectWifi();
  }
  SerialGPS.begin(9600);
}

void loop() { 

  float latitude, longitude;
  String datetime;
  
  isReadGPS = readGPS(&latitude, &longitude, &datetime);
  
  if(isReadGPS){
    Serial.println();
    SerialESP8266.begin(9600);
    delay(1000);
    bool isConnect = isConnectWifi();
    while(!isConnect){
      connectWifi(); 
      isConnect = isConnectWifi();
    }
    do{
      isRequestOK = sendRequest(latitude, longitude, datetime);
    }while(!isRequestOK);
    isRequestOK = false;
    SerialGPS.begin(9600);
    delay(9000);
  }
}

bool connectWifi() { 
  //Nos conectamos a una red wifi 
    SerialESP8266.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
    Serial.println("Connect..");
    SerialESP8266.setTimeout(10000); //Aumentar si demora la conexion
    if(SerialESP8266.find((char*) "OK")){
      Serial.println(ssid + "WIFI Connect");
      return true;
    }
    
    Serial.println("Error al conectarse en la red");
    SerialESP8266.setTimeout(2000);
    return false;
}

bool isConnectWifi() { 
  //Nos conectamos a una red wifi 
    SerialESP8266.println("AT+CWJAP?");
    Serial.println("is connect "+ ssid);
    SerialESP8266.setTimeout(10000); 
    char charBuf[ssid.length()];
    ssid.toCharArray(charBuf, ssid.length());
    if(SerialESP8266.find(charBuf)){
      Serial.println(ssid + " ok");
      return true;
    }
    Serial.println("Error: not connected " + ssid);
    SerialESP8266.setTimeout(2000);
    return false;
}

bool isOn(){
   SerialESP8266.println("AT");
  if(SerialESP8266.find((char*)"OK")){
    Serial.println("wifi Ok");
    return true;
  }
    Serial.println("Error:wifi");
    return false;

}

bool readGPS(float *lat, float *lon, String *datetime ){
  while(SerialGPS.available()){
  int c = SerialGPS.read(); 
    if(gps.encode(c)){
      float latitude, longitude;
      gps.f_get_position(&latitude, &longitude);
  
      *lat = latitude;
      *lon = longitude;
  
      gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,&hundredths);
      *datetime = String(year, DEC)+"-"+String(month, DEC)+"-"+String(day, DEC)+" "+String(hour, DEC)+":"+String(minute, DEC)+":"+String(second, DEC);
  
      gps.stats(&chars, &sentences, &failed_checksum);
      Serial.println(String(latitude)+String(longitude)+*datetime);
      return true;
    }
  }
  return false;
}


bool sendRequest(float latitude, float longitude, String datetime){
  if(NULL != latitude and NULL != longitude and NULL != datetime){
    //send tcp conexion
    SerialESP8266.println("AT+CIPSTART=\"TCP\",\"" + server + "\"," + port + ",7200");
    if( SerialESP8266.find("OK")){  
    
      //send header
      String data="latitude="+String(latitude,6)+"&longitude="+String(longitude,6)+"&datetime="+datetime;
      
      String peticionHTTP = "POST /writesample HTTP/1.1\r\n";
      peticionHTTP.concat("Host: 192.168.0.235:8888\r\n");
      peticionHTTP.concat("Accept: */*\r\n");
      peticionHTTP.concat("Content-Length: " + String(data.length()) + "\r\n");
      peticionHTTP.concat("Content-Type: application/x-www-form-urlencoded\r\n\r\n");
      peticionHTTP.concat(data);
      peticionHTTP.concat("\r\n");
     
      //Serial.println(peticionHTTP);
     //Serial.println(data);
     
      //send bytes sends
      SerialESP8266.print("AT+CIPSEND="+String(peticionHTTP.length())+"\r\n\r\n\r\n");
  
      //is ok >
      if(SerialESP8266.find(">")){
        Serial.println("Enviando HTTP . . .");
        SerialESP8266.println(peticionHTTP);
        // is ok
        if( SerialESP8266.find("SEND OK")){  
          Serial.println("Peticion HTTP enviada:");
          Serial.println();
          Serial.println(peticionHTTP);
          Serial.println("Esperando respuesta...");
                
          boolean fin_respuesta=false; 
          long tiempo_inicio=millis(); 
          String cadena="";
                
          while(fin_respuesta==false){
            while(SerialESP8266.available()>0){
              char c=SerialESP8266.read();
              Serial.write(c);
              cadena.concat(c);  
            }
            //error
            if(cadena.length()>100){
              Serial.println("Error:length max");    
              SerialESP8266.println("AT+CIPCLOSE");
              if( !SerialESP8266.find("OK")){
                Serial.println("End Conection");
                fin_respuesta=true;
              }
            }
            //error
            if((millis()-tiempo_inicio)>10000){
              Serial.println("TimeOut max");
              SerialESP8266.println("AT+CIPCLOSE");
              if( !SerialESP8266.find("OK")){
                Serial.println("End Conection");
                fin_respuesta=true;
              }
              
            }
            // is ok
            if(cadena.indexOf("CLOSED")>0){
              Serial.println("OK cadena");         
              fin_respuesta=true;
            }
          }
        }else{
          Serial.println("Error:HTTP");
        }            
      }else{
          Serial.println("Error:TCPdata");
      }
      return true;   
    }
  } 
  Serial.println("Error:TCPconection");
  return false;
 
}
  
