
#define MODE 2
#define CSW 3
#define CSR 8
#define RESET 9
  
  
#define PORT_MANIPULATION

int bytePins[8] =   { 14, 15 , 16, 17, 4, 5, 6, 7 }; 

void setDBReadMode() {
  #ifdef PORT_MANIPULATION
  DDRD = DDRD & B00001111; // Set Pin 4..7 as inputs. High nibble of databus.
  DDRC = DDRC & B11110000; // Set Analog pin 0..3 as inputs      
  #else
  for (int i=0;i<8;i++) {
    pinMode(bytePins[i], INPUT);
  }
  #endif
}

void setDBWriteMode() {
  #ifdef PORT_MANIPULATION  
  DDRD = DDRD | B11110000; // Set Pin 4..7 as outputs. High nibble of databus.
  DDRC = DDRC | B00001111; // Set Analog pin 0..3 as outputs
  #else
  for (int i=0;i<8;i++) {
    pinMode(bytePins[i], OUTPUT);
  }  
  #endif
}

void reset() {
  Serial.println("Resetting");
  digitalWrite(RESET, HIGH);
  delayMicroseconds(100);
  digitalWrite(RESET, LOW);
  delayMicroseconds(5);  
  digitalWrite(RESET, HIGH);
}

inline void setPort(unsigned char value) {
    #ifdef PORT_MANIPULATION
    PORTD = (PIND & 0x0F) | (value & 0xF0);
    PORTC = (PINC & 0xF0) | (value & 0x0F);
    #else
    unsigned char mask = 1;
    for (int i=0;i<8;i++) {
      digitalWrite(bytePins[i], value & mask);
      mask = mask<<1;
    }    
    #endif    
}

inline unsigned char readPort() {
    unsigned char memByte = 0;    
    #ifdef PORT_MANIPULATION
    memByte  = (PIND & 0xF0) | (PINC & 0x0F);
    #else
    unsigned char mask = 1;
    for (int i=0;i<8;i++) {
      if (digitalRead(bytePins[i])) {
        memByte = memByte | mask;
      } 
      mask = mask<<1;
    }        
    #endif  
    
    return memByte;
}

//Writes a byte to databus for register access
void writeByte( unsigned char value) {  
    setDBWriteMode();
    setPort(value); 
    digitalWrite(MODE, HIGH);        
    digitalWrite(CSW, LOW);            
    delayMicroseconds(10);
    digitalWrite(CSW, HIGH);
    setDBReadMode();
}

//Reads a byte from databus for register access
unsigned char  readByte( ) {
    unsigned char memByte = 0;
    digitalWrite(MODE, HIGH);            
    digitalWrite(CSR, LOW);        
    delayMicroseconds(10);
    memByte = readPort();
    digitalWrite(CSR, HIGH);
    return memByte;
}

//Writes a byte to databus for vram access
void writeByteToVRAM( unsigned char value) {     
    digitalWrite(MODE, LOW);        
    digitalWrite(CSW, LOW);            
    setDBWriteMode();  
    setPort(value);    
    delayMicroseconds(10);
    digitalWrite(CSW, HIGH);
    setDBReadMode();
    delayMicroseconds(10);       
}

//Reads a byte from databus for vram access
unsigned char  readByteFromVRAM( ) {
    unsigned char memByte = 0;
    digitalWrite(MODE, LOW);            
    digitalWrite(CSR, LOW);        
    delayMicroseconds(1);
    memByte = readPort();
    digitalWrite(CSR, HIGH);
    delayMicroseconds(10);    
    return memByte;
}


void setRegister(unsigned char registerIndex, unsigned char value) {
  writeByte(value);
  writeByte(0x80 | registerIndex);  
}

void setWriteAddress( unsigned int address) {
  writeByte((address & 0xFFC0)>>6);
  writeByte(0x40 | (address & 0x3F));  
}

void setReadAddress( unsigned int address) {
  writeByte((address & 0xFFC0)>>6);
  writeByte((address & 0x3F));  
}

void setup() {
  setDBReadMode();
  Serial.begin(115200);
  pinMode(MODE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(CSW, OUTPUT);
  pinMode(CSR, OUTPUT);  
  
  digitalWrite(RESET, HIGH);
  digitalWrite(MODE, HIGH);  
  digitalWrite(CSW, HIGH);    
  digitalWrite(CSR, HIGH);  

  reset();
  delay(2000);


  setRegister(0, 0x00);
  setRegister(1, 0xC0);

  setRegister(2, 0x00);   //Name table
  setRegister(3, 0x00);    
  setRegister(4, 0x00);   //Pattern generator   
  setRegister(5, 0x00);       
  setRegister(6, 0x00);         
  setRegister(7, 0x00);  
  
  
  
  Serial.println("Clearing RAM");
  setWriteAddress(0);
  for (int i = 0;i<16384;i++) {
    writeByteToVRAM(0);
  } 
  
  Serial.println("RAM Cleared, testing");  
  setWriteAddress(0);
  for (int i = 0;i<256;i++) {
    writeByteToVRAM(0);
  }
  setReadAddress(0);
  for (int i = 0;i<256;i++) {
    unsigned char value = readByteFromVRAM();
    //Serial.println(value);
  }
  
   setWriteAddress(0x0);  
   for (int i = 0x800;i<16384;i=i+8) {
   writeByteToVRAM(0x20);
   writeByteToVRAM(0x50);
   writeByteToVRAM(0x88);
   writeByteToVRAM(0x88);
   writeByteToVRAM(0xF8);
   writeByteToVRAM(0x88);
   writeByteToVRAM(0x88);
   writeByteToVRAM(0x00);   
   }
 
}

int i = 0;

void loop() { 
}


