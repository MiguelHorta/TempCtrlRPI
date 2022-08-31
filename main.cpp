#include "TinyWire.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "rpi_temp.h"


// Current pinout
// 1 | Reset (maybe tach)
// 2 | PB3 - Resistor 1
// 3 | PB4 - Resistor 2
// 4 | GND
// 5 | SDA - I2C
// 6 | PB6 - PWM 5V fan
// 7 | SCL - I2C
// 8 | VCC 

const static uint8_t t85_addr = 0x16;
static uint8_t last_selected_reg = 0;
/*extern */volatile uint16_t tach_count = 0;

struct fan_desc {
  // 159, which shall result in 25kHz
  const static uint8_t freq = 0x9F;
  uint8_t raw_dtc = freq;
  uint8_t duty_cycle = (raw_dtc * 100) / freq;
  uint16_t raw_rpm = 0x32;
  uint16_t rpm = ~0;
  target_mode_t target_mode = DUTY_CYCLE_MODE;
} fan;

struct fan_pid {
  const uint16_t step = 25000;
  volatile uint16_t count = 0;
  const uint8_t kp = 4;
  const uint8_t ki = 5;
  uint16_t acc_err;
} pid;

struct sys_fan_desc {
  const uint8_t RES0_VAL = 10; // *100
  const uint8_t RES1_VAL = 15; // *100
  const uint8_t RES2_VAL = 27; // *100
  sys_fan_level_t level = FAN_HIGHEST;
} sys_fan;

void setSysFanLevel(sys_fan_level_t lvl) {
  switch (lvl) {
  case FAN_IDLE:
    PORTB &= ~(1 << PB3 | 1 << PB4);
    break;
  case FAN_MEDIUM:
    PORTB |= (1 << PB4);
    PORTB &= ~(1 << PB3);
    break;
  case FAN_HIGH:
    PORTB |= (1 << PB3);
    PORTB &= ~(1 << PB4);
    break;
  case FAN_HIGHEST:
    PORTB |= (1 << PB3) | (1 << PB4);
    break;
  default:
    break;
  }
}

void PWM_Init(void) {
  // Set Pin6/PB1 to output
  DDRB = 1 << DDB1;

  OCR1C = fan.freq;
  OCR1A = fan.raw_dtc;

  TCCR1 |= 1 << CTC1 |   // clear on match with OCR1C
          1 << PWM1A |  // set output PB1
          1 << COM1A1 | // set output PB1
          2 << CS10;    // set prescaling to clk/16
  TIMSK |= 1 << TOIE1;
}

uint8_t convertDTCtoRaw(uint8_t freq, uint8_t dtc) {
  return ((dtc * freq) / 100);
}
void setFanDTC(uint8_t dtc) {
    fan.raw_dtc = convertDTCtoRaw(fan.freq, fan.duty_cycle);
    OCR1A = fan.raw_dtc;
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
    fan.target_mode = RPM_MODE;
    fan.rpm |= (write << 8);
    write = TinyWire.read();
    fan.rpm |= (write & 0xFF);
    break;
  case DUTY_CYCLE:
    fan.target_mode = DUTY_CYCLE_MODE;
    fan.duty_cycle = write;
    setFanDTC(fan.duty_cycle);
    break;
  case SYS_FAN_LEVEL:
    sys_fan.level = (sys_fan_level_t)write;
    setSysFanLevel(sys_fan.level);
    break;
  default:
    break;
  }
}

void onI2CRequest(void) {
  switch (last_selected_reg) {
  case RPM:
    TinyWire.send((fan.rpm >> 8) & 0xFF);
    TinyWire.send(fan.rpm & 0xFF);
    break;
  case DUTY_CYCLE:
    TinyWire.send(fan.duty_cycle);
    break;
  case CONTROL_MODE:
    TinyWire.send(fan.target_mode);
    break;
  case SYS_FAN_LEVEL:
    TinyWire.send((uint8_t)sys_fan.level);
    break;
  case RAW_RPM:
    TinyWire.send((fan.raw_rpm >> 8) & 0xFF);
    TinyWire.send(fan.raw_rpm & 0xFF);
    break;
  default:
    break;
  }
  last_selected_reg++;
}

void updatePID(void) {
  uint16_t err = fan.rpm - fan.raw_rpm;
  pid.acc_err += err;

  fan.duty_cycle = (err >> pid.kp) + (pid.acc_err >> pid.ki);
  if (fan.duty_cycle > 100) {
    fan.duty_cycle = 100;
  }
  setFanDTC(fan.duty_cycle);
}

int main(int argc, char **argv) {
  cli();

  PWM_Init();
  /* Enable tach interrupt 
  GIMSK |= (1 << PCIE);
  PCMSK |= (1 << PCINT5);
  */

  DDRB |= (1<<DDB3)|(1<<DDB4);
  PORTB |= (1<<PB3)|(1<<PB4);
  setSysFanLevel(FAN_HIGHEST);;

  TinyWire.begin(t85_addr);
  // sets callback for the event of a slave receive
  TinyWire.onReceive(onI2CReceive);
  TinyWire.onRequest(onI2CRequest);
  sei();

  while (1) {
    // if (pid.count >= pid.step) {
    //   fan.raw_rpm = (tach_count >> 1);
    //   if (fan.target_mode == RPM_MODE) {
    //       updatePID();
    //   }
    //   tach_count = 0;
    //   pid.count = 0;
    // }
  }

  return 0;
}

ISR( TIM1_OVF_vect  ) {
    pid.count++;
}
