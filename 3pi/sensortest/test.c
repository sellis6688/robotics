/*
  3PI template code for NYU "Intro to Robotics" course.
  Yann LeCun, 02/2009.
  This program was modified from an example program from Pololu. 
 */

// The 3pi include file must be at the beginning of any program that
// uses the Pololu AVR library and 3pi.
#include <pololu/3pi.h>
//#include <pololu/orangutan.h>

// This include file allows data to be stored in program space.  The
// ATmega168 has 16k of program space compared to 1k of RAM, so large
// pieces of static data should be stored in program space.
#include <avr/pgmspace.h>
#include <math.h>

// speed of the robot
int speed = 50;
// if =1 run the robot, if =0 stop
int run=0;

// Introductory messages.  The "PROGMEM" identifier 
// causes the data to go into program space.
const char hello[] PROGMEM = "Steve L";

/*Notes
 *1k of RAM 8k of flash.  PROGMEM places on flash?  More space on flash.
 *16 bits per int
 *long is 32 bits for our robot.
 *FPU - floating point unit - calculates floating point arithmetic.
 *fixed-point instead of floating points, we use ints.
 * multiply 1.35, e.g., by 1000 get 1350.
*/
 
// Data for generating the characters used in load_custom_characters
// and display_readings.  By reading levels[] starting at various
// offsets, we can generate all of the 7 extra characters needed for a
// bargraph.  This is also stored in program space.

const char levels[] PROGMEM = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111
};

const char melody[] PROGMEM = "!L16 V8 cdefgab>cbagfedc";
//const char zelda[] PROGMEM = "!T160 L16 V8 bbbb rrbr brbrbr bb.rar bb rrbr brbrbr bb.rar bb rrbr brbrbr bbrfrfr ffrfrfr";
const char bach[] PROGMEM = "!T240 L8 a gafaeada c+adaeafa >aa>bac#ada c#adaeaf4";
const char rhapsody[] PROGMEM = "O6 T40 L8 d#<b<f#<d#<f#<bd#f#"   
  "T80 c#<b-<f#<c#<f#<b-c#8"   
  "T180 d#b<f#d#f#>bd#f#c#b-<f#c#f#>b-c#8 c>c#<c#>c#<b>c#<c#>c#c>c#<c#>c#<b>c#<c#>c#"   
  "c>c#<c#>c#<b->c#<c#>c#c>c#<c#>c#<b->c#<c#>c#"   
  "c>c#<c#>c#f>c#<c#>c#c>c#<c#>c#f>c#<c#>c#"   
  "c>c#<c#>c#f#>c#<c#>c#c>c#<c#>c#f#>c#<c#>c#d#bb-bd#bf#d#c#b-ab-c#b-f#d#";  

char display_characters[9] = { ' ', 0, 1, 2, 3, 4, 5, 6, 255 };

// This function loads custom characters into the LCD.  Up to 8
// characters can be loaded; we use them for 7 levels of a bar graph.
void load_custom_characters()
{
	lcd_load_custom_character(levels+0,0); // no offset, e.g. one bar
	lcd_load_custom_character(levels+1,1); // two bars
	lcd_load_custom_character(levels+2,2); // etc...
	lcd_load_custom_character(levels+3,3);
	lcd_load_custom_character(levels+4,4);
	lcd_load_custom_character(levels+5,5);
	lcd_load_custom_character(levels+6,6);
	clear(); // the LCD must be cleared for the characters to take effect
}

// This function displays the sensor readings using a bar graph.
void display_bars(const unsigned int *s, const unsigned int *minv, const unsigned int* maxv) {
	// Initialize the array of characters that we will use for the
	// graph.  Using the space, and character 255 (a full black box).
	unsigned char i;
	for (i=0;i<5;i++) {
		int c = ((int)s[i]-(int)minv[i])*9/((int)maxv[i]-(int)minv[i]);
		c = (c<0)?0:(c>8)?8:c;
		// if (i==0) {print_long(s[0]); print_long(c); }
		print_character(display_characters[c]);
	}
}

void update_bounds(const unsigned int *s, unsigned int *minv, unsigned int *maxv) {
	int i;
	for (i=0; i<5; i++) { 
		if (s[i]<minv[i]) minv[i] = s[i];
		if (s[i]>maxv[i]) maxv[i] = s[i];
	}
}

