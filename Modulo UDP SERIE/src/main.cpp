#include <Arduino.h>
#include <avr/pgmspace.h>

String version_str = "1.0";

const char *pagina_IP_html PROGMEM = "<!DOCTYPE html>"
                                     "<html>"
                                     "<head>"
                                     "<title>Configuración de IP y puertos</title>"
                                     "<meta charset='UTF-8'>"
                                     "</head>"
                                     "<body>"
                                     "<h1>IP local</h1>"
                                     "</form>"
                                     "<form action='guardar_ip_local' method='post'>"
                                     "IP LOCAL:"
                                     "<input class='input1' name='local_ip' type='text'><br><br>"
                                     "PUERTO LOCAL:"
                                     "<input class='input1' name='local_port' type='text'><br><br>"
                                     "<input class='boton' type='submit' value='GUARDAR'/><br><br>"
                                     "</form>"
                                     "<br><h1>IP destino</h1>"
                                     "</form>"
                                     "<form action='guardar_ip_remota' method='post'>"
                                     "IP REMOTA:"
                                     "<input class='input1' name='remote_ip' type='text'><br><br>"
                                     "PUERTO REMOTO:"
                                     "<input class='input1' name='remote_port' type='text'><br><br>"
                                     "<input class='boton' type='submit' value='GUARDAR'/><br><br>"
                                     "</form>";

const char *pagina_wifi_html PROGMEM = "<!DOCTYPE html>"
                                       "<html>"
                                       "<head>"
                                       "<title>Configuración WiFi</title>"
                                       "<meta charset='UTF-8'>"
                                       "</head>"
                                       "<body>"
                                       "<h1> - - - Ingresar red - - - </h1> <br>"
                                       "</form>"
                                       "<form action='conectar' method='post'>"
                                       "SSID:"
                                       "<input class='input1' name='ssid' type='text'><br><br>"
                                       "PASSWORD:"
                                       "<input class='input1' name='pass' type='password'><br><br>"
                                       "<input class='boton' type='submit' value='CONECTAR'/><br><br>"
                                       "</form>"
                                       "<a href='/escanear'><button class='boton'>Escanear</button></a><br><br>";

/*
    //PARA GENERAR EL BINARIO:

    Abrir la barra de comandos, Ctrl+t (borrar el #)
    Pegar el siguiente comando:
    .pio\build\nodemcuv2\firmware.bin
    El archivo en binario quedara guardado en:
    "Carpeta del proyecto" ->  .pio\build\nodemcuv2\firmware.bin
    Recordar compilar el programa antes de ejecutar el comando!
*/

// BOTONES
#define Button_1 D5
#define Button_2 D6

//--- CONEXION CON PUERTO SERIE:
#define Serie_TX D3 // CON RX DEL OTRO DISPOSITIVO
#define Serie_RX D4 // CON TX DEL OTRO DISPOSITIVO
#include <SoftwareSerial.h>
SoftwareSerial Fenotipado(Serie_RX, Serie_TX); // RX, TX

/* PANTALLA OLED */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define ANCHO_PANTALLA 128 // Ancho pantalla OLED
#define ALTO_PANTALLA 64   // Alto pantalla OLED
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
void Pantalla_Conectando();
void Pantalla_conectado();
void Pantalla_desconectado(String);
void Limpiar_display();
void Pantalla_Alerta(int);
void Pantalla_error_conexion();
/* FIN DE PANTALLA OLED */

// ID del sensor
const String SensorID = String(ESP.getChipId(), HEX);
int flag = 0;

//---> EEMPROM
#include <EEPROM.h>

//---> WIFI
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer Servidor(80); /*el 80 es habitual, podes usar cualquier otro */

//---> HTTP
#include <WiFiClient.h>        //Para poder iniciar la peticion
#include <ESP8266HTTPClient.h> //Se encarga de las peticiones
const char *link = "http://httpbin.org/get";

//---> Libreria para realizar carga OTA
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater;

int dir_ssid = 0;
int dir_pass = 50;
String Estado_red = "";

