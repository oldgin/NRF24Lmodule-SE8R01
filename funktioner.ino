void init_io(void)
{
  digitalWrite(IRQq, 0);
  digitalWrite(CEq, 0);			// chip enable
  digitalWrite(CSNq, 1);                 // Spi disable	
}

/**************************************************
 * Function: SPI_RW();
 * 
 * Description:
 * Writes one unsigned char to nRF24L01, and return the unsigned char read
 * from nRF24L01 during write, according to SPI protocol
 **************************************************/
unsigned char SPI_RW(unsigned char Byte)
{
  unsigned char i;
  for(i=0;i<8;i++)                      // output 8-bit
  {
    if(Byte&0x80)
    {
      digitalWrite(MOSIq, 1);
    }
    else
    {
      digitalWrite(MOSIq, 0);
    }
    digitalWrite(SCKq, 1);
    Byte <<= 1;                         // shift next bit into MSB..
    if(digitalRead(MISOq) == 1)
    {
      Byte |= 1;       	                // capture current MISO bit
    }
    digitalWrite(SCKq, 0);
  }
  return(Byte);           	        // return read unsigned char
}
/**************************************************/

/**************************************************
 * Function: SPI_RW_Reg();
 * 
 * Description:
 * Writes value 'value' to register 'reg'
/**************************************************/
unsigned char SPI_RW_Reg(unsigned char reg, unsigned char value)
{
  unsigned char status;

  digitalWrite(CSNq, 0);                   // CSN low, init SPI transaction
  status = SPI_RW(reg);                   // select register
  SPI_RW(value);                          // ..and write value to it..
  digitalWrite(CSNq, 1);                   // CSN high again

  return(status);                   // return nRF24L01 status unsigned char
}
/**************************************************/

/**************************************************
 * Function: SPI_Read();
 * 
 * Description:
 * Read one unsigned char from nRF24L01 register, 'reg'
/**************************************************/
unsigned char SPI_Read(unsigned char reg)
{
  unsigned char reg_val;

  digitalWrite(CSNq, 0);           // CSN low, initialize SPI communication...
  SPI_RW(reg);                   // Select register to read from..
  reg_val = SPI_RW(0);           // ..then read register value
  digitalWrite(CSNq, 1);          // CSN high, terminate SPI communication
  
  return(reg_val);               // return register value
}
/**************************************************/

/**************************************************
 * Function: SPI_Read_Buf();
 * 
 * Description:
 * Reads 'unsigned chars' #of unsigned chars from register 'reg'
 * Typically used to read RX payload, Rx/Tx address
/**************************************************/
unsigned char SPI_Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char bytes)
{
  unsigned char status,i;

  digitalWrite(CSNq, 0);                  // Set CSN low, init SPI tranaction
  status = SPI_RW(reg);       	    // Select register to write to and read status unsigned char

  for(i=0;i<bytes;i++)
  {
    pBuf[i] = SPI_RW(0);    // Perform SPI_RW to read unsigned char from nRF24L01
  }

  digitalWrite(CSNq, 1);                   // Set CSN high again

  return(status);                  // return nRF24L01 status unsigned char
}
/**************************************************/

/**************************************************
 * Function: SPI_Write_Buf();
 * 
 * Description:
 * Writes contents of buffer '*pBuf' to nRF24L01
 * Typically used to write TX payload, Rx/Tx address
/**************************************************/
unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char bytes)
{
  unsigned char status,i;

  digitalWrite(CSNq, 0);                  // Set CSN low, init SPI tranaction
  status = SPI_RW(reg);             // Select register to write to and read status unsigned char
  for(i=0;i<bytes; i++)             // then write all unsigned char in buffer(*pBuf)
  {
    SPI_RW(*pBuf++);
  }
  digitalWrite(CSNq, 1);                   // Set CSN high again
  return(status);                  // return nRF24L01 status unsigned char
}
/**************************************************/

/**************************************************
 * Function: TX_Mode();
 * 
 * Description:
 * This function initializes one nRF24L01 device to
 * TX mode, set TX address, set RX address for auto.ack,
 * fill TX payload, select RF channel, datarate & TX pwr.
 * PWR_UP is set, CRC(2 unsigned chars) is enabled, & PRIM:TX.
 * 
 * ToDo: One high pulse(>10us) on CE will now send this
 * packet and expext an acknowledgment from the RX device.
 **************************************************/
