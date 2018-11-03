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
const  String ssid     = "SANTINO";         // The SSID (name) of the Wi-Fi network you want to connect to
const  String password = "2214263696";     // The password of the Wi-Fi network
const String server = "www.aprende-web.net";
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
          Serial.println("ESP8266 en modo Estacion");

  
  bool isConnect = connectWifi();
  while(!isConnect){
    isConnect = connectWifi();
  }
  SerialGPS.begin(9600);
}

bool connectWifi() { 
  //Nos conectamos a una red wifi 
    SerialESP8266.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
    Serial.println("Conectandose a la red ...");
    SerialESP8266.setTimeout(10000); //Aumentar si demora la conexion
    if(SerialESP8266.find((char*) "OK")){
      Serial.println(ssid + " WIFI conectado");
      return true;
    }
    
    Serial.println("Error al conectarse en la red");
    SerialESP8266.setTimeout(2000);
    return false;
}

bool isConnectWifi() { 
  //Nos conectamos a una red wifi 
    SerialESP8266.println("AT+CWJAP?");
    Serial.println("Verificando conexion a "+ ssid);
    SerialESP8266.setTimeout(10000); 
    char charBuf[ssid.length()];
    ssid.toCharArray(charBuf, ssid.length());
    if(SerialESP8266.find(charBuf)){
      Serial.println(ssid + " WIFI conectado");
      return true;
    }
    Serial.println("Error: not connected to " + ssid);
    SerialESP8266.setTimeout(2000);
    return false;
}

bool isOn(){
   SerialESP8266.println("AT");
  if(SerialESP8266.find((char*)"OK")){
    Serial.println("Respuesta AT correcto");
    return true;
  }
    Serial.println("Error en ESP8266");
    return false;

}

bool printgps(float *lat, float *lon ){
  while(SerialGPS.available()){
  int c = SerialGPS.read(); 
  if(gps.encode(c)){
    float latitude, longitude;
    gps.f_get_position(&latitude, &longitude);
Serial.print("Latitud/Longitud: "); 
Serial.print(latitude,5); 
Serial.print(", "); 
Serial.println(longitude,5);
*lat = latitude;
*lon = longitude;
    gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,&hundredths);
Serial.print("Fecha: "); Serial.print(day, DEC); Serial.print("/"); 
Serial.print(month, DEC); Serial.print("/"); Serial.print(year);
Serial.print(" Hora: "); Serial.print(hour, DEC); Serial.print(":"); 
Serial.print(minute, DEC); Serial.print(":"); Serial.print(second, DEC); 
Serial.print("."); Serial.println(hundredths, DEC);
Serial.print("Altitud (metros): "); Serial.println(gps.f_altitude()); 
Serial.print("Rumbo (grados): "); Serial.println(gps.f_course()); 
Serial.print("Velocidad(kmph): "); Serial.println(gps.f_speed_kmph());
Serial.print("Satelites: "); Serial.println(gps.satellites());
Serial.println();
    gps.stats(&chars, &sentences, &failed_checksum);
    return true;
  }
}
return false;
}

void loop() { 
//  if (SerialESP8266.available()) {
//    Serial.write(SerialESP8266.read());
//  }
//  if (Serial.available()) {
//    SerialESP8266.write(Serial.read());
//  }

  float latitude, longitude;
  isReadGPS = printgps(&latitude, &longitude);
  
  if(isReadGPS){
    Serial.println();
    SerialESP8266.begin(9600);
    delay(1000);
    bool isConnect = isConnectWifi();
    if(!isConnect){
      connectWifi(); 
    }
    Serial.print("latitud: "); Serial.println(latitude);
    Serial.print("longitud: "); Serial.println(longitude);
    sendRequest(latitude, longitude);
    SerialGPS.begin(9600);
    delay(4000);
  }
}
void sendRequest(float latitude, float longitude){
  //send tcp conexion
  SerialESP8266.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80,7200");
  if( SerialESP8266.find("OK")){  
    //send header
    String peticionHTTP= "GET /php/ejemplos/ej10destino.php?a=";
    peticionHTTP=peticionHTTP+String(latitude)+"&b="+String(longitude)+" HTTP/1.1\r\n";
    peticionHTTP=peticionHTTP+"Host: www.aprende-web.net\r\n";
    //send bytes sends
    SerialESP8266.print("AT+CIPSEND=");
    SerialESP8266.println(peticionHTTP.length());

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
          if(cadena.length()>1000){
            Serial.println("La respuesta a excedido el tamaño maximo");
                    
            SerialESP8266.println("AT+CIPCLOSE");
            if( SerialESP8266.find("OK")){
              Serial.println("Conexion finalizada");
            }
            fin_respuesta=true;
          }
          //error
          if((millis()-tiempo_inicio)>10000){
            Serial.println("Tiempo de espera agotado");
            SerialESP8266.println("AT+CIPCLOSE");
            if( SerialESP8266.find("OK")){
              Serial.println("Conexion finalizada");
            }
            fin_respuesta=true;
          }
          // is ok
          if(cadena.indexOf("CLOSED")>0){
            Serial.println();
            Serial.println("Cadena recibida correctamente, conexion finalizada");         
            fin_respuesta=true;
          }
        }
      }else{
        Serial.println("No se ha podido enviar HTTP.....");
      }            
    }  //is dont ok >
  }// dont conect to server
}
  
