#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

#include "inc/cpp-base64/base64.h"

#include <opencv2/opencv.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using namespace cv;

// COMPILE : g++ -std=c++17 -I /usr/include/boost -pthread camera_client.cpp -o camera_client

// Sends a WebSocket message and prints the response
int main(int argc, char** argv)
{
    try
    {
        // Check command line arguments.
        if(argc != 4)
        {
            std::cerr <<
                "Usage: camera_client <host> <port> <camera_id>\n" <<
                "Example:\n" <<
                "    camera_client 127.0.0.1 8080 2\n";
            return EXIT_FAILURE;
        }
        auto const host = argv[1];
        auto const port = argv[2];
        auto const camera_id = argv[3];

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ioc};
        websocket::stream<tcp::socket> ws{ioc};

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        net::connect(ws.next_layer(), results.begin(), results.end());

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));

        // Perform the websocket handshake
        ws.handshake(host, "/");

        VideoCapture cap;
        bool ok = cap.open(std::stoi(camera_id));
        if (!ok) {
            printf("No camera found\n");
            throw std::exception();
        }
        
        Mat frame;
        beast::flat_buffer buffer;
        while (true) {
            // std::cout << "Type a message: " << std::endl;
            // std::cin.getline(text, 10);
            // if (std::string(text) == "quit") {
            //     break;
            // }
            cap >> frame;
            std::vector<uchar> buf;
            imencode(".jpg", frame, buf);
            auto *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
            std::string img_encoded = base64_encode(enc_msg, buf.size());
            img_encoded += std::string(camera_id);

            // Send tuple, img and camera_id for server to know which camera to display

            // Send the message
            ws.write(net::buffer(img_encoded));

            // This buffer will hold the incoming message

            // Read a message into our buffer
            ws.read(buffer);
        }

        // Close the WebSocket connection
        ws.close(websocket::close_code::normal);

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer.data()) << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}