// ===================================================================================
// =							Setup Modbus Objects
// ===================================================================================

enum MB_FC {
  MB_FC_NONE           = 0,
  MB_FC_READ_COILS     = 1,
  MB_FC_READ_INPUT     = 2,
  MB_FC_READ_REGISTERS = 3,
  MB_FC_WRITE_COIL     = 5,
  MB_FC_WRITE_REGISTER = 6,
  MB_FC_WRITE_MULTIPLE_COILS = 15,
  MB_FC_WRITE_MULTIPLE_REGISTERS = 16
}FC;


int Socket = 0;
int PacketSize = 0;

uint8_t ByteArray[75]; // MODbus Data Packet Buffer

int Start, WordDataLength, ByteDataLength, CoilDataLength, MessageLength;
int length;

int Value;

void listenModBus();
void SetFC(int fc);
void Run();


void initModBus() {
  modBus_Server.begin();
}

void listenModBus() {
  modBus_Client =  modBus_Server.available();
  if (modBus_Client) {
    if (modBus_Client.connected())
    {
      modBus_Client.read(ByteArray, modBus_Client.available()); //set ByteArray with incomming data
      Run(); //proccess data and set Bytearray with outcome data
      modBus_Client.write(ByteArray, MessageLength); //send the data stored in ByteArray
    }
  }
}

void parseModBus(){
  SetFC(ByteArray[7]);  //Byte 7 of request is FC
  Start = word(ByteArray[8],ByteArray[9]); //The Data Address of the first coil to read
  length = word(ByteArray[10],ByteArray[11]); //The total number requested
}

void Run() {

  SetFC(ByteArray[7]);  //Byte 7 of request is FC
  word(ByteArray[10],ByteArray[11]); //The total number of coils requested

  if(FC == MB_FC_READ_COILS) //relays
  {
    Start = word(ByteArray[8],ByteArray[9]); //The Data Address of the first coil to read
    CoilDataLength = word(ByteArray[10],ByteArray[11]); //The total number of coils requested
    ByteDataLength = CoilDataLength / 8;
    if(ByteDataLength * 8 < CoilDataLength) ByteDataLength++;
    CoilDataLength = ByteDataLength * 8;
    ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
    ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).
    for(int i = 0; i < ByteDataLength ; i++)
    {
      for(int j = 0; j < 8; j++)
      {
        bitWrite(ByteArray[9 + i], j , getRelay(Start + i * 8 + j) );
      }
    }
    MessageLength = ByteDataLength + 9;

    FC = MB_FC_NONE;
  }
  
  if(FC == MB_FC_READ_INPUT) //digital state
  {
    Start = word(ByteArray[8],ByteArray[9]); //The Data Address of the first coil to read
    CoilDataLength = word(ByteArray[10],ByteArray[11]); //The total number of coils requested
    ByteDataLength = CoilDataLength / 8;
    if(ByteDataLength * 8 < CoilDataLength) ByteDataLength++;
    CoilDataLength = ByteDataLength * 8;
    ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
    ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).
    for(int i = 0; i < ByteDataLength ; i++)
    {
      for(int j = 0; j < 8; j++)
      {
        bitWrite(ByteArray[9 + i], j , getDigital(Start + i * 8 + j) );
      }
    }
    MessageLength = ByteDataLength + 9;

    FC = MB_FC_NONE;
  }
  
  //****************** Read Registers ******************
  //support Analog, OneWire, Counter
  if (FC == MB_FC_READ_REGISTERS) //4
  {

    Start = word(ByteArray[8], ByteArray[9]);
    WordDataLength = word(ByteArray[10], ByteArray[11]);
    ByteDataLength = WordDataLength * 2;
    ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
    ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).


    MessageLength = ByteDataLength + 9;

    for (int i = 0; i < WordDataLength; i++)
    {

      if (Start >= 0 && Start <= 9) //digital values 0-9
      {
        ByteArray[9 + ( i * 2)] = highByte(digital.State[i]);
        ByteArray[10 + (i * 2)] =  0;
      }

      else if (Start >= 10 && Start <= 19 )
      {
        int intAnalog =  getAnalogInt(i); //modBus answer is 2 bytes so downcast floating point into intger
        ByteArray[9 + (i * 2)] = highByte(intAnalog);
        ByteArray[10 + (i * 2)] =  lowByte(intAnalog);
        
      }

      else if (Start >= 20 && Start <= 29)
      {
        int intOneWire = (int)(oneWire.ValueTemp[i] * 10); //modBus answer is 2 bytes so downcast floating point into intger
        ByteArray[9 + ( i * 2)] = highByte(intOneWire);
        ByteArray[10 + (i * 2)] =  lowByte(intOneWire);
      }
      
      else if (Start >= 30 && Start <= 39)
      {
        /*
        reserved for Humid/Temp
        */
      }
      else if (Start >= 40 && Start <= 49)
      {
        /*
        reserved for PWM
        */
      }      
      
    }

    FC = MB_FC_NONE;
  }

  //****************** Write Register ******************
  if (FC == MB_FC_WRITE_REGISTER) // 6
  {

    ByteArray[5] = 6; //Number of bytes after this one.
    MessageLength = 12;

    Start = word(ByteArray[8], ByteArray[9]);
    Value = word(ByteArray[10], ByteArray[11]);

    //modBus_Client.write(ByteArray,MessageLength);

    if (Start > 19 && Start < 35)
    {
      setRuleOffByRelay(Start); //Turn off any rule that this relay is related to
      setRealyState(Start, Value); //set the new relay state as got from SNMP
    }
    else
    {
    }
    FC = MB_FC_NONE;
  }

}


// ===================================================================================
// =					Map the function code the the register function				 =
// ===================================================================================
void SetFC(int fc)
{
  if (fc == 1) FC = MB_FC_READ_COILS;
  if (fc == 2) FC = MB_FC_READ_INPUT;
  if (fc == 3) FC = MB_FC_READ_REGISTERS;
  if (fc == 6) FC = MB_FC_WRITE_REGISTER;
  
}

