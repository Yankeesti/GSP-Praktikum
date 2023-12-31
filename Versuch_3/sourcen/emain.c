﻿#include "simuc.h"

///////////////////////////////////////////////////////////////////////////////////////
// Bedingte Compilierung 
// Es darf immer nur ein "define" aktive, d.h. nicht auskommentiert, sein.
//
//#define V3_Aufgabe_1
#define V3_Aufgabe_2_und_3
//#define nachrichtenempfang_ueber_ports					
//#define timer_als_taktsignalgenerator					
//
///////////////////////////////////////////////////////////////////////////////////////




 #ifdef V3_Aufgabe_1

/*
	ES GELTEN DIE BEI "rollosteuerung_moore" AN GLEICHER STELLE VORANGESTELLTEN KOMMENTARE 
*/

// Einige Defines zum leichteren Wiederfinden
#define BIT_POS_IST_OBEN	0
#define BIT_POS_IST_UNTEN	1
#define BIT_POS_NACH_OBEN	4
#define BIT_POS_NACH_UNTEN	5
#define BIT_POS_FAHRE_NACH_OBEN	9
#define BIT_POS_MOTOR_AN	8
#define BIT_POS_FAHRE_NACH_UNTEN	10

typedef enum {hoch, runter, steht} STATE;	// Datentyp für den Zustand das Automaten.
											// Vergleichbar mit "TYPE .... IS" in VHDL.
typedef unsigned short int USHORT;			// Eigener Datentyp zur Abkuerzung

//Variablen
typedef struct {
    unsigned char hh;
    unsigned char mm;
    unsigned char ss;
} uhrzeit;

uhrzeit akt_zeit, hoch_zeit, runter_zeit;

void steuerungsfunktion    (	USHORT ist_oben, USHORT ist_unten, 
						USHORT nach_oben, USHORT nach_unten, 
						USHORT* p_fahre_nach_oben, 
						USHORT* p_fahre_nach_unten, 
                        STATE* p_state,
                        USHORT nach_oben_wegen_zeit,
                        USHORT nach_unten_wegen_zeit)
{

	// 5.)	switch-case-Konstrukt zur "Verwaltung" der Zustaende
	switch (*p_state) {

		// 6.)	Ein CASE je Zustand
		case steht:

			// 7a.)  .... Ausgabesignale bestimmen 
			*p_fahre_nach_unten=0; *p_fahre_nach_oben=0; 

			// 8.)    Eingabesignale auswerten und Zustandswechsel herbei fuehren
			//         Ein IF je Pfeil
            if (  ((ist_unten == 0) && (nach_unten == 1) && (nach_oben == 0)) || (nach_unten_wegen_zeit != 0 && ist_unten != 1)) {
				*p_state = runter; // Wechsel in den Zustand "runter"
			}
            if (((ist_oben == 0) && (nach_oben == 1)) || (nach_oben_wegen_zeit != 0 && ist_oben != 1) ){
				*p_state = hoch;  // Wechsel in den Zustand "hoch"
            }
			break;

		case runter:

			// 7a.)  Ausgabesignale bestimmen 
			*p_fahre_nach_unten=1; *p_fahre_nach_oben=0; 


			// 8.)    Eingabesignale auswerten und Zustandswechsel herbei fuehren
            if (ist_unten == 1) {
				*p_state = steht; // Wechsel in den Zustand "steht"
            }else if(nach_oben_wegen_zeit != 0){// Zeitsteuerung
                *p_state = hoch;
            }




			break;

		case hoch:

			// 7a.)  Ausgabesignale bestimmen 
            *p_fahre_nach_unten=0; *p_fahre_nach_oben=1;

			// 8.)    Eingabesignale auswerten und Zustandswechsel herbei fuehren
			if (ist_oben == 1){
				*p_state = steht; // Wechsel in den Zustand "steht"
            }else if(nach_unten_wegen_zeit != 0){// Zeitsteuerung
                *p_state = runter;
            }


			break;

		// 9.) nicht erlaubte Zustaende "abfangen"
		default:
			*p_state = runter;

	} // end switch	
} // end steuerungsfunktion()

/**
 * @brief compare_Time
 * @param eins
 * @param zwei
 * @return 1 wenn die zeiten ungleich sind und 0 wenn die Zeiten gleich sind
 */
