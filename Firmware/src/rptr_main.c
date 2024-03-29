/*
 * Standard DV-RPTR Firmware
 * -------------------------
 * This project is firmware of the AVR32 DV-RPTR, developed by J. Alte.
 * It processes various converting modes for digitized speech signals.
 * See www.digisolutions.de (german only) for more information about DV-Modem.
 *
 * Copyright (c) 2011 by Jan Alte, DO1FJN
 *
 *
 * rptr_main.c (main source file of DVfirmware)
 *
 *  Created on: 2011-07-26 (developed from DV-Modem)
 *      Author: Jan Alte, DO1FJN
 *
 *
 * This firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * All source files of DVfirmware are written in UTF-8 charset.
 *
 * Report:
 * 2011-07-26 V0.01  Erstes Firmware-Release
 * 2011-07-28 V0.02  Empfang -> D0-Pakete funktioniert nun. TX nicht getestet.
 * 2011-07-29 V0.03  Reception & Transmit uses buffers with 1-SYNC-Size (21 frames).
 * 		     rename all repeater-function calls from "dstar_xxx()" to "rptr_xxx()"
 * 2011-08-19 V0.04  RXSYNC received from PC results in TX (last header used)
 * 		     No CRC needed on USB (control with bool var 'rx_crc_disabled'
 *
 * 2011-08-30 V0.05  first version for DV-RPTR Target, w/o setup. cmd VERSION works
 * 2011-09-01 V0.06  large transmit buffer, new - I hope final - CMD set
 * 2011-09-02 V0.07  Checksum enable-function
 * 2011-09-03 V0.08  MODFDIS bit @ dac_init(), Test-Loop (0x1F cmd)
 *                   bugfix handle_pcdata() length check
 * 2011-09-05 V0.09  rptr_addtxvoice() enumeration logic bugfix
 * 2011-09-06 V0.10  gmsk: Additional Interrupt-Handler keeps critical Timer-based ADC-start, DAC-out
 * 		     with a minimum of jitter in the case of duplex operation
 * 2011-09-18 V0.12  various tests with demodulator.
 * 2011-09-22 V0.20  idle_timer feature, prevent TX, if not enabled. RX is swichable to.
 * 		     disabling RX+TX if USB cable a disconnected
 * 		     if TX switched off, DAC and REF-DAC turned to power-down
 * 2011-09-24 V0.30  add some bytes in header and voice-frame packets for future use.
 * 		     special-function commands RESET + FORCE-BOOTLOADER added
 * 2011-09-25 V0.30a Fixing header / voicedata bug (new offset)
 * 2011-09-29 V0.30b Fix a bug in usb_func() copy function
 * 2011-10-06 V0.31  using timeout of 5ms for incoming data (USB), flush incomplete pkts
 * 2011-10-07 V0.31a STA_CANDUPLEX_MASK > Bit6 in ConfigFlags indicates duplex mode possible
 * 2011-10-08 V0.40  add TX Status (enum see rptr_func.h) in GETSTATUS message
 * 		     new RSSI per frame feature returns a unsigned(16) value (4.85V = max = 1023).
 * 2011-10-11 V0.40a fixing memory overflow on weak signals, if a sync-pattern detected
 * 2011-10-19 V0.41  transmitting logic changed: see rptr_func.c
 * 2011-10-19 V0.42  Multi-Voice-Frame support
 * 2011-10-22 V0.43  EOT(TX) feature stops immediately, if buffer is empty (don't match buffer-pos)
 * 		     feature TESTLOOP removed, start TX with START message, wait until HEADER message
 * 2011-10-30 V0.44  conditional PATTERN checking, gmsk_func module now independent of D-Star related
 * 2011-11-05 V0.50  RC1; watchdog-feature, fixed to 30s, PREAMBLE-Detection-Logic
 * 2011-11-06 V1.00  same as RC1, changed standard config values (0.75Vss TXmod / 120ms TX-Delay
 * 2011-11-13 V1.10  first version with a simple TRX control
 * 2011-11-17 V1.10a a few changes allows Cfg-Applying with transmitting
 * 2011-12-28 V1.10b long-roger-beep bug fixed
 * 2012-01-05 V1.10c alternative #define SIMPLIFIED_FIFO to use a non-sorting Fifo of 420ms
 * 2012-01-11 V1.10d BugFix Pattern-Handling if PLL not correct an a to early match appears
 * 2012-01-22 V1.10e BugFix in controls.c module (dead carrier tx)
 * 2012-01-26 V1.20  THE LAST TRY: pic21mode.h, new reduced incompatible!!! interface
 *                   Replacement-Header, when VOICE w/o HEADER
 * 2012-01-30 V1.20a Only a simple Fifo logic, ignoring frame numberation completely
 * 2012-02-01 V1.11  TX-Buffer will be cleared before TX starts completely.
 *                   On longer gaps, a FRAME-SYNC is added every 21 silence-packet
 *                   A additional byte "transmit position" will be appear in STATUS pkt
 *                   A repacement-header will be created when tx starts with SYNC.
 * 2012-02-02 V1.11a BugFix: Icom-Voice sorting removed
 * 2012-04-05 V1.50  First protoype ambe-addon support (decode only)
 * 2012-04-15 V1.50b non permanent ambe-addon with configuration of dongle-mode
 * 2012-04-24 V1.60  AMBE analog configurations
 * 2012-04-25 V1.61  EEProm Config Routines
 * 2012-05-16 V1.61a EEProm Config Save Function
 * 2012-05-18 V1.62  Simple Buffer-based Save2EEprom
 * 2012-05-22 V1.63  Autocorrection RX (Replace RPT1/2 by repeater ACK burst)
 * 2012-05-24	     new TINYpacket for async answers
 * 2012-06-18 V1.63c Dongle-Mode: Own-Header transmitted later to PC
 * 2012-07-02 V1.65  Dual-Port-Communication: PCP2 works on serial Port (115k2) too.
 *                   See 'stdmodem.c' for details.
 * 2012-07-03 V1.66  New Features "RX auto inverse" and "half-duplex rx"
 * 2012-07-21 V1.66e DeadLock on SLEEP() on dgl_init() fixed (release-version only)
 * 2012-07-21 V1.67  C0-Config Flagbit 6 = PTTLOCKED (no external PTT_OUT)
 * 2012-07-24 V1.68  EOT-Command locks Stop-Position for current stream
 * 2012-08-01 V1.69  TWI blocking functions, used in controls, tlv320aic and rda1846
 * 2012-08-08 V1.69b Dongle PTT debounced, additional PTT-blocking feature, BugFixes
 * 2012-05-10 V1.70  Testing procedures for AMBE / MH-59 microphone
 *
 * ToDo:
 * - Serial (RS232) port configurable
 * + configurable Half-Duplex Mode (switch off RX-Timer while TX against feedbacks)
 * - 2nd ACK/NAK message on "Save2EEprom" job is done
 *   need a dynamic job polling (eeprom_status)
 * - USB not working after reset cmd, maybe a USB-unplug msg is needed
 * + no new features for release V1.00
 * + first SYNC detect -> Message (early switch on Transceiver)
 * + PC watchdog
 * + RSSI / SQL sampling while receiving
 * + Transmitter-State enumeration for GET_STATUS
 * + force-bootloader command
 * + enable / disable receiver (if disabled keep firmware alive by a idle-counter)
 * + enable / disable transmitter (logic only, return NAK on START / SYNC cmd if off)
 * + bugfix transmit-logic
 * + configuration: TXDelay, Modulation-Vss ...
 */


