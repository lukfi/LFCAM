#include <iostream>
#include "utils/about.h"
#include "video/videodevice.h"
#include "threads/iothread.h"
#include "utils/stringutils.h"

#include "fs/file.h"
#include "VideoController.h"
#include "RequestHandler.h"

/********** DEBUG SETUP **********/
#define ENABLE_SDEBUG
#define DEBUG_PREFIX "LFCAM"
#include "utils/screenlogger.h"
/*********************************/

#ifdef WCV_INPLACE
#include <QApplication>
#include <QDebug>
#include "imagewindow.h"
#endif

bool SaveRawImageToFile(LF::graphic::RawImage& ri, std::string path)
{
    LF::fs::File f(path);
    f.CreateOpen();
    return f.Put(ri.GetRawData(), ri.GetDataSize()) == ri.GetDataSize();
}

std::ostream& operator<<(std::ostream& os,LF::video::VideoDeviceInfo& dev)
{
    os << dev.mId << ": " << dev.mName;
    int i = 0;
    for (auto& f : dev.mFormats)
    {

        os << std::endl;
        os << "\t" << i++ << " * " << f;
    }
    return os;
}
//////////////////////////// TEST
#include <iostream>

class Consumer : public Tracked
{
public:
    void OnValue(int v)
    {
        std::cout << "Consumer::OnValue(" << v << ")\n";
    }

    ~Consumer()
    {
        std::cout << "~Consumer()\n";
    }
};

class Producer
{
public:
    Signal<void(int)> VALUE;

    void Generate()
    {
        VALUE.Emit(123);
    }

    ~Producer()
    {
        std::cout << "~Producer()\n";
    }
};
////////////

int main(int argc, char* argv[])
{

    ///////////////////////// TEST
#if 0
    Producer* p = new Producer();
    Consumer* c = new Consumer();

    // Connect the signal
    CONNECT(p->VALUE, Consumer, OnValue, c);

    std::cout << "Emit #1 (slots:" << p->VALUE.ConnectionsCount() << "):\n";
    p->Generate();     // EXPECT: prints "Consumer::OnValue(123)"

    std::cout << "Deleting consumer...\n";
    delete c;          // SHOULD automatically disconnect

    std::cout << "Emit #2 (slots:" << p->VALUE.ConnectionsCount() << "):\n";
    p->Generate();     // EXPECT: should NOT call OnValue

    delete p;
#endif
    //return 0;
    ///////////////////////////////












#ifdef WCV_INPLACE
    QApplication a(argc, argv);
    ImageWindow gWin;
    VideoController gController(gWin);
#else
    VideoController gController;
#endif
    RequestHandler gHandler(gController);
    BasicWebServer gServer(8080, BasicWebServer::NO_Server, &gHandler);

    //CONNECT(gWCV.CONNECTED, OnWCVConnected);
    //gWCV.Start(true, true);
    LF::threads::IOThread t;
    t.Start();

    std::cout << "Hello LFCAM: " << About::Version() << std::endl;


#ifdef WCV_INPLACE
    gWin.show();
    a.exec();
#endif

    t.Join();
}
