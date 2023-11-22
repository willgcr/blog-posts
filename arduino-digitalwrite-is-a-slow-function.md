# [Arduino] DigitalWrite is a slow function (why?)!

I wrote this text a few (quite a few) years ago for the Cyberia forum (an old school ethical hacking community)... I think it's somehow important so I decided to bring it back to life, have it reviewed and updated!

Here we go!

![A woman pointing to a computer with the legend "This is so slow"](https://i.giphy.com/media/bMdZu3fG2ZEBO/giphy.webp)

If you have an Arduino, or at least you've seen some Arduino code out there, you've certainly seen the `digitalWrite` function, right? It is the fundamental building block for a `Hello World` on the Arduino platform. `digitalWrite` is also one of the most important functions in the Arduino programming language. It basically outputs a logical value to a pin (by logical value you can understand a voltage), setting the desired pin to `HIGH = 1 (+5V)` or `LOW = 0 (0V)`.

If you have programmed an Arduino before, you probably never faced an issue with `digitalWrite`, I haven't. But as I'm very curious, I couldn't avoid analyzing it in more details and digging deeper on how it works.

Let's start with a simple code, we're gonna measure the time it takes to blink an LED 100 times:

```arduino
// simple-time-test.ino

void setup () {
	// Initializes the serial connection
	Serial.begin (9600);
	// Set pin 13 as output
	pinMode (13, OUTPUT);
	// Evaluates the time to blink the LED connected to the pin 13 (100x)
	int startTime = micros ();
	for (int i = 0; i < 100; i++) {
		digitalWrite (13, HIGH);
		digitalWrite (13, LOW);
	}
	int endTime = micros ();
	// Returns the result through the serial connection
	Serial.print ("Time: ");
	Serial.println (endTime - startTime);
}

void loop () {
}
```

The `micros()` function returns the time in microseconds (μs) that the Arduino is **ON** since the last **reset**. It's not difficult to understand what this code is doing.

The result I got on my test: `1060 μs`!

It's not that bad, right? Idk, let's compare to another method of doing such a thing... This time we're going to manipulate the Arduino's microcontroller pins directly. But how? Well, now things get a little bit low level, the Arduino's microcontroller is the "brain" of this tiny board, It is for the Arduino the equivalent of the CPU (processor) for your computer.

Let's take a deeper look in the Arduino architecture:

![Arduino Pinout Diagram](https://images.prismic.io/circuito/8e3a980f0f964cc539b4cbbba2654bb660db6f52_arduino-uno-pinout-diagram.png?auto=compress,format)

The image above shows the Arduino Uno pinout (which is the board I'm actually using here). This board features an [ATMega328P microcontroller](https://content.arduino.cc/assets/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf), this is an AVR &copy; 8-bit microcontroller... And what most matters to me is that there are three types of registers that can control GPIO ports on this architecture: `DDRx`, `PORTx` and `PINx`, where `x` is the port identifier (there are 3 registers of each one of these types).

- `DDRx`: Data Direction Register, configures the pins as either inputs or outputs.
- `PORTx`: Output port register. Sets the output value on pins configured as outputs. Enables or disables the pull-up resistor on pins configured as inputs.
- `PINx`: Input register, used to read an input signal. On some devices, this register can be used for pin toggling: writing a logic one to a PINx bit toggles the corresponding bit in PORTx, irrespective of the setting of the DDRx bit.

This is an 8-bit microcontroller, so each of these registers have 8 bits, these bits are used to control the pins they're linked to. Take a look back at the pinout diagram and see that each pin is linked to a port:

- `PORTB(PB)`: Digital Pins 8 to 13
- `PORTC(PC)`: Analog Pins A0 to A5
- `PORTD(PD)`: Digital Pins 0 to 7

I need to modify the output on the pin 13, so I'll need to manipulate the `PORTB`, right? Yep! The good thing is that each port can be directly accessed in our code, referencing its name as if it is a normal variable: `PORTB`, `PORTC`, `PORTD`.

As I explained, each of these registers stores 8 bits of information. Let's suppose that the bit 3 on `PORTD` is set to `HIGH (1)`, it means that the corresponding pin is going to have its output set to **+5V**. In this case, the bit 3 of `PORTD` is the digital pin **3**. How to know which pin corresponds to a specific bit? Easy: read the docs&nbsp;😜

To change the state of a pin, we just need to modify the value of the corresponding port register directly. If we want to set bit **3** to **HIGH** on `PORTD`, we do the following:

```arduino
PORTD = B00001000;
```

Well, not that easy! Doing this way I would be overwriting the data on the other bits, and as other pins are controlled by theses bits, I need to make sure I'm not changing them. To do so, I must use some boolean logic here (hopefully you did your homework):

```arduino
PORTD = PORTD | B00001000; // Pin 3 = HIGH (boolean OR)
```

To set the pin to **LOW**, I can use the boolean operator **AND**:

```arduino
PORTD = PORTD & B11110111; // Pin 3 = LOW (boolean AND)
```

There is also the short version:

```arduino
PORTD |= B00001000; // Pin 3 = HIGH (boolean OR)
PORTD &= B11110111; // Pin 3 = LOW (boolean AND)
```

Ok... It's time to blink that LED again, the digital pin 13 is controlled by the bit **5** on `PORTB`:

```arduino
// ports-time-test.ino

void setup () {
	// Initializes the serial connection
	Serial.begin (9600);
	// Set pin 13 as output
	pinMode (13, OUTPUT);
	// Evaluates the time to blink the LED connected to the pin 13 (100x)
	int startTime = micros ();
	for (int i = 0; i < 100; i++) {
		PORTB |= B00100000;
		PORTB &= B11011111;
	}
	int endTime = micros ();
	// Returns the result through the serial connection
	Serial.print ("Time: ");
	Serial.println (endTime - startTime);
}

void loop () {
}
```

And the amazing result: `56 μs`!

As I love Assembly, I can't resist on playing a little bit more, so here it goes the same code (same execution time too = 56 microsec), but way more beautiful! Just to make sure I can still do this kinda thing:

```arduino
// asm-time-test.ino

void setup () {
	// Initializes the serial connection
	Serial.begin (9600);
	// Set pin 13 as output
	asm ("sbi 0x4, 0x5");
	// Evaluates the time to blink the LED connected to the pin 13 (100x)
	int startTime = micros ();
	asm ("ldi r18, 0");
	asm ("for:");
	asm ("sbi %0, 0x5" :: "I" _SFR_IO_ADDR (PORTB));
	asm ("cbi %0, 0x05" :: "I" _SFR_IO_ADDR (PORTB));
	asm ("inc r18");
	asm ("cpi r18, 100");
	asm ("brlt for");
	int endTime = micros ();
	// Returns the result through the serial connection
	Serial.print ("Time: ");
	Serial.println (endTime - startTime);
}

void loop () {
}
```

Well, it's not over yet! As we can see (and prove), the **digitalWrite** is really a slow function, but why? Discussing this with a friend, he told me:

*"I don't like using Arduino libraries, they were built to help children learn to code and usually there are lots of unnecessary verifications, these libraries end up destroying performance, it's way better to know what we are doing and write efficient code!"*

I partially disagree. I believe that these easy-to-use libraries are awesome, and have a pivotal role in spreading the open-hardware and making it accessible, they are there to make the hardware prototyping user-friendly, and given the success of Arduino and other similar platforms, they're doing a great job!

I also believe that when we want to take hardware seriously, it's necessary to know what is happening under the hood, it's always good to have a fast, efficient and reliable code running. So, let's take a look at the **digitalWrite** inside:

```c
void digitalWrite(uint8_t pin, uint8_t val)
{
		uint8_t timer = digitalPinToTimer(pin);
		uint8_t bit = digitalPinToBitMask(pin);
		uint8_t port = digitalPinToPort(pin);
		volatile uint8_t *out;

		if (port == NOT_A_PIN) return;

		// If the pin that support PWM output, we need to turn it off
		// before doing a digital write.
		if (timer != NOT_ON_TIMER) turnOffPWM(timer);

		out = portOutputRegister(port);

		uint8_t oldSREG = SREG;
		cli();

		if (val == LOW) {
				*out &= ~bit;
		} else {
				*out |= bit;
		}

		SREG = oldSREG;
}
```

As you can see, it is doing similar to what we did before, manipulating the ports directly. It becomes slower because it checks whether the value entered is from a pin that actually exists on the Arduino, whether the pin is configured as “analog” output (PWM), and it also does something cooler, which is disabling interrupts during the pin state change, this ensures that the change will occur without problem.

And so? Does it worth saving a few microseconds with so many checks? Well, these verifications are not needed in some scenarios, only if you know exactly what you're doing and if you are a step beyond prototyping. But if you are a hardware enthusiast and like playing with Arduinos like me, just use **digitalWrite** and have fun!

That's all! If you have any thoughts on this, please fell free to reach me at [hello@willgcr.me](mailto:hello@willgcr.me)!

See ya,

Willian Rocha