//---> Nombres y contraseñas de las redes
#define TAM_SSID 50
#define TAM_PASS 50
char ssid[TAM_SSID];
char pass[TAM_PASS];

const char *passConf = "12345678"; // Contraseña para el modo AP

// Declaracion de funciones:
void Pagina_raiz();
void Pagina_wifi();
void Conectar_wifi();
void Escanear_redes();
void Conectar_nueva_red();
void Funcion_prueba();
void Configurar_servidor();
void Leer_EEPROM();
void Envio_Servidor(String);

void Verificar_conexion();

//---> Pagina HTML:
String Pagina_html = "";
String Pagina_html_fin = "</body>"
                         "</html>";
String mensaje_html = "";

#include <WiFiUDP.h>
// #include <WiFi.h>
WiFiUDP UDP;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; // buffer to hold incoming packet,

boolean ConnectUDP();
void SendUDP_ACK();
void SendUDP_Packet(String);
void Get_UDP(bool);

int time1, time2;
int flag_1;
int flag_udp;
int flag_permitir, flag_serie, flag_servidor, flag_boton_1, flag_boton_2, flag_pant_conect;
int time_wifi_1, time_wifi_2, flag_wifi_ap;
int reconexiones;
int time_pantalla;

int Status = WL_IDLE_STATUS;
int Status_ant = 99;
String status_string = "";

String Lectura_Serie = "";
String Lectura_Servidor = "";

void Send_UDP(IPAddress, unsigned int, String);

void Parpadeo(int, int);
void Pant_Llego_Serv(String);
void Pant_Llego_Serie(String);
void Pantalla_Boton(int);
void Pantallas();
void Leer_Serie_Fenotipado();
void Leer_Pulsadores();

//------Agregado:
String aux_1 = "";
String aux_2 = "";

void Guardar_ip_local();
void Pagina_ipconfig();
void Guardar_ip_remota();
void Leer_IP_EEPROM();
void Mostrar_ip_serie();
void Reset_node();
#define TAM_IP 20

// Variables para los argumentos del WebServer y EEPROM
int dir_localPort = 100;

char localIP_str[TAM_IP];
int dir_localIP = 125;

char remoteIP_str[TAM_IP];
int dir_remoteIP = 150;

int dir_remotePort = 175;

// Variables utilizadas para las funciones

IPAddress IP_local;
unsigned int PORT_local;

IPAddress IP_remote;
unsigned int PORT_remote;

IPAddress gateway(192, 168, 0, 4);
IPAddress subnet(255, 255, 255, 0);

//------------------------------------------ SETUP
void setup()
{
  //---> Iniciamos SERIE:
  Serial.begin(9600);
  Serial.print("\n Iniciando...");

  EEPROM.begin(512);
  Leer_IP_EEPROM();

  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);

  //---> Configuro los botones:
  pinMode(Button_1, INPUT_PULLUP);
  pinMode(Button_2, INPUT_PULLUP);

  Parpadeo(40, 10);

  //---> Inicio fenotipado
  Fenotipado.begin(9600);

  //---> Pantalla OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  //--- Para UDP:
  if (WiFi.config(IP_local, gateway, subnet) == false)
    Serial.println("Configuration failed.");

  //---> Configuracion de WIFI
  Conectar_wifi(); // La funcion EEPROM esta dentro de Conectar_wifi
  Configurar_servidor();

  ConnectUDP();

  // INICIALIZACION DE FLAGS
  flag_1 = 0;
  flag_udp = 0;
  flag_boton_1 = 0;
  flag_boton_2 = 0;
  flag_serie = 0;
  flag_servidor = 0;
  flag_pant_conect = 0;
  flag_permitir = 1;
  time_wifi_1 = millis();
  reconexiones = 0;

  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

//----------------------------- LOOP
void loop()
{
  //------------------------------- VERIFICAR CONEXION:
  Verificar_conexion();

  Servidor.handleClient(); /* Recibe las peticiones */

  //--------------------------------LEER BOTONES:
  Leer_Pulsadores();

  //--------------------------------RECIBO UDP:
  Get_UDP(true);

  //--------------------------------RECIBO SERIE:
  Leer_Serie_Fenotipado();

  //---------------------- Mostrar pantallas
  Pantallas();
}

