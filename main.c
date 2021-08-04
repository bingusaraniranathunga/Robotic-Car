#define F_CPU 16000000UL
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>


int r,b,g,y,z,k,c,j;
char s[10],w;
static volatile int p = 0;
static volatile int i = 0;
	
void mval(unsigned char a, unsigned char b, unsigned char c, unsigned char d);	

void USART_INIT(void){
	UCSR0B |= (1<<TXEN0)|(1<<RXEN0); //Transmission & Reception Enable (TXEN0=1, RXEN0=1)
	UCSR0C |=(1<<UCSZ01)|(1<<UCSZ00); 
	UBRR0L = 103; //Set baudrate as 9600
}

void USART_SEND(unsigned char c){
	while(!(UCSR0A&(1<<UDRE0))); //Wait until UDR empty
	UDR0 = c; //Load character to UDR0 register
	while(!(UCSR0A&(1<<TXC0))); //Wait until sent
}

void STRING_SEND(char *s){
	//send string using for loop
	for(int i=0;s[i]!=0;i++){
		USART_SEND(s[i]);
	}
}

void ADC_init(void){										

    ADMUX = (1<<REFS0)|(1<<MUX1);					// use AVCC and and A2 as input to adc 
	ADCSRA = (1<<ADEN)|(1<<ADPS1)|(1<<ADPS0); 	// set presclaer 128
}

int ADC_read(void){								

	ADCSRA |= (1<<ADSC);							// statrt conversion
	while(ADCSRA & (1<<ADSC));						// wait while convertion finishes
	return ADC;
}

int create(int count) //Calculate average value using needed ADC value.
{
	int w=0,t=0;
	int avg;
	
	for(int i=0;i<count;i++){
		w=ADC_read();
		t=t+w;
		_delay_ms(5);
	}
	avg=t/count;
	return avg;
}


void init(void){
	USART_INIT();
	ADC_init();
	DDRD|=(1<<4)|(1<<7)|(1<<5)|(1<<6);
	DDRB|=(1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5);
	DDRD|=(0<<2)|(1<<3);
	_delay_ms(50);
	EIMSK |= 1<<INT0;
	EICRA |= 1<<ISC00;

	sei();
}

void mval(unsigned char a, unsigned char b, unsigned char c, unsigned char d){
	OCR0A=a;//initialized registers using variables, control the width of PWM pins
	OCR1A=b;
	OCR0B=c;
	OCR1B=d;

}

void timer(void){
	
	TCCR0A |= (1 << WGM01) | (1 << WGM00)| (1 << COM0A1) |  (1 << COM0B1);//set timer0,enable fast PWM 
    TCCR0B |= (1 << CS00);//No prescaling
    
	
	TCCR1A  |=(1 << WGM12) | (1 << WGM10)| (1 << COM1A1) |  (1 << COM1B1);//set timer1,enable fast PWM
    TCCR1B |= (1 << CS10);//No prescaling
  
}

void motion(void){// initialization of pre define path
	
	timer();
	mval(0,110,127,0);  //forward
	for(int i=0;i<4;i++){
		c_detection();
		o_detection();
	}
	
	mval(0,0,250,0);    //right turn
	_delay_ms(1050);
	mval(0,0,0,0);
	mval(0,110,127,0);  //forward
	for(int i=0;i<3;i++){
		c_detection();
		o_detection();
	}
	
	mval(0,250,0,0);  //right turn
	_delay_ms(950);
	
	mval(0,110,127,0);  //forward
	for(int i=0;i<3;i++){
		c_detection();
		o_detection();
	}
	
	mval(0,250,0,0);  //right turn
	_delay_ms(900);
	
	mval(0,110,127,0);  //forward
	for(int i=0;i<11;i++){
		c_detection();
		o_detection();
	}
	
	mval(0,250,0,0);  //right turn
	_delay_ms(850);
	
	mval(0,110,127,0); //forward
	for(int i=0;i<4;i++){
		c_detection();
		o_detection();
	}
	
	mval(0,250,0,0);  //right turn
	_delay_ms(900);
	
	mval(0,110,127,0); //forward
	for(int i=0;i<2;i++){
		c_detection();
		o_detection();
	}
	
	mval(0,0,250,0);  //right turn
	_delay_ms(950);
	
	mval(0,110,127,0); //forward
	for(int i=0;i<2;i++){
		c_detection();
		o_detection();
	}
	
	
	mval(0,0,0,0);
	
}

void c_detection(void){
	
	DDRD|=(1<<4)|(1<<7);
	DDRB|=(1<<0);
	
	PORTB  |= (1<<PB0);
	_delay_ms(40);
	r=create(20); // red
	PORTB &= ~(1<<PB0);
	_delay_ms(15);
		
	PORTD  |= (1<<PD7);
	_delay_ms(40);
	g = create(20); //green
	PORTD &= ~(1<<PD7);
	_delay_ms(15);
		
	PORTD  |= (1<<PD4);
	_delay_ms(50);
	b=create(40);  //blue
	PORTD &= ~(1<<PD4);
	_delay_ms(15);
		
	y=(b+g)-r;
	z=((2*b)-(r+g));
	
	
	/////////////////////////////color detection///////////////////////////
	if(240<y && 180<z){
		STRING_SEND("5");
		STRING_SEND("\n");
		PORTB |=(1<<3)|(0<<4)|(0<<5);
	}else if(200<y && 200<z){
		STRING_SEND("7");
		STRING_SEND("\n");
		PORTB |=(0<<3)|(1<<4)|(0<<5);
	}else if(180<y && 130<z){
		STRING_SEND("6");
		STRING_SEND("\n");
		PORTB |=(0<<3)|(0<<4)|(1<<5);
	}
	
	STRING_SEND("\n");
}

int main(void){
	init();
	
	while(1){
		motion();
		
	}
	
}

///////////////////////////////obstacle detection/////////////////////////////
void o_detection(void){
	PORTD = 1<<PD3;
    _delay_us(55);
    PORTD = 0<<PD3;
    j = p;
	itoa(p,s,10);
	STRING_SEND(s);
	STRING_SEND("\t");
	if(p<15){
		STRING_SEND("1\n");//detect a obstacle
	}else{
		STRING_SEND("0\n");//no obstacle
	}
}
ISR(INT0_vect)
{
  if(i == 0)
  {
    TCCR2B |= 1<<CS20|1<<CS21|1<<CS22;
    i = 1;
  }
  else
  {
    TCCR2B = 0;
    p = TCNT2;
    TCNT2 = 0;
    i = 0;
  }
}
