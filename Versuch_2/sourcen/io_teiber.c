#include "simuc.h"
#include "io_treiber.h"

#include <stdio.h>

typedef struct BHandle_Data {
	BYTE Board_allocated;	// 1=allocated, 0=free
	BYTE Port_A_Direction;	// 1=Output, 0=Input
	BYTE Port_B_Direction;
	BYTE Port_C_Direction;
	BYTE Port_D_Direction;
} BHandle;



DSCB GlobalBoardHandle = NULL;  // Dies ist ein Zeiger (siehe typedef BHandle* DSCB in io_treinber.h),
                                // der noch auf einen gueltigen Speicherbereich "gebogen" werden muss.
                                // Dieser Speicherbereich muss dann in Init() mittels calloc() allokiert
                                // werden.



BYTE Init(DSCB* Zeiger_auf_BoardHandle, unsigned long int Steuerwort) {

    DSCB Lokales_BoardHandle;   // Es wird empfohlen innerhalb dieser Funktion mit diesem
                                // lokalen BoardHandle zu arbeiten. Ein Zugriff auf die
                                // Stukturkomponenten ist dann ganz normal, z.B. mittels
                                // Lokales_BoardHandle->Borad_allocated, moeglich.


    if (*Zeiger_auf_BoardHandle == NULL){	
        // Allokieren des Speichersbereiches und "biegen"  des lokalen Boadhandle auf diesen
        // Speicherbereich.
        Lokales_BoardHandle = (DSCB) calloc(1, sizeof(BHandle));

        // Auch den per Call-By-Reference uebergebenen Zeiger auf den reservierten Speicherbereich "biegen"
        *Zeiger_auf_BoardHandle = Lokales_BoardHandle;
        Lokales_BoardHandle -> Board_allocated = 1;
        GlobalBoardHandle = Lokales_BoardHandle;
    }else{
        Lokales_BoardHandle = *Zeiger_auf_BoardHandle;
    }

    // Ab hier nur noch mit dem lokalen BoardHande arbeiten
    // Mit Leben fuellen
    switch(Steuerwort){
    case 0x92:
        Lokales_BoardHandle->Port_A_Direction = 0;//Input
        Lokales_BoardHandle->Port_B_Direction = 0;//Input
        Lokales_BoardHandle->Port_C_Direction = 1;//Output
        Lokales_BoardHandle->Port_D_Direction = 1;//Output
        break;
    case 0x93:
        Lokales_BoardHandle->Port_A_Direction = 0;//Input
        Lokales_BoardHandle->Port_B_Direction = 0;//Input
        Lokales_BoardHandle->Port_C_Direction = 1;//Output
        Lokales_BoardHandle->Port_D_Direction = 0;//Input
        break;
    case 0x9A:
        Lokales_BoardHandle->Port_A_Direction = 0;//Input
        Lokales_BoardHandle->Port_B_Direction = 0;//Input
        Lokales_BoardHandle->Port_C_Direction = 0;//Input
        Lokales_BoardHandle->Port_D_Direction = 1;//Output
        break;
    case 0x9B:
        Lokales_BoardHandle->Port_A_Direction = 0;//Input
        Lokales_BoardHandle->Port_B_Direction = 0;//Input
        Lokales_BoardHandle->Port_C_Direction = 0;//Input
        Lokales_BoardHandle->Port_D_Direction = 0;//Input
        break;
    default:
        return 1;
    }
    //Ports configurieren


    unsigned short dir = 0;

    //Port 0 des Boards Konfiguieren
        //Port A
        if(Lokales_BoardHandle->Port_A_Direction == 1){//Port 0 Bit 0-7 als Output Konfigurieren
            dir = dir | 0xFF;
        }//Port 0 Bit 0-7 sind ohne das if bereits als input in dir konfiguriert

        //Port B
        if(Lokales_BoardHandle->Port_B_Direction == 1){//Port 0 Bit 8-15 als Output Konfigurieren
            dir = dir | 0xFF00;
        }//Port 0 Bit 8-15 sind ohne das if bereits als input in dir konfiguriert

    //Dir Register von Port 0 konfigurieren
        io_out16(DIR0,dir);

    //Port 1 des Boards Konfigurieren
        dir = 0;//hilfs variable wieder auf null setzen
        //Port C
        if(Lokales_BoardHandle->Port_C_Direction == 1){//Port 1 Bit 0-7 als Output Konfigurieren
           dir = dir | 0xFF;
        }//Port 1 Bit 0-7 sind ohne das if bereits als input in dir konfiguriert

        //Port D
        if(Lokales_BoardHandle->Port_D_Direction == 1){//Port 1 Bit 8-15 als Output Konfigurieren
            dir = dir | 0xFF00;
        }//Port 1 Bit 8-15 sind ohne das if bereits als input in dir konfiguriert
    //Dir Register von Port 1 konfigurieren
        io_out16(DIR1,dir);

	return 0;
}