USHORT compare_Time(uhrzeit eins, uhrzeit zwei){
    if( eins.hh == zwei.hh &&
        eins.mm == zwei.mm &&
        eins.ss == zwei.ss)
        return 0;
    return 1;
}

void timer1_init()
{
    /*	Zur Berechnung der Werte fuer den Prescaler und den Compare-Match:
        Bei einer Frequenz von 4 Mhz zaehlt der Timer mit einem Takt von 0,25us.
        Er kann mit seinen 65536 moeglichen Zaehlstaenden eine maximale Interrupt-Periode von 65536 * 0,25us = 16384us realisieren.
        Dies ist zu schnell. => Der Zaheler muss mittels des Prescalers langsamer eingestellt werden.
        Die ideale Untersetzung waere  50000us / 16384us = 3,0517.
        Da es diese Unterssetzung (Prescaler-Wert) nicht gibt waehlen wir den naechst groesseren Wert also 8.
        Der Zaehler zaehlt jetzt mit einem Takt vom 2us. => Die Interrupts haben bei einem Compare-Match-Wert von 65535
        eine Periode von 131072 us.Der Compare-Match-Wert muss auf 50000us/131072us*65536us = 25000 eingestellt werden.
    */

    unsigned short buf = 0;

    // TCRA1
    // Clock Source auf intern/8 Prescaler
    // Timer Modus Clear-Timer-On-Compare-Match
    buf = (1 << PS11) | (1 << TM10);
    io_out16(TCRA1, buf);

    // TCRB1
    // Counter Enable
    buf = (1 <<CE1);
    io_out16(TCRB1, buf);

    // TIMR1
    // Compare Match Interrupt enable
    buf = (1 << OCIE1);
    io_out16(TIMR1, buf);

    // CMPA1
    // Compare Match Register auf ...
    buf = 19000;
    io_out16(CMPA1, buf);
}

// Makros damit man beim Testen nicht so lange warten muss
// Die korrekten Werte stehen als Kommentare jeweils dahinter
#define SS_MAXW	5	//60
#define MM_MAXW	3	//60
#define HH_MAXW	3//24

//	Interrupt Service Routine
void timer1_oco1_isr()
{
    unsigned short int buf;
    akt_zeit.ss ++;

    if(akt_zeit.ss >= SS_MAXW){
        akt_zeit.ss = 0;
        akt_zeit.mm ++;

        if(akt_zeit.mm >= MM_MAXW){
            akt_zeit.mm = 0;
            akt_zeit.hh ++;

            if(akt_zeit.hh >= HH_MAXW){
                akt_zeit.hh = 0;
            }
        }
    }
    // Zuruecksetzen des Interrupt-Flags
    buf = io_in16(TIFR1);
    buf = buf & ~(1 << OCIF1);
    io_out16(TIFR1, buf);

}


