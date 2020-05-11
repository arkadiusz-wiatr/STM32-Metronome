# STM32-Metronome
Metronome on STM32 using LCD 2x16 display, diode and DAC conventer.

# Description

The goal of the project was create metronome using STM32 with DAC conventer.

The project has the following implemented functionalities:
- Display current BPM on LCD 2x16 HD44780
- Communication with computer using the USART interface with the circular buffer handling interrupts
- Flashing LED with each metronome hit

# Communication protocol frame

1. Unit of information is byte.
2. Start byte – 0xAA.
3. End byte – 0xAA.
4. If the character 0xAA is present in the payload, it is changed to 0xC8 0xC9.
5. If the character 0xC8 is present in the payload, it is changed to 0xC8 0xC7.
6. The next to last byte is the checksum.

check sum = (sum of payload + sender address + recipient address) % 256

7. Transmission speed: 9600 bits per second.
8. Sender address – 0xDC.
9. Recipient address – 0xDD.
10. Maximum frame length– 55 bytes.
11. Minimum frame length – 2 bytes.
12. Maximum payload length – 50 bytes.
