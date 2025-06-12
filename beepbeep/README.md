# BeepBeep Access Control Hardware

## HW Bringup

### Requirements

- Multimeter
- Computer
- 3.3V USB serial cable (NOT 5V).
- Micropython binary (included here)

### Steps

1. Check for soldering issues. Common issues include:
    - Shorts along the 5-pin IC
    - Check diode voltage across large PS diode. Check that voltage drop matches
    if probed directly vs across inductor and gnd.
    - Poor connection on ESP module pins (caused by gnd pad lifting)
2. Clean
3. Test continuity black on +, red on -
    - Good boards should measure ~123k
4. Add UART header, connect GND, TX, RX to FTDI Cable
5. Connect VIN to power supply and turn supply on
    - Expected current draw: 14 mA
6. hold "FLASH" and press "RESET"
    - Expected current draw: 19 mA
7. Solder remaining headers
8. Reconnect power and UART, program micropython
    - `python -m esptool --chip esp32s3 --port <serial_port> erase_flash`
    - `python -m esptool --chip esp32s3 --port <serial_port> write_flash -z 0 <micropython.bin>`
    - Device should consume ~22mA with micropython
