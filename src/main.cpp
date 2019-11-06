#include "TinyWire.h"
#include <avr/io.h>

const uint8_t t85_addr = 0x16;
const uint8_t freq = 0x7F;

struct fan_desc {
  uint8_t raw_dtc = freq;
  uint8_t duty_cycle = (raw_dtc * 100) / freq;
  uint8_t raw_tacht = 0;
  uint8_t rpm = 0;
} fan;

struct sensor_temperature_desc {
  uint8_t raw_read = 0;
  uint8_t temperature = 0;
} temp1, temp2;

void PWM_Init(void) {
  // Set Pin6/PB1 to output
  DDRB = 1 << DDB1;

  // 159, which shall result in 25kHz
  OCR1C = 0x9F;
  OCR1A = 0x7F;

  TCCR1 = 1 << CTC1 |   // clear on match with OCR1C
          1 << PWM1A |  // set output PB1
          1 << COM1A1 | // set output PB1
          2 << CS10;    // set prescaling to clk/16
}

void updateDutyCycle(void) {
  fan.raw_dtc = (fan.duty_cycle * freq) / 100;
  OCR1A = fan.raw_dtc;
}

void onI2CReceive(int howMany) {
  uint8_t target_dtc = 100;
  while (TinyWire.available() > 0) {
    target_dtc = TinyWire.read();
    fan.duty_cycle = target_dtc;
    updateDutyCycle();
  }
}

void onI2CRequest() {
  char buf[6];
  uint8_t i = 0;

  buf[0] = fan.rpm;
  buf[1] = ':';
  buf[2] = temp1.temperature;
  buf[3] = ':';
  buf[4] = temp2.temperature;
  buf[5] = '\0';

  for (i = 0; i < 6; i++) {
    TinyWire.send(buf[i]);
  }
}

int main(int argc, char **argv) {
  PWM_Init();

  TinyWire.begin(t85_addr);
  // sets callback for the event of a slave receive
  TinyWire.onReceive(onI2CReceive);
  TinyWire.onRequest(onI2CRequest);

  while (1)
    ;

  return 0;
}