void emain(void* arg) 
{
	STATE cstate;		// Variable für den Zustand das Automaten.

	// Variablen für die Eingabesignale. Eine Variable fuer jedes Signal.
	USHORT		nach_oben;  
	USHORT		nach_unten;
	USHORT		ist_oben;
	USHORT		ist_unten;
    USHORT      nach_oben_wegen_zeit, nach_unten_wegen_zeit;

	// Variablen für die Ausgabesignale. Eine Variable fuer jedes Signal.
	USHORT		fahre_nach_oben;
	USHORT		fahre_nach_unten;
	
	USHORT		input, output, last_output;

    //Hilfs Variablen
    unsigned short buf = 0;
    unsigned char stringbuf[100];


	INIT_BM_WITH_REGISTER_UI;	// Nur fuer Simulation


	// 1.)	Hardware konfigurieren
	io_out16(DIR1, 0xFF00); // Ausgang: Bits 15 bis 8   Eingang: Bits 7 bis 0 

	// 2.)	 Definition des Startzustandes. Entspricht dem asynchronen Reset in VHDL.

	cstate = runter;

    //Zeit variablen bestimmen
    hoch_zeit.hh = 1;
    hoch_zeit.mm = 0;
    hoch_zeit.ss = 3;

    runter_zeit.hh = 2;
    runter_zeit.mm = 0;
    runter_zeit.ss = 3;

    //Timer set
    // Zur Sicherheit vor Initialisierung den Interupt des PIC generell deaktivieren
    buf = io_in16(PICC);
    buf = buf &  ~(1 << PICE);
    io_out16(PICC, buf);

    // Timer 1 initialisieren
    timer1_init();

    // ISR registrieren
    setInterruptHandler(IVN_OC1, timer1_oco1_isr);

    // Uhrzeiterfragen ohne weitere Ueberpruefung
    putstring("Bitte die aktuelle Uhrzeit im Format hh:mm:ss eingeben\n");
    getstring(stringbuf);
    sscanf(stringbuf,"%d:%d:%d",&(akt_zeit.hh), &(akt_zeit.mm), &(akt_zeit.ss));

    // Interrupt des PIC jetzt zulassen
    buf = buf | (1 << PICE);
    io_out16(PICC, buf);

	// 3.) Unendliche Schleife. Ein Schleifendurchlauf entspricht einem Zyklus des Automaten
	while (1) { 
	
		SYNC_SIM; // Nur fuer Simulation

		// 4.)	Einlesen der Eingabesignale einmal je Zyklus
		input = io_in16(IN1);

		// extrahieren von "ist_oben" (BIT_POS_IST_OBEN)
		ist_oben = (input >> BIT_POS_IST_OBEN) & 0x01;

		// extrahieren von "ist_unten" (BIT_POS_IST_UNTEN)
		ist_unten = (input >> BIT_POS_IST_UNTEN) & 0x01;

		// extrahieren von "nach_oben" (BIT_POS_NACH_OBEN)
		nach_oben = (input >> BIT_POS_NACH_OBEN) & 0x01;

		// extrahieren von "nach_unten" (BIT_POS_NACH_UNTEN)
		nach_unten = (input >> BIT_POS_NACH_UNTEN) & 0x01;

        //Zeit steuer Signale bestimmern
        if(compare_Time(akt_zeit,hoch_zeit) == 0){//fahre hoch
            nach_oben_wegen_zeit = 1;
        }else{
            nach_oben_wegen_zeit = 0;
        }

        if(compare_Time(akt_zeit,runter_zeit) == 0){//fahre hoch
            nach_unten_wegen_zeit = 1;
        }else{
            nach_unten_wegen_zeit = 0;
        }
		
		// Aufruf der Steuerungsfunktion
		steuerungsfunktion    (	ist_oben, ist_unten, nach_oben, nach_unten,
                            &fahre_nach_oben, &fahre_nach_unten, &cstate,nach_oben_wegen_zeit,nach_unten_wegen_zeit	);


		// 7b.) Ausgabesignale ausgeben
		output=fahre_nach_unten<<BIT_POS_FAHRE_NACH_UNTEN;  
		output=output | (fahre_nach_oben<< BIT_POS_FAHRE_NACH_OBEN); 


		// Nur wirklich ausgeben wenn notwendig
		// Optimierung mittels bedigter Portausgabe
		if (last_output != output) {
			output=output | (1<< BIT_POS_MOTOR_AN);   // Nur fuer Bandmodell
			io_out16(OUT1, io_in16(OUT1) & 0x00FF);
			io_out16(OUT1, io_in16(OUT1) |  output);
			last_output = output;
		}

        //Zeit ausgeben
        sprintf(stringbuf,"Uhrzeit: %d:%d:%d\n",akt_zeit.hh,akt_zeit.mm,akt_zeit.ss);
        putstring(stringbuf);
	} // end while
} // end main
#endif // V3_Aufgabe_1


#ifdef V3_Aufgabe_2_und_3
#include "user_conf.h"

typedef struct {
	unsigned char hh; 
	unsigned char mm; 
	unsigned char ss; 
} uhrzeit; 




void do_param(unsigned char* auszuwertende_nachricht, uhrzeit* akt, uhrzeit* hoch, uhrzeit* runter) {
	// Diese Funktion muss noch mit Leben gefuellt werden.
	// ...
}



// Vorgegebene globale Variablen und Makros 
// Diese sind zum Teil auch in den Loesungen zu den relevanten Uebungsaufgaben schon vorhanden
#define MAX_MESSAGE_SIZE 100

unsigned char		byte_received;
unsigned char		nachricht[MAX_MESSAGE_SIZE];
unsigned char		flag_ready;
uhrzeit akt_zeit, hoch_zeit, runter_zeit;

// Hier muessen noch weitere globale Variablen und Makros eingefuegt werden
// ...
unsigned char buf;

void init_spi1(){ //SPI1 als slave konfigurieren
    io_out8(SPCR1,(1<<SPE1) | (1<<SPIE1));
    //Interupts Enablen
    io_out16(PICC,1<PICE)
}


