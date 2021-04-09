#include "esp_camera.h"
#include <WiFi.h>
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include <PubSubClient.h>
#define LED_BUILTIN 4

const char* ssid = "**********";
const char* password = "**********";
const char* mqtt_server = "**********";
const char* username = "**********";
const char* userpasswd = "**********";
const char* subtopic = "**********";
const char* pubtopic = "**********";
int timedelay = 1000;
unsigned char ledflag;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
//#define MSG_BUFFER_SIZE  (10000)
//char msg[MSG_BUFFER_SIZE];
int value = 0;
unsigned char skey[10] = {0x1d,0xe2,0x0f,0x8c,0x31,0xed,0x08,0xf8,0xdb,0x55};

//void startCameraServer();

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if(payload[0] == '1')
    ledflag = 1;
  else if(payload[0] == '0')
    ledflag = 0;
  if(payload[0] == '3')
    timedelay = (payload[1]-'0') * 1000 + (payload[2]-'0') * 100;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32cammnt";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), username, userpasswd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("mytopic", "hello world");
      // ... and resubscribe
      client.subscribe(subtopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 14;
  config.fb_count = 1;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
  s->set_brightness(s, 1);
  s->set_saturation(s, 1);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.setBufferSize(10000);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(ledflag)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1);
  }
  camera_fb_t *fb = esp_camera_fb_get();    //  抓拍图片
  digitalWrite(LED_BUILTIN, 0);
  if ( fb ) {
    Serial.printf("width: %d, height: %d, buf: 0x%x, len: %d\n", fb->width, fb->height, fb->buf, fb->len);
    for(int i=0, j=0; i < fb->len; i+=20, j++)
    {
      if(j>=10) j=0;
      *((fb->buf)+i) ^= skey[j];
    }
    client.publish(pubtopic, fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }
  client.loop();
  delay(timedelay);  // [ms]
}
