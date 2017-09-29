#include "persistent_config.h"


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
        
        aJson.addNumberToObject(root, "SENS", tcfg->sensType);
        
        aJson.addItemToObject(root, "NOTE", 
                aJson.createItem(tcfg->note));
        
        wbuf = aJson.print(root);
        aJson.deleteItem(root);
        
        wbuf_len = strlen(wbuf);
    
        f = SPIFFS.open("/config.json", "w");
        Serial.println("config.json open for write");
        f.write((uint8_t *)wbuf, wbuf_len);
        f.close();

        free(wbuf);


}


void dump_cfg_file() {
    
    File f;    
    int size;
    char * buf;

    Serial.println("################");
    Serial.println("DUMPING CFG FILE");
    
    f = SPIFFS.open("/config.json", "r");
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

    f = SPIFFS.open("/config.json", "r+");
        
    if (f) {
        f.read((uint8_t *)buf, 400);
        f.close();
    }
    else {
        return false;
    }
    
    Serial.println("config.json exist");
        
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
        tmpcfg.sensType = sens_type->valueint;
        Serial.printf("lcfg: SENS: %d\n", tmpcfg.port);
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

    if(otaport == srvp) {
        Serial.println("SRV Port cannot be equal to OTA Port");
        return false;
    }

    memcpy(tcfg, &tmpcfg, sizeof(thermoCfg));
    return true;
}
     
        
void setup_config(thermoCfg * tcfg) {
    
    Serial.println("setup config START");

    if (SPIFFS.begin()) {
        Serial.println("SPIFFS begin ok!");
        
        if (loadConfig(tcfg) == false) {
            Serial.println("loadconfig fail");
            writeConfig(tcfg);
        }
    }
    else 
    {
        Serial.println("SPIFFS begin fail!");
    }
    Serial.println("setup config RETURN\n");
}
