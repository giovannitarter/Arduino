#include "persistent_config.h"


void reset_config() {
    SPIFFS.remove(CONFIG_FILE_PATH);
}


void factoryConfig(thermoCfg * tcfg, char name []) {

        Serial.println("Factory config");
        
        strncpy(tcfg->name, name, MAX_ADDR); 

        strncpy(tcfg->essid, DEFAULT_WIFI_SSID, MAX_ADDR);
        strncpy(tcfg->pass, DEFAULT_WIFI_PASS, MAX_ADDR);
        
        strncpy(tcfg->server, DEFAULT_MQTT_SERVER, MAX_ADDR);
        tcfg->port = DEFAULT_MQTT_PORT;
        
        strncpy(tcfg->otaserver, DEFAULT_OTA_SERVER, MAX_ADDR);
        tcfg->otaport = DEFAULT_OTA_PORT;
        
        tcfg->sens_type = DEFAULT_SENS_TYPE;
        strncpy(tcfg->note, DEFAULT_NOTE, MAX_ADDR);
}


void writeConfig(thermoCfg * tcfg) {
     
        File f;   
        aJsonObject * root;
        char * wbuf;
        int wbuf_len;

        root = aJson.createObject();

        aJson.addItemToObject(root, "NAME", 
                aJson.createItem(tcfg->name));
        
        aJson.addItemToObject(root, "ESSID", 
                aJson.createItem(tcfg->essid));
        
        aJson.addItemToObject(root, "PASS", 
                aJson.createItem(tcfg->pass));
        
        aJson.addItemToObject(root, "SRV", 
                aJson.createItem(tcfg->server)); 
        aJson.addNumberToObject(root, "SRVP", tcfg->port);
        
        aJson.addItemToObject(root, "OTASRV", 
                aJson.createItem(tcfg->otaserver));
        aJson.addNumberToObject(root, "OTASRVP", tcfg->otaport); 
        
        aJson.addNumberToObject(root, "SENS", tcfg->sens_type);
        
        aJson.addItemToObject(root, "NOTE", 
                aJson.createItem(tcfg->note));
        
        wbuf = aJson.print(root);
        aJson.deleteItem(root);
        
        wbuf_len = strlen(wbuf);
    
        f = SPIFFS.open(CONFIG_FILE_PATH, "w");
        Serial.printf("File %s open for write", CONFIG_FILE_PATH);
        f.write((uint8_t *)wbuf, wbuf_len);
        f.close();

        free(wbuf);

        dump_cfg_file();
}


void dump_cfg_file() {
    
    File f;    
    int size;
    char * buf;

    Serial.println("################");
    Serial.println("DUMPING CFG FILE");
    
    f = SPIFFS.open(CONFIG_FILE_PATH, "r");
    if (f) {
        size = f.size();
        buf = (char *) malloc(size);
        f.readBytes(buf, size);
        f.close();

        Serial.println(buf);
        free(buf);
    }
    else {
        Serial.println("ERR: FILE does not exist");
    }
    Serial.println("################");
}


