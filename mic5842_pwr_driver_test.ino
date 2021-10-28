/**
 * This sketch is used to test the 8-bit serial-in latched power driver IC MIC5842
 * (should also work for MIC5841 as they only differ in maximum voltage ratings).
 *   Copyright (c) 2021, maehw; for details check LICENSE
 */

/* I/O configuration */
#if defined(__AVR_ATmega32U4__) /* working with an Arduino Leonardo here */
  /* SCK and MISO pins are usually used for SPI communication, 
   * but here "bitbanged" as GPIOs.
   */
  #define CLK_PIN              (15) /* "SPI_CLOCK"/SCK */
  #define STROBE_PIN           ( 4) /* "MOTOR_LATCH"/"ML"/STROBE */
  #define SIN_DRIVER_PIN       (16) /* "MOTOR_DATA_IN"/"MDI"/MOSI/serial data in/SIN */
#endif

/* assume nOE (output enable) is tighed to low (i.e. always enabled);
 * but this could also be added as GPIO output pin
 */
/* serial data out ("MOTOR_DATA_OUT"/"MDO") could be monitored to check if the data really has been shifted through */

#define NUM_OUTPUTS            (8)    /* Number of inputs to shift in serially/ parallel outputs; one IC has 8 output pins to drive;
                                       * in order to shift through multiple devices SOUT/SIN can be daisy-chained */
#define CLK_PERIOD_HALF_US     (25)   /* Approximate (minimum) half clock period (low/high duration) */
#define WRITE_DELAY_MS         (1000) /* Delay between consecutive writes ("pause") */
#define STROBE_HIGH_DUR        (1)    /* Duration to keep the strobe signal high */
#define SERIAL_BAUDRATE        (115200)
#define SERIAL_PRINT_WELCOME   (1)

void setup()
{
  /* I/O configuration */
  pinMode(CLK_PIN, OUTPUT);
  pinMode(STROBE_PIN, OUTPUT);
  pinMode(SIN_DRIVER_PIN, OUTPUT); /* "data in" as seen from the IC */

  digitalWrite(CLK_PIN, LOW); /* idle low clock */
  digitalWrite(STROBE_PIN, LOW);
  digitalWrite(SIN_DRIVER_PIN, LOW);

  /* pause before final start */
  delay(2000);

  Serial.begin(SERIAL_BAUDRATE);
#if SERIAL_PRINT_WELCOME
  Serial.println("MIC5842 Power Driver Test");
  Serial.println("-------------------------");
#endif
}

void clock_cylce()
{
  digitalWrite(CLK_PIN, LOW);
  delayMicroseconds(CLK_PERIOD_HALF_US);
  digitalWrite(CLK_PIN, HIGH);
  delayMicroseconds(CLK_PERIOD_HALF_US);
}

void set_output_value(uint32_t value)
{
  for( int k = 0; k < NUM_OUTPUTS; k++ )
  {
    /* set serial data */
    if( value & (1 << k) )
    {
      digitalWrite(SIN_DRIVER_PIN, HIGH);
    }
    else
    {
      digitalWrite(SIN_DRIVER_PIN, LOW);
    }
    clock_cylce(); /* data is shifted in on the rising edge of CLK */
  }
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(SIN_DRIVER_PIN, LOW);

#error Does not seem to work (yet). The strobe signal does not have any effect.
  delayMicroseconds(1);
  digitalWrite(STROBE_PIN, HIGH);
  delayMicroseconds(STROBE_HIGH_DUR);
  digitalWrite(STROBE_PIN, LOW);
  delayMicroseconds(1);
}

void loop()
{
  Serial.println("All 1s pattern");
  set_output_value(0xFFFFFFFF); /* set output, LSB is shifted out first */

  delay(WRITE_DELAY_MS); /* no clock cycling during the given period */

  Serial.println("All 0s pattern");
  set_output_value(0x00000000); /* toggle pattern */

  delay(WRITE_DELAY_MS); /* no clock cycling during the given period */
}
