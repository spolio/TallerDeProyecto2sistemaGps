#include <SoftwareSerial.h>


SoftwareSerial SerialESP8266(10, 11); // RX, TX

 const  String ssid     = "SANTINO";         // The SSID (name) of the Wi-Fi network you want to connect to
 const  String password = "2214263696";     // The password of the Wi-Fi network

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

void loop() { 
  if (SerialESP8266.available()) {
    Serial.write(SerialESP8266.read());
  }
  if (Serial.available()) {
    SerialESP8266.write(Serial.read());
  }

//  bool isConnect = isConnectWifi();
//  if(!isConnect){
//    connectWifi(); 
//  }
//  delay(5000);

}
