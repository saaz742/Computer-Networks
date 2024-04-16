#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// second part: go-back-n

/* The simulated network environment
 *
 * A call to procedure tolayer3() sends packets into the medium
 * (i.e., into the network layer).Your procedures A_input() and B_input()
 * are called when a packet is to be delivered from the medium to your protocol layer.
 *
 * The medium is capable of corrupting and losing packets. However, it
 * will not reorder packets. When you compile your procedures and with
 * the rest of the simulator and run the resulting program, you will
 * be asked to specify values regarding the simulated network environment:
 *
 *    Number of messages to simulate:
 *      The simulator (and your routines) will stop as soon as this number
 *      of messages have been passed down from layer 5 to your protocol,
 *      regardless of whether or not all of the messages have been correctly delivered.
 *      Thus, you need not worry about undelivered or unACK'ed messages
 *      still in your sender or in the channel when the emulator stops.
 *      Note that if you set this value to 1, your program will terminate immediately,
 *      before the message is delivered to the other side. Thus,
 *      this value should always be greater than 1.
 *    Loss:
 *      You are asked to specify a packet loss probability.
 *      A value of 0.1 would mean that one in ten packets (on average) is lost.
 *    Corruption:
 *      You are asked to specify a packet corruption probability.
 *      A value of 0.2 would mean that one in five packets (on average)
 *      have their bits corrupted. Note that the contents of the payload,
 *      sequence, ack, or checksum fields can be corrupted.
 *      You must implement a checksum mechanism that covers the the data,
 *      sequence, and ack fields of the message.
 *    Tracing:
 *      Setting a tracing value of 1, 2 or 3 will print out useful information
 *      about what is going on inside the emulation (e.g., what's happening
 *      to packets and timers). A tracing value of 0 will turn this off.
 *      A tracing value greater than 2 will display all sorts of odd messages
 *      that are for the emulator-debugging purposes (but that
 *      could also help you debug your code).
 *    Average time between messages from sender's layer5:
 *      You can set this value to any non-zero, positive value.
 *      Note that the smaller the value you choose, the faster
 *      packets will be be arriving to your sender.
 **/

/* a "msg" is the data unit passed from layer 5  to layer  */
/* 4 (your code).  It contains the data (characters) to be delivered */
/* to layer 5 (using the message struct) via your transport level */
/* protocol entities.         */
struct msg
{
  char data[20];
};

/* a packet is the data unit passed from layer 4 (your code) to layer */
/* 3.  Note the pre-defined packet structure, which you  */
/* must follow. */
struct pkt
{
  int seqnum;
  int acknum;
  int checksum;
  char payload[20];
};

/* Prototypes of callable routines */

/* calling_node is either 0 (for starting the A-side timer) or 1 */
/* (for starting the B side timer),  and increment is a float value */
/* indicating the amount of time that will pass before the timer interrupts. */
/* A's timer should only be started (or stopped) by A-side routines, */
/* and similarly for the B-side timer. */
/* To give you an idea of the appropriate increment value to use: */
/* a packet sent into the network takes an average of 5 time units */
/* to arrive at the other side when there are no other messages in the medium. */
void starttimer(int calling_node, float increment);

/* calling_node is either 0 (for stopping the A-side timer) */
/* or 1 (for stopping the B side timer). */
void stoptimer(int calling_node);

/* calling_node is either 0 (for the A-side send) or 1 */
/* (for the B side send), and packet is a structure of type pkt. */
/* Calling this routine will cause the packet to be sent into */
/* the network, destined for the other node. */
void tolayer3(int calling_node, struct pkt packet);

/* calling_node is either 0 (for A-side delivery to layer 5) */
/* or 1 (for B-side delivery to layer 5), and message is */
/* a structure of type msg. */
/* Calling this routine will cause data to be passed up to layer 5. */
/* With unidirectional data transfer (which is what you have to implement), */
/* you would only be calling this routine with calling_node equal to 1*/
/* (delivery to the B-side). */
void tolayer5(int calling_node, struct msg message);

