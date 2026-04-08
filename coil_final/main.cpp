//COIL milestone3/final webserver
#include "crow_all.h"
#include "packet.h"
#include "socket.h"
#include <iostream>


//its quite annoying that it treats crow as having errors
//and intellisence doesn't work. I can't even see function names

using namespace crow;
using namespace std;

#define APPROPRIATE_SIZE 150 //this should be big enough for the buffer, I think?
//kind of arbitrary but I forget what the biggest size the sim sent was

int main(){
    crow::SimpleApp app;

    MySocket* soc;//keeps in scope, otherwise each crow_route function is creating and destroying them
    //but we don't want to initialize this yet, so here we are
    int pktnum;

    //just going to copy basic routes from a previous assignment because what else are they gonna do?
    CROW_ROUTE(app, "/")
    ([](const request &req, response &res) {
    //I couldn't really think of a better way of opening the file, so I mostly copied the example code
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
    CROW_ROUTE(app,"/connect/<string>/<int>")//.methods(HTTPMethod::Post)
    ([&soc](const request &req, response &res, string IPadr, int Portno){
    //    if(*soc){
            //if tcp, send sleep, kill tcp, kill tcp serv
    //        delete soc;//
    //    }
        soc = new MySocket(CLIENT,IPadr,Portno,UDP,APPROPRIATE_SIZE);
                //instructions specifically state that this route should be for UDP ONLY

        //reroute stderr somehow

        res.set_header("Content-Type","text/plain");
        res.code=200;
        res.write("opened udp socket");
        res.end();
    });

    //my solution to the requirements stating the previous was for tcp only
    CROW_ROUTE(app,"/connectTCP/<string>/<int>").methods(HTTPMethod::Post)
    ([&soc](const request &req, response &res, string IPadr, int Portno){
       // if(*soc){
            //if tcp, send sleep, kill tcp, kill tcp serv
         //   delete soc;//
        //}
        soc = new MySocket(CLIENT,IPadr,Portno,TCP,APPROPRIATE_SIZE);
        soc->ConnectTCP();

        //reroute stderr somehow

        res.set_header("Content-Type","text/plain");
        res.code=200;
        res.write("opened udp socket");
        res.end();
    });

    CROW_ROUTE(app,"/telecommand/").methods(HTTPMethod::Put)
    ([](const request &req, response &res){

    });

    //uhh I think there was a typo on the instructions for this one lol. 'telementry'
    CROW_ROUTE(app,"/telemetry_request/").methods(HTTPMethod::Get)//:GET
    ([&soc](const request &req, response &res){
     //   if(!*soc){
     //       res.set_header("Content-Type","text/plain");
       //     res.code=401;//forbidden, seems appropriate? since this occurs when no connection
         //   res.write("You must connect first");
      //  }

        PktDef telem;

        telem.SetCmd(RESPONSE);
        telem.SetPktCount(4);

        char* packed = telem.GenPacket();

        //the beauty of socket is we don't care who or what it is
        soc->SendData(packed,telem.GetLength());

        //telem sends 2 packets back
        char rc[APPROPRIATE_SIZE];
        int rcsize=soc->GetData(rc);

        PktDef telack(rc);
        printf("\nwe got: ack: %d, cmd: %d, count: %d, %s\n", telack.GetAck(), telack.GetCmd(), telack.GetPktCount(), telack.GetBodyData());

        if(telack.GetAck()){
            char rc2[APPROPRIATE_SIZE];
            int rcsize2=soc->GetData(rc2);

            PktDef teldat(rc2);
            printf("\nwe got: ack: %d, cmd: %d, count: %d, %s\n", teldat.GetAck(), teldat.GetCmd(), teldat.GetPktCount(), teldat.GetBodyData());

            res.set_header("Content-Type","text/plain");
            res.code=200;
            res.write(teldat.GetBodyData());
        }
        else{
            res.set_header("Content-Type","text/plain");
            res.code=400;//change
            res.write(telack.GetBodyData());
        }
        res.end();
    });

    CROW_ROUTE(app,"/routing_table")
    ([](const request &req, response &res){

    });


    app.port(27500).multithreaded().run();
    return 0;
}
