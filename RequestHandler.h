#pragma once

#include "utils/json.h"
#include <unordered_map>
#include "../Common/Utils/BasicWebServer/basicwebserver.h"
#include "database/database.h"

class RequestHandler : public BasicWebServerRequestHandler
{
public:
    RequestHandler();
    void HandleRequest(HTTPRequest& request, HTTPResponse& response, BasicWebServerClient* client) override;

private:
    void HandlePOST(HTTPRequest& request, HTTPResponse& response, BasicWebServerClient* client);
    void HandleGET(HTTPRequest& request, HTTPResponse& response, BasicWebServerClient* client);
    LF::db::Database* mDatabase{ nullptr };
    Authenticator mAuthenticator;
    std::string mWWWLocation;
};