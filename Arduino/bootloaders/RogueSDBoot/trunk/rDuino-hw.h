// Hardware definition file for rDuino LEDHead

#define LED_DDR           DDRB
#define LED_PORT          PORTB
#define LED_PIN           PINB
#define LED               PINB0

#define HW_SET_SDBOOT_PIN_PULLUP()  PORTD = (1 << PORTD4)
#define HW_SDBOOT_PIN_IS_HIGH()  (bit_is_set(PIND, PIND4))
#define HW_SDBOOT_PIN_IS_LOW()   (bit_is_clear(PIND, PIND4))