#include "defines.h"		// general defines
#include "hw_defs.h"		// LEDs

#include "controls.h"
#include "gpio_func.h"		// GPIO-functions and macros (PTT, LEDs...)
#include "int_func.h"		// idle_timer()
#include "twi_func.h"

#include "dac_func.h"
#include "usb_func.h"
#include "rs232_func.h"
#include "rptr_func.h"		// realtime-handler part of HF I/O

#include "transceiver.h"
#include "dongle.h"

#include "storage.h"
#include "config.h"
#include "stdmodem.h"

#include "intc.h"
#include "compiler.h"

#include "mh59_com.h"		// Test 1-2-3


U32		trx_capabilities;
U32		dgl_capabilities;

/* Neue Logik ToDo
 * AMBE-Modul vorhanden: MH-59 Mikrofon init und Test -> rs232 wird abgeschaltet, wenn keine Reaktion innerhalb des Startfensters (normale Up/Dn Tasten)
 * (autosense MH-59; states: MH59_UNDEFINED, MH59_PRES, MH59_ABSENT)
 * kein AMBE-Modul vorhanden: rs232 mit 115k2 initialisieren, PCP2 auf Serial freischalten, PacketZiel festlegen (USB?)
 * USB-Statusänderung: PacketZiel umschalten (USB fertig enumeriert -> USB, USB abgesteckt -> Serial)
 * new fct: int cdc_isactive(void)
 */


void startup_modeinit(void) {
  // 1. have a AMBE addon?
  if (dgl_capabilities!=0) {

    mh59_init();			// using USART for MH-59 serial link...

    // check for C0 + C2 config, init RX/TX if AMBE addon configured
    if (is_config_loaded(0xC0) && is_config_loaded(0xC2)) {
      if (RPTR_is_set(RPTR_AMBEDECODEHF) || dgl_can_encode_by_ptt()) {
	// HF decode to hear it? Special con: Enable AMBE-RX even PTT is enabled
	trx_receive();			// set receiving
	rptr_receive();			// enable receiving
	RPTR_set(RPTR_AMBEDECODEHF);	// set AMBE-Decoding
	status_control |= STA_RXENABLE_MASK;
      } // fi from HA active
      if (dgl_can_encode_by_ptt()) {
	dac_power_ctrl(true);
	set_dac_power_mode(TWI_DAC_POWERUP);
	status_control |= STA_TXENABLE_MASK;
      } // fi microphone PTT
    } // fi config
  } else {	// fi have AMBE

    rs232_init(115200, no_flow);	// Enable serial port (2nd PCP2 in the moment)
    // setup 2nd PCP2 Link (swiching to, if no USB connection)

  }
  // init RPTR flags (clear all non permanent):
  RPTR_clear(RPTR_INDICATOR_MASK);
}



