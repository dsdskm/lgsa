#include "pch.h"
#include "CRest.h"
#include <iostream>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <nlohmann/json.hpp>
#include <fstream>

using namespace std;
using json = nlohmann::json;

// Constructor
CRest::CRest() {}

// Member function

json CRest::send(string method, string path, json body) {
    json res_json;
    try {
        // Create a client session
        Poco::URI uri("http://" + url + "/" + path);
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());

        string requestBody = body.dump();

        // Prepare the request
        if (method == "GET") {
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.getPathAndQuery(), Poco::Net::HTTPRequest::HTTP_1_1);
            request.setHost(uri.getHost());
            ostream& requestStream = session.sendRequest(request);
        }
        else if (method == "POST") {
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery(), Poco::Net::HTTPRequest::HTTP_1_1);
            request.setHost(uri.getHost());
            request.setContentType("application/json");
            request.setContentLength(requestBody.length());
            ostream& requestStream = session.sendRequest(request);
            requestStream << requestBody;
        }
        else if (method == "PUT") {
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_PUT, uri.getPathAndQuery(), Poco::Net::HTTPRequest::HTTP_1_1);
            request.setHost(uri.getHost());
            request.setContentType("application/json");
            request.setContentLength(requestBody.length());
            ostream& requestStream = session.sendRequest(request);
            requestStream << requestBody;
        }
        else if (method == "DELETE") {
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_DELETE, uri.getPathAndQuery(), Poco::Net::HTTPRequest::HTTP_1_1);
            request.setHost(uri.getHost());
            request.setContentType("application/json");
            request.setContentLength(requestBody.length());
            ostream& requestStream = session.sendRequest(request);
            requestStream << requestBody;
        }

        // Get the response
        Poco::Net::HTTPResponse response;
        std::istream& responseStream = session.receiveResponse(response);

        // Read the response body
        std::string responseBody;
        Poco::StreamCopier::copyToString(responseStream, responseBody);

        // Print the response status and body
        //std::cout << "Response Status: " << response.getStatus() << std::endl;
        //std::cout << "Response Body:\n" << responseBody << std::endl;

        res_json = json::parse(responseBody);

        // Clean up
        session.reset();
    }
    catch (Poco::Exception& ex) {
        std::cerr << "Poco Exception: " << ex.displayText() << std::endl;
        //return 1;
    }
    catch (std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        //return 1;
    }

    return res_json;
}

string CRest::getUrlFromText() {
    ifstream file("MARS_SERVER_URL.txt");
    string res = url;

    if (file.is_open()) {
        getline(file, res);
        file.close();
    }
    return res;
}

void CRest::setUrl(string _url) {
    url = _url;
}