void rf_switch_bank(unsigned char bankindex)
{
    unsigned char temp0,temp1;
    temp1 = bankindex;

    temp0 = SPI_RW(iRF_BANK0_STATUS);

    if((temp0&0x80)!=temp1)
    {
        SPI_RW_Reg(iRF_CMD_ACTIVATE,0x53);
    }
}



void se8r01_powerup()
{
        rf_switch_bank(iBANK0);
        SPI_RW_Reg(iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG,0x03);
        SPI_RW_Reg(iRF_CMD_WRITE_REG|iRF_BANK0_RF_CH,0x32);
        SPI_RW_Reg(iRF_CMD_WRITE_REG|iRF_BANK0_RF_SETUP,0x48);
        SPI_RW_Reg(iRF_CMD_WRITE_REG|iRF_BANK0_PRE_GURD,0x77); //2450 calibration


}


void se8r01_calibration()
{


        rf_switch_bank(iBANK1);

        gtemp[0]=0x40;
        gtemp[1]=0x00;
        gtemp[2]=0x10;
        gtemp[3]=0xE6;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_PLL_CTL0, gtemp, 4);

        gtemp[0]=0x20;
        gtemp[1]=0x08;
        gtemp[2]=0x50;
        gtemp[3]=0x40;
        gtemp[4]=0x50;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_CAL_CTL, gtemp, 5);

        gtemp[0]=0x00;
        gtemp[1]=0x00;
        gtemp[2]=0x1E;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_IF_FREQ, gtemp, 3);

        gtemp[0]=0x29;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_FDEV, gtemp, 1);

        gtemp[0]=0x00;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_DAC_CAL_LOW, gtemp, 1);

        gtemp[0]=0x7F;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_DAC_CAL_HI, gtemp, 1);

        gtemp[0]=0x02;
        gtemp[1]=0xC1;
        gtemp[2]=0xEB;
        gtemp[3]=0x1C;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_AGC_GAIN, gtemp, 4);

        gtemp[0]=0x97;
        gtemp[1]=0x64;
        gtemp[2]=0x00;
        gtemp[3]=0x81;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_RF_IVGEN, gtemp, 4);

        rf_switch_bank(iBANK0);

        digitalWrite(CEq, 1);
        delayMicroseconds(30);
        digitalWrite(CEq, 0);

        delayMicroseconds(50);                            // delay 50ms waitting for calibaration.

        digitalWrite(CEq, 1);
        delayMicroseconds(30);
        digitalWrite(CEq, 0);

        delayMicroseconds(50);                            // delay 50ms waitting for calibaration.
        // calibration end

   }
  
  void se8r01_setup()
  
  {



        gtemp[0]=0x28;
        gtemp[1]=0x32;
        gtemp[2]=0x80;
        gtemp[3]=0x90;
        gtemp[4]=0x00;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK0_SETUP_VALUE, gtemp, 5);

        delayMicroseconds(2);

        rf_switch_bank(iBANK1);

        gtemp[0]=0x40;
        gtemp[1]=0x01;
        gtemp[2]=0x30;
        gtemp[3]=0xE2;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_PLL_CTL0, gtemp, 4);

        gtemp[0]=0x29;
        gtemp[1]=0x89;
        gtemp[2]=0x55;
        gtemp[3]=0x40;
        gtemp[4]=0x50;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_CAL_CTL, gtemp, 5);

        gtemp[0]=0x29;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_FDEV, gtemp, 1);

        gtemp[0]=0x55;
        gtemp[1]=0xC2;
        gtemp[2]=0x09;
        gtemp[3]=0xAC;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_RX_CTRL, gtemp, 4);

        gtemp[0]=0x00;
        gtemp[1]=0x14;
        gtemp[2]=0x08;
        gtemp[3]=0x29;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_FAGC_CTRL_1, gtemp, 4);

        gtemp[0]=0x02;
        gtemp[1]=0xC1;
        gtemp[2]=0xCB;
        gtemp[3]=0x1C;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_AGC_GAIN, gtemp, 4);

        gtemp[0]=0x97;
        gtemp[1]=0x64;
        gtemp[2]=0x00;
        gtemp[3]=0x01;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_RF_IVGEN, gtemp, 4);

        gtemp[0]=0x2A;
        gtemp[1]=0x04;
        gtemp[2]=0x00;
        gtemp[3]=0x7D;
        SPI_Write_Buf(iRF_CMD_WRITE_REG|iRF_BANK1_TEST_PKDET, gtemp, 4);

        rf_switch_bank(iBANK0);
        


    }
  
