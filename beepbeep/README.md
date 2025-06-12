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
8. Reconnect power and UART (ONLY GND, TX, RX)
9. Enter programming mode:
    - Hold flash button
    - Press and release reset button
10. Program micropython
    - `./flash-upython.sh /dev/ttyUSB0`
    - Device should consume ~22mA with micropython