void Leer_EEPROM()
{

  for (int i = 0; i < 50; i++)
  {
    ssid[i] = 0;
    pass[i] = 0;
  }
  EEPROM.get(dir_ssid, ssid);
  EEPROM.get(dir_pass, pass);
  /*
  Serial.print("\n--------------------------------");
  Serial.print("\n Datos leidos de la EEPROM:");
  Serial.print("\n SSID: " + String(ssid));
  Serial.print("\n PASS: " + String(pass));
  */
}

void Configurar_servidor()
{
  //---> Agregar para carga OTA:
  httpUpdater.setup(&Servidor);

  Servidor.on("/", Pagina_raiz);
  Servidor.on("/wifi", Pagina_wifi);
  Servidor.on("/escanear", Escanear_redes);
  Servidor.on("/conectar", Conectar_nueva_red);
  //--Agregado:
  Servidor.on("/ipconfig", Pagina_ipconfig);
  Servidor.on("/guardar_ip_local", Guardar_ip_local);
  Servidor.on("/guardar_ip_remota", Guardar_ip_remota);
  Servidor.on("/reset", Reset_node);
  //---
  Servidor.begin();
  Serial.println("\n Servidor iniciado");
}

/* Prueba para pasar de char a String */
void Funcion_prueba()
{
  // #define TAM_CADENA 50
  int TAM_CADENA = 50;
  char cadena[TAM_CADENA];

  int TAM_STRING = 0;
  String string_origen = "Hola probando strings! 1233";
  String string_destino = "";

  //---> Ponemos toda la cadena de char en 0
  for (int i = 0; i < TAM_CADENA; i++)
  {
    cadena[i] = 0;
  }

  //---> Mostramos los valores (antes de hacer el pasaje):
  Serial.print("\n String inicio: " + string_origen);
  Serial.print("\n String vacio: " + string_destino);
  Serial.print("\n Cadena: " + String(cadena));

  TAM_STRING = string_origen.length(); // Largo del string

  for (int i = 0; i < TAM_STRING; i++)
  {
    cadena[i] = string_origen[i]; // Copiamos el string a la cadena
  }

  for (int i = 0; i < TAM_CADENA; i++)
  {
    if (cadena[i] != 0)
      string_destino += cadena[i]; // Vamos concatenando en el String cada posicion del char
  }

  //---> Mostramos los resultados
  Serial.print("\n ------------> Luego de convertir:");
  Serial.print("\n String inicio: " + string_origen);
  Serial.print("\n String vacio: " + string_destino);
  Serial.print("\n Cadena: " + String(cadena));
}

void Conectar_nueva_red()
{
  String ssid_aux = "";
  String password_aux = "";
  Serial.print("\n Datos recibidos guardados!");

  ssid_aux = String(Servidor.arg("ssid"));
  password_aux = String(Servidor.arg("pass"));

  for (int i = 0; i < TAM_SSID; i++)
  {
    ssid[i] = ssid_aux[i]; // Copiamos el string a la cadena
  }
  for (int i = 0; i < TAM_PASS; i++)
  {
    pass[i] = password_aux[i]; // Copiamos el string a la cadena
  }

  Serial.print("\n -----------------------------------");
  Serial.print("\n Argumentos recibidos del server");
  Serial.print("\n ssid: " + String(ssid));
  Serial.print("\n pass: " + String(pass));
  Serial.print("\n -----------------------------------");

  EEPROM.put(dir_ssid, ssid);
  EEPROM.commit();
  EEPROM.put(dir_pass, pass);
  EEPROM.commit();

  mensaje_html = "\n Configuración guardada! Se intentará realizar la conexión.";
  Pagina_wifi();
  Conectar_wifi();
  mensaje_html = "";
}