/**
 * @brief byteEmpfangeIsr
 * Interupt Service Routine speichert das empfange Byte in byte_recieved
 * und setzt das SPI Interupt Flag auf 0
 */
void  byteEmpfangenIsr(){
    //Byte einlesen
    byte_received = io_in8(SPDR1);
    //Spi Interupt Flag auf 0 setzen
    buf = io_in8(SPSR1);
    buf = buf & ~(1<< SPIF1);
    io_out8(SPSR1,buf);
}

void emain(void* arg)
{
	char string[300];
	 
	INIT_BM_WITH_REGISTER_UI;

	// Hier die notwendigen Initialisierungen einfuegen.
	// ...
    init_spi1();
    setInterruptHandler(IVN_SPI1,byteEmpfangenIsr);
    //setInterruptHandler(IVN_SPI2,byteEmpfangenIsr);
    
	while(1) {
#ifndef USER_PROG_2
		putstring("Sie haben USER_PROG_2 nicht definiert\n");
#endif
		if(flag_ready==1){
			putstring((char*)nachricht);
			putstring("\n");

			sprintf(string, "Akt:%d:%d:%d  Hoch:%d:%d:%d  Runter:%d:%d:%d\n", akt_zeit.hh,akt_zeit.mm,akt_zeit.ss, hoch_zeit.hh,hoch_zeit.mm,hoch_zeit.ss, runter_zeit.hh,runter_zeit.mm,runter_zeit.ss);
			putstring(string);
			flag_ready=0;
		}	
	}
		
}



//################AB HIER STEHT ALLES FUER DAS SENDER-PROGRAMM #################################################

void init_spi2(){//Als Master konfiguriere
    io_out8(SPCR2,((1<<SPE1) | (1<<MSTR1)))
}

void emain_sender(void* arg)
 {
	 unsigned char i;
	 unsigned char parametriere_akt_zeit[]		=	"#A000005";
	 unsigned char parametriere_hoch_zeit[]		=	"#B000105";
	 unsigned char parametriere_runter_zeit[]	=	"#C000159";
     unsigned char buffer = 0;
	 
	 // Hier die notwendigen Initialisierungen einfuegen.
	 // ...
    init_spi2();


	 while(1) { 
		 i=0;
		 do  {
                
			 // Hier den Code fuer das Versenden eines Bytes einfuegen
			 // ...

             // Slave Selektieren
             buffer = io_in8(SPCR2);
             buffer = buffer & ~(1 << notSS2);
             io_out8(SPCR2,buffer);

             // Zu sendendes Byte in shift register schreiben
            io_out8(SPDR2,parametriere_akt_zeit[i]);
             //warte bis übertragung beendet
            do{
                buffer = io_in8(SPSR2);
                buffer = buffer >> SPIF2;
                buffer = buffer & 0x01;
            }while(buffer == 0);


			 // Damit der Empfaenger genuegen Zeit zum Reagieren (Einlesen des Bytes) hat
			 // muss hier geweartet werden.
			 ms_wait(10);	

             //Slave deselctieren
             buffer = io_in8(SPCR2);
             buffer = buffer | (1 << notSS2);
             io_out8(SPCR2,buffer);

			 i++;
             if(parametriere_akt_zeit[i] == 0){//abfragen ob das String ende Zeichen erreicht wurde
                 i = 0;
             }
         } while(i<127);

	  }
		
 }


#endif //V3_Aufgabe_2_und_3


 #ifdef nachrichtenempfang_ueber_ports
// Sinnvoll zu nutzende Makros
#define COM_SIGNAl_PIN				0		// Pin ueber den der Interrupts ausgeloest wird
#define COM_DATA_IN_REGISTER		IN0		// Register ueber den das Byte eingelesen wird
#define MAX_MESSAGE_SIZE			100		// Maximale Laenge einer Nachricht
#define STARTBYTE					0xFE	// Wert des Start-Bytes
#define ENDBYTE						0xEF	// Wert des Ende-Bytes


typedef enum {warte_auf_start_byte, warte_auf_end_byte} STATE_N;


// Globale Variablen fuer die ISR
unsigned char		nachricht[MAX_MESSAGE_SIZE];
unsigned char		flag_ready;
STATE_N				comstate=warte_auf_start_byte;
unsigned long int	byte_counter;



