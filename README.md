HX711
=====

Raspberry PI HX711 weight scale server
-----------------------------------------

This server reads values from a specialised 24-bit (ADC) for weight scales, hx711. Some documentation can be found here:   
https://github.com/sparkfun/HX711-Load-Cell-Amplifier/blob/master/datasheets/hx711_english.pdf

It's available for cheap [on Amazon - Weighing Sensor AD Module Dual-channel 24-bit A/D Conversion HX711](https://www.amazon.com/gp/product/B00M6POINC?ie=UTF8&tag=sendmethingie-20&camp=1789&linkCode=xm2&creativeASIN=B00M6POINC) at US$7 now.
And it works with load cells such as these [4PCS 50Kg Body Load Cell Weighing Sensor Resistance strain Half-bridge](https://www.amazon.com/gp/product/B071ZYYJHJ/ref=as_li_tl?ie=UTF8&tag=sendmethingie-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B071ZYYJHJ&linkId=9b59bd8bfa4485115fbc566b2fd9982a) currently at US$9 which are the ones I use.

One you have wired the stuff together wire the CLK pin to the Raspberry's BCM24 pin, and the DATA pin to the BCM25 pin. You can use something else of course, but then you'll need to change the values in the source.

To compile the server, just run:

    make

To run it, you can use the following options:

    -v --verbose			print every recorded value
    
    -f <file> --file=<file>		write values to file <file>
    
    -m <n> --multiplier=<n>		multiplier to get the weight in kilograms from the hx711 output
                           		mine is 11500 (I guess the unit is in mV/kg or something)
				
    -t <n> --threshold=<n>		threshold for considering that the weight has changed, in kg
                          		default is 0.25 so 250 grams.
    

You will probably need to experiment a bit before you find the right multiplier for you installation, you can just use water for calibration as 1 L is 1 kg, making it easy to measure if you don't have known weights.

The server monitors both channels continuously, and outputs the separate values, as well as the total. If you wire the sensors by pairs, it can help you determine the front/rear or left/right balance of weight for example. Channel B is less precise, but for my application it's not really an issue.

At launch, the server takes a first measure and uses it as tare, from then on all output is relative to tare.

Values are not filtered in any way, so some are bound to be anomalous because the Raspberry Pi isn't a realtime platform and not entirely adequate for this application.

A systemd unit file is provided:   

    hx711.service
    
You can set your arguments there.
