/* 
 gurov was here, use this code, or don't, whatever, I don't care. If you see a giant bug with a billion legs, please let me know so it can be squashed

*/


#include <stdio.h>
#include "gb_common.h"
#include <math.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>

#define CLOCK_PIN	24
#define DATA_PIN	25

#define CHANNEL_A_128 0
#define CHANNEL_A_64  2
#define CHANNEL_B_32  1

#define SCK_ON  (GPIO_SET0 = (1 << CLOCK_PIN))
#define SCK_OFF (GPIO_CLR0 = (1 << CLOCK_PIN))
#define DT_R    (GPIO_IN0  & (1 << DATA_PIN))

void           reset_converter(void);
unsigned long  read_cnt(int debug, int channel);
void           set_gain(int r);
void           setHighPri (void);


long offset=0;

void setHighPri (void)
{
  struct sched_param sched ;

  memset (&sched, 0, sizeof(sched)) ;

  sched.sched_priority = 10 ;
  if (sched_setscheduler (0, SCHED_FIFO, &sched))
    printf ("Warning: Unable to set high priority\n") ;
}


void setup_gpio()
{
  INP_GPIO(DATA_PIN);
  INP_GPIO(CLOCK_PIN);  OUT_GPIO(CLOCK_PIN);
  SCK_OFF;

//   GPIO_PULL = 0;
//   short_wait();
//   GPIO_PULLCLK0 = 1 << DATA_PIN;
 //  short_wait();
//   GPIO_PULL = 0;
//   GPIO_PULLCLK0 = 0;

/*   GPIO_PULL = 2;
   short_wait();
   GPIO_PULLCLK0 = 1 << DATA_PIN;
   short_wait();
   GPIO_PULL = 0;
   GPIO_PULLCLK0 = 0;*/
}

void unpull_pins()
{
   GPIO_PULL = 0;
//   short_wait();
   GPIO_PULLCLK0 = 1 << DATA_PIN;
//   short_wait();
   GPIO_PULL = 0;
   GPIO_PULLCLK0 = 0;
} // unpull_pins

int main(int argc, char **argv)
{
  long reading = 0;
  float calibration_factor = 1; /* mV/kg */
  float threshold = 0.25;       /* kg    */
  int b;

  FILE *out = stdout;
	
  static int verbose = false;

  int c;

  while (1)
    {
		static struct option long_options[] = {
          {"verbose",    no_argument,       &verbose, true},
          {"file",       required_argument, 0, 'f'},
          {"multiplier", required_argument, 0, 'm'},
          {"threshold",  required_argument, 0, 't'},
          {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long(argc, argv, "af:", long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1) {
        break;
	  }

      switch (c) {
        case 0:
          if (long_options[option_index].flag != 0) {
            break;
		  }
          break;
        case 'f':
		  out = fopen(optarg, "a");
		  if (!out) {
			  perror("fopen");
		  }
          break;
        case 'm':
		  calibration_factor = atof(optarg);
          break;
        case 't':
		  threshold = atof(optarg);
          break;
        case '?':
		  exit(1);
          break;
        default:
          abort ();
        }
	}

  setHighPri();
  setup_io();
  setup_gpio();
  reset_converter();
	
  float tare_a = read_cnt(0, CHANNEL_A_64) / 2;
  float tare_b = read_cnt(0, CHANNEL_B_32);

  if (verbose) {
	  fprintf(stderr, "tare A: %f\n", tare_a);
	  fprintf(stderr, "tare B: %f\n", tare_b);
  }

  float last_a = 0;
  float last_b = 0;

  while (true) {
    float reading_a = (read_cnt(0, CHANNEL_A_64) / 2 - tare_a) / calibration_factor;
    float reading_b = (read_cnt(0, CHANNEL_B_32) - tare_b) / calibration_factor;

	if (verbose) {
        fprintf(stderr, "%lu\tA=%f\tB=%f\ttotal=%f\n", (unsigned long)time(NULL), reading_a, reading_b, reading_a + reading_b);
	}

    if (abs(reading_a - last_a) > threshold || abs(reading_b - last_b) > threshold) {
        last_a = reading_a;
        last_b = reading_b;

        fprintf(out, "%lu\tA=%f\tB=%f\ttotal=%f\n", (unsigned long)time(NULL), reading_a, reading_b, reading_a + reading_b);
		fflush(out);
    }

    nanosleep((const struct timespec[]){{1, 0L}}, NULL);
  }
  unpull_pins();
  restore_io();
  fclose(out);
}
	


void reset_converter(void) {
	SCK_ON;
	usleep(60);
	SCK_OFF;
	usleep(60);
}

void set_gain(int r) {
	int i;

// r = 0 - 128 gain ch a
// r = 1 - 32  gain ch b
// r = 2 - 64  gain ch a

	for (i=0;i<25+r;i++) {
		SCK_ON;
		SCK_OFF;
	}
}

unsigned long read_cnt(int debug, int gain) {
	long count;
	int i;
	int b;

  count = 0;

    reset_converter();
    set_gain(gain);

  while( DT_R ) {
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL); 
  }
	b++;
	b++;
	b++;
	b++;

  for(i=0; i<24; i++) {
	SCK_ON;
        count = count << 1;
	b++;
	b++;
	b++;
	b++;
        SCK_OFF;
	b++;
	b++;
        if (DT_R > 0 ) { count++; }
//	b++;
//	b++;
  }
	
	SCK_ON;
	b++;
	b++;
	b++;
	b++;
	SCK_OFF;
	b++;
	b++;
	b++;
	b++;
//  count = ~0x1800000 & count;
//  count = ~0x800000 & count;


  if (count & 0x800000) {
	count |= (long) ~0xffffff;
  }

  if (debug) {
      // If things are broken this will show actual data
      for (i=31; i>=0; i--) {
        printf("%d ", ((count - offset) & ( 1 << i )) != 0 );
      }
      printf("n: %10d     -  ", count - offset);
      printf("\n");
  }
	
  return abs(count);
}