void Pagina_raiz()
{
  Serial.print("\n Enviando Pagina_raiz");
  Pagina_html = "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<title>Dispositivo</title>"
                "<meta charset='UTF-8'>"
                "</head>"
                "<body>"
                "<h1> - - - Configuración del dispositivo - - - </h1> <br>";

  /*
    Pagina_html += "<p>Versión: \"" + version_str + "\"</p>";

    Pagina_html += "<p>Red: \"" + String(ssid) + "\", estado: " + Estado_red + "</p>";
    Pagina_html += "<a href='/'><button class='boton'>Actualizar estado</button></a><br>";

    Pagina_html += "<br><br><p> IP local: " + IP_local.toString() + ", puerto local: " + String(PORT_local) + "</p>";
    Pagina_html += "<p> IP remota: " + IP_remote.toString() + ", puerto remoto: " + String(PORT_remote) + "</p>";
  */

  // BOTON CONFIGURAR WIFI:
  Pagina_html += "<br><br><br><br><a href='/wifi'><button class='boton'>Configurar WiFi</button></a>";
  // BOTON CONFIGURAR IP:
  Pagina_html += "<br><br><a href='/ipconfig'><button class='boton'>Configurar IP</button></a><br>";

  Servidor.send(200, "text/html", Pagina_html + Pagina_html_fin);
}

void Escanear_redes()
{
  Serial.print("\n Enviando Escanear_redes");
  int cant_redes = WiFi.scanNetworks(); // devuelve el número de redes encontradas
  Serial.println("escaneo terminado");
  if (cant_redes == 0) // si no encuentra ninguna red
  {
    Serial.println("no se encontraron redes");
    mensaje_html = "no se encontraron redes";
  }
  else
  {
    Serial.print("\n " + String(cant_redes) + " redes encontradas!");
    mensaje_html = "";
    mensaje_html = "<p> Redes encontradas: </p><br>";
    for (int i = 0; i < cant_redes; ++i)
    {
      // agrega al STRING "mensaje" la información de las redes encontradas
      mensaje_html += "<p>" + String(i + 1) + ": " + WiFi.SSID(i) + " </p>\r\n";
      delay(20);
    }
    Serial.println(mensaje_html);
    Pagina_wifi();
    mensaje_html = "";
  }
}

void Pagina_wifi()
{
  Serial.print("\n Enviando Pagina_wifi");
  String boton_back = "<br><a href='/'>[Back]</a><br>";

  Servidor.send(200, "text/html", pagina_wifi_html + mensaje_html + boton_back + Pagina_html_fin);
}

/* Funcion Conectar wifi */
void Conectar_wifi()
{
  Leer_EEPROM(); // Leemos el SSID y PASS para conectar
  Serial.print("\n\n-----------------------------");
  Serial.print("\n Conectando WIFI...");
  Serial.print("\n SSID: " + String(ssid));
  Serial.print("\n Password: " + String(pass));

  if (flag == 0)
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pass);
    flag = 1;
  }
  else if (flag == 1)
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
  }

  int tiempo_max = 5000;
  int time = 0;
  int time_ant = millis();
  while (WiFi.status() != WL_CONNECTED && (time - time_ant) < tiempo_max)
  {
    delay(1);
    time = millis();
    Pantalla_Conectando();
  }
  Limpiar_display();
  if (WiFi.status() != WL_CONNECTED)
  { // No se pudo realizar la conexion. Iniciamos AP
    Serial.print("\n Fallo la conexion a la red: " + String(ssid));
    Estado_red = "desconectado";
    // Iniciamos el AP:
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Sensor_" + String(SensorID), passConf);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("\n\n Conectarse a la red: Sensor_" + String(SensorID));
    Serial.print("\n Con la contraseña: " + String(passConf));
    Serial.print("\n IP del access point: ");
    Serial.print(myIP);
    Serial.print("\n-----------------------------");
    Pantalla_desconectado("Desconectado");
  }
  else
  { // Se realizo la conexion correctamente
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    Serial.print("\n Se conecto a la red: " + String(ssid) + " correctamente!");
    Serial.print("\n Con la IP: ");
    Serial.print(WiFi.localIP());
    Serial.print("\n-----------------------------\n");
    Estado_red = "conectado";
    Pantalla_conectado();
  }
}
/* Fin de funcion */

