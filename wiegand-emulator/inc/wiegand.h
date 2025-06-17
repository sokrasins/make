#ifndef WIEGAND_H_
#define WIEGAND_H_

#include <status.h>

/** 
 * @brief Initialize the wiegand emulator
 * @param d0 Pin assigned to the d0 signal
 * @param d1 Pin assigned to the d1 signal
 * @param p_width The width of the wiegand pulse in microseconds
 * @param p_int The pulse interval in microseconds
 **/
status_t wiegand_init(int d0, int d1, int p_width, int p_int);

/** 
 * @brief Set the pulse parameters of the emulator
 * @param p_width Width of pulse in microseconds
 * @param p_int Interval between two pulses
 **/
void wiegand_pulse_set(int p_width, int p_int);

/** 
 * @brief Emulate a wiegand signal
 * @param facility code
 * @param user_id
 * @return STATUS_OK
 **/
status_t wiegand_send(uint8_t facility, uint16_t user_id);

#endif /*WIEGAND_H_*/