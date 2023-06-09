#include "simuc.h"
#include "io_treiber.h"


// Die Verwendung der folgenden Zustandsnamen ist verbindlich
typedef enum {hoch, runter, steht} STATE;

// Die folgenden Defines muessen sinnvoll genutzt werden
// Einige Defines zum leichteren Wiederfinden
#define BIT_POS_IST_OBEN	0
#define BIT_POS_IST_UNTEN	1
#define BIT_POS_NACH_OBEN	4
#define BIT_POS_NACH_UNTEN	5
#define BIT_POS_NOTAUS 7

#define BIT_POS_MOTOR_AN	0
#define BIT_POS_FAHRE_NACH_OBEN	1
#define BIT_POS_FAHRE_NACH_UNTEN	2
#define BIT_POS_LED 3


// Hier die Steuerungsfunktion definieren
// ...
void steuerungsfunktion(BYTE ist_oben,BYTE ist_unten,BYTE nach_oben,BYTE nach_unten,BYTE notaus,BYTE* fahre_nach_oben,BYTE* fahre_nach_unten,BYTE* led,STATE* state){
    switch(*state){
    case hoch:
        //ausgabe signale bestimmen
        *fahre_nach_oben = 1;
        *fahre_nach_unten = 0;
        *led = 1;

        if(ist_oben == 1 || notaus == 1){
            *state = steht;
        }
        break;
    case runter:
        //ausgabe signale bestimmen
        *fahre_nach_oben = 0;
        *fahre_nach_unten = 1;
        *led = 1;

        if(ist_unten == 1 || notaus == 1){
            *state = steht;
        }
        break;
    case steht:
        //ausgabe signalebestimmen
        *fahre_nach_oben = 0;
        *fahre_nach_unten = 0;
        *led = 0;
        if((nach_oben == 1)&& (ist_oben == 0) && (notaus == 0)){
            *state = hoch;
        }else if((nach_unten == 1) && (ist_unten == 0) && (notaus == 0)){
            *state = runter;
        }
        break;
    default:
        *state = runter;
        break;
    }
}

void emain(void* arg) 
{

    // Hier alle benoetigten Variablen deklarieren
    //..
    BYTE nach_unten,nach_oben,ist_unten,ist_oben,notaus;
    BYTE  fahre_nach_unten = 0;
    BYTE fahre_nach_oben = 0;
    BYTE led = 0;

    BYTE portC;
    BYTE portD_nullMaske = ~(0 |(1 <<BIT_POS_FAHRE_NACH_UNTEN) | (1 <<BIT_POS_FAHRE_NACH_OBEN) | (1 <<BIT_POS_MOTOR_AN)); // Maske mit allen bits die editiert werden null
    BYTE portD_output;
    STATE state = steht;
    //Variablen zum teste
    /*
    BYTE Port_outPut = 0;
    unsigned short temp = 0;*/

	INIT_BM_WITH_REGISTER_UI; // Hier unbedingt einen Break-Point setzen !!!

	
	// Hier die Treiberfunktionen aufrufen und testen (Aufgabe 1)
    // ..
    Init(&GlobalBoardHandle,0x9A);



	// Ab hier beginnt die Endlosschleife fuer den Automaten (Aufgabe 2)
    while(1) {

		SYNC_SIM; 

		// Hier die Eingabesignale einlesen
		// ...
        InputByte(GlobalBoardHandle,PC,&portC);
        nach_unten = (portC>>BIT_POS_NACH_UNTEN) & 0x1;
        nach_oben = (portC>>BIT_POS_NACH_OBEN) & 0x1;
        ist_unten = (portC>>BIT_POS_IST_UNTEN) & 0x1;
        ist_oben = (portC>>BIT_POS_IST_OBEN) & 0x1;
        notaus = (portC>>BIT_POS_NOTAUS) & 0x1;
        /*
         //Test InputByte Methode
        InputByte(BoardHandle,0,&Port_outPut);
        InputByte(BoardHandle,1,&Port_outPut);
        InputByte(BoardHandle,2,&Port_outPut);
        InputByte(BoardHandle,3,&Port_outPut);
        short temp = 0;
        */
        /*
        //Test OutPutByte Methode
        temp++;
        OutputByte(BoardHandle,PC,temp);
        temp++;
        OutputByte(BoardHandle,PD,temp);
        */



		// Hier die Steuerungsfunktion aufrufen
		// ...
        steuerungsfunktion(ist_oben,ist_unten,nach_oben,nach_unten,notaus,&fahre_nach_oben,&fahre_nach_unten,&led,&state);



		// Hier die Ausgabesignale ausgeben
        //Port D einlesen
        //um den prozess zu beschleunigen könnte man diese zeile auch einmal vor der endlos schleife einlesen und dann immer die werte des vorherigen durchlaufs verwenden,
        //allerdings könnte es sein das die Bits noch anderweitig verwendet werden un deshalb lese ich sie zur sicherheit hier nochmal ein
        InputByte(GlobalBoardHandle,PD,&portD_output);//Port D einlesen
        portD_output = portD_output & portD_nullMaske;;//editierbaren bits auf 0 setzen
        portD_output = portD_output | ((fahre_nach_unten <<BIT_POS_FAHRE_NACH_UNTEN) | (fahre_nach_oben <<BIT_POS_FAHRE_NACH_OBEN) | (led <<BIT_POS_LED) | (1<<BIT_POS_MOTOR_AN));
         OutputByte(GlobalBoardHandle,PD,portD_output);

	} // while(1)..

	
}



  
