1- Used Hardware:

- ATmega32 Microcontroller.

- Multiplexed 6 7_segments.

- Multiplexed 4 7_segments.

- Buttons, Resistors.



2- Required Functionalities:

-stopwatch used to increment time every one second using Timer1 CTC mode.

- Reset button used to stop the stopwatch using INT0 and set it to zero and wait for another signal from the start button.

- Pause button used to stop counting on the stopwatch using INT1.

- Resume button used to continue counting on the stopwatch using INT2.



3- Added Functionalities:

- Push button used to start counting on the 6 multiplexed 7_segments 

   "Start the stopwatch"

- when the reset button is pressed, the last time on the 6 multiplexed 7_segment will appear on the 4 multiplexed 7_segment.
