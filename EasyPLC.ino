struct Input{
  bool NO;
  bool NC;
  bool NO_Rising;
  bool NC_Rising;
  bool NO_Falling;
  bool NC_Falling;
  int LastState;
};

struct Output{
  bool NO;
  bool NC;
  bool NO_Rising;
  bool NC_Rising;
  bool NO_Falling;
  bool NC_Falling;
  bool Coil;
  int LastState;
};

struct Memmory{
  bool NO;
  bool NC;
  bool NO_Rising;
  bool NC_Rising;
  bool NO_Falling;
  bool NC_Falling;
  bool Coil;
  int LastState;
};

struct Timer{
  bool Contact;
  bool NO;
  bool NC;
  bool NO_Rising;
  bool NC_Rising;
  bool NO_Falling;
  bool NC_Falling;
  bool Coil;
  bool LastState;
  unsigned long TStamp;
  unsigned long Target;
};

struct SRState{
  bool State;
};



enum HardWareInputPin {
  HW_X0=2, //Map Pin 2 as X0
  HW_X1=3,
  HW_X2=4,
  HW_X3=5,
  HW_X4=50,
  HW_X5=52
};

enum HardWareOutputPin {
  HW_Y0=8, //Map Pin 8 as Y0
  HW_Y1=9,
  HW_Y2=10,
  HW_Y3=11,
  HW_Y4=12,
  HW_Y5=13
};

Input X[16];
Output Y[16];
Memmory M[100]; //register M

uint16_t D[100]; //register D
Timer T[100]; //timer Struct
int SM8012;
int SM8013;
int SM8014;
unsigned long TStamp[4];

void RunSpecialRelay(){
  unsigned long currentMillis = millis();
  if(currentMillis-TStamp[0] >= (unsigned long)50){ //0.1 Sec Relay
    SM8012 = ~SM8012;
    TStamp[0] = currentMillis;
  }
  if(currentMillis-TStamp[1] >= (unsigned long)500){ //1 Sec Relay
    SM8013 = ~SM8013;
    TStamp[1] = currentMillis;
    
  }
  if(currentMillis-TStamp[2] >= (unsigned long)30000){ //1 min Relay
    SM8014 = ~SM8014;
    TStamp[2] = currentMillis;
  }
}

void TimerBlock(bool condition,Timer &Timer,unsigned long Target){
  Timer.Target = Target;
  unsigned long currentMillis = millis();
  if(condition){
    if(currentMillis-Timer.TStamp >= Timer.Target){
      Timer.NO = true;
      Timer.NC = false;
    }else{
      Timer.NO = false;
      Timer.NC = true;
    }
  }else{
    Timer.TStamp = currentMillis;
      Timer.NO = false;
      Timer.NC = true;
  }
}

void SET(bool resetcondition ,Memmory &mem){
  if(resetcondition){
    mem.NO = true;
    mem.NC = false;
  }
}

void RST(bool resetcondition ,Memmory &mem){
  if(resetcondition){
    mem.NO = false;
    mem.NC = true;
  }
}

bool SETRST(SRState &mem,bool setcondition,bool resetcondition){
  if(setcondition || mem.State){
    mem.State = true;
  }
  if(resetcondition){
    mem.State = false;
  }
  return mem.State;
}

void CoilBlock(bool Condition,Output &OutCoil){
  OutCoil.Coil = Condition;
  OutCoil.NO = Condition;

  if(OutCoil.NO){
    OutCoil.NC = false;
  }else{
    OutCoil.NC = true;
  }

  OutCoil.NC_Falling = true;
  OutCoil.NO_Falling = false;
  OutCoil.NC_Rising = true;
  OutCoil.NO_Rising = false;

  if(OutCoil.LastState != Condition){
    if(OutCoil.LastState == true && Condition == false){
       OutCoil.NO_Falling = true;
       OutCoil.NC_Falling = false;
    }else if(OutCoil.LastState == false && Condition == true){
       OutCoil.NC_Rising = false;
       OutCoil.NO_Rising = true;
    }
    OutCoil.LastState = Condition;
  }
}

void MapGPIO(){
  
  X[0].NC = digitalRead(HW_X0);
  X[1].NC = digitalRead(HW_X1);
  X[2].NC = digitalRead(HW_X2);
  X[3].NC = digitalRead(HW_X3);
  X[4].NC = digitalRead(HW_X4);
  X[5].NC = digitalRead(HW_X5);

    for(int i=0;i<=5;i++){
      X[i].NC_Falling = true;
      X[i].NC_Rising = true;
      X[i].NO_Falling = false;
      X[i].NO_Rising = false;

        if(X[i].NC){ 
          X[i].NO = false;
        }else{
          X[i].NO = true;
        }
    }

  if(micros()-TStamp[3] >= 500){
    for(int i=0;i<=5;i++){
      if(X[i].NO == true && X[i].LastState == false){
          X[i].NC_Rising = false;
          X[i].NO_Rising = true;
      }else if(X[i].NO == false && X[i].LastState == true){
          X[i].NC_Falling = false;
          X[i].NO_Falling = true;
      }
      X[i].LastState = X[i].NO;
    }
    TStamp[3] = micros();
  }


  digitalWrite(HW_Y0, Y[0].Coil);
  digitalWrite(HW_Y1, Y[1].Coil);
  digitalWrite(HW_Y2, Y[2].Coil);
  digitalWrite(HW_Y3, Y[3].Coil);
  digitalWrite(HW_Y4, Y[4].Coil);
  digitalWrite(HW_Y5, Y[5].Coil);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(HW_X0, INPUT_PULLUP);
  pinMode(HW_X1, INPUT_PULLUP);
  pinMode(HW_X2, INPUT_PULLUP);
  pinMode(HW_X3, INPUT_PULLUP);
  pinMode(HW_X4, INPUT_PULLUP);
  pinMode(HW_X5, INPUT_PULLUP);

  pinMode(HW_Y0, OUTPUT);
  pinMode(HW_Y1, OUTPUT);
  pinMode(HW_Y2, OUTPUT);
  pinMode(HW_Y3, OUTPUT);
  pinMode(HW_Y4, OUTPUT);
  pinMode(HW_Y5, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  TimerBlock(X[0].NO,T[0],2000); //Line 1 Timer 2000ms = 2 sec
  CoilBlock(T[0].NO, Y[0]); //Line 2
  SET(X[1].NO_Rising, M[0]); //Line 3 When Input X1 Rising Edge SET M0 
  RST(X[2].NO_Rising, M[0]); //Line 4 When Input X2 Rising Edge RESET M0
  CoilBlock(M[0].NO && SM8012, Y[1]); //M0 And Special Relay Blink 0.1 second
  CoilBlock((X[4].NO || Y[5].NO) && X[5].NC, Y[5]); //Self Holding Example

  RunSpecialRelay();
  MapGPIO();

}