/* Funcion pantalla CONECTANDO... */
void Pantalla_Conectando()
{
  const String SensorID = String(ESP.getChipId(), HEX);
  int time = 150;
  int x = 64, y = 45;
  int tam_text = 1; // 1o2
  int indice_y = 15;
  int indice_y2 = 0;

  String aux = "Conectando...";
  String aux2 = "- " + String(ssid) + " -";
  int tam = aux.length();
  int indice_x = 64 - (6 * (int(tam / 2)));
  tam = aux2.length();
  int indice_x2 = 64 - (6 * (int(tam / 2)));

  // Serial.print("\n i:" + String(i));

  display.clearDisplay();
  display.drawCircleHelper(x, y, 10, 1, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(tam_text);
  display.setCursor(indice_x, indice_y);
  display.println(aux);
  display.setCursor(indice_x2, indice_y2);
  display.println(aux2);
  display.display();
  delay(time);

  display.clearDisplay();
  display.drawCircleHelper(x, y, 10, 2, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(tam_text);
  display.setCursor(indice_x, indice_y);
  display.println(aux);
  display.setCursor(indice_x2, indice_y2);
  display.println(aux2);
  display.display();
  delay(time);

  display.clearDisplay();
  display.drawCircleHelper(x, y, 10, 4, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(tam_text);
  display.setCursor(indice_x, indice_y);
  display.println(aux);
  display.setCursor(indice_x2, indice_y2);
  display.println(aux2);
  display.display();
  delay(time);

  display.clearDisplay();
  display.drawCircleHelper(x, y, 10, 8, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(tam_text);
  display.setCursor(indice_x, indice_y);
  display.println(aux);
  display.setCursor(indice_x2, indice_y2);
  display.println(aux2);
  display.display();
  delay(time);
}
/* Fin de funcion PANTALLA CONECTANDO */

/* Funcion PANTALLA Conexion OK*/
void Pantalla_conectado()
{
  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Version: ");
  display.println(version_str);

  display.setCursor(0, 30);
  display.print("IP: ");
  display.println(WiFi.localIP().toString());

  display.setCursor(0, 15);
  display.println("SSID:");
  display.setCursor(35, 15);
  display.println(ssid);

  String aux = "Conectado";
  int tam = aux.length();
  int indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 50);
  display.print(aux);

  display.display();
}
/* Fin de funcion PANTALLA Conectado OK */

/* Funcion PANTALLA Desconectada */
void Pantalla_desconectado(String estado)
{

  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print(estado);

  display.setCursor(0, 20);
  display.print("Conectar con la red: ");
  display.setCursor(0, 30);
  display.println("Sensor_" + String(SensorID));

  /*
    display.setCursor(0, 20);
    display.println("SensorID:");
    display.setCursor(55, 20);
    display.println(SensorID);
  */
  display.setCursor(0, 50);
  display.print("IP:");
  display.println(WiFi.softAPIP().toString());

  /*
    String aux = "Desconectado";
    int tam = aux.length();
    int indice_x = 64 - (6 * (int(tam / 2)));
    display.setCursor(indice_x, 50);
    display.print(aux);
  */
  display.display();
}
/* Fin de funcion PANTALLA Desconectado */

/* Funcion limpiar display */
void Limpiar_display()
{
  display.clearDisplay();
  display.display();
}
/* Fin de funcion limpar display */

/* Funcion error conexion pantalla */
void Pantalla_error_conexion()
{
  display.clearDisplay();
  display.display();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  String aux = "Error en la red:";
  int tam = aux.length();
  int indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 20);
  display.print(aux);

  aux = ssid;
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 40);
  display.print(aux);

  // Serial.print("\n Status wifi: " + String(WiFi.status()));

  display.display();
}
/* fin de funcion */

//----------------- FUNCIONES PARA UDP

boolean ConnectUDP()
{

  Serial.println();
  Serial.println("Starting UDP");

  // in UDP error, block execution
  if (UDP.begin(PORT_local) != 1)
  {
    Serial.println("Connection failed");
    while (true)
    {
      delay(1000);
    }
  }

  Serial.println("UDP successful");
  return false;
}

void Get_UDP(bool sendACK = true)
{
  int packetSize = UDP.parsePacket();
  // if (packetSize)
  if (UDP.available() > 0)
  {
    // read the packet into packetBufffer
    UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

    Serial.println();
    Serial.print("[" + String(packetSize) + "]: ");
    Serial.print(UDP.remoteIP());
    Serial.print(" : ");
    Serial.print(UDP.remotePort());
    Serial.print(" : ");
    Serial.write((uint8_t *)packetBuffer, (size_t)packetSize);

    Lectura_Servidor = "";
    for (int h = 0; h < packetSize; h++)
    {
      Lectura_Servidor += packetBuffer[h];
    }

    flag_servidor = 1;

    Serial.print("\nEnviando lo recibido a Fenotipado: " + Lectura_Servidor);
    Fenotipado.write((uint8_t *)packetBuffer, (size_t)packetSize);
  }
}

void Send_UDP(IPAddress ip_f, unsigned int port_f, String str_f)
{

  // Mostramos en pantalla por serie
  Serial.print("\nSe enviara: \"" + str_f + "\" a la IP: ");
  Serial.print(ip_f);
  Serial.print(" y puerto: " + String(port_f));

  // Enviamos por UDP
  UDP.beginPacket(ip_f, port_f);
  UDP.write(str_f.c_str()); //.c_str() para convertic a const char*
  UDP.endPacket();

  // Parpadeo(50, 2);
}

void Pant_Llego_Serv(String str)
{

  display.clearDisplay();
  display.display();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);

  String aux = "SERVIDOR";
  int tam = aux.length();
  int indice_x = 64 - (12 * (int(tam / 2)));
  display.setCursor(indice_x, 0);
  display.print(aux);

  display.setTextSize(1);
  aux = "envia:";
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 20);
  display.print(aux);

  aux = str;
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  if (indice_x < 0)
    indice_x = 0;
  display.setCursor(indice_x, 35);
  display.print(aux);

  flag_udp = 1;
  time1 = millis();

  display.display();
}

