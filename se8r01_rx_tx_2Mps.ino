//this is a copy and paste job made by F2k

#include "se8r01.h"
byte gtemp[5];
byte k=0;
//***************************************************
#define TX_ADR_WIDTH    4   // 5 unsigned chars TX(RX) address width
#define TX_PLOAD_WIDTH  6  // 32 unsigned chars TX payload

unsigned char TX_ADDRESS[TX_ADR_WIDTH]  = 
{
  0x34,0x43,0x10,0x10
}; // Define a static TX address


byte mode ='r';      //r=rx, t=tx

unsigned char rx_buf[TX_PLOAD_WIDTH] = {0}; // initialize value
unsigned char tx_buf[TX_PLOAD_WIDTH] = {0};
//***************************************************
void setup() 
{
  pinMode(CEq,  OUTPUT);
  pinMode(SCKq, OUTPUT);
  pinMode(CSNq, OUTPUT);
  pinMode(MOSIq,  OUTPUT);
  pinMode(MISOq, INPUT);
  pinMode(IRQq, INPUT);

  Serial.begin(9600);
  init_io();                        // Initialize IO port
  unsigned char status=SPI_Read(STATUS);
  Serial.print("status = ");    
  Serial.println(status,HEX);     
  Serial.println("*******************Radio starting*****************");
 

 digitalWrite(CEq, 0);
  delay(1);
se8r01_powerup();
se8r01_calibration();
se8r01_setup();
radio_settings();
 if (mode=='r')
 {//rx mode
 SPI_RW_Reg(WRITE_REG|iRF_BANK0_CONFIG, 0x3f); 
 Serial.println("*******************RX mode************************"); 
 }
 else
 {//tx mode
 SPI_RW_Reg(WRITE_REG|iRF_BANK0_CONFIG, 0x3E);
  Serial.println("*******************TX****************************");
 }

digitalWrite(CEq, 1);
}



void loop() 
{
  if (mode=='r')
  {
    RXX();
    }
  else
  {
    TXX();
  }
   
  
}


void RXX()
{
 
    if(digitalRead(IRQq)==LOW)
    {
      delay(1);      //read reg too close after irq low not good
    unsigned char status = SPI_Read(STATUS);  
  
    if(status&STA_MARK_RX)                                                 // if receive data ready (TX_DS) interrupt
    {
      SPI_Read_Buf(RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH);             // read playload to rx_buf
      SPI_RW_Reg(FLUSH_RX,0); // clear RX_FIFO
     Serial.print("rx_buf[i]");      
      for(byte i=0; i<TX_PLOAD_WIDTH; i++)
      {
          Serial.print(" : ");
          Serial.print(rx_buf[i]);                              // print rx_buf
      }
      Serial.println(" ");

      SPI_RW_Reg(WRITE_REG+STATUS,0xff);
    }
     else{

    SPI_RW_Reg(WRITE_REG+STATUS,0xff);

     }
        
    }
  delay(1);

}


 void TXX()
 {
       for(byte i=0; i<TX_PLOAD_WIDTH; i++)
        tx_buf[i] = k++; 
        
    unsigned char status = SPI_Read(STATUS); 
    
  
      SPI_RW_Reg(FLUSH_TX,0);
      SPI_Write_Buf(WR_TX_PLOAD,tx_buf,TX_PLOAD_WIDTH);     
    
    SPI_RW_Reg(WRITE_REG+STATUS,0xff);   // clear RX_DR or TX_DS or MAX_RT interrupt flag
    Serial.println(status,HEX);    
    delay(500);
 
 }


void radio_settings()
{
        
        SPI_RW_Reg(WRITE_REG|iRF_BANK0_EN_AA, 0x01);          //enable auto acc on pip 1
        SPI_RW_Reg(WRITE_REG|iRF_BANK0_EN_RXADDR, 0x01);      //enable pip 1
        SPI_RW_Reg(WRITE_REG|iRF_BANK0_SETUP_AW, 0x02);        //4 byte adress
        
        SPI_RW_Reg(WRITE_REG|iRF_BANK0_SETUP_RETR, B00001010);        //lowest 4 bits 0-15 rt transmisston higest 4 bits 256-4096us Auto Retransmit Delay
        SPI_RW_Reg(WRITE_REG|iRF_BANK0_RF_CH, 40);
        SPI_RW_Reg(WRITE_REG|iRF_BANK0_RF_SETUP, 0x4f);        //2mps 0x4f
        //SPI_RW_Reg(WRITE_REG|iRF_BANK0_DYNPD, 0x01);          //pipe0 pipe1 enable dynamic payload length data
        //SPI_RW_Reg(WRITE_REG|iRF_BANK0_FEATURE, 0x07);        // enable dynamic paload lenght; enbale payload with ack enable w_tx_payload_noack
        
 SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);  //from tx
 SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // Use the same address on the RX device as the TX device
 SPI_RW_Reg(WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH); // Select same RX payload width as TX Payload width
        
}
        
    
