#include <Arduino.h>

//--- CONEXION CON PUERTO SERIE:
#define Serie_TX D3 // CON RX DEL OTRO DISPOSITIVO
#define Serie_RX D4 // CON TX DEL OTRO DISPOSITIVO
#include <SoftwareSerial.h>
SoftwareSerial NODE_UDP(Serie_RX, Serie_TX); // RX, TX

void Parpadeo(int, int);

// BOTONES
#define Button_1 D6
#define Button_2 D7
#define Button_3 D8

void setup()
{
  //---> Configuro los botones:
  pinMode(Button_1, INPUT_PULLUP);
  pinMode(Button_2, INPUT_PULLUP);
  pinMode(Button_3, INPUT_PULLUP);

  pinMode(D0, OUTPUT);
  digitalWrite(D0,HIGH);

  //---> Iniciamos SERIE:
  Serial.begin(9600);
  Serial.print("\n Iniciando...");

  //---> Inicio fenotipado
  NODE_UDP.begin(9600);
}

void loop()
{

  if (NODE_UDP.available() > 0)
  {
    Serial.print("\nLlegaron datos por serie...");
    String recibido = NODE_UDP.readString();
    Serial.print("\nString leido: " + recibido);
    Parpadeo(10, 10);
  }

  // CUANDO PRESIONO UN BOTON...
  if (digitalRead(Button_1) == LOW)
  {
    Serial.print("\nEnviando 1");
    NODE_UDP.print("<CALIBRAR>");

    Parpadeo(50, 3);

    while (digitalRead(Button_1) == LOW)
      delay(1);
  }

  else if (digitalRead(Button_2) == LOW)
  {
    Serial.print("\nEnviando 2");
    NODE_UDP.print("<CONTINUAR>");

    Parpadeo(50, 3);

    while (digitalRead(Button_2) == LOW)
      delay(1);
  }


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