#ifndef RS485_h
#define RS485_h

#include "Arduino.h"

class RS485 {
	private://*** PRIVATE ***//	
	  Stream *mySerial;
    HardwareSerial *hs;
		uint8_t com_ctrl_pin_in;		//Transmitdeaktivierung, Eingang: Wenn HIGH ist es nicht möglich zu senden
		uint8_t com_ctrl_pin_out;		//Transmitdeaktivierung, Ausgang: Wird HIGH gesetzt für Transmitdeaktivierung anderer
		uint8_t tx_rx_ctrl_pin;			//DE Pin Steuerung des Tx Pin RE Pin Steuerung des Rx Pin
		String uart_address;			//Bus Adresse
		char uart_find_address[10];		//Bus Adresse
		void transmit_routine(uint8_t highlow){
      //Auszuführende Routine vor und nach dem Senden von Daten
      //über die UART-Schnittstelle in Verbindung mit dem MAX485 Chip
      digitalWrite(com_ctrl_pin_out,highlow);
      digitalWrite(tx_rx_ctrl_pin,highlow);
    }
	public: //*** PUBLIC ***//

    /***************************************************************************
     * Initaliserung der RS485
     *    Paramter[in]  &port
     *                    SerialPort: Serial, Serial1, Serial2, Serial3
     ***************************************************************************/    
    RS485(HardwareSerial &port, uint8_t com_ctrl_pin_in, uint8_t com_ctrl_pin_out, uint8_t tx_rx_ctrl_pin){
      hs = &port;
      mySerial = &port;
      /* Defintion der Steuerpins und der RS485 Adresse */
      this->uart_address = uart_address;
      this->com_ctrl_pin_in = com_ctrl_pin_in;
      this->com_ctrl_pin_out = com_ctrl_pin_out;
      this->tx_rx_ctrl_pin = tx_rx_ctrl_pin;
    }

    /***************************************************************************
     * Initaliserung RS485
     *    Paramter[in]  Baudrate in Bd
     *                    Übertragungsgeschwindigkeit UART
     *                  Timeout in ms  
     *                    Wartezeit auf Daten 
     ***************************************************************************/    
	  void init(String uart_address, uint32_t BAUDRATE, uint32_t TIMEOUT = 5){
      uart_address.toCharArray(uart_find_address, 10);
      if(BAUDRATE > 115200){ BAUDRATE = 115200; } //Höhere Baudraten unterstützt der Arduino nicht
      hs -> begin(BAUDRATE);                      //Initialisierung des UART-Ports 1 mit der eingebenen Baudraten
      mySerial -> setTimeout(TIMEOUT);            //Wartedauer bis Daten eintreffen
      //***Deklarieren der Pins als Aus- und Eingänge
      pinMode(com_ctrl_pin_in,INPUT);   
      pinMode(com_ctrl_pin_out,OUTPUT);  
      pinMode(tx_rx_ctrl_pin,OUTPUT); 
      digitalWrite(com_ctrl_pin_out,LOW);         //Schaltet den Com_Ctrl_Pin inaktiv, da keine Daten übertragen werden
      digitalWrite(tx_rx_ctrl_pin,LOW);           //Schaltet Rx Pin aktiv, damit jederzeit Datenempfangen werden können
                    //Schaltet Tx Pin inaktiv, damit jederzeit Datenempfangen werden können
                    //ist tx_ctrl_pin auf HIGH können keine Daten mehr empfangen werden
    }
    /***************************************************************************
     * 
     *   
     *                   
     *               
     *    
     ***************************************************************************/   
    uint8_t transmit(String rs485_receiver_address, uint8_t index, uint8_t sub_index, uint8_t num_of_data, uint8_t data[62]){
            //Adresse des Empfängers   , Datenthema , Dateninformation ,Datenanzahl     ,  Daten
      //***Abfrage ob ein anderer Teilnehmer gerade den Datenbus belegt
      uint32_t loop_counter = 0; //Schleifenzähler, für Durchlaufbegrenzung
      //Prüfen ob ein anderer Teilnehmer bereits Daten überträgt, durch Abfrage des Pegels am com_ctrl_pin_in
      while(digitalRead(com_ctrl_pin_in) == HIGH){
        loop_counter += 1;
        delayMicroseconds(2);
        if(loop_counter == 1000){break;}
      }
      if(loop_counter < 1000){
        transmit_routine(1);
        mySerial -> print(rs485_receiver_address);  //Überträgt die Empfänger adresse
        mySerial -> write(index);         //Überträgt den Datenthema wie z.B.:Antrieb
        mySerial -> write(sub_index);       //Überträgt die Dateninformation wie z.B.: Lenkwinkel
        for(uint8_t counter = 0; counter < num_of_data; counter ++){
          mySerial -> write(data[counter]);     //Datenübertragung
        }
        mySerial -> write('*');           //Zeichen für das Ende der Datenübertragung
        mySerial -> flush();                  //Wartet bis die Daten übertragen werden
        transmit_routine(0);
        //Wurden Daten übertragen, wird eine 1 zurückgegeben
        //keine Angabe ob die Daten auch empfangen wurden
        return 1; 
      }else{//Wurden keine Daten übertragen, wird eine 0 zurückgegeben
        return 0;}  
    }
    /***************************************************************************
     * Initaliserung der Verbindung zwischen dem Ardino und dem Bluetooth-Modul
     *   
     *                   
     *               
     *    
     ***************************************************************************/    
		uint8_t * receive(){
      static uint8_t buffer[64];
      if(mySerial -> find(uart_find_address) == true){
        //***Auslese aus der dem UART-Buffer
        mySerial -> readBytesUntil('*',buffer,64);
      }else{
        for(uint8_t delete_counter = 0; delete_counter < 64; delete_counter += 1){
          buffer[delete_counter] = 0;
        }
      }
      
      return(buffer);
    }
};

#endif
