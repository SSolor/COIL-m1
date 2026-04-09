//COIL milestone3/final webserver
#include "crow_all.h"
#include "packet.h"
#include "socket.h"
#include <iostream>

#define LOG stdout
//its quite annoying that it treats crow as having errors
//and intellisence doesn't work. I can't even see function names

using namespace crow;
using namespace std;

#define APPROPRIATE_SIZE 150 //this should be big enough for the buffer, I think?
//kind of arbitrary but I forget what the biggest size the sim sent was

int main(){
    crow::SimpleApp app;

    MySocket* soc=nullptr;//keeps in scope, otherwise each crow_route function is creating and destroying them
    //but we don't want to initialize this yet, so here we are
    int packetno=0;

    //just going to copy basic routes from a previous assignment because what else are they supposed to do?
    CROW_ROUTE(app, "/")
    ([](const request &req, response &res) {
        ifstream in("../public/index.html", ifstream::in);
        if (in) {
            ostringstream contents;
            contents << in.rdbuf();
            in.close();

            res.set_header("Content-Type","text/html");
            res.code=200;
            res.write(contents.str());
        }
        else {
            res.code = 404;
            res.write("not found");
        }
        res.end();
    });
    CROW_ROUTE(app, "/get_style/<string>")
    ([](const request &req, response &res, string filename) {
        ifstream in("../public/styles/"+filename,ifstream::in);
        if(in){
            ostringstream contents;
            contents <<in.rdbuf();
            in.close();

            res.set_header("Content-Type","text/css");
            res.code=200;
            res.write(contents.str());
        }
        else{

            res.code = 404;
            res.write("not found");
        }
        res.end();
    });
    CROW_ROUTE(app, "/get_image/<string>")
    ([](const request &req, response &res, string filename) {
        ifstream in("../public/images/"+filename,ifstream::in);
        if(in){
            ostringstream contents;
            contents <<in.rdbuf();
            in.close();

            res.set_header("Content-Type","image/png"); //png only, sorry
            //could parse the string for the file extension and do it based off that,
            //or make routes for other image types, but i'm not sure it's worth it
            res.code=200;
            res.write(contents.str());
        }
        else{
            res.code = 404;
            res.write("not found");
        }
        res.end();
    });
    CROW_ROUTE(app, "/get_script/<string>")
    ([](const request &req, response &res, string filename) {
        ifstream in("../public/scripts/"+filename,ifstream::in);
        if(in){
            ostringstream contents;
            contents <<in.rdbuf();
            in.close();

            res.set_header("Content-Type","text/javascript");
            res.code=200;
            res.write(contents.str());
        }
        else{
            res.code = 404;
            res.write("not found");
        }
        res.end();
    });


    //robot routes
    CROW_ROUTE(app,"/connect/<string>/<int>").methods(HTTPMethod::Post)
    ([&soc](const request &req, response &res, string IPadr, int Portno){
//idk why the capture list needs that its already a pointer
        if(soc){
            fprintf(LOG,"log: deleted existing soc\n");
            delete soc;//calls close() and allat
            soc=nullptr;
        }
        json::rvalue json_body = json::load(req.body);
        if (!json_body) {
            res.set_header("Content-Type","text/plain");
            res.code = 400;  // Bad Request
            res.write("Invalid JSON format");
            res.end();
            return;
        }
        ConnectionType typ = (ConnectionType) json_body["Ctype"].i();
       
        try{
            soc = new MySocket(CLIENT,IPadr,Portno,typ,APPROPRIATE_SIZE);
        } catch (int e){
            res.code=500;//internal error
            res.write("failed to create socket");
            res.end();
            return;
        }
        fprintf(LOG,"log: setup socket\n");

        if(typ==TCP){
            try{
                soc->ConnectTCP();
            } catch (int e){
                res.set_header("Content-Type","text/plain");
                res.code=500;
                res.write("tcp failed to connect");
                res.end();
                return;
            }
            fprintf(LOG,"log: TCP connected\n");
            //technically we don't even need this as the function covers for it, but its better to have it
        }

        res.set_header("Content-Type","text/plain");
        res.code=200;
        if(typ==TCP)
            res.write("opened and connected tcp socket");
        else if(typ==UDP)
            res.write("opened udp socket");
        res.end();
    });

    CROW_ROUTE(app,"/telecommand/").methods(HTTPMethod::PUT)
    ([&soc, &packetno](const request &req, response &res){
        if(!soc){
            res.set_header("Content-Type","text/plain");
            res.code=401;//forbidden, seems appropriate? since this occurs when no connection
            res.write("You must connect first");
        }
        else{
            json::rvalue json_body = json::load(req.body);
            if (!json_body) {
                res.code = 400;  // Bad Request
                res.write("Invalid JSON format");
                res.end();
                return;
            }
            int cmd = json_body["command"].i();

            PktDef commnd;
            ostringstream results;//string stream lets us format and not worry about buffers

            commnd.SetPktCount(packetno);
            packetno++;
            if(cmd == FORWARD || cmd ==BACKWARD){
                commnd.SetCmd(DRIVE);

                DriveBody drive;
                drive.Direction = cmd;
                drive.Duration = json_body["duration"].i();
                drive.Power = json_body["speed"].i();

                commnd.SetBodyData((char*)&drive,sizeof(drive));
            }
            else if(cmd == LEFT || cmd == RIGHT){
                commnd.SetCmd(DRIVE);

                TurnBody turn;
                turn.Direction = cmd;
                turn.Duration = json_body["duration"].i();

                commnd.SetBodyData((char*)&turn,sizeof(turn));
            }
            else if(cmd == 5){//that is, sleep; the website uses the same command but they were distinct according to our defines
                commnd.SetCmd(SLEEP);
            }
            else{
                res.code = 400;  // Bad Request
                res.write("Invalid JSON format");
                res.end();
                return;
            }
            char* packed = commnd.GenPacket();

            //the beauty of socket is we don't care how it's connected
            soc->SendData(packed,commnd.GetLength());
            fprintf(LOG,"log: attempted to send packet\n");

            //getting the response back:
            char rc[APPROPRIATE_SIZE];
            int rcsize=soc->GetData(rc);
            if(rcsize <0){
                res.code=500;
                res.write("failed to retrieve data");
                res.end();
                return;
            }
            PktDef telack(rc);
            fprintf(LOG,"log: recieved packet\n");

            //printing raw
            results <<"ACKNOWLEDGMENT:\nsize: "<<rcsize<<"\nraw value:\n";
            for(int i=0; i<rcsize;i++){
                int v = rc[i];
                results<<v<<" ";
            }
            //printing formated
            results<<"\nAck: "<<telack.GetAck()<<" |Command: "<<telack.GetCmd()<<" |pkt num: "<<telack.GetPktCount()<<" |body:\n"<<telack.GetBodyData();

            //since sleep is the kill command:
            if(soc->GetCType()==TCP && cmd ==5){
                soc->DisconnectTCP();
                fprintf(LOG,"log: attempted to disconnected tcp\n");
            }

            res.write(results.str());
        }
        res.end();
    });

    //uhh I think there was a typo on the instructions for this one lol. 'telementry'
    CROW_ROUTE(app,"/telemetry_request/").methods(HTTPMethod::Get)//:GET
    ([&soc, &packetno](const request &req, response &res){
        if(!soc){
            res.set_header("Content-Type","text/plain");
            res.code=401;//forbidden, seems appropriate? since this occurs when no connection
            res.write("You must connect first");
        }
        else{
            PktDef telem;
            ostringstream results;//string stream lets us format and not worry about buffers

            telem.SetCmd(RESPONSE);
            telem.SetPktCount(packetno);
            packetno++;
            char* packed = telem.GenPacket();

            //the beauty of socket is we don't care how it's connected
            soc->SendData(packed,telem.GetLength());
            fprintf(LOG,"log: attempted to send packet\n");

            //getting the response back:
            char rc[APPROPRIATE_SIZE];
            int rcsize=soc->GetData(rc);
            if(rcsize <0){
                res.code=500;
                res.write("failed to retrieve data");
                res.end();
                return;
            }
            PktDef telack(rc);
            fprintf(LOG,"log: recieved packet\n");

            //printing raw
            results <<"ACKNOWLEDGMENT:\nsize: "<<rcsize<<"\nraw value:\n";
            for(int i=0; i<rcsize;i++){
                int v = rc[i];
                results<<v<<" ";
            }
            //printing formated
            results<<"\nAck: "<<telack.GetAck()<<" |Command: "<<telack.GetCmd()<<" |pkt num: "<<telack.GetPktCount()<<" |body:\n"<<telack.GetBodyData();

            if(telack.GetAck()){
                char rc2[APPROPRIATE_SIZE];
                int rcsize2=soc->GetData(rc2);
                if(rcsize2 <0){
                    res.code=500;
                    results<<"\nfailed to retrieve telemetry";
                    res.write(results.str());
                    res.end();
                    return;
                }
                fprintf(LOG,"log: recieved additional telem packet\n");

                PktDef teldat(rc2);
                telemetry teletele;
                memcpy(&teletele,teldat.GetBodyData(),sizeof(teletele));

                //printing raw
                results <<"\nRESPONSE:\nsize: "<<rcsize2<<"\nraw value:\n";
                for(int i=0; i<rcsize2;i++){
                    int v = rc2[i];
                    results<<v<<" ";
                }
                //printing formated
                results<<"\nAck: "<<teldat.GetAck()<<" |Command: "<<teldat.GetCmd()<<" |pkt num: "<<teldat.GetPktCount()<<" |body:\n"<<teldat.GetBodyData();
                results<<"last packet: "<<teletele.LastPktCounter<<" grade: "<<teletele.CurrentGrade<<" hits "<<teletele.HitCount<<" heading "<<teletele.Heading;
                results<<" lascmd "<<teletele.LastCmd<<" lascmdval "<<teletele.LastCmdValue<<" lastcmdpow "<<teletele.LastCmdPower<<"\n";

                res.set_header("Content-Type","text/plain");
                res.code=200;
            }
            else{ //nack or some other failure
                res.set_header("Content-Type","text/plain");
                res.code=500;//change

                results<<"\nNO RESPONSE";
            }
            res.write(results.str());
        }   
        res.end();
    });

    CROW_ROUTE(app,"/routing_table/<string>/<int>/<string>/<int>").methods(HTTPMethod::Post)
    ([&soc](const request &req, response &res, string lisIPadr, int lisPortno, string senIPadr,int senPortno){
        PktDef emergency;
        emergency.SetCmd(SLEEP);
        emergency.SetPktCount(0);
        char* pemgcy = emergency.GenPacket();

        if(soc){
            fprintf(LOG,"log: deleted existing soc\n");
            delete soc;
            soc=nullptr;
        }

        json::rvalue json_body = json::load(req.body);
        if (!json_body) {
            res.code = 400;  // Bad Request
            res.write("Invalid JSON format");
            res.end();
            return;
        }
        ConnectionType typ = (ConnectionType) json_body["Ctype"].i();

        MySocket listensoc = MySocket(SERVER,lisIPadr,lisPortno,typ,APPROPRIATE_SIZE);
        fprintf(LOG,"log: started listening\n");

        try{
            soc = new MySocket(CLIENT,senIPadr,senPortno,typ,APPROPRIATE_SIZE);
        } catch (int e){
            res.code=500;
            res.write("failed to create forward socket");
            res.end();
            return; 
        }
        fprintf(LOG,"log: connected to forward\n");
        if(typ==TCP){
            fprintf(LOG,"log: establishing connection with listening socket\n");
            try{
                listensoc.ConnectTCP(); 

                fprintf(LOG,"log: establishing connection with forward socket\n");

                soc->ConnectTCP(); 
            }catch (int e){
                res.code=500;
                res.write("failed to establish connections");
                res.end();
            }
            fprintf(LOG,"log: connected both listening and forward\n");
        }
        bool on=true;
        while(on){


            char fromapp[APPROPRIATE_SIZE];
            int fasize=listensoc.GetData(fromapp);
            fprintf(LOG,"log: recieved packet from listener\n");

            soc->SendData(fromapp,fasize);
            fprintf(LOG,"log: attempted to forward packet\n");

            char fromdest[APPROPRIATE_SIZE];
            int fdsize=soc->GetData(fromdest);
            fprintf(LOG,"log: recieved packet from forward\n");

            listensoc.SendData(fromdest,fdsize);
            fprintf(LOG,"log: attempted to send packet back to listener\n");

            PktDef fdinfo(fromdest);
            switch(fdinfo.GetCmd()){
                case DRIVE:
                //drives send 1 recieve 1
                    fprintf(LOG,"log: DRIVE was recieved\n");
                    break;
                case SLEEP:
                //sleep is kill
                    fprintf(LOG,"log: SLEEP was recieved\n");
                    if(typ==TCP){
                        soc->DisconnectTCP();
                        listensoc.DisconnectTCP();
                        listensoc.KillTCPServ();
                        fprintf(LOG,"log: disconnected and killed tcp\n");
                    }
                    on=false;
                    break;
                case RESPONSE:
                    fprintf(LOG,"log: RESPONSE was recieved\n");
                    if(fdinfo.GetAck()){
                        char fromdest2[APPROPRIATE_SIZE];
                        int fd2size=soc->GetData(fromdest2);
                        fprintf(LOG,"log: recieved additional telem from forward\n");

                        listensoc.SendData(fromdest2,fd2size);
                        fprintf(LOG,"log: attempted to send packet back to listener\n");
                    }
                    break;
                default:
                    fprintf(LOG,"log: non-command recieved. something went wrong\n");
                    if(typ==TCP){
                        soc->DisconnectTCP();
                        listensoc.DisconnectTCP();
                        listensoc.KillTCPServ();
                        
                        fprintf(LOG,"log: disconnected and killed tcp\n");
                    }
                    on=false;
                    break;
            }
        }

        res.set_header("Content-Type","text/plain");
        res.code=200;
        res.write("listener disconnected. ceasing forwarding");
        res.end();
    });


    app.port(27500).multithreaded().run();
    return 0;
}