/********* YOU SHOULD WRITE THE NEXT SEVEN ROUTINES *********/

#define TIMER 20
#define SIZE 20
#define A 0
#define B 1

#define BUFFER_SIZE 64
#define WINDOW_SIZE 8

int A_seq_num = 1;
int B_seq_num = 0;
int ack = 0;

struct pkt A_packet[BUFFER_SIZE];
struct pkt B_packet;
int A_base = 1;
int A_buffer = 1;

int cal_checksum(struct pkt packet)
{
  int checksum = 0;
  checksum = packet.seqnum + packet.acknum;
  for (int i = 0; i < SIZE; i++)
  {
    checksum += packet.payload[i];
  }
  return checksum;
}

/* called from layer 5, passed the data to be sent to other side. Return a 1 if
data is accepted for transmission, negative number otherwise */
int A_output(message)
struct msg message;
{
  if (A_buffer >= BUFFER_SIZE + A_base)
  {
    printf("A_output: buffer full\n");
    return -1;
  }
  struct pkt *packet = &A_packet[A_buffer % BUFFER_SIZE];
  packet->seqnum = A_buffer;
  strcpy(packet->payload, message.data);
  packet->checksum = cal_checksum(*packet);
  ++A_buffer;
  while (A_seq_num - A_base < WINDOW_SIZE && A_seq_num < A_buffer)
  {
    struct pkt *pckt = &A_packet[A_seq_num % BUFFER_SIZE];
    tolayer3(A, *pckt);
    if (A_base == A_seq_num)
    {
      starttimer(A, TIMER);
    }
    ++A_seq_num;
  }
  return 1;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet) struct pkt packet;
{
  if (packet.acknum < A_base)
  {
    printf("ack error\n");
    return;
  }
  if (!cal_checksum(packet))
  {
    printf("checksum error\n");
    return;
  }
  A_base = packet.acknum + 1;
  if (A_seq_num == A_base)
  {
    stoptimer(A);
    while (A_seq_num - A_base < WINDOW_SIZE && A_seq_num < A_buffer)
    {
      struct pkt *pckt = &A_packet[A_seq_num % BUFFER_SIZE];
      tolayer3(A, *pckt);
      if (A_base == A_seq_num)
      {
        starttimer(A, TIMER);
      }
      ++A_seq_num;
    }
  }
  else
  {
    starttimer(A, TIMER);
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  for (int i = A_base; i < A_seq_num; ++i)
  {
    struct pkt *packet = &A_packet[i % BUFFER_SIZE];
    tolayer3(A, *packet);
  }
  starttimer(A, TIMER);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  A_seq_num = 1;
  A_base = 1;
  A_buffer = 1;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet) struct pkt packet;
{
  if (packet.seqnum != B_seq_num)
  {
    printf("seq error\n");
    tolayer3(B, B_packet);
    return;
  }
  if (!cal_checksum(packet))
  {
    printf("check error\n");
    tolayer3(B, B_packet);
    return;
  }
  printf("B_input: received packet %d\n", packet.seqnum);
  struct msg massage;
  tolayer5(B, massage);
  // strcpy(massage.data, packet.payload);
  B_packet.checksum = cal_checksum(B_packet);
  B_packet.acknum = B_seq_num;
  tolayer3(B, B_packet);
  ++B_seq_num;
}

/* called when B's timer goes off */
void B_timerinterrupt()
{
  printf("NO B_timerinterrupt\n");
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  B_seq_num = 1;
  memset(B_packet.payload, 0, 20);
  B_packet.seqnum = -1;
  B_packet.acknum = 0;
  B_packet.checksum = cal_checksum(B_packet);
}
/****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how the emulator
designed , you're welcome to look at the code - but again, you should have
to, and you definitely should not have to modify anything.
******************************************************************/

struct event
{
  float evtime;       /* event time */
  int evtype;         /* event type code */
  int eventity;       /* entity where event occurs */
  struct pkt *pktptr; /* ptr to packet (if any) assoc w/ this event */
  struct event *prev;
  struct event *next;
};
struct event *evlist = NULL; /* the event list */

/* use for bidirectional transfer of data */
#define BIDIRECTIONAL 0

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER5 1
#define FROM_LAYER3 2

#define A 0
#define B 1

int TRACE = 1;            /* for my debugging */
int nsim = 0;             /* number of messages from layer 5 to 4 so far */
int nsimmax = 0;          /* number of msgs to generate, then stop */
float simul_time = 0.000; /* global simulation simul_time */
float lossprob;           /* probability that a packet is dropped  */
float corruptprob;        /* probability that one bit is packet is flipped */
float lambda;             /* arrival rate of messages from layer 5 */
int ntolayer3;            /* number sent into layer 3 */
int nlost;                /* number lost in media */
int ncorrupt;             /* number corrupted by media*/
int randseed;             /* random number seed */

/* use only for biderectional data transfer */
int B_output(message) /* need be completed only for extra credit */
struct msg message;
{

  return 0;
}

/****************** EVENT LIST ROUTINE  *************/
/* Event list manipulation routines                 */
/****************************************************/
void insertevent(p) struct event *p;
{
  struct event *q, *qold;

  if (TRACE > 2)
  {
    printf("            INSERTEVENT: time is %lf\n", simul_time);
    printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
  }
  q = evlist; /* q points to header of list in which p struct inserted */
  if (q == NULL)
  { /* list is empty */
    evlist = p;
    p->next = NULL;
    p->prev = NULL;
  }
  else
  {
    for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
      qold = q;
    if (q == NULL)
    { /* end of list */
      qold->next = p;
      p->prev = qold;
      p->next = NULL;
    }
    else if (q == evlist)
    { /* front of list */
      p->next = evlist;
      p->prev = NULL;
      p->next->prev = p;
      evlist = p;
    }
    else
    { /* middle of list */
      p->next = q;
      p->prev = q->prev;
      q->prev->next = p;
      q->prev = p;
    }
  }
}

void printevlist()
{
  struct event *q;
  printf("--------------\nEvent List Follows:\n");
  for (q = evlist; q != NULL; q = q->next)
  {
    printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype, q->eventity);
  }
  printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(AorB) int AorB; /* A or B is trying to stop timer */
{
  struct event *q;

  if (TRACE > 2)
    printf("          STOP TIMER: stopping timer at %f\n", simul_time);
  /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
  for (q = evlist; q != NULL; q = q->next)
    if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
    {
      /* remove this event */
      if (q->next == NULL && q->prev == NULL)
        evlist = NULL;          /* remove first and only event on list */
      else if (q->next == NULL) /* end of list - there is one in front */
        q->prev->next = NULL;
      else if (q == evlist)
      { /* front of list - there must be event after */
        q->next->prev = NULL;
        evlist = q->next;
      }
      else
      { /* middle of list */
        q->next->prev = q->prev;
        q->prev->next = q->next;
      }
      free(q);
      return;
    }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

void starttimer(int AorB, float increment)
// int AorB; /* A or B is trying to stop timer */
// float increment;
{

  struct event *q;
  struct event *evptr;

  if (TRACE > 2)
    printf("          START TIMER: starting timer at %f\n", simul_time);
  /* be nice: check to see if timer is already started, if so, then  warn */
  /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
  for (q = evlist; q != NULL; q = q->next)
    if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
    {
      printf("Warning: attempt to start a timer that is already started\n");
      return;
    }

  /* create future event for when timer goes off */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtime = simul_time + increment;
  evptr->evtype = TIMER_INTERRUPT;
  evptr->eventity = AorB;
  insertevent(evptr);
}

/************************** TOLAYER3 ***************/
double random_number()
{

  // generate a uniform random number in the interval [0,1)
  return (double)1.0 * rand() / (RAND_MAX + 1.0);
}

void init_random(unsigned int randseed)
{

  // initialize the random number generator
  if (!randseed)
  {
    srand((unsigned int)time(NULL));
  }
  else
    srand(randseed);
}

void tolayer3(AorB, packet) int AorB; /* A or B is trying to stop timer */
struct pkt packet;
{
  struct pkt *mypktptr;
  struct event *evptr, *q;
  float lastime, x;
  int i;

  ntolayer3++;

  /* simulate losses: */
  if (random_number() < lossprob)
  {
    nlost++;
    if (TRACE > 0)
      printf("          TOLAYER3: packet being lost\n");
    return;
  }

  /* make a copy of the packet student just gave me since he/she may decide */
  /* to do something with the packet after we return back to him/her */
  mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
  mypktptr->seqnum = packet.seqnum;
  mypktptr->acknum = packet.acknum;
  mypktptr->checksum = packet.checksum;
  for (i = 0; i < 20; i++)
    mypktptr->payload[i] = packet.payload[i];
  if (TRACE > 2)
  {
    printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
           mypktptr->acknum, mypktptr->checksum);
    for (i = 0; i < 20; i++)
      printf("%c", mypktptr->payload[i]);
    printf("\n");
  }

  /* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype = FROM_LAYER3;      /* packet will pop out from layer3 */
  evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */
                                    /* finally, compute the arrival time of packet at the other end.
                                       medium can not reorder, so make sure packet arrives between 1 and 10
                                       time units after the latest arrival time of packets
                                       currently in the medium on their way to the destination */
  lastime = simul_time;
  for (q = evlist; q != NULL; q = q->next)
    if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
      lastime = q->evtime;
  evptr->evtime = lastime + 1.0 + 9.0 * random_number();

  /* simulate corruption: */
  if (random_number() < corruptprob)
  {
    ncorrupt++;
    if ((x = random_number()) < .75)
      mypktptr->payload[0] = 'Z'; /* corrupt payload */
    else if (x < .875)
      mypktptr->seqnum = 999999;
    else
      mypktptr->acknum = 999999;
    if (TRACE > 0)
      printf("          TOLAYER3: packet being corrupted\n");
  }

  if (TRACE > 2)
    printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
}

void tolayer5(AorB, msgReceived) int AorB;
struct msg msgReceived;
{
  int i;
  if (TRACE > 2)
  {
    printf("          TOLAYER5: data received: ");
    for (i = 0; i < 20; i++)
      printf("%c", msgReceived.data[i]);
    printf("\n");
  }
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival(int entity)
{
  double x;
  struct event *evptr;

  if (TRACE > 2)
    printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

  x = lambda * random_number() * 2; /* x is uniform on [0,2*lambda] */
                                    /* having mean of lambda        */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtime = simul_time + x;
  evptr->evtype = FROM_LAYER5;

  if (entity)
    evptr->eventity = entity;
  else
  {
    if (BIDIRECTIONAL && (random_number() > 0.5))
      evptr->eventity = B;
    else
      evptr->eventity = A;
  }
  insertevent(evptr);
}

/*************** INITIALIZATION ROUTINE  *************/
/* Read input from user and initalize parameters     */
/*****************************************************/
void init()
{
  int i;
  float sum, avg;

  printf("-----  Stop and Wait Network Simulator -------- \n\n");
  printf("Enter the number of messages to simulate: ");
  scanf("%d", &nsimmax);
  printf("Enter packet loss probability [enter 0.0 for no loss]: ");
  scanf("%f", &lossprob);
  printf("Enter packet corruption probability [0.0 for no corruption]: ");
  scanf("%f", &corruptprob);
  printf("Enter average time between messages from sender's layer5 [ > 0.0]: ");
  scanf("%f", &lambda);
  printf("Enter a seed for the random number generator [0 will provide a random seed]: ");
  scanf("%d", &randseed);
  printf("Enter TRACE [0,1,2,3]: ");
  scanf("%d", &TRACE);

  /* init random number generator */
  init_random(randseed);

  sum = 0.0; /* test random number generator for students */
  for (i = 0; i < 1000; i++)
    sum = sum + random_number(); /* should be uniform in [0,1) */
  avg = sum / 1000.0;
  if ((avg < 0.25) || (avg > 0.75))
  {
    printf("It is likely that random number generation on your machine\n");
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine random_number() in the emulator code. Sorry. \n");
    exit(0);
  }

  ntolayer3 = 0;
  nlost = 0;
  ncorrupt = 0;

  simul_time = 0.0;         /* initialize simul_time to 0.0 */
  generate_next_arrival(0); /* initialize event list */
}

/********************* MAIN ROUTINE  *****************/
/* Main simulation loop and handling of events       */
/*****************************************************/
int main(void)
{
  struct event *eventptr;
  struct msg msg2give;
  struct pkt pkt2give;

  int i, j;

  /* initialize our data structures and read parameters */
  init();

  /* call the user's init functions */
  A_init();
  B_init();

  /* loop forever... */
  while (1)
  {
    eventptr = evlist; /* get next event to simulate */
    if (eventptr == NULL)
    {
      printf("INTERNAL PANIC: Event list is empty! This should NOT have happened.\n");
      break;
    }
    evlist = evlist->next; /* remove this event from event list */
    if (evlist != NULL)
      evlist->prev = NULL;
    if (TRACE >= 2)
    {
      printf("\nEVENT time: %f,", eventptr->evtime);
      printf("  type: %d", eventptr->evtype);
      if (eventptr->evtype == 0)
        printf(", timerinterrupt  ");
      else if (eventptr->evtype == 1)
        printf(", fromlayer5 ");
      else
        printf(", fromlayer3 ");
      printf(" entity: %d\n", eventptr->eventity);
    }
    simul_time = eventptr->evtime; /* update simul_time to next event time */
    if (nsim == nsimmax)
      break; /* all done with simulation */
    if (eventptr->evtype == FROM_LAYER5)
    {

      /* fill in msg to give with string of same letter */
      j = nsim % 26;
      for (i = 0; i < 20; i++)
        msg2give.data[i] = 97 + j;
      if (TRACE > 2)
      {
        printf("          MAINLOOP: data given to student: ");
        for (i = 0; i < 20; i++)
          printf("%c", msg2give.data[i]);
        printf("\n");
      }

      if (eventptr->eventity == A)
        j = A_output(msg2give);
      else
        j = B_output(msg2give);

      if (j < 0)
      {
        if (TRACE >= 1)
          printf("          MAINLOOP: data NOT accepted by layer 4 (student code)\n");
        /* set up future arrival for the same entity*/
        generate_next_arrival(eventptr->eventity);
      }
      else
      {
        nsim++;
        if (TRACE >= 1)
          printf("          MAINLOOP: data accepted by layer 4 (student code)\n");
        /* set up future arrival */
        generate_next_arrival(0);
      }
    }
    else if (eventptr->evtype == FROM_LAYER3)
    {
      pkt2give.seqnum = eventptr->pktptr->seqnum;
      pkt2give.acknum = eventptr->pktptr->acknum;
      pkt2give.checksum = eventptr->pktptr->checksum;
      for (i = 0; i < 20; i++)
        pkt2give.payload[i] = eventptr->pktptr->payload[i];
      if (eventptr->eventity == A) /* deliver packet by calling */
        A_input(pkt2give);         /* appropriate entity */
      else
        B_input(pkt2give);
      free(eventptr->pktptr); /* free the memory for packet */
    }
    else if (eventptr->evtype == TIMER_INTERRUPT)
    {
      if (eventptr->eventity == A)
        A_timerinterrupt();
      else
        B_timerinterrupt();
    }
    else
    {
      printf("INTERNAL PANIC: unknown event type \n");
      break;
    }
    free(eventptr);
  }

  printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n", simul_time, nsim);
  return 0;
}