#ifdef TC_A1_OUT
#define TIMER_CMR_TEST_VALUE	( AVR32_TC_NONE << AVR32_TC_BSWTRG_OFFSET | \
    AVR32_TC_NONE << AVR32_TC_BEEVT_OFFSET | \
    AVR32_TC_NONE << AVR32_TC_BCPC_OFFSET | \
    AVR32_TC_NONE << AVR32_TC_BCPB_OFFSET | \
    AVR32_TC_NONE << AVR32_TC_ASWTRG_OFFSET | \
    AVR32_TC_NONE << AVR32_TC_AEEVT_OFFSET | \
    2 << AVR32_TC_ACPC_OFFSET | \
    1 << AVR32_TC_ACPA_OFFSET | \
    1 << AVR32_TC_WAVE_OFFSET | \
    AVR32_TC_WAVSEL_UP_AUTO << AVR32_TC_WAVSEL_OFFSET | \
    FALSE << AVR32_TC_ENETRG_OFFSET | \
    AVR32_TC_EEVT_TIOB_INPUT << AVR32_TC_EEVT_OFFSET | \
    AVR32_TC_EEVTEDG_NO_EDGE << AVR32_TC_EEVTEDG_OFFSET | \
    FALSE << AVR32_TC_CPCDIS_OFFSET | \
    FALSE << AVR32_TC_CPCSTOP_OFFSET | \
    AVR32_TC_BURST_NOT_GATED << AVR32_TC_BURST_OFFSET | \
    0 << AVR32_TC_CLKI_OFFSET | \
    AVR32_TC_TCCLKS_TIMER_CLOCK2 << AVR32_TC_TCCLKS_OFFSET )
#endif





int main(void) {
  Disable_global_interrupt();		// Disable all interrupts.

  // *** Initialization-Section ***
  init_hardware();

  // the follow code can be used for a crystal measurement test:
#ifdef TC_A1_OUT
  // Programming a 15MHz Signal on PA21 (Pin45 µC / Pin15 I/O connector)
  AVR32_TC.channel[1].cmr = TIMER_CMR_TEST_VALUE;
  AVR32_TC.channel[1].rc  = 2;
  AVR32_TC.channel[1].ra  = 1;
  AVR32_TC.channel[1].ccr = AVR32_TC_SWTRG_MASK | AVR32_TC_CLKEN_MASK;
  while(1) {
    SLEEP();				// do nothing, until some IRQ wake the CPU
    watchdog();				// do external watchdog for STM706T
  }
#endif

  INTC_init_interrupts();		// Initialize interrupt vectors.
  rptr_init_hardware();
  usb_init();				// Enable VBUS-Check

  // *** initializing constant parts of I/O structures ***

  rptr_init_data();			// Default Header "noCall"
  init_modemdata();			// Initialisation of persitent Pkts (Header, Voice)

  Enable_global_interrupt();		// Enable all interrupts.
  idle_timer_start();			// keep uC alive to handle external WD & SLEEP() resume

  // *** initializing ADDon boards (needs enabled interrupts + idle timer)
  trx_capabilities = trx_init();	// init optional TRX module
  dgl_capabilities = dgl_init();	// Test for a AMBE addon board and initialize it...

  cfg_apply_c0();			// setup default config (if none from eeprom)

  // (start idle_timer before "startup_modeinit()")

  // *** loading permanent stored config data ***
  if (load_internal_eeprom()) {
    load_configs_from_eeprom();
    while (twi_busy()) {		// wait until all I²C devices are well-configured
      SLEEP();
    } // ehliw
    startup_modeinit();			// setup operation mode on power-up
  } // fi

  // no other activity (USB)

  LEDs_Off();

  while (TRUE) {			// *** Main-Loop ***

    SLEEP();				// do nothing, until some IRQ wake the CPU

    watchdog();				// do external watchdog for STM706T

    handle_hfdata();			// look, if something to do with the HF interface

    usb_handler();			// handle USB-based reception and enumeration

    handle_pcdata();			// got a message / pkt from pc/gateway? handle it.

    if (RPTR_CONFIGURED) {		// only, when configured (C0 setup from EE/PC)
      dgl_function();			// do DONGLE function, if AMBE board present
    }

    //mh59_handle();

  } // ehliw MAINloop
}

// MAIN Ende

