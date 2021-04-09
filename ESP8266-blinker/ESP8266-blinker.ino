#define BLINKER_PRINT Serial
#define BLINKER_MIOT_LIGHT
#define BLINKER_WIFI

#include <Blinker.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include "DHT.h"

typedef unsigned char uint8;
typedef unsigned short uint16;

char auth[] = "********";
#define STASSID "********"
#define STAPSK  "********"
#define PIN 15
#define LEDNUM 40
#define DHTTYPE DHT11
#define DHTPIN 13
DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel pixels(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);
BlinkerSlider SliderH("barh");
BlinkerSlider SliderS("bars");
BlinkerSlider SliderV("barv");
BlinkerButton Button1("bt1");
BlinkerButton Button2("bt2");
BlinkerButton Button3("bt3");
BlinkerButton Button4("bt4");
BlinkerNumber Temp("temp");
BlinkerNumber Hum("hum");

float temp_read = 20;
float hum_read = 30;
uint8 circle = 0;
uint16 barh = 0;
uint8  bars = 0;
uint8  barv = 0;

void setup() {
	//UDP
	Serial.begin(115200);
	//dht11
	dht.begin();
	//ws2812
	pixels.begin();
	pixels.show();

	Blinker.begin(auth, STASSID, STAPSK);//WiFi.SSID().c_str(), WiFi.psk().c_str());
	//BLINKER_DEBUG.stream(BLINKER_PRINT); 
	SliderH.attach(sliderH_callback);
	SliderS.attach(sliderS_callback);
	SliderV.attach(sliderV_callback);
	Button1.attach(button1_callback);
	Button2.attach(button2_callback);
	Button3.attach(button3_callback);
	Button4.attach(button4_callback);
	Blinker.attachHeartbeat(heartbeat);
	readTH();
	Blinker.delay(100);
}

void loop() {
	static uint8 cnt = 0;
	cnt++;
	Blinker.run();
	if(circle != 0)
		setRainbowCircle(LEDNUM-1, circle);
	if(cnt == 10)
		readTH();
	Blinker.delay(50);
}

void readTH()
{
	float h = 0, t = 0;
	h = dht.readHumidity();
	t = dht.readTemperature();
	if (isnan(h) || isnan(t)) {
		temp_read = 0;
		hum_read = 0;
		return;
	}
	temp_read = t * 0.5 + temp_read * 0.5;
	hum_read = h * 0.5 + hum_read * 0.5;
}

void heartbeat()
{
	Temp.print(temp_read);
	Hum.print(hum_read);
}

void setSingleColor(uint16 h, uint8 s, uint8 v)
{
	pixels.fill(pixels.ColorHSV(h,s,v), 0, LEDNUM);
	pixels.show();
}

void setRainbowColor(uint8 colornum)
{
	for(int i = 0; i < LEDNUM; i++)
	{
		pixels.setPixelColor(i, pixels.ColorHSV( ((i<<16)/colornum)&0xffff, 255, 255 ) );
	}
	pixels.show();
}

void setRainbowCircle(uint8 colornum, uint8 spd)
{
	static uint16 offset = 0;
	for(uint8 i = 0; i < LEDNUM; i++)
	{
		pixels.setPixelColor(i, pixels.ColorHSV( ((i<<16)/colornum+offset)&0xffff, 255, 255 ) );
	}
	offset += spd * 8;
	pixels.show();
}

void setRedBlue()
{
	for(uint8 i = 0; i < LEDNUM; i++)
	{
		pixels.setPixelColor(i, pixels.ColorHSV( (int)(i*2.5+135)<<8, 255, 255 ) );
	}
	pixels.show();
}

void sliderH_callback(int32_t value)
{
	circle = 0;
	barh = value;
	pixels.fill(pixels.ColorHSV(barh, bars, barv), 0, LEDNUM);
	pixels.show();
	Button1.print("on");
}

void sliderS_callback(int32_t value)
{
	circle = 0;
	bars = value;
	pixels.fill(pixels.ColorHSV(barh, bars, barv), 0, LEDNUM);
	pixels.show();
	Button1.print("on");
}

void sliderV_callback(int32_t value)
{
	circle = 0;
	barv = value;
	pixels.fill(pixels.ColorHSV(barh, bars, barv), 0, LEDNUM);
	pixels.show();
	Button1.print("on");
}

void button1_callback(const String & state)
{
	circle = 0;
	pixels.clear();
	pixels.show();
	Button1.print("off");
}

void button2_callback(const String & state)
{
	readTH();
	heartbeat();
}

void button3_callback(const String & state)
{
	static int8 mode = -1;
	mode++;
	if (mode > 2)
		mode = 0;
	circle = 0;
	if (mode == 0)
		setRainbowColor(LEDNUM-1);
	else if (mode == 1)
		circle = 100;
	else if (mode == 2)
		circle = 200;
	Button1.print("on");
}

void button4_callback(const String & state)
{
	circle = 0;
	setRedBlue();
	Button1.print("on");
}