BYTE InputByte(DSCB BoardHandle, BYTE Port, BYTE *DigitalValue) {
    
    // Mit Leben fuellen

    if(BoardHandle == NULL && BoardHandle->Board_allocated == 0){//ungültiges Board Handel
        return 1;
    }

    switch(Port){
        case PA:
                *DigitalValue = io_in16(IN0);//Port 0 einlesen, das niederwertige bit wird automatisch isoliert da Byte nur ein Byte aufnehmen kann
                return 0;
            break;
        case PB:
                *DigitalValue = io_in16(IN0) >> 8;//Port 0 einlesen und das höherwertige bit isolieren
                return 0;
            break;

        case PC:
                *DigitalValue = io_in16(IN1);//Port eins einlesen , das niederwertige bit wird automatisch isoliert da Byte nur ein Byte aufnehmen kann
                return 0;
            break;
        case PD:
                *DigitalValue = io_in16(IN1)>> 8;//Port eins einlesen und das höherwertige bit isolieren
                return 0;
            break;
        default:
            return 2;
    }
}

BYTE OutputByte(DSCB BoardHandle, BYTE Port, BYTE DigitalValue) {

    if(BoardHandle == NULL && BoardHandle->Board_allocated == 0){//ungültiges Board Handel
        return 1;
    }

    unsigned short portValue;
    switch(Port){
    case PA:
        if(BoardHandle->Port_A_Direction == 1){//Ueberpruefen ob Port auf schreiben konfiguriert ist
            portValue = io_in16(OUT0);//wert von out Put einlesen
            portValue = portValue & 0xFF00;//niederwertigen Byte auf null setzen
            portValue = portValue | DigitalValue;//Digital Value einsetzen
            io_out16(OUT0,portValue);
            return 0;
        }else{
            return 3;//Port ist nicht auf ausgang konfiguriert
        }
        break;
    case PB:
        if(BoardHandle->Port_B_Direction == 1){//Ueberpruefen ob Port auf schreiben konfiguriert ist
            portValue = io_in16(OUT0);//wert von out Put einlesen
            portValue = portValue & 0x00FF;//niederwertigen Byte auf null setzen
            portValue = portValue | (DigitalValue<<8);//Digital Value einsetzen
            io_out16(OUT0,portValue);
            return 0;
        }else{
            return 3;//Port ist nicht auf ausgang konfiguriert
        }
        break;
    case PC:
        if(BoardHandle->Port_C_Direction == 1){//Ueberpruefen ob Port auf schreiben konfiguriert ist
            portValue = io_in16(OUT1);//wert von out Put einlesen
            portValue = portValue & 0xFF00;//niederwertigen Byte auf null setzen
            portValue = portValue | DigitalValue;//Digital Value einsetzen
            io_out16(OUT1,portValue);
            return 0;
        }else{
            return 3;//Port ist nicht auf ausgang konfiguriert
        }
        break;
    case PD:
        if(BoardHandle->Port_D_Direction == 1){//Ueberpruefen ob Port auf schreiben konfiguriert ist
            portValue = io_in16(OUT1);//wert von out Put einlesen
            portValue = portValue & 0x00FF;//niederwertigen Byte auf null setzen
            portValue = portValue | (DigitalValue<<8);//Digital Value einsetzen
            io_out16(OUT1,portValue);
            return 0;
        }else{
            return 3;//Port ist nicht auf ausgang konfiguriert
        }
        break;
    default:
        return 2;
    }

	return 0;
}

BYTE Free(DSCB BoardHandle) {
    if(BoardHandle == NULL){//ungültiges Board Handel
        return 1;
    }
    BoardHandle->Board_allocated = 0;
	return 0;
}