// return line position
// YOU MUST WRITE THIS.
long line_position(unsigned int *s, unsigned int *minv, unsigned int *maxv) {
  int i;
  long position;
  long numSum = 0;
  long denSum = 0;
  int sense[] = {0, 0, 0, 0, 0};
  
  for(i = 0; i < 5; i+=1){
    sense[i] = (100*((s[i]-minv[i])))/(maxv[i]-minv[i]);
    //sense[i] = s[i];
    //Here we could possibly print a message indicating that there is no line
    //if that is indeed the case according to the sensors.
    numSum += (sense[i])*(i-2)*100;
    denSum += (sense[i]);
      if(denSum == 0)
	denSum += 1;
  }

  //print_long(sense[2]);

  position = numSum/denSum;
  //We may use something like a status variable, denoting whether the previous
  //sensor was activated.  In that case we can tell that there are contiguous
  //sensors being activated. (i.e. a thicker line).
    return position;
}

// Make a little dance: Turn left and right
void dance(unsigned int *s, unsigned int *minv, unsigned int *maxv) {
	int counter;
	for(counter=0;counter<80;counter++)	{
	  read_line_sensors(s, IR_EMITTERS_ON);
	  update_bounds(s, minv, maxv);
		if(counter < 20 || counter >= 60)
			set_motors(40,-40);
		else
			set_motors(-40,40);
		// Since our counter runs to 80, the total delay will be
		// 80*20 = 1600 ms.
		delay_ms(20);
	}
	
	set_motors(0,0);
}


void runIt(int val) 
{
  //val=(int)val*.08;
  set_motors(speed+val, speed-val);
}

// Initializes the 3pi, displays a welcome message, calibrates, and
// plays the initial music.
void initialize()
{
  // This must be called at the beginning of 3pi code, to set up the
  // sensors.  We use a value of 2000 for the timeout, which
  // corresponds to 2000*0.4 us = 0.8 ms on our 20 MHz processor.
  pololu_3pi_init(2000);
  load_custom_characters(); // load the custom characters
  // display message
  print_from_program_space(hello);
  lcd_goto_xy(0,1);
  print("Press B");

}

// This is the main function, where the code starts.  All C programs
// must have a main() function defined somewhere.
int main()
{
  // global array to hold sensor values
  unsigned int sensors[5]; 
  // global arrays to hold min and max sensor values
  // for calibration
  unsigned int minv[5], maxv[5]; 
  // line position relative to center
  int oldposition = 0;
  int position = 0;
  int i;
  long integral;
  int kp = 0;
  float kd;
  float ka;
  float ut;
  long difference;
  //char play_switch = 0;

  // set up the 3pi, and wait for B button to be pressed
  initialize();

  read_line_sensors(sensors,IR_EMITTERS_ON);
  for (i=0; i<5; i++) { minv[i] = maxv[i] = sensors[i]; }
    
    

  // Display calibrated sensor values as a bar graph.
  dance(sensors, minv, maxv);
  while(1) {
    if (button_is_pressed(BUTTON_B)) { 
      run = 1-run; 
      delay(200);
    }
    if (button_is_pressed(BUTTON_A)) { 
      //speed -= 10; delay(100);
      kp = kp - 1;
      delay(100);
    }
    if (button_is_pressed(BUTTON_C)) { 
      kp = kp + 1;
      //speed += 10; 
      delay(100);
      // play_switch += 1;
    }
    
    // Read the line sensor values
    read_line_sensors(sensors, IR_EMITTERS_ON);
    // update minv and maxv values,
    // and put normalized values in v
    //update_bounds(sensors, minv, maxv);

    //update_bounds(sensors, minv, maxv);

    // compute line positon
    oldposition = position;
    position = line_position(sensors, minv, maxv);
    difference = (position - oldposition)/10;
    ut = (position*kp)/100;
    ut = (position*kp)/100 + difference*kd;
    //    ut = position*kp + integral*ka + difference * kd;
    if (run) {
      runIt(ut);
    }
    if(!run) {set_motors(0, 0);}
    //if(play_switch){
    //play_from_program_space(rhapsody);
    //play_switch = 0;
    //}

    // display bargraph
    clear();
      //print_long(position);
      print_long(kp);
      lcd_goto_xy(4, 0);  
      print_long(ut);
    lcd_goto_xy(0,1);
    // for (i=0; i<8; i++) { print_character(display_characters[i]); }
    display_bars(sensors, minv, maxv);
      delay(10);
   
  }
}
