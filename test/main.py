import ulogging
import time, utime
from machine import reset
import hardware
import utils
import gc

ulogging.basicConfig(level=ulogging.DEBUG)
logger = ulogging.getLogger("main")

hardware.buzzer_off()
hardware.rgb_led_set(hardware.RGB_OFF)
hardware.led_off()

# setup RFID
# setup wiegand reader
import uwiegand

rfid_reader = uwiegand.Wiegand(
    6,
    7,
    uid_32bit_mode=True,
    timer_id=0
)


logger.info("Starting main loop...")
hardware.led_on()
time.sleep(0.5)
hardware.led_off()
hardware.rgb_led_set(hardware.RGB_BLUE)


#print_device_standby_message()

door_previous_state = hardware.get_door_sensor_state()
in_1_previous_state = hardware.get_in_1_state()

hardware.out_1_on()

logger.info("Polling for cards")

while True:
    hardware.feedWDT()
    if card := rfid_reader.read_card():
        card = str(card)
        logger.info(f"Got a card: {card}")

        last_card_id = card
        card = None

