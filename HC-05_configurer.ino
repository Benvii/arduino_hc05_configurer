/*
 * Script to configure HC-05 ZS-040 bluetooth module.
 * Use the arduino Serial monitor and follow the instructions.
 * 
 * Wiring :
 * - Arduino PIN 10 <-> HC-05 RX
 * - Arduino PIN 11 <-> HC-05 TX
 * - Arduino PIN 9 <-> HC-05 State
 * 
 * \author Benjamin BERNARD, animateur aux Petits Hackers (maison du libre Brest) benjamin.bernard [ at]lph.wtf
 * \see Web documentation 
 * \license GPL V3.
 */

#include <SoftwareSerial.h>

/**
 * Wiring configuration.
 */
#define BT_RX 10
#define BT_TX 11
#define BT_STATE 9
#define BT_SPEED_AT 38400
#define LED_PIN 13

#define LF 13

SoftwareSerial bt(BT_TX, BT_RX);

/**
 * \brief Read all content from SoftwareSerial object. Replace \n (13) by \0.
 *  
 *  \param s    Pointer to a SoftwareSerial object.
 *  \param res  Pointer to char buffer to store response.
 */
String readLine(SoftwareSerial& s, String& res){
  res="";
  while(s.available()){
    res += (char)s.read();
    if(int p = res.indexOf(LF)){
      res[p]='\0';
    }
  }
  return res;
}

/**
 * \brief Read all content from Serial object. Replace \n (13) by \0.
 *  
 *  \param res  Pointer to char buffer to store response.
 *  \return Length of res.
 */
int readLineSerial(String& res){
  unsigned int i = 0;
  while(Serial.available()){
    res += (char)Serial.read();
    if(int p = res.indexOf(LF)){
      res[p]='\0';
    }
    i++;
    
    delay(1);
  }
  return i;
}


/**
 * \brief Fix user input, depends on the mode of  the Arduino IDE terminal : NF, CR, CR + NF ...
 * 
 * \param userInput user input.
 */
void fixTermInput(String &userInput){
  unsigned int i = 0;
  while(userInput[i]!='\0' && userInput[i]!='\n'){
    i++;
  }
  
  userInput[i]='\n';
  userInput[i+1]='\0';
}


/**
 * \brief Send AT command to bt. Timeout 1s.
 * 
 * \param cmd       Command.
 * \param response  String to store the response.
 * \return true if response is 'OK'.
 */
boolean bt_sendATCommand(const String& cmd, String& response){
  bt.println(cmd);

  unsigned int i=0;
  while(bt.available()==0 && i < 100){ // wait for response data
    delay(10);
    i++;
  }
  readLine(bt, response);
  
  return sizeof(response)>=2 && response[0]=='O' && response[1]=='K';
}


/**
 * \brief Check that the BT module is in AT mode. Return 'OK' after 'AT' command.
 * 
 * \return true if it's in AT mode.
 */
boolean bt_checkATenabled(){ // to test
  String r;
  boolean isInAt = bt_sendATCommand(String("AT"), r);
  return isInAt;
}


/**
 * \brief Display menu on Serial, to explain how to set AT mode.
 */
void dislay_help_enabl_ATMode(){
  Serial.println("Vous devez mettre le module HC-05 (ZS-040) en mode AT : ");
  Serial.println("1 - Débrancher l'alimentation du module");
  Serial.println("2 - Maintenir le bouton switch du module");
  Serial.println("3 - Alimenter le module");
  Serial.println("4 - Relancher le bouton switch quand la LED clignotte toutes les 2 secondes");
}


/**
 * \brief Display avaible configuration options.
 */
void display_configure_menu(){
  Serial.println("");
  Serial.println("Menu de configuration du HC-05 : ");
  Serial.println("(N) - Configurer le nom du module");
  Serial.println("(P) - Changer la passkey (code d'apairage)");
  Serial.println("(AT+...) - Entrez directement une commmande AT (cf doc sur wiki)");
  Serial.println("");
  Serial.println("Entrez une option (ex: N) :");
}


/**
 * \brief Interactive module name setter. Ask user for new module name.
 */
void setModuleName(){
  String userInput, cmdAT, repAT;
  unsigned int len; 
  Serial.println("Entrer un nom pour le module ?");

  while(Serial.available()==0){}

  len = readLineSerial(userInput);
  if(len>=1 && userInput[0]!=' '){ // prevent some names
      fixTermInput(userInput);
      cmdAT="AT+NAME="+userInput;
      
      if( bt_sendATCommand(cmdAT, repAT) ){
        Serial.println("Le nom du module est maintenant : "+userInput);
      }
      Serial.println(repAT);
  }
}


/**
 * \brief Interactive module passkey setter. Ask user for new passkey.
 */
void setPassKey(){
  String userInput, cmdAT, repAT;
  unsigned int len; 
  Serial.println("Entrer une nouvelle passkey (par exemple : 1234 ) ?");

  while(Serial.available()==0){}

  len = readLineSerial(userInput);
  if(len>=1 && userInput[0]!=' '){ // prevent some names
      fixTermInput(userInput);
      cmdAT="AT+PSWD="+userInput;
      if( bt_sendATCommand(cmdAT, repAT) ){
        Serial.println("La nouvelle passkey est maintenant : "+userInput);
      }
  }
}

void setup() {
  // Serial
  Serial.begin(9600);

  // BT
  pinMode(BT_STATE, OUTPUT);
  digitalWrite(BT_STATE, HIGH);
  bt.begin(BT_SPEED_AT);
}

void loop() {
  String userInput;
  String cmdAT;
  String repAT;
  unsigned int len;
  
  if(bt_checkATenabled()){
    Serial.println("Votre module est bien en mode AT :) ");
    display_configure_menu();

    while(Serial.available()<=0){}

    len = readLineSerial(userInput);
    
    if(len >= 1){
      fixTermInput(userInput);
      switch((char)userInput[0]){
        case 'N':
          setModuleName();
          break;
        case 'P':
          setPassKey();
          break;
        default:
          Serial.println("Cette option n'existe pas, envoie de la commande directement comme commande AT.");
          Serial.println("AT > "+userInput);

          bt_sendATCommand(userInput, repAT);
          
          Serial.println(repAT);
          break;
      }
    }

    delay(100);
  }else{
    dislay_help_enabl_ATMode();

    Serial.println("... prochain test dans 3 secondes ...");
    delay(3000);
  }
  
  Serial.println("");
}
