#include <SPI.h>
#include <Ethernet.h>
#include <TinkerKit.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char serverName[] = "iotsummit.azure-mobile.net";
int serverPort = 80;
char pageName[] = "/api/sendtemp";
char DeviceID[] = "Makati";
char params[32] = "";

EthernetClient client;
int totalCount = 0;
unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

TKThermistor therm(I0); 
TKLightSensor ldr(I1);

//30 Minutes = 1800000 Milliseconds
//30 Seconds = 30000 Milliseconds
#define delayMillis 10000UL

void setup() {
  Serial.begin(9600);

  // disable SD SPI
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  Serial.print(F("Starting ethernet..."));
  if(!Ethernet.begin(mac)) Serial.println(F("failed")); else Serial.println(Ethernet.localIP());

  delay(2000);
  Serial.println(F("Ready"));  
}

void loop()
{
  thisMillis = millis();
  
  if(thisMillis - lastMillis > delayMillis)
  {
    float TempC = therm.readCelsius();     	
    float TempF = therm.readFahrenheit();  
    int LDR = ldr.read();   
    
    totalCount++; 
    lastMillis = thisMillis;
    if(!postPage(serverName,serverPort,pageName,params,TempC,LDR,DeviceID)) Serial.print(F("Fail ")); else Serial.print(F("Pass "));   
    Serial.println(totalCount,DEC);
  }    
}

byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData,double Tempdata, int LDRdata, char* DeviceIDdata)
{
  int inChar;
  char outBuf[64];
  char dest[100];
  char Tempconvert[10];  
  char LDRconvert[5];

  Serial.println(F("-------------------------------------------------"));  
  Serial.print(F("connecting..."));

  if(client.connect(domainBuffer,thisPort))
  {
    Serial.println(F("connected"));  
    
    dtostrf(Tempdata, 0, 2, Tempconvert); 
    sprintf(LDRconvert,"%d", LDRdata); 
   
    strcpy(dest, page );
    strcat(dest, "?temp=");
    strcat(dest, Tempconvert);    
    strcat(dest, "&location=");  
    strcat(dest, DeviceIDdata);
    strcat(dest, "&ldr=");     
    strcat(dest, LDRconvert);
    
    Serial.println(dest);
    
    sprintf(outBuf,"POST %s HTTP/1.1",dest); 
    client.println(outBuf);    
    
    sprintf(outBuf,"Host: %s",domainBuffer); 
    client.println(outBuf);    
    
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(thisData));
    client.println(outBuf);

    client.print(thisData);
  }
  else
  {
    Serial.println(F("failed"));
    return 0;
  }

  int connectLoop = 0;

  while(client.connected())
  {
    while(client.available())
    {
      inChar = client.read();
      connectLoop = 0;
    }

    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      Serial.println(F("Timeout"));
      client.stop();
    }
  }

  Serial.println(F("disconnecting...."));
  client.stop();
  return 1;
}
