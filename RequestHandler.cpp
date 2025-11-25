#include "RequestHandler.h"
#include "utils/systemutils.h"

/********** DEBUG SETUP **********/
#define ENABLE_SDEBUG
#define DEBUG_PREFIX "Handler"
#include "utils/screenlogger.h"
/*********************************/

#include "video/videodevice.h"

RequestHandler::RequestHandler()
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
        std::vector<LF::video::VideoDeviceInfo> devices = LF::video::VideoDevice::GetDevicesList();

        json::JSON devsJson = json::Array();
        for (auto& d : devices)
        {
            json::JSON obj = json::Object();
            obj["id"] = d.mId;
            obj["name"] = d.mName;
            obj["internal"] = d.mInternalName;
            obj["formats"] = json::Array();

            int i = 0;
            for (auto& f : d.mFormats)
            {
                json::JSON fmt = json::Object();
                fmt["id"] = i;
                fmt["colorspace"] = str(f.colorspace);
                fmt["info"] = f.additionalInfo;
                fmt["width"] = f.resolution.width;
                fmt["height"] = f.resolution.height;
                ++i;

                obj["formats"].append(fmt);
            }
            devsJson.append(obj);
        }
        
        response.mResponse = HTTPResponse::Response_t::OK;
        response.mContentType = LF::www::ContentType_t::ApplicationJson;
        response.mContent = devsJson.dump();
    }
}
