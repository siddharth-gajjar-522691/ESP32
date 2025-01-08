#define ssid_LnT  "**LMB-HZW-WCMS**"
#define pwd_LnT   "Power@1234"
#define ip_LnT    "10.7.74.158"
#define port_LnT  80

#define ssid_office "i Technology"
#define pwd_office  "qwerty1234"
#define ip_office   "i-technology.in"
#define port_office 80

#define SID_HOME_SSID "JioFiber-exyKz"
#define SID_HOME_PWD  "ahm2cai7QuahShib"

/*
using namespace std;
  
// Library classes
class Wifi {
public:
    virtual string getPassword() = "";
    virtual string getSsid() = "";
};

class LTPassword : public Wifi {
public:
    string getPassword()  {
        return pwd_LnT;
    }
  string getSsid()  {
        return ssid_LnT;
    }
};
class ItechPassword : public Vehicle {
    public:
    string getPassword()  {
        return "qwerty1234";
    }
  string getSsid()  {
        return ssid_office;
    }
};


  
// Client (or user) class
class DetectWifi {
public:
    DetectWifi(string wifiList[])  {
    for(int i=0; i < wifiList.length();i++){
      string wifiName = wifiList[i];
        if(wifiName == ssid_LnT){
            wifi = new LTPassword();
        }
        else if(wifiName == ssid_office){
            wifi = new ItechPassword();
        }
      }
    }
  
    ~Client()   {
        if (wifi)
        {
            wifi = NULL;
        }
    }
  
    Wifi* getWifi() {
        return wifi;
    }
private:
    Wifi *wifi;
};


int select_wifi() {
  string wifiList = ["LT","ITECH","Sidharth"];
    DetectWifi *detectWifi = new DetectWifi(wifiList);
    Wifi * wifi = detectWifi->getWifi();
    string password = wifi->getPassword();
    string ssid = wifi->getSsid();
}*/
