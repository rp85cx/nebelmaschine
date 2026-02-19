# alte doku der nebelmaschine mit arduino uno

Dokumentation für die reparierte, neu "verdrahtete" Nebelmaschine Stairville M-Fog 1500

Source Code "fogm_buttoncntrl.ino" für Arduino Uno

**Arduino Pinouts zu:**
- 9: ready-led
- 3: status-led 2
- 5: status-led 1
- 10: Max6675 CS
- 12: Max6675 SO
- 13: Max6675 SCK
- 8: Heat Relay
- 7: Pump Relay
- A1: Control Button
- 4: TM1637 CLK (dev)
- 2: TM1637 DIO (dev)

**Status LED (1: Orange/Gelb, 2: Grün):**
- Power: 1 dunkel
- Heat: 1 hell
- Ready: 2 leuchtet
- Output: 2 blinkt


- Eingebautes Control board noch unfunktional, außerdem Pumpe nur an/aus 
- Externer Stecker pinouts (GX16, 4-Pin):
	- 1: 5 (sled 1, 5v!) kabel: gelb
	- 2: 3 (sled 2, 5v!) kabel: grün
	- 3: a1 (trigger to gnd) kabel: braun/orange
	- 4: gnd kabel: weiß
- Sicherung: F8AL250V 5 * 20mm
- Temps:
	- heizen bis 230°
	- ab 240° keine möglichkeit mehr drüber zu gehen
	- pumpe geht ab 210°
- 2x 10a relais für Pumpe/Heater  
