#include "SDFunction.h"
#include "webinterface.h"

File root;

void SDhandleRoot() {
  root = SD.open("/");
  String res = SDFunction::printDirectory(root, 0);
  web_interface->web_server.send(200, "text/html", res);
}

void SDhandleRoot1() {
  root = SD.open("/data");
  String res = SDFunction::printDirectory(root, 0);
  web_interface->web_server.send(200, "text/html", res);
}


bool loadFromSDCARD(String path){
  path.toLowerCase();
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".txt")) dataType = "text/plain";
  else if(path.endsWith(".csv")) dataType = "text/plain";
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".js")) dataType = "text/js";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".log")) dataType = "text/log";
  else if(path.endsWith(".bin")) dataType = "text/bin";
  else if(path.endsWith(".zip")) dataType = "application/zip";  
  //debugln(dataType);
  File dataFile = SD.open(path.c_str());

  if (!dataFile)
    return false;

  if (web_interface->web_server.streamFile(dataFile, dataType) != dataFile.size()) {
    LOGLN("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}
void handleRoot() {
  //web_interface->web_server.send_P(200, "text/html", html_main);
  if(loadFromSDCARD("/main.html")){
  }
  else{
    SDhandleRoot();
  }
}
