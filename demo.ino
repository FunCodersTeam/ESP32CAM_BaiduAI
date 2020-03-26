#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFiClientSecure.h>
#include <base64.h>

#define SSID              "......"
#define PASSWORD          "......"
#define ACCESS_TOKEN      "......"
#define Json_begin        "{\"image\":\""
#define Json_end          "\",\"image_type\":\"BASE64\",\"group_id_list\":\"......\"}"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

static void Baidu_AI(camera_fb_t* &fb){
    WiFiClientSecure client;
    if(client.connect("aip.baidubce.com",443)){
        Serial.println("Connection succeeded");
        client.println("POST /rest/2.0/face/v3/search?access_token=" + String(ACCESS_TOKEN) + " HTTP/1.1");
        client.println(F("Host: aip.baidubce.com"));
        client.println("Content-Length: " + String(base64_enc_len(fb->len) + len(Json_begin Json_end)));
        client.println(F("Content-Type: application/json"));
        client.println();
        client.print(F(Json_begin));
        for (int i = 0; i < fb->len; i += 3) //分段发送
        client.print(base64::encode((fb->buf + i),3));
        client.print(F(Json_end));
        Serial.println("Waiting for response...");
        uint8_t i = 0;
        while (!client.available()){
            i += 1;
            delay(100);
            if(i > 200){ //timeout
                Serial.println("No response...");
                break;
            }
        }
        while (client.available()){
            Serial.print(char(client.read()));  
        }
        client.stop();
        Serial.println();
        }else Serial.println("Connection failed");
}

//计算base64编码后的长度
__attribute__((always_inline)) int base64_enc_len(int len) {
  return (len + 2 - ((len + 2) % 3)) / 3 * 4;
}

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG,0); //关闭欠压检测
    Serial.begin(115200);
    WiFi.begin(SSID,PASSWORD);
    while (WiFi.status() != WL_CONNECTED){
        Serial.println("Waitting for wifi connection...");
        delay(500);
    }
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
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
    while(esp_camera_init(&config) != ESP_OK){
        Serial.println("Waitting for camera init...");
        delay(500);
    }
}

void loop()
{
    while(WiFi.status() != WL_CONNECTED){ //断线重连
        WiFi.reconnect();
        delay(500);
    }
    camera_fb_t *fb = esp_camera_fb_get();
    if(!fb)return;
    Serial.println("Got image");
    Baidu_AI(fb);
    esp_camera_fb_return(fb);
}