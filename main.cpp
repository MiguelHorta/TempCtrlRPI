#include "TinyWire.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "rpi_temp.h"

const uint8_t t85_addr = 0x16;
// 159, which shall result in 25kHz
const uint8_t freq = 0x9F;
static uint8_t last_selected_reg = 0;

struct fan_desc {
  uint8_t raw_dtc = freq;
  uint8_t duty_cycle = (raw_dtc * 100) / freq;
  uint8_t raw_tach = ~0;
  uint8_t rpm = ~0;
  target_mode_t target_mode = DUTY_CYCLE_MODE;
} fan;

struct sensor_temperature_desc {
  uint8_t raw_read = ~0;
  uint8_t temperature = ~0;
} temp1, temp2;

void PWM_Init(void) {
  // Set Pin6/PB1 to output
  DDRB = 1 << DDB1;

  OCR1C = freq;
  OCR1A = fan.raw_dtc;

  TCCR1 = 1 << CTC1 |   // clear on match with OCR1C
          1 << PWM1A |  // set output PB1
          1 << COM1A1 | // set output PB1
          2 << CS10;    // set prescaling to clk/16
}

uint8_t convertDTCtoRaw(uint8_t freq, uint8_t dtc) {
  return ((dtc * freq) / 100);
}

void onI2CReceive(int howMany) {
    uint8_t reg, write;

    reg = TinyWire.read();

    if (howMany == 1) {
        last_selected_reg = reg;
        return;
    }

    write = TinyWire.read();
    /* Else handle writes */
    switch (reg) {
    case RPM:
      /* not implemented */
      fan.target_mode = RPM_MODE;
      fan.rpm = write;
      break;
    case DUTY_CYCLE:
      fan.target_mode = DUTY_CYCLE_MODE;
      fan.duty_cycle = write;
      fan.raw_dtc = convertDTCtoRaw(freq, fan.duty_cycle);
      break;
    default:
      break;
    }
}

void onI2CRequest(void) {
  switch (last_selected_reg) {
  case RPM:
    TinyWire.send(fan.rpm);
    break;
  case DUTY_CYCLE:
    TinyWire.send(fan.duty_cycle);
    break;
  case CONTROL_MODE:
    TinyWire.send(fan.target_mode);
    break;
  case TEMP_1:
    TinyWire.send(temp1.temperature);
    break;
  case TEMP_2:
    TinyWire.send(temp2.temperature);
    break;
  case RAW_RPM:
    TinyWire.send(fan.raw_tach >> 1);
    break;
  case RAW_TEMP_1:
    TinyWire.send(temp1.raw_read);
    break;
  case RAW_TEMP_2:
    TinyWire.send(temp2.raw_read);
    break;
  default:
    break;
  }
}

int main(int argc, char **argv) {
  cli();

  PWM_Init();
  TinyWire.begin(t85_addr);
  // sets callback for the event of a slave receive
  TinyWire.onReceive(onI2CReceive);
  TinyWire.onRequest(onI2CRequest);

  sei();

  while (1);

  return 0;
}