void Pantalla_Enviar()
{

  display.clearDisplay();
  display.display();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);

  String aux = "SERIE";
  int tam = aux.length();
  int indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 0);
  display.print(aux);

  display.setTextSize(1);
  aux = "Llego dato desde";
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 30);
  display.print(aux);

  aux = "el serie";
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 35);
  display.print(aux);

  flag_udp = 1;
  time1 = millis();

  display.display();
}

void Pantalla_Boton(int num)
{

  display.clearDisplay();
  display.display();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);

  String aux = "BOTON ";
  aux += String(num);
  int tam = aux.length();
  int indice_x = 64 - (12 * (int(tam / 2)));
  display.setCursor(indice_x, 0);
  display.print(aux);

  display.setTextSize(1);
  aux = "Enviando al";
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 30);
  display.print(aux);

  aux = "servidor";
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 45);
  display.print(aux);

  flag_udp = 1;
  time1 = millis();

  display.display();
}

void Parpadeo(int time1, int cant)
{

  for (int x = 0; x < cant; x++)
  {
    digitalWrite(D0, LOW);
    delay(time1);
    digitalWrite(D0, HIGH);
    delay(time1);
  }
}

void Pant_Llego_Serie(String str)
{

  display.clearDisplay();
  display.display();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);

  String aux = "SERIE";
  int tam = aux.length();
  int indice_x = 64 - (12 * (int(tam / 2)));
  display.setCursor(indice_x, 0);
  display.print(aux);

  display.setTextSize(1);
  aux = "envia:";
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 20);
  display.print(aux);

  aux = str;
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  if (indice_x < 0)
    indice_x = 0;
  display.setCursor(indice_x, 35);
  display.print(aux);

  flag_udp = 1;
  time1 = millis();

  display.display();
}

