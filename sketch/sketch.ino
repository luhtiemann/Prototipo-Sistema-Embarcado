#include <WiFi.h>
#include <PubSubClient.h>
#include <HX711.h>
#include <ESP32Servo.h>

#define DOUT1 23
#define CLK1 22

#define DOUT2 5
#define CLK2 18

#define SERVO_PIN 21

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_TOPIC_SENSOR1 "coleguinhas/pote"
#define MQTT_TOPIC_SENSOR2 "coleguinhas/reservatorio"
#define MQTT_TOPIC_SERVO "coleguinhas/servo"

HX711 scale1;
HX711 scale2;
Servo servo;

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
    delay(10);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println("WiFi connected");
}
int pos = 0;

void callback(char* topic, byte* payload, unsigned int length) {
    String msg;

    // Obtem a string do payload recebido
    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c;
    }
  
    if (strcmp(topic, "coleguinhas/servo") == 0) {
        if (msg.equals("1")) {
            Serial.println("Servo ligado");
            servo.write(180);
            delay(5000);
            servo.write(90);
        }
    }
}


void reconnect() {
  //A função mqttClient.connected() verifica se existe uma conexão ativa.  Depende do Broker, a conexão pode se manter ativa, ou desativar a cada envio de msg.  
  while (!client.connected()) {
      //Se entrou aqui, é porque não está conectado. Então será feito uma tentativa de conexão infinita, até ser conectado.  
      Serial.println("Conectando ao Broker MQTT");
      //define o nome da ESP na conexão. Está sendo gerado um nome aleatório, para evitar ter duas ESPs com o mesmo nome. Neste caso, uma derrubaria a outra.  
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      //Realiza a conexão com a função “client.connect”. Caso seja um sucesso, entra no if e imprime “Conectado ao Broker MQTT.” 
      if (client.connect(clientId.c_str())) {
        Serial.println("Conectado ao Broker MQTT.");
      }
  }
  client.subscribe("coleguinhas/servo");
}

void setup() {
    Serial.begin(9600);
    servo.attach(SERVO_PIN, 500, 2400);
    scale1.begin(DOUT1, CLK1);
    scale2.begin(DOUT2, CLK2);
    setup_wifi();
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    float weight1 = scale1.get_units()*2.38095238095;
    float weight2 = scale2.get_units()*2.38095238095;

    Serial.print("pote: ");
    Serial.println(weight1);
    Serial.print("reservatorio: ");
    Serial.println(weight2);


    client.publish(MQTT_TOPIC_SENSOR1, String(weight1).c_str());
    client.publish(MQTT_TOPIC_SENSOR2, String(weight2).c_str());

    delay(5000);
}
