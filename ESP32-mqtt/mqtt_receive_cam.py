import paho.mqtt.client as mqtt
import time
import cv2

HOST = "**********"
PORT = 1883
mytopic = "**********"
skey = [0x1d,0xe2,0x0f,0x8c,0x31,0xed,0x08,0xf8,0xdb,0x55]#自定义的密匙

def client_loop():
    client_id = time.strftime('py%Y%m%d%H%M%S',time.localtime(time.time()))
    client = mqtt.Client(client_id)    # ClientId不能重复，所以使用当前时间
    client.username_pw_set("joey", "123abc")  # 必须设置，否则会返回「Connected with result code 4」
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(HOST, PORT, 60)
    client.subscribe(mytopic)
    client.loop_forever()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))


def on_message(client, userdata, msg):
    print("["+msg.topic+"]")
    print(len(msg.payload))
    data = bytearray(msg.payload)
    i = 0
    j = 0
    while i<len(msg.payload):
        if j>=10:
            j = 0
        data[i] = msg.payload[i] ^ skey[j];
        i=i+20
        j=j+1
    with open('image.jpg', 'wb') as image_file:
        image_file.write(data)
    img = cv2.imread("image.jpg")
    img = cv2.resize(img,(640,480))  #修改图片的尺寸
    cv2.imshow("img",img)
    cv2.waitKey(1)

if __name__ == '__main__':
    client_loop()