bool loadConfig(thermoCfg * tcfg) {
   
    char buf[400];
    aJsonObject* root; 
    File f; 
    
    thermoCfg tmpcfg; 
    memset(&tmpcfg, 0, sizeof(thermoCfg));
    
    dump_cfg_file();

    f = SPIFFS.open(CONFIG_FILE_PATH, "r+");
        
    if (f) {
        f.read((uint8_t *)buf, 400);
        f.close();
    }
    else {
        return false;
    }
    
    Serial.printf("File %s exist\n", CONFIG_FILE_PATH);
        
    root = aJson.parse(buf);    
    if (root == NULL) {
        Serial.println("JSON parsing failed");
        return false;
    }
        
    aJsonObject * name = aJson.getObjectItem(root, "NAME");    
    if (name) {
        strncpy(tmpcfg.name, name->valuestring, MAX_ADDR);
        Serial.printf("lcfg: NAME: %s\n", tmpcfg.name);
    }
    else {
        Serial.println("Cannot parse NAME");
        return false;
    }
    
    aJsonObject * srv = aJson.getObjectItem(root, "SRV");    
    if (srv) {
        strncpy(tmpcfg.server, srv->valuestring, MAX_ADDR);
        Serial.printf("lcfg: SRV: %s\n", tmpcfg.server);
    }
    else {
        Serial.println("Cannot parse SRV");
        return false;
    }
    
    aJsonObject * srvp = aJson.getObjectItem(root, "SRVP");    
    if (srv) {
        tmpcfg.port = srvp->valueint;
        Serial.printf("lcfg: SRVP: %d\n", tmpcfg.port);
    }
    else {
        Serial.println("Cannot parse SRVP");
        return false;
    }
    
    aJsonObject * otasrv = aJson.getObjectItem(root, "OTASRV");    
    if (otasrv) {
        strncpy(tmpcfg.otaserver, srv->valuestring, MAX_ADDR);
        Serial.printf("lcfg: OTASRV: %s\n", tmpcfg.otaserver);
    }
    else {
        Serial.println("Cannot parse OTASRV");
        return false;
    }
    
    aJsonObject * otaport = aJson.getObjectItem(root, "OTASRVP");    
    if (otaport) {
        tmpcfg.otaport = otaport->valueint;
        Serial.printf("lcfg: OTASRVP: %d\n", tmpcfg.otaport);
    }
    else {
        Serial.println("Cannot parse OTASRVP");
        return false;
    }
    
    aJsonObject * essid = aJson.getObjectItem(root, "ESSID");    
    if (essid) {
        strncpy(tmpcfg.essid, essid->valuestring, MAX_ADDR);
        Serial.printf("lcfg: ESSID: %s\n", tmpcfg.essid);
    }
    else {
        Serial.println("Cannot parse ESSID");
        return false;
    }
    
    aJsonObject * pass = aJson.getObjectItem(root, "PASS");    
    if (pass) {
        strncpy(tmpcfg.pass, pass->valuestring, MAX_ADDR);
        Serial.printf("lcfg: PASS: %s\n", tmpcfg.pass);
    }
    else {
        Serial.println("Cannot parse PASS");
        return false;
    }
    
    aJsonObject * sens_type = aJson.getObjectItem(root, "SENS");    
    if (sens_type) {
        tmpcfg.sens_type = sens_type->valueint;
        Serial.printf("lcfg: SENS: %d\n", tmpcfg.sens_type);
    }
    else {
        Serial.println("Cannot parse SENS");
        return false;
    }
    
    aJsonObject * note = aJson.getObjectItem(root, "NOTE");    
    if (note) {
        strncpy(tmpcfg.note, note->valuestring, MAX_ADDR);
        Serial.printf("lcfg: NOTE: %s\n", tmpcfg.note);
    }
    else {
        Serial.println("Cannot parse NOTE");
        return false;
    }
    
    aJson.deleteItem(root);

    memcpy(tcfg, &tmpcfg, sizeof(thermoCfg));
    return true;
}
    

bool verify_config(thermoCfg * tcfg) {

    bool res;

    res = true;

    if (strlen(tcfg->name) == 0) {
        res = false;
    }
    
    if (res) {
        if(tcfg->otaport == tcfg->port) {
            Serial.println("SRV Port cannot be equal to OTA Port");
            res = false;
        }
    }

    return res;
}
 

void setup_config(thermoCfg * tcfg, char name[]) {
    
    bool need_factory;
    need_factory = true;

    Serial.println("setup config START");

    if (SPIFFS.begin()) {
        Serial.println("SPIFFS begin ok!");
        
        if (loadConfig(tcfg)) {
            
            if (verify_config(tcfg)) {
                need_factory = false;
            }
            else {
                Serial.println("verify fail");
            }
        }
        else {
            Serial.println("loadconfig fail");
        }
            
        if (need_factory) {
            factoryConfig(tcfg, name);
            writeConfig(tcfg);
        }
    }
    else 
    {
        Serial.println("SPIFFS begin fail!");
    }
    Serial.println("setup config RETURN\n");
}
