# V2 der geupgradeden Nebelmaschine
### dual core - warum nicht?

>[!NOTE]
> Disclaimer: der Code wurde basically komplett ohne KI geschrieben, wenn nur kleines problem solving deswegen bitte nicht wundern wenn der unstrukturiert oder unklar wie sonst noch was ist :)

### How to use:

Encoder unter dem Display drehen, klick für Action bzw in Menu gehen, langes Drücken um aus jeder Page zurück ins Menu zu kommen

Einzelne Pages sind wahrscheinlich selbsterklärend, Einstellungen werden auch nach Aus-Anmachen gespeichert. DMX kann auf der Page "DMX Adresse" und "DMX Debug" gedebuggt werden, bei erstem welchen Wert er empfängt, bei letzterem wie viele DMX Packets pro Sekunde ankommen.




Falls jemand je das Ding aufmachen und Troubleshooten muss, ist hier das Layout wie die Stecker in die Platine kommen

> [!WARNING]
> **NIEMALS** die Nebelmaschine aufmachen während sie an ist, und falls man sie offen Testen muss **AUF KEINEN FALL** irgendwas im inneren anfassen. Dabei liegen **230V Netzspannung** an, die zum schlimmsten führen können!

![Layout of the PCB with notes for where to plug in what](/images/pcb_layout.png)


### Hardware:
- ESP32 Wroom
- Max485 für DMX 
- Max6675 für Temperatur via Thermocouple
- Rotary Encoder für main Eingabe an einem
- Noname 128x64 OLED Display
- paar LEDs, Wiederstände uä

### Credits:
- [Upir](https://www.youtube.com/@upir_upir)
- [Randomnerdtutorials](https://randomnerdtutorials.com)