void Pantallas()
{
  //--------------------------------MOSTRANDO EN PANTALLA:
  time2 = millis();
  if (flag_permitir == 1)
  {
    if (flag_servidor == 1) // SERVIDOR
    {
      Pant_Llego_Serv(Lectura_Servidor);
      flag_servidor = 0;
      flag_permitir = 0;
      flag_pant_conect = 1;
      time_pantalla = 3000;
    }
    else if (flag_serie == 1) // SERIE
    {
      Pant_Llego_Serie(Lectura_Serie);
      flag_serie = 0;
      flag_permitir = 0;
      flag_pant_conect = 1;
      time_pantalla = 3000;
    }
    else if (flag_boton_1 == 1) // BOTON 1
    {
      Pantalla_Boton(1);
      flag_boton_1 = 0;
      flag_permitir = 0;
      flag_pant_conect = 1;
      time_pantalla = 1000;
    }
    else if (flag_boton_2 == 1) // BOTON 2
    {
      Pantalla_Boton(2);
      flag_boton_2 = 0;
      flag_permitir = 0;
      flag_pant_conect = 1;
      time_pantalla = 1000;
    }

    else if (flag_pant_conect == 1)
    {
      Pantalla_conectado();
      flag_pant_conect = 0;
    }
    time1 = millis();
  }

  if ((time2 - time1) > time_pantalla)
  {
    flag_permitir = 1;
    time1 = time2;
  }
}

void Leer_Serie_Fenotipado()
{
  if (Fenotipado.available() > 0)
  {
    Serial.print("\nLlegaron datos desde Fenotipado");

    //--- Llego un dato serie, y lo mando por UDP:
    Lectura_Serie = "";
    Lectura_Serie = Fenotipado.readString();

    flag_serie = 1;

    // IPAddress IPdestino(192, 168, 0, 109);
    // unsigned int destinationPort = 2000;

    Send_UDP(IP_remote, PORT_remote, Lectura_Serie);
  }
}

void Leer_Pulsadores()
{
  //--------------------------------BOTON 1:
  if (digitalRead(Button_1) == LOW)
  {

    String msg = "<BOTON 1>";
    Send_UDP(IP_remote, PORT_remote, msg);

    flag_boton_1 = 1;

    while (digitalRead(Button_1) == LOW)
      delay(1);
  }
  //--------------------------------BOTON 2:
  else if (digitalRead(Button_2) == LOW)
  {
    String msg = "<BOTON 2>";
    Send_UDP(IP_remote, PORT_remote, msg);

    flag_boton_2 = 1;

    while (digitalRead(Button_2) == LOW)
      delay(1);
  }
}

void Verificar_conexion()
{

  Status = WiFi.status();

  if (Status != Status_ant)
  {
    Status_ant = Status;
    Serial.println("****************************");
    if (Status == WL_IDLE_STATUS)
      status_string = "Estado IDLE"; // 0
    else if (Status == WL_NO_SSID_AVAIL)
      status_string = "No encuentra el SSID"; // 1
    else if (Status == WL_SCAN_COMPLETED)
      status_string = "Escaneo completado"; // 2
    else if (Status == WL_CONNECTED)
      status_string = "Conectado"; // 3
    else if (Status == WL_CONNECT_FAILED)
      status_string = "Error al conectar"; // 4
    else if (Status == WL_CONNECTION_LOST)
      status_string = "Conexion perdida"; // 5
    else if (Status == WL_WRONG_PASSWORD)
      status_string = "Contraseña erronea"; // 6
    else if (Status == WL_DISCONNECTED)
      status_string = "Desconectado"; // 7
    else
      status_string = String(Status);
    Serial.println(status_string);

    if (Status == WL_CONNECTED)
    {
      Pantalla_conectado();
      Send_UDP(IP_remote, PORT_remote, "< Conectado >");
      Estado_red = "conectado";
    }
    else
    {
      Pantalla_desconectado(status_string);
      Estado_red = "desconectado";
    }
  }
}

