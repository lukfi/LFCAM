#include "RequestHandler.h"
#include "utils/systemutils.h"
#include "utils/stringutils.h"
#include "VideoController.h"

#include "utils/json.h"

/********** DEBUG SETUP **********/
#define ENABLE_SDEBUG
#define DEBUG_PREFIX "Handler"
#include "utils/screenlogger.h"
/*********************************/

RequestHandler::RequestHandler(VideoController& controller) :
    mController(controller)
{
    mAuthenticator.SetPassword("gftkr");
    mWWWLocation = LF::utils::GetExecutableDir() + "/www";
    SDEB("www path: %s", mWWWLocation.c_str());
}

void RequestHandler::HandleRequest(HTTPRequest& request, HTTPResponse& response, BasicWebServerClient* client)

{
    SDEB("%s URI: %s", (request.mRequest == HTTPRequest::ePOST ? "POST" : "GET "), request.mUri.c_str());

    if (request.mRequest == HTTPRequest::ePOST)
    {
        HandlePOST(request, response, client);
    }
    else
    {
        HandleGET(request, response, client);
    }
}

void RequestHandler::HandlePOST(HTTPRequest& request, HTTPResponse& response, BasicWebServerClient* client)
{
    auto decodedRequest = DecodedHTTPRequest(request);

    if (decodedRequest.GetPath() == "/api/control")
    {
        json::JSON controlJson = json::JSON::Load(request.mContent);
        if (controlJson["action"].ToString() == "stop")
        {
            SDEB("Stop");
            mController.StopDevice();
        }
        else if (controlJson["action"].ToString() == "start")
        {

            SDEB("Start");
            int id = -1;
            int formatId = -1;

            bool ok = false;
            id = controlJson["id"].ToInt(ok);
            if (ok)
            {
                formatId = controlJson["formatId"].ToInt(ok);
            }
            if (ok)
            {
                mController.StartDevice(id, formatId);
            }
        }

        response.mResponse = HTTPResponse::Response_t::OK;
    }
}

void RequestHandler::HandleGET(HTTPRequest& request, HTTPResponse& response, BasicWebServerClient* client)
{
    auto decodedRequest = DecodedHTTPRequest(request);

    if (decodedRequest.GetPath() == "/")
    {
        HandleRequestFS(request, response, false, nullptr, mWWWLocation);
    }
    else if (decodedRequest.GetPath() == "/api/dev")
    {
        response.mResponse = HTTPResponse::Response_t::OK;
        response.mContentType = LF::www::ContentType_t::ApplicationJson;
        response.mContent = mController.GetAllDevicesInfo();
    }
    else if (decodedRequest.GetPath() == "/api/status")
    {
        response.mResponse = HTTPResponse::Response_t::OK;
        response.mContentType = LF::www::ContentType_t::ApplicationJson;
        response.mContent = "{\"runningId\":null, \"formatId\":null}";
        int id, formatId = 0;
        if (mController.CurrentlyRunning(id, formatId))
        {
            response.mContent = LF::utils::sformat("{\"runningId\":%d, \"formatId\":%d}", id, formatId);
        }
    }
}
