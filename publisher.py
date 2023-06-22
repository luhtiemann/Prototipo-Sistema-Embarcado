import paho.mqtt.client as mqtt
import time

pote_value = None
reservatorio_value = None

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
    else:
        print("Connection failed with error code", rc)

def temp_pote(client, userdata, message):
    global pote_value
    pote_value = float(message.payload.decode("utf-8"))
    print("pote:", pote_value)

def temp_reservatorio(client, userdata, message):
    global reservatorio_value
    reservatorio_value = float(message.payload.decode("utf-8"))
    print("reservatorio:", reservatorio_value)

mqttBroker = "broker.emqx.io"
mqttPort = 1883

client = mqtt.Client("Coleguinhas")
client.connect(mqttBroker, mqttPort)
client.on_connect = on_connect
client.message_callback_add("coleguinhas/pote", temp_pote)
client.subscribe("coleguinhas/pote")
client.message_callback_add("coleguinhas/reservatorio", temp_reservatorio)
client.subscribe("coleguinhas/reservatorio")

while True:
    if pote_value is not None and reservatorio_value is not None:
        if pote_value < 500 and reservatorio_value > 500:
            print("Condição satisfeita. Publicando mensagem para acionar o servo.")
            client.publish("coleguinhas/servo", 1)
            time.sleep(15)
    client.loop()