//-------------------------------------------
//---------------AGREGADO:
//-------------------------------------------

void Pagina_ipconfig()
{
  Serial.print("\n Enviando Pagina_ipcongif");
  String boton_back = "<br><a href='/'><button class='boton'>Back</button></a><br><br>";

  String Boton_reset = "<br><br><a href='/reset'><button class='boton'>Reset</button></a><br>";

  Servidor.send(200, "text/html", pagina_IP_html + mensaje_html +  Boton_reset + boton_back + Pagina_html_fin);
}

void Guardar_ip_local()
{

  aux_1 = String(Servidor.arg("local_ip"));
  Serial.print("\nRecibo ip String: ");
  Serial.print(aux_1);

  aux_2 = String(Servidor.arg("local_port"));
  Serial.print("\nRecibo puerto String: ");
  Serial.print(aux_2);

  for (int i = 0; i < TAM_IP; i++)
  {
    localIP_str[i] = aux_1[i]; // Copiamos el string a la cadena
  }

  EEPROM.put(dir_localIP, localIP_str);
  EEPROM.commit();

  PORT_local = aux_2.toInt();
  EEPROM.put(dir_localPort, PORT_local);
  EEPROM.commit();

  Leer_IP_EEPROM();

  mensaje_html = "\n IP y puerto local guardado!";
  Pagina_ipconfig();
  mensaje_html = "";
}

void Guardar_ip_remota()
{

  aux_1 = String(Servidor.arg("remote_ip"));
  Serial.print("\nRecibo ip String: ");
  Serial.print(aux_1);

  aux_2 = String(Servidor.arg("remote_port"));
  Serial.print("\nRecibo puerto String: ");
  Serial.print(aux_2);

  for (int i = 0; i < TAM_IP; i++)
  {
    remoteIP_str[i] = aux_1[i]; // Copiamos el string a la cadena
  }

  EEPROM.put(dir_remoteIP, remoteIP_str);
  EEPROM.commit();

  PORT_remote = aux_2.toInt();
  EEPROM.put(dir_remotePort, PORT_remote);
  EEPROM.commit();

  Leer_IP_EEPROM();

  mensaje_html = "\n IP y puerto remoto guardado!";
  Pagina_ipconfig();
  mensaje_html = "";
}

void Mostrar_ip_serie()
{
  Serial.print("\n--------------------------------");
  Serial.print("\nIPs y Puertos (tipo IPAddress):");
  Serial.print("\n LOCAL IP: ");
  Serial.print(IP_local);
  Serial.print("\n LOCAL PORT: " + String(PORT_local));
  Serial.print("\n REMOTE IP: ");
  Serial.print(IP_remote);
  Serial.print("\n REMOTE PORT: " + String(PORT_remote));
  Serial.print("\nWiFi status: " + String(WiFi.status()));
  Serial.print("\n--------------------------------");
}

void Leer_IP_EEPROM()
{
  for (int i = 0; i < TAM_IP; i++)
  {
    localIP_str[i] = 0;
    remoteIP_str[i] = 0;
  }

  EEPROM.get(dir_localIP, localIP_str);
  EEPROM.get(dir_localPort, PORT_local);
  EEPROM.get(dir_remoteIP, remoteIP_str);
  EEPROM.get(dir_remotePort, PORT_remote);

  //------ Pasamos a las variables tipo IPAdress:
  IP_local.fromString(localIP_str);

  IP_remote.fromString(remoteIP_str);

  Mostrar_ip_serie();
}

void Reset_node()
{
  Serial.print("\n Se reseteara la placa...");

  Pagina_html = "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<title>Dispositivo</title>"
                "<meta charset='UTF-8'>"
                "</head>"
                "<body>"
                "<p> Reseteando modulo... </p>";

  Servidor.send(200, "text/html", Pagina_html + Pagina_html_fin);
  delay(200);
  ESP.reset();
}