#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <Arduino.h>

constexpr adc1_channel_t VBAT_CHANNEL = ADC1_CHANNEL_1; // GPIO2
constexpr uint8_t AGND_PIN = 5;
constexpr uint8_t LED_PIN = 21;
constexpr uint8_t LED_LEVEL = LOW;

static int16_t vbatGet(adc1_channel_t channel) {
  constexpr uint8_t MEDIAN = 3;

  esp_adc_cal_characteristics_t adc_chars;
  uint32_t measures[MEDIAN];

  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, (adc_bits_width_t)ADC_WIDTH_BIT_DEFAULT, 0, &adc_chars);
  adc1_config_width((adc_bits_width_t)ADC_WIDTH_BIT_DEFAULT);
  adc1_config_channel_atten(channel, ADC_ATTEN_DB_11);
  for (uint8_t i = 0; i < MEDIAN; ++i) {
    if (esp_adc_cal_get_voltage((adc_channel_t)channel, &adc_chars, &measures[i]) != ESP_OK)
      return -1;
  }
  for (uint8_t i = 0; i < MEDIAN - 1; ++i) {
    for (uint8_t j = i + 1; j < MEDIAN; ++j) {
      if (measures[j] < measures[i]) {
        uint32_t t = measures[i];

        measures[i] = measures[j];
        measures[j] = t;
      }
    }
  }
  return measures[MEDIAN / 2];
}

void setup() {
  Serial.begin(115200);

  pinMode(AGND_PIN, OUTPUT);
  digitalWrite(AGND_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_LEVEL);
/*
#if ARDUINO_USB_CDC_ON_BOOT
  while (! Serial)
    delay(5000);
#endif
*/
  {
    int16_t mv = vbatGet(VBAT_CHANNEL);

    if (mv < 0)
      Serial.println("ADC read error!");
    else
      Serial.printf("VBAT: %0.3f V\n", mv / 500.0);
  }
  Serial.flush();
  esp_deep_sleep_disable_rom_logging();
  esp_sleep_enable_timer_wakeup(10000000); // 10 sec.
  esp_deep_sleep_start();
}

void loop() {}
