
var  Timer = [[1,1,1,1,1,1,1,1,1,1,1,1,1,1],
[0,1,2,3,4,5,6,7,8,9,10,11,12,13],
[0,1,1,0,0,0,0,0,0,0,0,0,0,0],
[50,50,50,50,60,46,60,46,60,46,60,46,60,46]
];
var  State = [2,15,17,19,3,5,16
  ];
function get_data(){
var battery = 47;
var vin = 15;
var brightness = 50;
var temperature = 25;
var humidity = 70;
var device_one_state = 0;
var json = "{";
//-------------------------
json += "\"Data\":0,";
json += "\"state\":";//  "state":
if (device_one_state == true) json += "\"on\"";
else json += "\"off\"";
json += ",\"volt\":";//  "volt":
json += "\"" + String(vin) + "V\"";
json += ",\"battery\":";//  "battery":
json += "\"" + String(battery) + "%\"";
json += ",\"bright\":";//  "bright":
json += "\"" + String(brightness) + "%\"";
json += ",\"temp\":";//  "state":
json += "\"" + String(temperature) + "C\"";
json += ",\"humi\":";//  "state":
json += "\"" + String(humidity) + "%\"";
json += ",\"Regs\":[";//  "state":
json += "{\"RegID\":";
json += Timer[0][0];
json += ",\"RegAddr\":";
json += Timer[1][0];
json += ",\"RegType\":";
json += Timer[2][0];
json += ",\"RegValue\":";
if(Timer[3][0] > 100){Timer[3][0] = 0;}
json += Timer[3][0];
json += "}";
for(var i = 1 ; i < 14 ; i++){
json += ",{\"RegID\":";
json += Timer[0][i];
json += ",\"RegAddr\":";
json += Timer[1][i];
json += ",\"RegType\":";
json += Timer[2][i];
json += ",\"RegValue\":";
if(Timer[3][i] > 100){Timer[3][i] = 0;}
json +=  60 + Math.round(20 * Math.random());
json += "}";
}
json += "],\"RegsList\":[";//  "state":
json += "{\"regs\":";
json += Timer[1][0];;
json += "}";
for(var i = 1 ; i < 14 ; i++){//18191
json += ",{\"regs\":";
json += Timer[1][i];
json += "}";
}
json += "],\"IDList\":[";//  "state":
json += "{\"id\":";
json += 0;
json += "}";
for(var i = 1 ; i < 10 ; i++){//18191
json += ",{\"id\":";
json += i;
json += "}";
}
json += "],\"NETList\":[";//  "state":
json += "{\"id\":";
json += 0;
json += "}";
for(var i = 1 ; i < 10 ; i++){//18191
json += ",{\"id\":";
json += i;
json += "}";
}
json += "]}";
var e = json;
// console.log("contentLog:" + json);
loadAppSetup(e);
}