void init_gpio_0_1()
{
	unsigned short hilfe = 0;
	
	// ### PORT 1
	// Interrupt fuer Bit 0 von PORT1 enable
	hilfe = io_in16(EIE1) | (1 << COM_SIGNAl_PIN); 
	io_out16(EIE1, hilfe); 
	
	// Das Bit 0 von PORT 1 ist Eingang
	hilfe = io_in16(DIR1) & ~(1 << COM_SIGNAl_PIN); 
	io_out16(DIR1, hilfe);  


	// ### PORT 0
	// Die unter 8 Bit von PORT0 sind Eingang
	hilfe = io_in16(DIR0) & 0xFF00;

	io_out16(DIR1, hilfe); 
		
}


void steuerungsfunktion(	unsigned char byte_received, unsigned long int* byte_zaehler, unsigned char* 						empfangene_nachricht, unsigned char* ready, STATE_N* state)
{
	
	switch (*state) {

		case warte_auf_start_byte:
			// Uebergang nach warte_auf_end_byte
			if ( byte_received == STARTBYTE) {
				// Etwas tun am Uebergang.
				*byte_zaehler=0;
				empfangene_nachricht[*byte_zaehler]=byte_received;
				*byte_zaehler=*byte_zaehler+1;

				// Zustandswechsel
				*state = warte_auf_end_byte; 
			}
			break;

		case warte_auf_end_byte:	
			// Uebergang nach warte_auf_start_byte
			if (byte_received == ENDBYTE)  {
				// Etwas tun am Uebergang.
				empfangene_nachricht[*byte_zaehler]=byte_received;
				*byte_zaehler=*byte_zaehler+1;
	
				*ready=1;

				// Zustandwechsel
				*state = warte_auf_start_byte; 
			}

			if (*byte_zaehler == MAX_MESSAGE_SIZE-1) {
				// Die Nachricht ist zu lang und kann dahr nicht gueltig sein!
				// Zustandwechsel
				*state = warte_auf_start_byte; 
			}

			// Uebergang auf sich selbst nur damit etwas getan wird.
			if (byte_received == STARTBYTE) {	
				// Etwas tun am Uebergang.
				*byte_zaehler=0;
				empfangene_nachricht[*byte_zaehler]=byte_received;
				*byte_zaehler=*byte_zaehler+1;

				// Zustandwechsel
				*state = warte_auf_end_byte;	// Ist ueberfluessing dient aber hoffentlich 
										// dem Verstaendnis
			}

			// Uebergang auf sich selbst nur damit etwas getan wird.
			if ((byte_received != STARTBYTE) && (byte_received != ENDBYTE) 
				&& (*byte_zaehler < MAX_MESSAGE_SIZE-1)) {		
				// Etwas tun am Uebergang.
				empfangene_nachricht[*byte_zaehler]=byte_received;
				*byte_zaehler=*byte_zaehler+1;

				// Zustandwechsel
				*state = warte_auf_end_byte;	// Ist ueberfluessing dient aber hoffentlich 
										// dem Verstaendnis
			}

			break;

		// 9.) nicht erlaubte Zustaende "abfangen"
		default:
			*state = warte_auf_start_byte;
	} // switch

}



void ISR_EXT_INT0()
{
	unsigned short int hilfe = 0;
	unsigned char buf;

	// Einlesen des Datenbytes
	buf=(unsigned char) (io_in16(COM_DATA_IN_REGISTER) & 0x00FF);

	// Aufruf der Steuerungsfunktion
	steuerungsfunktion(buf, &byte_counter, &(nachricht[0]), &flag_ready, &comstate);

	// Zureucksetzen des Interrupt-Flags
	hilfe = io_in16(EIF1) & ~(1<<COM_SIGNAl_PIN);
	io_out16(EIF1, hilfe);
	return;
}


void emain(void* arg)
{
	 unsigned short int buf;

	 INIT_REGISTER_UI_WITHOUT_BM	

	 // Zur Sicherheit vor Initialisierung den Interrupt des PIC generell deaktivieren 
	 buf = io_in16(PICC); 
	 buf = buf &  ~(1 << PICE); 
	 io_out16(PICC, buf); 

	 // Initialisieren der Ports
	 init_gpio_0_1();

	 // Registrieren der ISRs in der Interupt-Vektor-Tabelle
	 setInterruptHandler(IVN_EI100, ISR_EXT_INT0);
	
	 // Interrupt des PIV jetzt zulassen.
	 buf = buf | (1 << PICE); 
	 io_out16(PICC, buf); 
	

	 while(1)
	 {
		
	 }
	
}
#endif // nachrichtenempfang_ueber_ports


