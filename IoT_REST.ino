#include <SPI.h>
#include <Ethernet.h> // PARA W5100 DESCOMENTAR Y COMENTAR EL DE ABAJO 
//#include <Ethernet2.h> // PARA W5500 DESCOMENTAR Y COMENTAR EL DE ARRIBA 

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

bool ConectionStarted = false;

int timers[] =   {-1, -1, -1, -1, -1, -1, -1, -1, -1};
int states[] =   {0, 0, 0, 0, 0, 0, 0, 0, 0};

// VALORES PARA IP FIJA
IPAddress ip(10, 0, 3, 100);
IPAddress subnet(255, 255, 0, 0);
IPAddress dnServer(8, 8, 8, 8);
IPAddress gateway(10, 0, 0, 1);

// INICIA SERVER
EthernetServer server(80);

void Connect()
{
  // EN CASO QUE SE REQUIERA IP FIJA DESCOMENTAR Y COMENTAR LA DE ABAJO
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);

  // PARA TOMAR POR DHCP DESCOMENTAR ESTA LINEA Y COMENTAR LA DE ARRIBA
  //Ethernet.begin(mac);

  // AJUSTO EL INTERVALO DE LAS RETRANSMICIONES A 200ms
  Ethernet.setRetransmissionTimeout(200);

  // AJUSTO LOS REINTENTOS DE RETRANSMICION A SOLO 5 O SEA 1 SEGUNDO MAXIMO EN INTENTOS DE CONEXION
  Ethernet.setRetransmissionCount(5);
  
  // INICIA SERVIDOR HTTP
  server.begin();

  if (ConectionStarted == false)
  {
    // INDICA POR SERIAL LA IP OBTENIDA
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
    ConectionStarted = true;
  }
}

void setup() 
{
  // DEFINE GPIO
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  // AJUSTA LOS GPIO A CERO
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);
  digitalWrite(8,HIGH);
  digitalWrite(9,HIGH);

  // INICIA SERIAL
  Serial.begin(9600);
}


void loop() 
{
  // REINICIO LA RED EN CADA LOOP POR SI LA CONEXION SE CAE, SE REINICIE SOLA
  Connect();
  
  EthernetClient client = server.available();
  client.setTimeout(100);
  
  // SI HAY UNA SOLICITUD EJECUTA ACCIONES
  if (client) 
  {
    Serial.println("Conexion entrante");
    
    String cadena="";
    boolean currentLineIsBlank = true;
    int RELE = 0;
    int TIME = 0;
    int STATE = 0;
    int TimeOut = 300;  
    int Ciclos = 0;
       
    while (client.connected()) 
    {
      Ciclos++;
      
      RELE = 0;
      TIME = 0;
      STATE = 0;
          
      if (client.available()) 
      {
        char c = client.read();
        cadena.concat(c);

        if (c == '\n' && currentLineIsBlank)
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
          client.println();
          
          int switchString=cadena.indexOf("switch="); 
          int timeString=cadena.indexOf("time=");
          int stateString=cadena.indexOf("state=");

          if(cadena.substring(switchString, switchString + 8)=="switch=0")
            RELE = 0;
          if(cadena.substring(switchString, switchString + 8)=="switch=1")
            RELE = 1;
          else if(cadena.substring(switchString, switchString + 8)=="switch=2")
            RELE = 2;
          else if(cadena.substring(switchString, switchString + 8)=="switch=3")
            RELE = 3;
          else if(cadena.substring(switchString, switchString + 8)=="switch=4")
            RELE = 4;
          else if(cadena.substring(switchString, switchString + 8)=="switch=5")
            RELE = 5;
          else if(cadena.substring(switchString, switchString + 8)=="switch=6")
            RELE = 6;
          else if(cadena.substring(switchString, switchString + 8)=="switch=7")
            RELE = 7;
          else if(cadena.substring(switchString, switchString + 8)=="switch=8")
            RELE = 8;

          if(cadena.substring(stateString, stateString + 7)=="state=0")
            STATE = 0;
          else if(cadena.substring(stateString, stateString + 7)=="state=1")
            STATE = 1;

          if(cadena.substring(timeString, timeString + 6)=="time=0")
            TIME = 0;
          else if(cadena.substring(timeString, timeString + 6)=="time=1")
            TIME = 1000;
          else if(cadena.substring(timeString, timeString + 6)=="time=2")
            TIME = 2000;
          else if(cadena.substring(timeString, timeString + 6)=="time=3")
            TIME = 3000;
          else if(cadena.substring(timeString, timeString + 6)=="time=4")
            TIME = 4000;
          else if(cadena.substring(timeString, timeString + 6)=="time=5")
            TIME = 5000;
          else if(cadena.substring(timeString, timeString + 6)=="time=6")
            TIME = 6000;
          else if(cadena.substring(timeString, timeString + 6)=="time=7")
            TIME = 7000;
          else if(cadena.substring(timeString, timeString + 6)=="time=8")
            TIME = 8000;
          else if(cadena.substring(timeString, timeString + 6)=="time=9")
            TIME = 9000;

          timers[RELE] = TIME;
          states[RELE] = STATE;

          String ToResponse = "{status:[";
          for (int thisPin = 1; thisPin <= 8; thisPin++) 
          {
            ToResponse = ToResponse + "{\"rele\":\"" + String(thisPin) + "\", \"state\":\"" + String(states[thisPin]) + "\" },";
          }
          ToResponse = ToResponse + "{\"rele\":\"\", \"state\":\"\" }]}";

          Serial.println("Devolviendo estados");
          client.println(ToResponse);
          client.println();
          break;
        }
      }

      if (TimeOut > 0)
        TimeOut = TimeOut - 1;
      else
      {
        Serial.println("Tiempo agotado");
        break;
      }
    }
    client.stop();

    Serial.println("Fin de conexion");

    delay(100);
  }
 
  for (int thisPin = 0; thisPin <= 8; thisPin++) 
  {
    // reduzco el timer siempre
    if (timers[thisPin] >= 0)
      timers[thisPin] = timers[thisPin] - 1;

    // Revierto el estado si el timer llega a 1
    if (timers[thisPin] == 1)
    {
      if (states[thisPin] == 0)
        states[thisPin] = 1;
      else
        states[thisPin] = 0;
    }

    // Activo el pin segun lo planeado
    if (states[thisPin] == 0)
    {
      digitalWrite(thisPin + 1, HIGH);
    }
    else
    {
      digitalWrite(thisPin + 1, LOW);
    } 
  }

  delay(1);
}
