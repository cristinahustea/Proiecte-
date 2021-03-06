char date_primite;
unsigned int numar_CAN;
int contor_litera=0;

// initializarea USART 
void serial_init()
{
  UBRR0=103;// viteza 9600 biti/sec
  UCSR0B=(1<<RXEN0)|(1<<TXEN0);//activare recepție și transmisie
  UCSR0C=(1<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);// 2 biți stop, 8 biți date
   
}

//functie de receptie pentru aprinderea si stingerea ledului
unsigned char receptie_date(void)
{
    //while(!(UCSR0A&(1<<RXC0)));//așteaptă un numar receptionat in UDR0
      return UDR0;//furnizează ca rezultat octetul primit
}

//functie de transmisie 
void transmisie_date(unsigned char date_detrimis)
{
    while(!(UCSR0A&(1<<UDRE0)));//așteaptă  pentru eliberarea registrului UDR0 de la transmisia precedenta  
        UDR0=date_detrimis;//transmite datele
}

//fuctia de aprindere si stingere a unui led
void AprinsStins() 
{
  DDRB|=(1<<4);
  
  date_primite=receptie_date();
  
  if(date_primite=='A')
  { 
    PORTB|=(1<<4);
  }
  else if(date_primite=='S')
  { 
    PORTB&=~(1<<4);
  }
}
void adc_init() //functie de activare CAN
{ 
  //Se seteaza corespunzator registrului de stare si control ADCSRA
  ADCSRA |= ((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0));
  ADMUX |= (1<<REFS0); 
  ADCSRA |= (1<<ADEN);
  ADCSRA |= (1<<ADSC);
}

uint16_t read_adc(uint8_t channel) //functie de citire de la Portul Analog‐Numeric
{
  ADMUX &= 0xF0;
  ADMUX |= channel;
  ADCSRA |= (1<<ADSC); //Se seteaza ADSC pe ”1” din registrul ADCSRA
  while(ADCSRA & (1<<ADSC)); //Se verifica in bucla valoarea bitului ADSC 
  //Cand acesta devine ”0” conversia s-a incheiat si se poate prelua rezultatul din registrul ADC
  return ADC;
}
void PWM()
 {
    //FADE IN
    for(int i=0; i<256; i++)
    {
    OCR2B =i;
    _delay_ms(3.92);
    }
    //O SECUNDA APRINS
    OCR2B =255;
    _delay_ms(1000);
     //FADE OUT
    for(int i=256; i>0; i--)
    {
    OCR2B =i;
    _delay_ms(3.92);
    }
    //O SECUNDA STINS
    OCR2B =0;
    _delay_ms(1000);
  
 }
//setez directia utilizata pentru afisajul cu 7 segmente 
// valoarea '1' logic inseamna iesire
 void litera_init()  
{
    DDRB |=(1<<3)|(1<<1);
    DDRD |=(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<2);
}
//resetez afisajul cu 7 segmente 
// valoarea '0' logic inseamna ca sunt stinse
 void resetare_litera()
 {
    PORTB&=~((1<<3)|(1<<1));
    PORTD&=~((1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<2));
 }
 //pun pe '1' logic doar segmentele pentru afisarea lui 'C'
 void litera_C() 
 {
   resetare_litera();
   PORTB|=(1<<1);
   PORTD|=(1<<6)|(1<<5)|(1<<4);
 }
 //pun pe '1' logic doar segmentele pentru afisarea lui 'M'
 void litera_M()
 {
   resetare_litera();
   PORTB|=(1<<1)|(1<<3);
   PORTD|=(1<<5);
 }
 //pun pe '1' logic doar segmentele pentru afisarea lui 'H'
 void litera_H()
{
   resetare_litera(); 
   PORTB|=(1<<3);
   PORTD|=(1<<6)|(1<<5)|(1<<7);
 }
 void PWM_init()
 {
   //folosesc TIMER 2 pentru PWM 
   
   DDRD |=(1<<3); // pentru PWM
   
   //NON INVERTING MODE
   TCCR2A |=(1<<COM2B1); 
   
   //FAST PWM
   TCCR2A |=(1<<WGM21)|(1<<WGM20);
   
   //PRESCALER=1024 CLOCK SELECT
   TCCR2B |=(1<<CS22)|(1<<CS20)|(1<<CS21); 
 
}
void TEMP()
{
  //am calculat temperatura din tensiune, apoi am aflat partea intreaga si fractionara (convertite din int in char  ), urmand sa le concateneaz 
   char parte_intreaga[100], parte_fractionara[100];
   int tempint;
  
   float temp;
   float valoare_volti;
   numar_CAN = read_adc(0);
   valoare_volti=(((float)numar_CAN * 5000)/1023);
   temp=(valoare_volti)/10;
  
   tempint = (int)(temp * 100);
  
   itoa(tempint/100,  parte_intreaga, 10); //itoa- conversie din int in char  
   itoa(abs(tempint%100), parte_fractionara, 10);
   
   strcat(parte_intreaga, "."); //strcat- concateneaza
   strcat(parte_intreaga, parte_fractionara);
   strcat(parte_intreaga, "\n");
  
   for(int i = 0; i < strlen(parte_intreaga); i++)
  {
    transmisie_date(parte_intreaga[i]);
  }
  
}
void timer_1_secunda()
{
  DDRB |=(1<<5);
  
  //folosesc TIMER 1 pentru a numara o secunda
  
  //resetez registrii de control 
   TCCR1A=0;
   TCCR1B=0;
   TCCR1C=0;
   
   //modul de operare CTC
   TCCR1B|=(1<<WGM12);
   
   //PRESCALER 1024
   TCCR1B|=(1<<CS12)|(1<<CS10);
   
   //Setare valoare registru de comparatie 
   OCR1A=15624;
   
   //Setez intreruperea
   TIMSK1|=(1<<OCIE1A);
}
void setup() 
{
  adc_init();
  serial_init();
  PWM_init();
  litera_init();
  timer_1_secunda();
   
   sei(); 
 

}
//rutina de tratare a intreruperii
ISR(TIMER1_COMPA_vect)
{
  PORTB^=(1<<5); // se aprinde la o secunda ledul conectat la PB5
    
  if(contor_litera==0) //la 0 secunde afisajul cu 7 segmente indica 'H'
  {
  litera_H();
  }
  else if(contor_litera==1) //la o secunda afisajul cu 7 segmente indica 'M'
  {
  litera_M(); 
  }
  else if(contor_litera==2) //la doua secunde afisajul cu 7 segmente indica 'C'
  {
  litera_C(); 
  }
  contor_litera++;
  if (contor_litera>=3) //cand depaseste valoarea de doua secunde se reseteaza
  {
    contor_litera=0;
  }  
  
  TEMP();
}
void loop() 
{
  
  AprinsStins();
  PWM();

}