#ifdef timer_als_taktsignalgenerator

// Globale Variablen zur Kommunikation mit der ISR
unsigned long int zt=0, hh=0, mm=0, ss=0;

void timer1_init()
{
	/*	Zur Berechnung der Werte fuer den Prescaler und den Compare-Match:
		Bei einer Frequenz von 4 Mhz zaehlt der Timer mit einem Takt von 0,25us.
		Er kann mit seinen 65536 moeglichen Zaehlstaenden eine maximale Interrupt-Periode von 65536 * 0,25us = 16384us realisieren.
		Dies ist zu schnell. => Der Zaheler muss mittels des Prescalers langsamer eingestellt werden.
		Die ideale Untersetzung waere  50000us / 16384us = 3,0517.
		Da es diese Unterssetzung (Prescaler-Wert) nicht gibt waehlen wir den naechst groesseren Wert also 8.
		Der Zaehler zaehlt jetzt mit einem Takt vom 2us. => Die Interrupts haben bei einem Compare-Match-Wert von 65535 
		eine Periode von 131072 us.Der Compare-Match-Wert muss auf 50000us/131072us*65536us = 25000 eingestellt werden.
	*/

	unsigned short buf = 0;
	
	// TCRA1
	// Clock Source auf intern/8 Prescaler
	// Timer Modus Clear-Timer-On-Compare-Match
	buf = (1 << PS11) | (1 << TM10);
	io_out16(TCRA1, buf);

	// TCRB1
	// Counter Enable
	buf = (1 <<CE1);
	io_out16(TCRB1, buf);

	// TIMR1
	// Compare Match Interrupt enable
	buf = (1 << OCIE1);
	io_out16(TIMR1, buf);

	// CMPA1
	// Compare Match Register auf ...
	buf = 25000;
	io_out16(CMPA1, buf);
} 

// Makros damit man beim Testen nicht so lange warten muss
// Die korrekten Werte stehen als Kommentare jeweils dahinter
#define ZT_MAXW	3	//20
#define SS_MAXW	3	//60
#define MM_MAXW	3	//60
#define HH_MAXW	3	//24

//	Interrupt Service Routine
void timer1_oco1_isr()
{
	unsigned char stringbuf[100];
	unsigned short int buf;

	zt++;

	if(zt==ZT_MAXW){
		zt=0;
		ss++;
	}

	if(ss==SS_MAXW){
		ss=0;
		mm++;
	}

	if(mm==MM_MAXW){
		mm=0;
		hh++;
	}
	if(hh==HH_MAXW){
		hh=0;
	}

	sprintf(stringbuf,"zt=%d  Uhrzeit: %d:%d:%d\n",zt,hh,mm,ss);
	// Achtung die Verwendung von putstring() kann in der Simulationsumgebung
	// zu einem Deadlock fuehren, d.h. alles bleibt einfach stehen.
	// Dahr ist dies hier (zur Sicherheit) auskommentiert.
	// putstring(stringbuf);

	// Zuruecksetzen des Interrupt-Flags
	buf = io_in16(TIFR1);
	buf = buf & ~(1 << OCIF1);
	io_out16(TIFR1, buf);

}



 void emain(void* arg)
 {
	unsigned short buf = 0;
	unsigned char stringbuf[100];

	INIT_BM_WITH_REGISTER_UI; // Nur zur Simulation

	// Zur Sicherheit vor Initialisierung den Interupt des PIC generell deaktivieren
	buf = io_in16(PICC);
	buf = buf &  ~(1 << PICE);
	io_out16(PICC, buf);

	// Timer 1 initialisieren
	timer1_init();

	// ISR registrieren
	setInterruptHandler(IVN_OC1, timer1_oco1_isr);

	// Uhrzeiterfragen ohne weitere Ueberpruefung
	putstring("Bitte die aktuelle Uhrzeit im Format hh:mm:ss eingeben\n");
	getstring(stringbuf);
	sscanf(stringbuf,"%d:%d:%d",&hh, &mm, &ss);

	// Interrupt des PIC jetzt zulassen
	buf = buf | (1 << PICE);
	io_out16(PICC, buf);

	while(1);	
 }
#endif // timer_als_taktsignalgenerator
